#include <gtest/gtest.h>
#include "../../src/core/schema/Schema.h"

using namespace testing;
using namespace std;
using namespace Skilo;
using namespace Skilo::Schema;

TEST(SCHEMA_TEST,SORTED_CRUD_TEST) {
    std::string schema_str="{\
                           \"$schema\": {\
                               \"product name\":{\
                                   \"type\":\"string\"\
                               },\
                               \"product id\":{\
                                   \"type\":\"integer\"\
                               }\
                               \"price\":{\
                                   \"type\":\"float\"\
                               },\
                               \"composition\": {\
                                   \"type\": \"array\",\
                                   \"items\": {\
                                       \"type\": \"string\"\
                                   }\
                               },\
                               \"dimensions\": {\
                                   \"$schema\": {\
                                       \"length\": {\"type\": \"float\"},\
                                       \"width\": {\"type\": \"float\"},\
                                       \"height\": {\"type\": \"float\"}\
                                   }\
                               }\
                           }\
                       }";
    Document schema_document(0,0,schema_str);
    CollectionSchema schema(schema_document);


}
