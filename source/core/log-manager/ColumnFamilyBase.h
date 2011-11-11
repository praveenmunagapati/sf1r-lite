#ifndef _COLUMN_FAMILY_BASE_H_
#define _COLUMN_FAMILY_BASE_H_

#include "CassandraConnection.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace sf1r {

class CassandraColumnFamily
{

public:
    CassandraColumnFamily() : exist_(false)
    {}

    virtual ~CassandraColumnFamily() {}

    virtual bool updateRow() const = 0;

    virtual bool deleteRow() = 0;

    virtual bool getRow() = 0;

protected:
    bool exist_;
};

template <typename ColumnFamilyType>
bool createColumnFamily()
{
    return CassandraConnection::instance().createColumnFamily(
            ColumnFamilyType::cf_name,
            ColumnFamilyType::cf_column_type,
            ColumnFamilyType::cf_comparator_type,
            ColumnFamilyType::cf_sub_comparator_type,
            ColumnFamilyType::cf_comment,
            ColumnFamilyType::cf_row_cache_size,
            ColumnFamilyType::cf_key_cache_size,
            ColumnFamilyType::cf_read_repair_chance,
            ColumnFamilyType::cf_column_metadata,
            ColumnFamilyType::cf_gc_grace_seconds,
            ColumnFamilyType::cf_default_validation_class,
            ColumnFamilyType::cf_id,
            ColumnFamilyType::cf_min_compaction_threshold,
            ColumnFamilyType::cf_max_compaction_threshold,
            ColumnFamilyType::cf_row_cache_save_period_in_seconds,
            ColumnFamilyType::cf_key_cache_save_period_in_seconds,
            ColumnFamilyType::cf_compression_options);
}

template <typename ColumnFamilyType>
bool truncateColumnFamily()
{
    return CassandraConnection::instance().truncateColumnFamily(ColumnFamilyType::cf_name);
}

template <typename ColumnFamilyType>
bool dropColumnFamily()
{
    return CassandraConnection::instance().dropColumnFamily(ColumnFamilyType::cf_name);
}

#define DEFINE_COLUMN_FAMILY_COMMON_ROUTINES(ClassName) \
public: \
    static const std::string cf_name; \
    static const std::string cf_column_type; \
    static const std::string cf_comparator_type; \
    static const std::string cf_sub_comparator_type; \
    static const std::string cf_comment; \
    static const double cf_row_cache_size; \
    static const double cf_key_cache_size; \
    static const double cf_read_repair_chance; \
    static const std::vector<org::apache::cassandra::ColumnDef> cf_column_metadata; \
    static const int32_t cf_gc_grace_seconds; \
    static const std::string cf_default_validation_class; \
    static const int32_t cf_id; \
    static const int32_t cf_min_compaction_threshold; \
    static const int32_t cf_max_compaction_threshold; \
    static const int32_t cf_row_cache_save_period_in_seconds; \
    static const int32_t cf_key_cache_save_period_in_seconds; \
    static const std::map<std::string, std::string> cf_compression_options; \
    \
    static bool createColumnFamily() \
    { \
        return ::sf1r::createColumnFamily<ClassName>(); \
    } \
    \
    static bool truncateColumnFamily() \
    { \
        return ::sf1r::truncateColumnFamily<ClassName>(); \
    } \
    \
    static bool dropColumnFamily() \
    { \
        return ::sf1r::dropColumnFamily<ClassName>(); \
    } \


}

#endif
