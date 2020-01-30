#include "StorageEngine.h"
#include <g3log/g3log.hpp>

namespace Skilo {
namespace Storage{

StorageEngine::StorageEngine(const std::string &db_path):_db_path(db_path)
{
    //TODO consider using bloom filter
    _options.IncreaseParallelism();
    _options.create_if_missing=true;
    _options.write_buffer_size=4*1310724;
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

bool StorageEngine::batch_write(const StorageEngine::Batch &batch)
{
    rocksdb::WriteBatch db_batch;
    for(const auto &[key,val]:batch.data){
        db_batch.Put(rocksdb::Slice(key),rocksdb::Slice(val));
    }
    rocksdb::Status status=_db->Write(rocksdb::WriteOptions(),&db_batch);
    return status.ok();
}

bool StorageEngine::remove(const std::string &key)
{
    rocksdb::Status status=_db->Delete(rocksdb::WriteOptions(),key);
    return status.ok();
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

bool StorageEngine::contains(const std::string &key)
{
    std::string value;
    return this->get(key,value)==StorageEngine::FOUND;
}

} //namespace Storage

} //namespace skilo
