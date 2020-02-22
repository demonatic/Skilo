# Schema

​	Skilo的Schema主要目的是为了让用户定义插入Collection的文档应该遵循什么样的模式，有哪些字段，哪些字段需要被索引而能被搜索到。用户创建新Colletion时，必须在json中指定Schema字段，Skilo会根据Schema信息构造一个Field树。

​	如我们工程实践最终要搭建一个食谱搜索网站，则Schema中至少需要食谱名、tips、所需食材数组，每种食材包括名称和用量字段，处理步骤也是一个数组，里面包括content和图片url信息：

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

​	![]([https://github.com/demonatic/Image-Hosting/blob/master/Skilo/Skilo%20Schema.png](https://github.com/demonatic/Image-Hosting/blob/master/Skilo/Skilo Schema.png))

​	每个Field主要字段有field名称,和类型、具有层级关系的路径，属性表和子field表：

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

​	任何客户端发来的json请求都先会被rapidjson库解析，Schema类接收创建新Collection请求的json树，依据json树在构造Field构造函数中递归构建出相应的field树：

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

