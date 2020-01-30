#ifndef STORAGE_H
#define STORAGE_H

#include <memory>
#include <string_view>
#include <rocksdb/db.h>
#include <rocksdb/write_batch.h>
#include <rocksdb/options.h>
#include <rocksdb/merge_operator.h>
#include <rocksdb/transaction_log.h>

/// @class Storage Layer for Skilo
/// @brief Abstraction for underlying KV storage

namespace Skilo {
namespace Storage{

class StorageEngine
{
public:
    enum Status{
        FOUND,
        NOT_FOUND,
        ERROR
    };
    struct Batch{
        void put(std::string_view key,std::string_view val){
            data.emplace_back(key,val);
        }
        std::vector<std::pair<std::string_view,std::string_view>> data;
    };

public:
    StorageEngine(const std::string &db_path);
    ~StorageEngine();

    void close();

    /// @brief If the database contains an entry for "key" store the
    ///        corresponding value in *value and return FOUND.
    ///        If there is no entry for "key" leave *value unchanged,
    ///        return NOT_FOUND if no error occurs,
    Status get(const std::string &key,std::string &value) const;

    /// @brief return true if the "key" exists
    bool contains(const std::string &key);

    /// @brief Set the database entry for "key" to "value".
    ///        If "key" already exists, it will be overwritten.
    bool insert(const std::string &key,const std::string &value);

    /// @brief all data in this batch will be atomically written into db
    bool batch_write(const Batch &batch);

    /// @return true if key is not exist or been removed successfully
    bool remove(const std::string &key);

private:
    rocksdb::DB *_db;
    rocksdb::Options _options;
    const std::string _db_path;

};

} //namespace storage
} //namespace skilo
#endif // STORAGE_H
