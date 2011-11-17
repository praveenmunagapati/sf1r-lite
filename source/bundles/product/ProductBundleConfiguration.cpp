#include "ProductBundleConfiguration.h"

#include <boost/algorithm/string.hpp>

namespace sf1r
{
ProductBundleConfiguration::ProductBundleConfiguration(const std::string& collectionName)
    : ::izenelib::osgi::BundleConfiguration("ProductBundle-"+collectionName, "ProductBundleActivator" )
    , mode_(0)
    , collectionName_(collectionName)
{}

void ProductBundleConfiguration::setSchema(const std::set<PropertyConfig, PropertyComp>& schema)
{
    schema_ = schema;
}


}
