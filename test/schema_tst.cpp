#include <gtest/gtest.h>
#include "../../src/core/schema/Schema.h"

using namespace testing;
using namespace std;
using namespace Skilo;
using namespace Skilo::Schema;

TEST(SCHEMA_TEST,PARSE_TEST) {
    std::string schema_str="{\
                            \"type\":\"object\",\
                            \"$fields\": {\
                               \"product name\":{\
                                   \"type\":\"string\",\
                                   \"index\":true\
                               },\
                               \"product id\":{\
                                   \"type\":\"integer\"\
                               },\
                               \"price\":{\
                                   \"type\":\"float\"\
                               },\
                               \"composition\": {\
                                   \"type\": \"array\",\
                                   \"$items\": {\
                                       \"type\": \"string\"\
                                   }\
                               },\
                               \"dimensions\": {\
                                   \"type\":\"object\",\
                                   \"$fields\": {\
                                       \"length\": {\"type\": \"float\"},\
                                       \"width\": {\"type\": \"float\"},\
                                       \"height\": {\"type\": \"float\"}\
                                   }\
                               }\
                           }\
                       }";
    std::string json_str="{\
                             \"product name\":\"Car Model\",\
                             \"product id\":1001,\
                             \"price\":6.5,\
                             \"composition\":[\"Engine\",\"Wheel\",\"Body\"],\
                             \"dimensions\":{\
                                 \"length\":12.0,\
                                 \"width\":4.5,\
                                 \"height\":3.6\
                             }\
                         }";\
    Document schema_document(0,0,schema_str);

    CollectionSchema schema(schema_document);
    Field &root_field=*schema.get_root_field();
    EXPECT_EQ(root_field.name,"$schema");
    EXPECT_EQ(root_field.type,FieldType::OBJECT);
    EXPECT_EQ(root_field["product name"].type,FieldType::STRING);
    EXPECT_EQ(root_field["product name"].name,"product name");
    EXPECT_EQ(root_field["product id"].type,FieldType::INTEGER);
    EXPECT_EQ(root_field["product id"].name,"product id");
    EXPECT_EQ(root_field["price"].type,FieldType::FLOAT);
    EXPECT_EQ(root_field["price"].name,"price");
    EXPECT_EQ(root_field["composition"].type,FieldType::ARRAY);
    EXPECT_EQ(root_field["composition"].name,"composition");
    EXPECT_EQ(root_field["composition"]["$items"].type,FieldType::STRING);
    EXPECT_EQ(root_field["composition"]["$items"].name,"$items");
    EXPECT_EQ(root_field["dimensions"].type,FieldType::OBJECT);
    EXPECT_EQ(root_field["dimensions"].name,"dimensions");
    EXPECT_EQ(root_field["dimensions"]["length"].type,FieldType::FLOAT);
    EXPECT_EQ(root_field["dimensions"]["length"].name,"length");
    EXPECT_EQ(root_field["dimensions"]["width"].type,FieldType::FLOAT);
    EXPECT_EQ(root_field["dimensions"]["width"].name,"width");
    EXPECT_EQ(root_field["dimensions"]["height"].type,FieldType::FLOAT);
    EXPECT_EQ(root_field["dimensions"]["height"].name,"height");

    Field::ArrtibuteValue attr=root_field["product name"].arrtibute("index");
    EXPECT_EQ(std::get<bool>(attr),true);

    Document document(0,0,json_str);
    std::optional<std::string> err_str=schema.validate(document);
    if(err_str){
        cout<<err_str.value()<<endl;
        EXPECT_EQ(err_str,std::nullopt);
    }

}
