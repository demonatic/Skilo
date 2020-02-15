#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "utility/ConfigReader.h"

using namespace testing;
using namespace std;
using namespace Skilo;

class ReadStructuredDataTester:public ::testing::Test{
public:
    void read_config_test(const std::string &file_path){
        IStructuredDataReader* config=ConfigurationLoader::get_config(file_path.c_str());

        auto map=config->get_flat_kv_map();
//        print_parse_result(map);
        ///Test to_kvmap function correctness
        for(auto pair:map){
            auto ptree=config->get_tree(pair.first);
            auto vec=pair.second;
            ASSERT_TRUE(std::find(vec.begin(),vec.end(),ptree.data())!=vec.end());
        }
    }

    void get_put_test(const std::string &file_path){
        IStructuredDataReader* config=ConfigurationLoader::get_config(file_path.c_str());

        std::string put_string="success";
        config->put<std::string>("glossary.GlossDiv.GlossList.GlossEntry.GlossDef.para",put_string,true);
        ASSERT_EQ(config->get<std::string>("glossary.GlossDiv.GlossList.GlossEntry.GlossDef.para"),put_string);

        double put_double=3.1415926;
        config->put<double>("glossary.GlossDiv.GlossList.GlossEntry.GlossDef.para",put_double,true);
        ASSERT_FLOAT_EQ(config->get<double>("glossary.GlossDiv.GlossList.GlossEntry.GlossDef.para"),put_double);

        int put_int=666;
        config->put<int>("glossary.GlossDiv.GlossList.GlossEntry.GlossTerm",put_int,true);
        ASSERT_EQ(config->get<int>("glossary.GlossDiv.GlossList.GlossEntry.GlossTerm"),put_int);

        std::string put_string_2="Hello World";
        config->put<std::string>("glossary.GlossDiv.GlossList.GlossEntry.GlossSee",put_string_2,true);
        ASSERT_EQ(config->get<std::string>("glossary.GlossDiv.GlossList.GlossEntry.GlossSee"),put_string_2);

        int put_int_2=5;
        config->put<int>("glossary.GlossDiv.title",put_int_2,true);
        ASSERT_EQ(config->get<int>("glossary.GlossDiv.title"),put_int_2);

        double put_double_2=935332545.12;
        config->put<double>("glossary.GlossDiv.GlossList.GlossEntry.ID",put_double_2,true);
        ASSERT_FLOAT_EQ(config->get<double>("glossary.GlossDiv.GlossList.GlossEntry.ID"),put_double_2);
    }

    void print_parse_result(NodeMap &map){
        ///print all parse result
        for(auto pair:map){
            for(auto elm:pair.second){
                cout<<pair.first.data()<<"--------"<<elm<<endl;
            }
        }
    }

    void read_write_json_test(const std::string file_path){
        auto read_writer=std::make_unique<JsonReader>();
        std::string str_json="{\
                             \"boolean\": true,\
                             \"null\": null,\
                             \"number\": 123,\
                             \"object\": {\
                               \"a\": \"b\",\
                               \"c\": \"d\",\
                               \"e\": \"f\"\
                             },\
                             \"string\": \"Hello World\"\
                           }";

        std::stringstream ss(str_json);
        read_writer->read_from_stream(ss);
        read_writer->write_to_file(file_path);

        auto read_writer_2=std::make_unique<JsonReader>();
        read_writer_2->read_from_file(file_path);
        ASSERT_TRUE(read_writer_2->get_tree("")==read_writer->get_tree(""));
    }

};

TEST_F(ReadStructuredDataTester, TestGetPut)
{
    get_put_test("dataset/test_json_1.txt");
}

TEST_F(ReadStructuredDataTester,TestNodeMap){
    read_config_test("dataset/test_json_1.txt");
    read_config_test("dataset/test_json_2.txt");
    read_config_test("dataset/test_json_3.txt");
    read_config_test("dataset/test_xml_1.txt");
    read_config_test("dataset/test_xml_2.txt");
}

TEST_F(ReadStructuredDataTester,TestReadWrite){
    read_write_json_test("/tmp/test_json");
}
