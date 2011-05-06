#include <common/SFLogger.h>
#include <document-manager/DocumentManager.h>
#include <query-manager/QueryIdentity.h>
#include <query-manager/ActionItem.h>
#include <ranking-manager/RankQueryProperty.h>
#include <common/SFLogger.h>

#include "SearchManager.h"
#include "CustomRanker.h"
#include "SearchCache.h"

#include <util/swap.h>
#include <util/get.h>

#include <boost/algorithm/string.hpp>

#include <fstream>
#include <algorithm>

namespace sf1r
{

const unsigned SEARCH_MANAGER_CACHE_SIZE = 1000;

SearchManager::SearchManager(std::set<PropertyConfig, PropertyComp> schema,
                             const boost::shared_ptr<IDManager>& idManager,
                             const boost::shared_ptr<DocumentManager>& documentManager,
                             const boost::shared_ptr<IndexManager>& indexManager,
                             const boost::shared_ptr<RankingManager>& rankingManager
                            )
        : schemaMap_()
        , indexManagerPtr_(indexManager)
        , documentManagerPtr_(documentManager)
        , rankingManagerPtr_(rankingManager)
        , queryBuilder_()
        , pSorterCache_(0)
{
    for (std::set<PropertyConfig, PropertyComp>::iterator iter = schema.begin(); iter != schema.end(); ++iter)
        schemaMap_[iter->getName()] = *iter;

    pSorterCache_ = new SortPropertyCache(indexManagerPtr_.get());
    queryBuilder_.reset(new QueryBuilder(indexManager,documentManager,idManager,schemaMap_));
    cache_.reset(new SearchCache(SEARCH_MANAGER_CACHE_SIZE));
    rankingManagerPtr_->getPropertyWeightMap(propertyWeightMap_);
}

SearchManager::~SearchManager()
{
    if (pSorterCache_)
        delete pSorterCache_;
}


void SearchManager::reset_cache()
{
    pSorterCache_->setDirty(true);
    cache_->clear();
    queryBuilder_->reset_cache();
}

/// @brief change working dir by setting new underlying componenets
void SearchManager::chdir(const boost::shared_ptr<IDManager>& idManager,
                          const boost::shared_ptr<DocumentManager>& documentManager,
                          const boost::shared_ptr<IndexManager>& indexManager)
{
    indexManagerPtr_ = indexManager;
    documentManagerPtr_ = documentManager;

    // init query builder
    queryBuilder_.reset(new QueryBuilder(indexManager, documentManager, idManager, schemaMap_));

    // clear cache
    delete pSorterCache_;
    pSorterCache_ = new SortPropertyCache(indexManagerPtr_.get());
    cache_.reset(new SearchCache(SEARCH_MANAGER_CACHE_SIZE));
}

bool SearchManager::search(SearchKeywordOperation& actionOperation,
                           std::vector<unsigned int>& docIdList,
                           std::vector<float>& rankScoreList,
                           std::vector<float>& customRankScoreList,
                           std::size_t& totalCount,
                           int topK,
                           int start)
{
CREATE_PROFILER ( cacheoverhead, "SearchManager", "cache overhead: overhead for caching in searchmanager");

START_PROFILER ( cacheoverhead )

    QueryIdentity identity;
    makeQueryIdentity(identity, actionOperation.actionItem_, start);

    if (cache_->get(identity, rankScoreList, customRankScoreList, docIdList, totalCount))
    {
        // cache hit, ignore topK
#if 0 ///commonSet is not required any more
        docIdList.resize(commonSet.size());
        std::transform(commonSet.begin(), commonSet.end(),
                       docIdList.begin(),
                       boost::bind(&DocumentItem::id, _1));
#endif
STOP_PROFILER ( cacheoverhead )
        return true;
    }

STOP_PROFILER ( cacheoverhead )

    // cache miss
    if (doSearch_(actionOperation,
                  docIdList,
                  rankScoreList,
                  customRankScoreList,
                  totalCount,
                  topK,
                  start))
    {
 START_PROFILER ( cacheoverhead )
        cache_->set(identity, rankScoreList, customRankScoreList, docIdList, totalCount);
 STOP_PROFILER ( cacheoverhead )
        return true;
    }

    return false;
}

bool SearchManager::doSearch_(SearchKeywordOperation& actionOperation,
                              std::vector<unsigned int>& docIdList,
                              std::vector<float>& rankScoreList,
                              std::vector<float>& customRankScoreList,
                              std::size_t& totalCount,
                              int topK,
                              int start)
{
    CREATE_PROFILER ( dociterating, "SearchManager", "doSearch_: doc iterating");
    CREATE_PROFILER ( preparedociter, "SearchManager", "doSearch_: SearchManager_search : build doc iterator");
    CREATE_PROFILER ( preparerank, "SearchManager", "doSearch_: prepare ranker");
    CREATE_PROFILER ( preparesort, "SearchManager", "doSearch_: prepare sort");
    CREATE_PROFILER ( preparegroupby, "SearchManager", "doSearch_: prepare group by");
    CREATE_PROFILER ( computerankscore, "SearchManager", "doSearch_: overall time for scoring a doc");

    unsigned int collectionId = 1;
    std::vector<std::string> indexPropertyList( actionOperation.actionItem_.searchPropertyList_ );
#if PREFETCH_TERMID
    std::stable_sort (indexPropertyList.begin(), indexPropertyList.end());
#endif
    typedef std::vector<std::string>::size_type property_size_type;
    property_size_type indexPropertySize = indexPropertyList.size();
    std::vector<propertyid_t> indexPropertyIdList(indexPropertySize);
    std::transform(
                indexPropertyList.begin(),
                indexPropertyList.end(),
                indexPropertyIdList.begin(),
                boost::bind(&SearchManager::getPropertyIdByName, this, _1)
    );

    if ( actionOperation.noError() )
    {
        START_PROFILER ( preparedociter )
        sf1r::TextRankingType& pTextRankingType = actionOperation.actionItem_.rankingType_;

        // references for property term info
        typedef std::map<std::string, PropertyTermInfo> property_term_info_map;
        const property_term_info_map& propertyTermInfoMap =
            actionOperation.getPropertyTermInfoMap();
        // use empty object for not found property
        const PropertyTermInfo emptyPropertyTermInfo;

        // build term index maps
        std::vector<std::map<termid_t, unsigned> >
            termIndexMaps(indexPropertySize);
        typedef std::vector<std::string>::const_iterator property_list_iterator;
        for (std::vector<std::string>::size_type i = 0;
             i != indexPropertyList.size(); ++i)
        {
            const PropertyTermInfo::id_uint_list_map_t& termPositionsMap =
                izenelib::util::getOr(
                    propertyTermInfoMap,
                    indexPropertyList[i],
                    emptyPropertyTermInfo
                ).getTermIdPositionMap();

            unsigned index = 0;
            typedef PropertyTermInfo::id_uint_list_map_t::const_iterator
                term_id_position_iterator;
            for (term_id_position_iterator termIt = termPositionsMap.begin();
                 termIt != termPositionsMap.end(); ++termIt)
            {
                termIndexMaps[i][termIt->first] = index++;
            }
        }

        std::vector<boost::shared_ptr<PropertyRanker> > propertyRankers;
        rankingManagerPtr_->createPropertyRankers(pTextRankingType, indexPropertySize, propertyRankers);
        bool readTermPosition = propertyRankers[0]->requireTermPosition();

        MultiPropertyScorer* pDocIterator =  NULL;
        std::vector<QueryFiltering::FilteringType>& filtingList
                                                        = actionOperation.actionItem_.filteringList_;
        Filter* pFilter = NULL;
        try
        {
            if (!filtingList.empty())
                queryBuilder_->prepare_filter(filtingList, pFilter);

            queryBuilder_->prepare_dociterator(
                actionOperation,
                collectionId,
                propertyWeightMap_,
                indexPropertyList,
                indexPropertyIdList,
                readTermPosition,
                termIndexMaps,
                pDocIterator
            );
            if (NULL == pDocIterator)
            {
                if (pFilter) delete pFilter;
                return false;
            }
        }
        catch (std::exception& e)
        {
            if (pDocIterator) delete pDocIterator;
            if (pFilter) delete pFilter;
            return false;
        }
        STOP_PROFILER ( preparedociter )

        START_PROFILER ( preparerank )

        ///prepare data for rankingmanager;
        DocumentFrequencyInProperties dfmap;
        CollectionTermFrequencyInProperties ctfmap;

        pDocIterator->df_ctf(dfmap, ctfmap);

        vector<RankQueryProperty> rankQueryProperties(indexPropertySize);

        for (property_size_type i = 0; i < indexPropertySize; ++i)
        {
            const std::string& currentProperty = indexPropertyList[i];

            rankQueryProperties[i].setNumDocs(
                indexManagerPtr_->getIndexReader()->numDocs()
            );

            rankQueryProperties[i].setTotalPropertyLength(
                documentManagerPtr_->getTotalPropertyLength(currentProperty)
            );

            const PropertyTermInfo::id_uint_list_map_t& termPositionsMap =
                izenelib::util::getOr(
                    propertyTermInfoMap,
                    currentProperty,
                    emptyPropertyTermInfo
                ).getTermIdPositionMap();

            typedef PropertyTermInfo::id_uint_list_map_t::const_iterator
                term_id_position_iterator;
            unsigned queryLength = 0;
            unsigned index = 0;
            for (term_id_position_iterator termIt = termPositionsMap.begin();
                 termIt != termPositionsMap.end(); ++termIt, ++index)
            {
                rankQueryProperties[i].addTerm(termIt->first);
                rankQueryProperties[i].setTotalTermFreq(
                    ctfmap[currentProperty][termIt->first]
                );
                rankQueryProperties[i].setDocumentFreq(
                    dfmap[currentProperty][termIt->first]
                );

                queryLength += termIt->second.size();
                if (readTermPosition)
                {
                    typedef PropertyTermInfo::id_uint_list_map_t::mapped_type
                        uint_list_map_t;
                    typedef uint_list_map_t::const_iterator uint_list_iterator;
                    for (uint_list_iterator posIt = termIt->second.begin();
                         posIt != termIt->second.end(); ++posIt)
                    {
                        rankQueryProperties[i].pushPosition(*posIt);
                    }
                }
                else
                {
                    rankQueryProperties[i].setTermFreq(termIt->second.size());
                }
            }

            rankQueryProperties[i].setQueryLength(queryLength);
        }

        for(size_t i = 0; i < indexPropertySize; ++i )
        {
            propertyRankers[i]->setupStats(rankQueryProperties[i]);
        }


        STOP_PROFILER ( preparerank )

        START_PROFILER ( preparegroupby )
        ///constructing sorter
        CustomRankerPtr customRanker;
        Sorter* pSorter = NULL;
        try
        {
            std::vector<std::pair<std::string , bool> >& sortPropertyList = actionOperation.actionItem_.sortPriorityList_;
            if (!sortPropertyList.empty())
            {
                for (std::vector<std::pair<std::string , bool> >::iterator iter = sortPropertyList.begin(); iter != sortPropertyList.end(); ++iter)
                {
                    std::string fieldNameL = iter->first;
                    boost::to_lower(fieldNameL);
                    // sort by custom ranking
                    if (fieldNameL == "custom_rank")
                    {
                        // prepare custom ranker
                        cout << "[SearchManager] add custom_rank property to Sorter .." << endl; //
                        customRanker = actionOperation.actionItem_.customRanker_;
                        customRanker->printESTree(); //test
                        if (!customRanker->setPropertyData(pSorterCache_)) {
                            // error info
                            cout << customRanker->getErrorInfo() << endl;
                            return false;
                        }
                        customRanker->printESTree(); //test

                        if (!pSorter) pSorter = new Sorter(pSorterCache_);
                        SortProperty* pSortProperty = new SortProperty("CUSTOM_RANK", CUSTOM_RANKING_PROPERTY_TYPE, SortProperty::CUSTOM, iter->second);
                        pSorter->addSortProperty(pSortProperty);
                        continue;
                    }
                    // sort by rank
                    if (fieldNameL == "_rank")
                    {
                        if (!pSorter) pSorter = new Sorter(pSorterCache_);
                        SortProperty* pSortProperty = new SortProperty("RANK", UNKNOWN_DATA_PROPERTY_TYPE, SortProperty::SCORE, iter->second);
                        pSorter->addSortProperty(pSortProperty);
                        continue;
                    }
                    // sort by date
                    if (fieldNameL == "date")
                    {
                        if (!pSorter) pSorter = new Sorter(pSorterCache_);

                        sflog->info(SFL_SRCH, 130103);
                        //std::cout << "sort by date" << std::endl;
                        //if (!pSorter) pSorter = new Sorter(pSorterCache_, iter->second);
                        SortProperty* pSortProperty = new SortProperty(iter->first, INT_PROPERTY_TYPE, iter->second);
                        pSorter->addSortProperty(pSortProperty);
                        continue;
                    }

                    // sort by arbitrary property
                    boost::unordered_map<std::string, PropertyConfig>::iterator it = schemaMap_.find(iter->first);
                    //std::cout << "sort by " << iter->first << std::endl;
                    if (it == schemaMap_.end())
                        continue;

                    PropertyConfig& propertyConfig = it->second;
                    if (!propertyConfig.isIndex()||propertyConfig.isAnalyzed())
                        continue;

                    PropertyDataType propertyType = propertyConfig.getType();
                    switch (propertyType)
                    {
                        case INT_PROPERTY_TYPE:
                        case FLOAT_PROPERTY_TYPE:
                        case NOMINAL_PROPERTY_TYPE:
                        case UNSIGNED_INT_PROPERTY_TYPE:
                        case DOUBLE_PROPERTY_TYPE:
                        {
                            if (!pSorter) pSorter = new Sorter(pSorterCache_);
                            SortProperty* pSortProperty = new SortProperty(iter->first, propertyType, iter->second);
                            pSorter->addSortProperty(pSortProperty);
                            break;
                        }
                        default:
                            sflog->error(SFL_SRCH, 110101, "Sort by properties other than int, float, double type");
                            DLOG(ERROR) << "Sort by properties other than int, float, double type"; // TODO : Log
                            break;
                    }
                }
            }
        }
        catch (std::exception& e)
        {
            delete pDocIterator;
            if (pSorter) delete pSorter;
            if (pFilter) delete pFilter;
            return false;
        }

        STOP_PROFILER ( preparegroupby )

        START_PROFILER ( preparesort )

        ///constructing collector
        HitQueue* scoreItemQueue = NULL;

        int heapSize = topK + start;

        ///sortby
        if ( pSorter )
            scoreItemQueue = new PropertySortedHitQueue(pSorter, heapSize);
        else
            scoreItemQueue = new ScoreSortedHitQueue(heapSize);

        STOP_PROFILER ( preparesort )

        START_PROFILER ( dociterating )
        try
        {
            totalCount = 0;
            while (pDocIterator->next())
            {
                if (pFilter)
                    if (!pFilter->test(pDocIterator->doc()))
                        continue;


                STOP_PROFILER ( dociterating )
                ScoreDoc scoreItem(pDocIterator->doc());
                START_PROFILER ( computerankscore )
                ++totalCount;
                scoreItem.score = pDocIterator->score(
                    rankQueryProperties,
                    propertyRankers
                );
                STOP_PROFILER ( computerankscore )

                START_PROFILER ( computecustomrankscore )
                if (customRanker.get())
                {
                    scoreItem.custom_score = customRanker->evaluate(scoreItem.docId);
                }
                START_PROFILER ( computecustomrankscore )

                scoreItemQueue->insert(scoreItem);
                START_PROFILER ( dociterating )
            }
            STOP_PROFILER ( dociterating )

            size_t count = start > 0 ? (scoreItemQueue->size() - start) : scoreItemQueue->size();
            docIdList.resize(count);
            rankScoreList.resize(count);
            if (customRanker.get())
            {
                customRankScoreList.resize(count);
            }

            ///Attention
            ///Given default lessthan() for prorityqueue
            ///pop will get the elements of priorityqueue with lowest score
            ///So we need to reverse the output sequence
            float min = (std::numeric_limits<float>::max) ();
            float max = - min;
            for (size_t i = 0; i < count; ++i)
            {
                ScoreDoc pScoreItem = scoreItemQueue->pop();
                docIdList[count - i - 1] = pScoreItem.docId;
                rankScoreList[count - i - 1] = pScoreItem.score;
                if (customRanker.get())
                {
                    customRankScoreList[count - i - 1] = pScoreItem.custom_score; // should not be normalized
                }

                if (pScoreItem.score < min)
                {
                    min = pScoreItem.score;
                }
                // cannot use "else if" because the first score is between min
                // and max
                if (pScoreItem.score > max)
                {
                    max = pScoreItem.score;
                }
                /*
                DocumentItem* item = pScoreItem->pItem;
                //izenelib::util::swapBack(commonSet, *item);
                commonSet.push_front(DocumentItem());
                using std::swap;
                swap(commonSet.front(), *item);
                */
                //delete pScoreItem;
            }

            // normalize rank score to [0, 1]
            float range = max - min;
            if (range != 0)
            {
                for (std::vector<float>::iterator it = rankScoreList.begin(),
                                               itEnd = rankScoreList.end();
                     it != itEnd; ++it)
                {
                    *it -= min;
                    *it /= range;
                }
            }
            else
            {
                std::fill(rankScoreList.begin(), rankScoreList.end(), 1.0F);
            }
        }
        catch (std::exception& e)
        {
            delete pDocIterator;
            delete scoreItemQueue;
            if (pSorter) delete pSorter;
            if (pFilter) delete pFilter;
            return false;
        }
        delete pDocIterator;
        delete scoreItemQueue;
        if (pSorter) delete pSorter;
        if (pFilter) delete pFilter;
        return true;
    }
    else
        return false;
}

propertyid_t SearchManager::getPropertyIdByName(const std::string& name) const
{
    typedef boost::unordered_map<std::string, PropertyConfig>::const_iterator
        iterator;
    iterator found = schemaMap_.find(name);
    if (found != schemaMap_.end())
    {
        return found->second.getPropertyId();
    }
    else
    {
        return 0;
    }
}

void SearchManager::printDFCTF(DocumentFrequencyInProperties& dfmap, CollectionTermFrequencyInProperties ctfmap)
{
    cout << "\n[Index terms info for Query]" << endl;
    DocumentFrequencyInProperties::iterator dfiter;
    for (dfiter = dfmap.begin(); dfiter != dfmap.end(); dfiter++)
    {
        cout << "property: " << dfiter->first << endl;
        ID_FREQ_UNORDERED_MAP_T::iterator iter_;
        for (iter_ = dfiter->second.begin(); iter_ != dfiter->second.end(); iter_++)
        {
            cout << "termid: " << iter_->first << " DF: " << iter_->second << endl;
        }
    }
    cout << "-----------------------" << endl;
    CollectionTermFrequencyInProperties::iterator ctfiter;
    for (ctfiter = ctfmap.begin(); ctfiter != ctfmap.end(); ctfiter++)
    {
        cout << "property: " << ctfiter->first << endl;
        ID_FREQ_UNORDERED_MAP_T::iterator iter_;
        for (iter_ = ctfiter->second.begin(); iter_ != ctfiter->second.end(); iter_++)
        {
            cout << "termid: " << iter_->first << " CTF: " << iter_->second << endl;
        }
    }
    cout << "-----------------------" << endl;
}

} // namespace sf1r