#include <gtest/gtest.h>
#include "../../src/core/schema/Schema.h"

using namespace testing;
using namespace std;
using namespace Skilo;
using namespace Skilo::Schema;

TEST(SCHEMA_TEST,PARSE_TEST) {
    std::string schema_str="{\"name\":\"products\",\
                             \"schema\":{\
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
                                   },\
                                   \"supplier\":{\
                                        \"type\":\"array\",\
                                        \"$items\":{\
                                            \"type\":\"object\",\
                                            \"$fields\":{\
                                                 \"name\": {\"type\": \"string\"},\
                                                 \"e-mail\": {\"type\": \"string\"}\
                                            }\
                                        }\
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
                             },\
                             \"supplier\":\
                              [\
                                {\"name\":\"apple.Inc\",\"e-mail\":\"apple@123.com\"},\
                                {\"name\":\"google.Inc\",\"e-mail\":\"google@123.com\"}\
                              ]\
                         }";\
    CollectionMeta collection_meta(schema_str);

    CollectionSchema schema(collection_meta);
    Field &root_field=*schema.get_root_field();
    EXPECT_EQ(root_field.name,"");
    EXPECT_EQ(root_field.type,FieldType::OBJECT);
    EXPECT_EQ(root_field["product name"].type,FieldType::STRING);
    EXPECT_EQ(root_field["product name"].name,"product name");
    EXPECT_EQ(root_field["product name"].path,"product name");

    EXPECT_EQ(root_field["product id"].type,FieldType::INTEGER);
    EXPECT_EQ(root_field["product id"].name,"product id");
    EXPECT_EQ(root_field["product id"].path,"product id");

    EXPECT_EQ(root_field["price"].type,FieldType::FLOAT);
    EXPECT_EQ(root_field["price"].name,"price");
    EXPECT_EQ(root_field["price"].path,"price");

    EXPECT_EQ(root_field["composition"].type,FieldType::ARRAY);
    EXPECT_EQ(root_field["composition"].name,"composition");
    EXPECT_EQ(root_field["composition"].path,"composition");
    EXPECT_EQ(root_field["composition"]["$items"].type,FieldType::STRING);
    EXPECT_EQ(root_field["composition"]["$items"].name,"$items");
    EXPECT_EQ(root_field["composition"]["$items"].path,"composition.$items");

    EXPECT_EQ(root_field["dimensions"].type,FieldType::OBJECT);
    EXPECT_EQ(root_field["dimensions"].name,"dimensions");
    EXPECT_EQ(root_field["dimensions"].path,"dimensions");
    EXPECT_EQ(root_field["dimensions"]["length"].type,FieldType::FLOAT);
    EXPECT_EQ(root_field["dimensions"]["length"].name,"length");
    EXPECT_EQ(root_field["dimensions"]["length"].path,"dimensions.length");
    EXPECT_EQ(root_field["dimensions"]["width"].type,FieldType::FLOAT);
    EXPECT_EQ(root_field["dimensions"]["width"].name,"width");
    EXPECT_EQ(root_field["dimensions"]["width"].path,"dimensions.width");
    EXPECT_EQ(root_field["dimensions"]["height"].type,FieldType::FLOAT);
    EXPECT_EQ(root_field["dimensions"]["height"].name,"height");
    EXPECT_EQ(root_field["dimensions"]["height"].path,"dimensions.height");

    EXPECT_EQ(root_field["supplier"].type,FieldType::ARRAY);
    EXPECT_EQ(root_field["supplier"]["$items"].type,FieldType::OBJECT);
    EXPECT_EQ(root_field["supplier"]["$items"].path,"supplier.$items");
    EXPECT_EQ(root_field["supplier"]["$items"]["name"].type,FieldType::STRING);
    EXPECT_EQ(root_field["supplier"]["$items"]["name"].path,"supplier.$items.name");
    EXPECT_EQ(root_field["supplier"]["$items"]["e-mail"].type,FieldType::STRING);
    EXPECT_EQ(root_field["supplier"]["$items"]["e-mail"].path,"supplier.$items.e-mail");

    Field::ArrtibuteValue attr=root_field["product name"].arrtibute("index");
    EXPECT_EQ(std::get<bool>(attr),true);

    Document document(json_str);
    ASSERT_STREQ(document.get_value("product name").GetString(),"Car Model");
    EXPECT_EQ(document.get_value("dimensions.length").GetFloat(),12.0);
    ASSERT_STREQ(document.get_value("supplier.1.name").GetString(),"google.Inc");

    schema.validate(document);
}
