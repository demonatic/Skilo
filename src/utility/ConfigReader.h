#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <regex>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace Skilo {

using NodeMap=std::unordered_map<std::string,std::vector<std::string>>; ///NodePath,NodeVals
using PTree=boost::property_tree::ptree;

/**
 * @brief The IStructuredDataReader class is intended to read-deserialization and
 * write-serialization structured data like Json XML etc.
 * The parse result is represented by boost::property_tree:
 *  struct ptree {
 *     data_type data;                         // data associated with the node
 *     list< pair<key_type, ptree> > children; // ordered list of named children
 *  };
 */

class IStructuredDataReader
{
public:
    virtual ~IStructuredDataReader()=default;

    /// @brief parse the structured data from the given file
    virtual void read_from_file(const std::string &file_path)=0;
    /// @brief parse the structured data from the given stringstream
    virtual void read_from_stream(std::stringstream &str_stream)=0;

    /// @brief write the structured data to the given file
    virtual void write_to_file(const std::string &file_path)=0;

    /// @brief write the structured data to the given stringstream
    virtual void write_to_stream(std::stringstream &str_stream)=0;

    /// @param path: The path of the desired value
    /// @return the value retrieved
    /// @throw std::run_time_error when value does not exist
    template<typename T>
    T get(const std::string &path);

    /// @brief put the val to the given path of the tree
    /// @param replace: whether to replace the value if it is already exist
    /// @return true if insert successfully
    template<typename T>
    bool put(const std::string &path, const T &val, bool replace=true);

    /// @brief  get the val in string of the given path
    /// @return the val if exists, or nil if not
    virtual boost::optional<std::string> get_val_str(const std::string &path)=0;

    /// @brief set the node data of the given path to val
    /// @param replace: true to replace the val if the data already exists
    /// @return true if the val is already set
    virtual bool set_str_val(const std::string &path, const std::string &val, bool replace)=0;

    /// @brief   convert the sub tree of given path to a flat key-value map
    /// @note    the pair->first in the map is the relative path to the path parameter with path seprarator "."
    ///              pair->second is it's value
    virtual NodeMap get_flat_kv_map(const std::string &path={})=0;

    /// @brief  return the sub tree of the given path
    /// @default throw not implemented
    virtual PTree get_tree(const std::string &path);

    /// @brief  set the sub tree of the given path to the tree
    /// @default throw not implemented
    virtual void set_tree(const std::string &path, const PTree &tree);


};

/// A base class for parse structure config files using boost::property_tree
class StructuredDataReaderBase:public IStructuredDataReader,public boost::noncopyable{

public:
    StructuredDataReaderBase(){}

    virtual ~StructuredDataReaderBase() override=default;


    ///@throw   the function will throw if it was not a valid path
    ///@warning the json file should not contain an array
    virtual NodeMap get_flat_kv_map(const std::string &path=std::string()) override;

    virtual boost::optional<std::string> get_val_str(const std::string &path) override;

    virtual bool set_str_val(const std::string &path, const std::string &val,bool replace=true) override;

    ///@throw  the function will throw if it was not a valid path
    virtual PTree get_tree(const std::string &path) override;

    virtual void set_tree(const std::string &path, const PTree &tree) override;

protected:
    ///this will hold the parse result
    boost::property_tree::ptree ptree_;

    static const char PATH_SEPARATOR='.';
};

class JsonReader:public StructuredDataReaderBase{
public:

    void read_from_file(const std::string &file_path) override;
    void read_from_stream(std::stringstream &str_stream) override;
    void write_to_file(const std::string &file_path) override;
    void write_to_stream(std::stringstream &str_stream) override;

};

class XMLReader:public StructuredDataReaderBase{
public:

    void read_from_file(const std::string &file_path) override;
    void read_from_stream(std::stringstream &str_stream) override;
    void write_to_file(const std::string &file_path) override;
    void write_to_stream(std::stringstream &str_stream) override;

};

class ConfigurationLoader
{
    enum class SupportedFormat{
        XML,
        Json,
    };

    static class FormatRecognizer{
    public:
        static SupportedFormat recognize_format(std::stringstream &ss){

            SupportedFormat result;
            std::string content=ss.str();

            if(std::regex_search(content,std::regex("\\{[\\S\\s]*:[\\S\\s]*\\}")))
                result=SupportedFormat::Json;

            else if(std::regex_search(content,std::regex("<[\\w \\t]+>+")))
                result=SupportedFormat::XML;

            else
                throw std::runtime_error("Fail to recognize file format");

            return result;
        }
    }_format_recognizer;

public:
    /// @brief get static member of config reader
    ///        if it is nullptr, it will call read_config_from_file
    ///        to initialize with default file
    /// @throw the same as read_config_from_file
    static IStructuredDataReader* get_config(const char *file_path);

    /// @brief read config from the given file
    /// @throw the function will throw when it can't determine the data format
    ///        or couldn't parse the content with the corresponding parser
    static void read_config_from_file(const std::string &file_path);


private:
    static std::unique_ptr<IStructuredDataReader> _config_reader;

};

} //namespace Skilo

#endif // CONFIGREADER_H
