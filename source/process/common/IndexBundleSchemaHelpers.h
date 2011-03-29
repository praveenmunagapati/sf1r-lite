#ifndef SF1R_PROCESS_COMMON_INDEXBUNDLE_SCHEMA_HELPER_H
#define SF1R_PROCESS_COMMON_INDEXBUNDLE_SCHEMA_HELPER_H
/**
 * @file process/common/IndexBundleSchemaHelpers.h
 * @author Ian Yang
 * @date Created <2010-07-12 11:14:22>
 * @brief Helper functions for IndexBundleSchema
 */

#include <bundles/index/IndexBundleConfiguration.h>

#include <vector>
#include <string>

namespace sf1r {

bool getPropertyConfig(
    const IndexBundleSchema& schema, 
    PropertyConfig& config
);

bool getPropertyConfig(
    const IndexBundleSchema& schema,
    const std::string& name, 
    PropertyConfig& type
);

/// Get default properties in which search is performed when search.in is not
/// specified.
void getDefaultSearchPropertyNames(
    const IndexBundleSchema& schema,
    std::vector<std::string>& names
);

/// Get default properties in result if "select" is not specified.
void getDefaultSelectPropertyNames(
    const IndexBundleSchema& schema,
    std::vector<std::string>& names
);

bool isPropertySortable(
    const IndexBundleSchema& schema,
    const std::string& property
);

bool isPropertyFilterable(
    const IndexBundleSchema& schema,
    const std::string& property
);

sf1r::PropertyDataType getPropertyDataType(
    const IndexBundleSchema& schema,
    const std::string& property
);

} // namespace sf1r

#endif // SF1R_PROCESS_COMMON_INDEXBUNDLE_SCHEMA_HELPER_H
