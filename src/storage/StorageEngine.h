#ifndef STORAGE_H
#define STORAGE_H

#include <memory>
#include <functional>
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
    using Batch=rocksdb::WriteBatch;

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
    bool contains(const std::string &key) const;

    /// @brief Set the database entry for "key" to "value".
    ///        If "key" already exists, it will be overwritten.
    bool insert(const std::string &key,const std::string &value);

    /// @brief all data in this batch will be atomically written into db
    bool batch_write(Batch &batch);

    /// @brief scan the database and add any value whose key starts with 'prefix' to 'values'
    void load_with_prefix(const std::string &prefix,std::vector<std::string> &values) const;

    /// @brief scan the database and call 'callback' for any value whose key starts with 'prefix'
    void scan_for_each(const std::string &prefix,std::function<void(const std::string_view value)> callback) const;

    /// @return true if key is not exist or been removed successfully
    bool remove(const std::string &key);

    void flush();

private:
    rocksdb::DB *_db;
    rocksdb::Options _options;
    const std::string _db_path;

};

} //namespace storage
} //namespace skilo
#endif // STORAGE_H
