# Schema

​		Skilo的Schema主要目的是为了让用户定义插入Collection的文档应该遵循什么样的模式，有哪些字段，哪些字段需要被索引而能被搜索到。用户创建新Colletion时，必须在json中指定Schema字段，Skilo会根据Schema信息构造一个Field树。

​		如我们工程实践最终要搭建一个食谱搜索网站，则Schema中至少需要食谱名、tips、所需食材数组，每种食材包括名称和用量字段，处理步骤也是一个数组，里面包括content和图片url信息：

```json
"schema":{
        "type":"object",
        "$fields": {
            "recipe_name":{
                "type":"string",
                "index":true
            },
            "tips":{
                "type":"string"
            },
            "ingredients": {
                "type": "array",
                "$items": {
                    "type": "object",
                    "$fields":{
                        "note": {"type": "string"},
                        "title": {"type": "string", "index":true}
                    }
                }
            },
            "steps": {
                "type": "array",
                "$items": {
                    "type": "object",
                    "$fields":{
                        "content": {"type": "string"，"index":true},
                        "image": {"type": "string"}
                    }
                }
            }
        }
}
```

​	Skilo会根据该Schema构造出如下一棵Field树：

![](https://github.com/demonatic/Image-Hosting/blob/master/Skilo/Skilo%20Schema.png)

​	每种子类型Field都继承了Field父类，均拥有名称、类型、具有层级关系的路径，属性表和子field表：

```c++
struct Field{
    using ArrtibuteValue=std::variant<std::string,bool,int,float>;
    std::string name;
    std::string path; //path in a tree, aaa.bbb.cc
    enum class FieldType{
        INTEGER,
        FLOAT,
        STRING,
        BOOLEAN,
        OBJECT,
        ARRAY,
	}type;
    std::unordered_map<std::string,ArrtibuteValue> attributes;
    std::unordered_map<std::string,std::unique_ptr<Field>> sub_fields;
};
```

​		任何客户端发来的json请求都先会被rapidjson库解析，Schema类接收创建新Collection请求的json树，依据json树在构造Field构造函数中递归构建出相应的field树：

```c++
Field::Field(const std::string &name,const std::string &path,const rapidjson::Value &schema)
{
    this->name=name;
    this->type=get_field_type(schema);
    this->path=path.empty()?name:path+"."+name;
    parse_arrtibutes(schema); //解析并存储该字段的属性信息到map中

    if(type==FieldType::ARRAY){
        if(!schema.HasMember(item_keyword)){
            throw InvalidFormatException("array type must have \""+std::string(item_keyword)+"\" keyword");
        }
        std::unique_ptr<Field> sub_field=create_field(item_keyword,this->path,schema[item_keyword]); //递归为Array类型字段构建子树
        this->sub_fields[sub_field->name]=std::move(sub_field);
    }
    if(type==FieldType::OBJECT){
        if(!schema.HasMember(field_keyword)){
            throw InvalidFormatException("object type must have \""+std::string(field_keyword)+"\" keyword");
        }
        //递归为Object类型字段构建子树
        this->parse_sub_fields(schema[field_keyword],this->path);
    }
}
```

```c++
void Field::parse_sub_fields(const rapidjson::Value &sub_schema,const std::string &path)
{
    if(!sub_schema.IsObject()) return;
    for(rapidjson::Value::ConstMemberIterator it=sub_schema.MemberBegin();it!=sub_schema.MemberEnd();++it){
        std::unique_ptr<Field> sub_field=create_field(it->name.GetString(),path,it->value);
        if(sub_field){
            sub_fields[sub_field->name]=std::move(sub_field);
        }
    }
}
```

​		Field树构建完后使用[访问者模式](https://www.tutorialspoint.com/design_pattern/visitor_pattern.htm "Title") 来遍历这棵树，使用访问者模式可以将对这棵树的操作集中在具体的访问者中，而保持了Field类的简洁，也易于增添新的操作。迭代器也能通过调用结点对象的特定操作来遍历整个树结构，但迭代器不能对具有不同元素类型的对象结构进行操作，而访问者模式没有这种限制，它可以访问不具有相同父类的对象。

​		FieldVisitor类定义了两种类型的visit接口，一种可以直接访问Field树，另一种传入json树与schema中的field树同步进行遍历。每一种Field子类型都实现了两种accept类型的操作，以此实现double-dispatch: 即根据Field结点类型和访问者类型来决定执行何种操作。

```c++
struct FieldVisitor{
    virtual ~FieldVisitor()=default;

    virtual void visit_field_array(const FieldArray *field_array){}
    virtual void visit_field_object(const FieldObject *field_object){}
    virtual void visit_field_string(const FieldString *field_string){}
    ....
    
    virtual void visit_field_array(const FieldArray *field_array,const rapidjson::Value &document){}
    virtual void visit_field_object(const FieldObject *field_object,const rapidjson::Value &document){}
    virtual void visit_field_string(const FieldString *field_string,const rapidjson::Value &document){}
    ...
};

void FieldObject::accept(FieldVisitor &field_visitor, const rapidjson::Value &document)
{
    field_visitor.visit_field_object(this,document);
    //递归accept FieldObject的子field
    for(const auto &[field_name,field]:sub_fields){
        rapidjson::Value::ConstMemberIterator it=document.FindMember(field_name.c_str());
        if(it==document.MemberEnd()){
            throw InvalidFormatException("...");
        }
        field->accept(field_visitor,it->value);
    }
}
...
void FieldArray::accept(FieldVisitor &field_visitor)
{
    field_visitor.visit_field_array(this);
    //递归accept FieldArray中的item
    this->sub_fields[item_keyword]->accept(field_visitor);
}
...
```

​		需要直接访问Field树的类有ColletionIndexes，它将根据schema信息来根据需要为一些字段创建索引。通过继承FieldVisior，实现visit_field_string接口，即可为schema中field类型为string并且index属性为true的字段创建倒排索引，存入内部的index map中。这样，便可以根据需要为不同类型字段创建不同索引，如后续可为数值型字段构建kd-tree索引。

```c++
class CollectionIndexes:public Schema::FieldVisitor{
public:
    CollectionIndexes(const Schema::CollectionSchema &schema){
        schema.accept(*this); //create indexes according to fields in the schema
    }
protected:
    virtual void visit_field_string(const Schema::FieldString *field_string) override{
        auto it=field_string->attributes.find("index");
        if(it==field_string->attributes.end())
            return;
        
        const Schema::Field::ArrtibuteValue &index_option_value=it->second;
        if(std::get<bool>(index_option_value)){ //如果index属性为true则为其创建倒排索引
            _indexes.emplace(field_string->path,InvertIndex());
        }
    }
private:
	std::unordered_map<std::string,InvertIndex> _indexes; //<field_path,index>
}
```

需要让json树和field树同步遍历的有SchemaValidator和IndexWriter。

用户需要创建新文档时需要通过SchemaValidator来验证传入的文档是否和Schema描述的结构相一致，因此对于传入的document in json中的每一个结点都要判断与其对应位置的field结点类型是否一致。

上述document校验完schema后需要写入索引，对document不同类型的字段可能会有不同的写入方式，如string类型字段将先通过预先配置的分词策略来进行分词，再根据field的path路径找到该字段的倒排索引进行写入。

```c++
class IndexWriter:public Schema::FieldVisitor{
public:
    //IndexWriter用于将文档中的信息写入索引中，分词策略由schema指定
    IndexWriter(CollectionIndexes &indexes,TokenizeStrategy *tokenizer);
    
    void index_in_memory(const Schema::CollectionSchema &schema,const Document &document){
        ...
        schema.accept(*this,document.get_raw()); //索引该文档
    }

	//将文档的string字段用分词器分词后，插入倒排索引
    virtual void visit_field_string(const Schema::FieldString *field_string,const rapidjson::Value &document) override{
        InvertIndex *index=_indexes.get_index(field_string->path);//根据field_path找到索引
        if(!index) return;
        //分词
        std::unordered_map<std::string, std::vector<uint32_t>> word_offsets;
        word_offsets=_tokenizer->tokenize(document.GetString());
        //将处理后的内容插入该字段对应的倒排索引中
        IndexRecord record{_seq_id,std::move(word_offsets)};
        index->add_record(record);
    }
}
```

* ### 未来展望

  目前Schema只实现了最基本的功能，未来将会加入更多内容，如在attrubute中指定facet属性，指定排序方式等。