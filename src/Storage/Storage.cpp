#include "Storage.h"
#include "../../3rd/include/NanoLog/NanoLog.hpp"

Storage::Storage(const std::string &dir):_dir_path(dir)
{
    //TODO consider using bloom filter
    _options.IncreaseParallelism();
    _options.error_if_exists=true;
    rocksdb::Status status=rocksdb::DB::Open(_options,dir,&_db);
    if(!status.ok()){
        LOG_CRIT<<"Error while initialize database: "<<status.ToString();
        exit(-1);
    }
}

Storage::~Storage()
{
    if(_db){
        close();
    }
}

bool Storage::close()
{
    bool ok=true;
    rocksdb::Status status=_db->Close();
    if(!status.ok()){
        LOG_WARN<<"Error while close database: "<<status.ToString();
        ok=false;
    }
    delete _db;
    _db=nullptr;
    return ok;
}

bool Storage::insert(const std::string &key, const std::string &value)
{
    rocksdb::Status status=_db->Put(rocksdb::WriteOptions(),key,value);
    return status.ok();
}

bool Storage::batch_write(const Storage::Batch &batch)
{
    rocksdb::WriteBatch db_batch;
    for(const auto &[key,val]:batch){
        db_batch.Put(rocksdb::Slice(key),rocksdb::Slice(val));
    }
    rocksdb::Status status=_db->Write(rocksdb::WriteOptions(),&db_batch);
    return status.ok();
}

bool Storage::remove(const std::string &key)
{
    rocksdb::Status status=_db->Delete(rocksdb::WriteOptions(),key);
    return status.ok();
}

Storage::Status Storage::get(const std::string &key, std::string &value)
{
    rocksdb::Status status=_db->Get(rocksdb::ReadOptions(),key,&value);
    if(status.ok()&&!status.IsNotFound()){
        return Status::FOUND;
    }
    if(status.IsNotFound()){
        return Status::NOT_FOUND;
    }
    LOG_WARN<<"Error while fetching key: "<<key<<"  status="<<status.ToString();
    return Status::ERROR;
}

bool Storage::contains(const std::string &key)
{
    std::string value;
    return this->get(key,value)==Storage::FOUND;
}
