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

class Storage
{
    enum Status{
        FOUND,
        NOT_FOUND,
        ERROR
    };
    using Batch=std::vector<std::pair<std::string_view,std::string_view>>;

public:
    Storage(const std::string &dir);
    ~Storage();

    void close();

    /// @brief If the database contains an entry for "key" store the
    ///        corresponding value in *value and return FOUND.
    ///        If there is no entry for "key" leave *value unchanged,
    ///        return NOT_FOUND if no error occurs,
    Status get(const std::string &key,std::string &value);

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
    const std::string _dir_path;

};

#endif // STORAGE_H
