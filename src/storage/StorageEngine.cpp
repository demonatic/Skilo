#include "StorageEngine.h"
#include <g3log/g3log.hpp>
#include <iostream>
namespace Skilo {
namespace Storage{

StorageEngine::StorageEngine(const std::string &db_path):_db_path(db_path)
{
    LOG(INFO)<<"initialize database from dir:"<<db_path;
    _options.IncreaseParallelism();
    _options.create_if_missing=true;
//    _options.write_buffer_size=4*1310724;
    _options.max_background_flushes=2;
    _options.compression=rocksdb::CompressionType::kSnappyCompression;
    rocksdb::Status status=rocksdb::DB::Open(_options,db_path,&_db);
    if(!status.ok()){
        LOG(FATAL)<<"Error while initialize database: "<<status.ToString();
        exit(-1);
    }
}

StorageEngine::~StorageEngine()
{
    if(_db){
        flush();
        close();
    }
}

void StorageEngine::close()
{
    rocksdb::Status status=_db->Close();
    if(!status.ok()){
        LOG(WARNING)<<"Error while close database: "<<status.ToString();
    }
    delete _db;
    _db=nullptr;
}

bool StorageEngine::insert(const std::string &key, const std::string &value)
{
    rocksdb::Status status=_db->Put(rocksdb::WriteOptions(),key,value);
    return status.ok();
}

bool StorageEngine::batch_write(StorageEngine::Batch &batch)
{
    rocksdb::Status status=_db->Write(rocksdb::WriteOptions(),&batch);
    return status.ok();
}

void StorageEngine::load_with_prefix(const std::string &prefix, std::vector<std::string> &values) const
{
    rocksdb::Iterator *it=_db->NewIterator(rocksdb::ReadOptions());
    for(it->Seek(prefix);it->Valid()&&it->key().starts_with(prefix);it->Next()){
        values.push_back(it->value().ToString());
    }
    delete it;
}

void StorageEngine::scan_for_each(const std::string &prefix, std::function<void(const std::string_view value)> callback) const
{
    rocksdb::Iterator *it=_db->NewIterator(rocksdb::ReadOptions());
    for(it->Seek(prefix);it->Valid()&&it->key().starts_with(prefix);it->Next()){
        callback(it->value().ToString());
    }
    delete it;
}


bool StorageEngine::remove(const std::string &key)
{
    rocksdb::Status status=_db->Delete(rocksdb::WriteOptions(),key);
    return status.ok();
}

void StorageEngine::flush()
{
    rocksdb::FlushOptions options;
    _db->Flush(options);
}

StorageEngine::Status StorageEngine::get(const std::string &key, std::string &value) const
{
    rocksdb::Status status=_db->Get(rocksdb::ReadOptions(),key,&value);
    if(status.ok()&&!status.IsNotFound()){
        return Status::FOUND;
    }
    if(status.IsNotFound()){
        return Status::NOT_FOUND;
    }
    LOG(WARNING)<<"Error while fetching key: "<<key<<"  status="<<status.ToString();
    return Status::ERROR;
}

bool StorageEngine::contains(const std::string &key) const
{
    std::string value;
    return this->get(key,value)==StorageEngine::FOUND;
}

} //namespace Storage

} //namespace skilo
