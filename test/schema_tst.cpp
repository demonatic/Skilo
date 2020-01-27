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
                                   \"type\":\"string\"\
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
                             \"product id\":\"1001\",\
                             \"price\":\"6.5\",\
                             \"composition\":[\"Engine\",\"Wheel\",\"Body\"],\
                             \"dimensions\":{\
                                 \"length\":\"12.0\",\
                                 \"width\":\"4.5\",\
                                 \"height\":\"3.0\"\
                             }\
                         }";\
    Document schema_document(0,0,schema_str);

    CollectionSchema schema(schema_document);
    Document document(0,0,json_str);


}
