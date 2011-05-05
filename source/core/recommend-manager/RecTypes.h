/**
 * @file RecTypes.h
 * @author Jun Jiang
 * @date 2011-04-18
 */

#ifndef REC_TYPES_H
#define REC_TYPES_H

#ifndef SF1R_NO_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#else
# ifdef HAVE_STDINT_H
#  include <stdint.h>
# else
#  ifdef HAVE_SYS_TYPES_H
#   include <sys/types.h>
#  endif
#  ifdef HAVE_STDDEF_H
#   include <stddef.h>
#  endif
# endif
#endif

#include <ir/id_manager/IDGenerator.h>
#include <idmlib/resys/ItemCoVisitation.h>
#include <idmlib/resys/incremcf/IncrementalItemCF.h>

#include <set>
#include <string>

namespace sf1r
{

typedef uint32_t userid_t;
typedef uint32_t itemid_t;
typedef uint32_t orderid_t;

enum RecommendType
{
    FREQUENT_BUY_TOGETHER = 0,
    BUY_ALSO_BUY,
    VIEW_ALSO_VIEW,
    BASED_ON_PURCHASE_HISTORY,
    BASED_ON_BROWSE_HISTORY,
    BASED_ON_SHOP_CART,
    RECOMMEND_TYPE_NUM
};

typedef std::set<itemid_t> ItemIdSet;
typedef izenelib::ir::idmanager::UniqueIDGenerator<std::string, uint32_t, ReadWriteLock> RecIdGenerator;

typedef idmlib::recommender::ItemCoVisitation<idmlib::recommender::CoVisitFreq> CoVisitManager;
typedef idmlib::recommender::IncrementalItemCF ItemCFManager;

} // namespace sf1r

#endif // REC_TYPES_H
