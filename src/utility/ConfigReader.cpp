#include "ConfigReader.h"
#include <boost/algorithm/string/predicate.hpp>

namespace Skilo {

template<> std::string IStructuredDataReader::get(const std::string &path)
{
    boost::optional<std::string> str_val=get_val_str(path);
    if(!str_val.is_initialized()){
        throw std::runtime_error("Can't find val with the given path:"+path);
    }
    return str_val.value();
}

template<> int IStructuredDataReader::get(const std::string &path)
{
    boost::optional<std::string> str_val=get_val_str(path);
    if(!str_val.is_initialized()){
        throw std::runtime_error("Can't find val with the given path:"+path);
    }
    return std::stoi(str_val.value());
}

template<> size_t IStructuredDataReader::get(const std::string &path)
{
    boost::optional<std::string> str_val=get_val_str(path);
    if(!str_val.is_initialized()){
        throw std::runtime_error("Can't find val with the given path:"+path);
    }
    return std::stoul(str_val.value());
}

template<> bool IStructuredDataReader::get(const std::string &path)
{
    boost::optional<std::string> str_val=get_val_str(path);
    if(!str_val.is_initialized())
        throw std::runtime_error("Can't find val with the given path:"+path);

    using boost::algorithm::iequals;
    auto &val=str_val.get();
    return (iequals(val,"true")||iequals(val,"yes")||val=="1");
}

template<> double IStructuredDataReader::get(const std::string &path)
{
    boost::optional<std::string> str_val=get_val_str(path);
    if(!str_val.is_initialized()){
        throw std::runtime_error("Can't find val with the given path:"+path);
    }
    return std::stod(str_val.value());
}


template<> bool IStructuredDataReader::put(const std::string &path, const std::string &val, bool replace){
    return set_str_val(path,val,replace);
}

template<> bool IStructuredDataReader::put(const std::string &path, const int &val, bool replace){
    return set_str_val(path,std::to_string(val),replace);
}

template<> bool IStructuredDataReader::put(const std::string &path, const size_t &val, bool replace){
    return set_str_val(path,std::to_string(val),replace);
}

template<> bool IStructuredDataReader::put(const std::string &path, const double &val, bool replace){
    return set_str_val(path,std::to_string(val),replace);
}

template<> bool IStructuredDataReader::put(const std::string &path, const bool &val, bool replace){
    return set_str_val(path,val?"true":"false",replace);
}

NodeMap StructuredDataReaderBase::get_flat_kv_map(const std::string &path){
    NodeMap map;
    auto sub_tree=path.empty()?ptree_:ptree_.get_child(path);

    //the iterator by default iterators over the tree without descending into lower levels
    //we have to explicitly DFS traverse
    std::function<void(const PTree &ptree,std::string key)> traverser=[&](const PTree &ptree,std::string key){
        if(!ptree.data().empty())
            map[key].emplace_back(std::move(ptree.data()));  //get the actual data in this node

        for(PTree::const_iterator it=ptree.begin();it!=ptree.end();it++){
            //first is a string holding the parent node
            //second is a ptree
            std::string sub_path=key+(key.empty()?"":std::string(1,PATH_SEPARATOR))+it->first;
            traverser(it->second,sub_path);
        }
    };

    traverser(sub_tree,path);
    return map;
}

PTree IStructuredDataReader::get_tree([[maybe_unused]] const std::string &path){
    throw std::runtime_error("Method Get Tree Not Implemented");
}

void IStructuredDataReader::set_tree([[maybe_unused]] const std::string &path,[[maybe_unused]] const PTree &tree)
{
    throw std::runtime_error("Method Set Tree Not Implemented");
}

boost::optional<std::string> StructuredDataReaderBase::get_val_str(const std::string &path){
    return ptree_.get_optional<std::string>(decltype(ptree_)::path_type(path,PATH_SEPARATOR));
}

bool StructuredDataReaderBase::set_str_val(const std::string &path, const std::string &val,bool replace)
{
    auto str_optional=ptree_.get_optional<std::string>(path);
    //if the path exist and not want to replace the val
    if(str_optional.is_initialized() && replace==false){
        return false;
    }
    ptree_.put(path,val);
    return true;
}

PTree StructuredDataReaderBase::get_tree(const std::string &path){
    return ptree_.get_child(path);
}

void StructuredDataReaderBase::set_tree(const std::string &path, const PTree &tree)
{
    ptree_.put_child(path,tree);
}

void JsonReader::read_from_file(const std::string &file_path){
    try {
        ptree_.clear();
        boost::property_tree::read_json(file_path,ptree_);
    } catch (const boost::property_tree::ptree_error &err) {
        throw std::runtime_error("Unable to read from JSON file: "+file_path+" err:"+err.what());
    }
}

void JsonReader::read_from_stream(std::stringstream &str_stream){
    try {
        ptree_.clear();
        boost::property_tree::read_json(str_stream,ptree_);
    } catch (const boost::property_tree::ptree_error &err) {
        throw std::runtime_error("Unable to read JSON from Stream: "+str_stream.str()+" err:"+err.what());
    }
}

void JsonReader::write_to_file(const std::string &file_path)
{
    try {
        boost::property_tree::write_json(file_path,ptree_);
    } catch (const boost::property_tree::ptree_error &err) {
        throw std::runtime_error("Unable to write JSON to file: "+file_path+" err:"+err.what());
    }
}

void JsonReader::write_to_stream(std::stringstream &str_stream)
{
    try {
        boost::property_tree::write_json(str_stream,ptree_,true);
    } catch (const boost::property_tree::ptree_error &err) {
        throw std::runtime_error(std::string("Unable to write JSON to stream  err:")+err.what());
    }
}

void XMLReader::read_from_file(const std::string &file_path){
    try {
        ptree_.clear();
        boost::property_tree::read_xml(file_path,ptree_);
    } catch (const boost::property_tree::ptree_error &err) {
        throw std::runtime_error("Unable to read from XML file: "+file_path+" err:"+err.what());
    }
}

void XMLReader::read_from_stream(std::stringstream &str_stream){
    try {
        ptree_.clear();
        boost::property_tree::read_xml(str_stream,ptree_);
    } catch (const boost::property_tree::ptree_error &err) {
        throw std::runtime_error("Unable to read XML from Stream: "+str_stream.str()+" err:"+err.what());
    }
}

void XMLReader::write_to_file(const std::string &file_path)
{
    try {
        boost::property_tree::write_xml(file_path,ptree_);
    } catch (const boost::property_tree::ptree_error &err) {
        throw std::runtime_error("Unable to write XML to file: "+file_path+" err:"+err.what());
    }
}

void XMLReader::write_to_stream(std::stringstream &str_stream)
{
    try {
        boost::property_tree::write_xml(str_stream,ptree_,true);
    } catch (const boost::property_tree::ptree_error &err) {
        throw std::runtime_error(std::string("Unable to write XML to stream  err:")+err.what());
    }
}

void ConfigurationLoader::read_config_from_file(const std::string &file_path)
{
    std::ifstream file(file_path,std::ifstream::ate|std::ifstream::in);

    if(!file.is_open())
        throw std::runtime_error("fail to open file: \""+file_path+"\"");

    std::streampos file_size=file.tellg();
    file.seekg(std::ios::beg);

    std::vector<char> read_buf(static_cast<size_t>(file_size));
    file.read(read_buf.data(),file_size);
    std::stringstream ss;
    ss.rdbuf()->pubsetbuf(read_buf.data(),file_size);

    try {
        SupportedFormat format=_format_recognizer.recognize_format(ss);

        switch (format) {
            case SupportedFormat::XML: _config_reader.reset(new XMLReader); break;
            case SupportedFormat::Json: _config_reader.reset(new JsonReader); break;
        }
    } catch (std::exception &e) {
        std::cerr<<"Fail when read config from file \""<<file_path<<" \":"<<e.what();
        throw;
    }

    _config_reader->read_from_stream(ss);
}

IStructuredDataReader *ConfigurationLoader::get_config(const char *file_path)
{
    auto p_instance=_config_reader.get();
    if(!p_instance){
        try {
            read_config_from_file(file_path);
            p_instance=_config_reader.get();
        } catch (std::exception &e) {
            std::cerr<<"fail when init config: "<<e.what();
            throw;
        }
    }
    return p_instance;
}

std::unique_ptr<IStructuredDataReader> ConfigurationLoader::_config_reader;


} //namespace Skilo
