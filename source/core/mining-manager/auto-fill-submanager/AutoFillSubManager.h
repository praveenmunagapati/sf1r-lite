/*!
 \file	AutoFillSubmanager.h
 \author	Jinglei&&Kevin
 \brief	AutoFillSubManager will get the search keyword list which match the prefix of the keyword user typed.
 */
#if !defined(_AUTO_FILL_SUBMANAGER_)
#define _AUTO_FILL_SUBMANAGER_

#include <vector>
#include <iostream>
#include <fstream>
#include <string.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <log-manager/LogManager.h>
#include <mining-manager/taxonomy-generation-submanager/LabelManager.h>
#include <util/ThreadModel.h>
#include <util/ustring/UString.h>

#include <boost/filesystem.hpp>
#include <boost/thread/shared_mutex.hpp>
#include "word_completion_table.hpp"
#include <util/singleton.h>
#include <idmlib/util/directory_switcher.h>

namespace sf1r
{
/**
 * @brief AutoFillSubManager handles the autoFill request from user.
 * 1. Auto-fill sub-manager uses inverted structure that based on B-Tree to index
 *    the rank sensitive results for given prefixes.
 * 2. It could handle large data and give a fast overall auto-completion speed based
 *    on the incremental caching of B-Tree.
 * It contains 2 main interfaces.
 * buildIndex  : To build the index.
 * getAutoFillList : To get the list of matched items for a given prefix.
 */
class AutoFillSubManager: public boost::noncopyable
{
    typedef std::pair<count_t, izenelib::util::UString> ItemValueType;
    typedef WordCompletionTable<count_t> Trie_;
    
public:
  explicit AutoFillSubManager();
  ~AutoFillSubManager();

private:
    std::string fillSupportPath_;
    std::string triePath_;
    /**
     * @brief the returned number of items.
     */
    uint32_t top_;
    /**
     * @brief the timePeriodfor update query Log.
     */
    uint32_t queryUpdateTime_;
    Trie_* autoFillTrie_;
    boost::shared_mutex mutex_;
    idmlib::util::DirectorySwitcher* dir_switcher_;
public:
    
    static AutoFillSubManager* get()
    {
      return izenelib::util::Singleton<AutoFillSubManager>::get();
    }
    
    bool Init(const std::string& fillSupportPath, uint32_t top=10, uint32_t queryUpdateTime=24);

	   
    /**
     * @brief build the B-tree index for the collection and query log.
     * @param queryList The recent queries got from query log.
     */
    bool buildIndex(const std::list<std::pair<izenelib::util::UString, uint32_t> >& queryList,
                     const std::vector<boost::shared_ptr<LabelManager> >& label_manager_list);

	bool buildIndex(const std::list<std::pair<izenelib::util::UString, uint32_t> >& queryList	);
    
	
    /**
     * @brief Get the autocompletion list for a give query.
     * @param query The given query from BA.
     * @param list The returned autocompletion result for the query string.
     */
    bool getAutoFillList(const izenelib::util::UString& query, std::vector<izenelib::util::UString>& list);

private:
	

    // worker for update the query log information.
//     void updateLogWorker();
// 
//     void updateRecentLog();

    void updateTrie(Trie_* trie, const izenelib::util::UString& key , const ItemValueType& value);

    void buildTrieItem(Trie_* trie, const izenelib::util::UString& key , uint32_t value);
    void buildTrieItem(Trie_* trie, const izenelib::util::UString& key, const ItemValueType& item);
    
};
} // end - namespace sf1r
#endif // _AUTO_FILL_SUBMANAGER_
