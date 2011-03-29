///
/// @file ontology_manager.h
/// @brief ontology node value
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-12-21
///

#ifndef SF1R_ONTOLOGY_MANAGER_H_
#define SF1R_ONTOLOGY_MANAGER_H_
#include <sdb/IndexSDB.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <common/type_defs.h>
#include <common/ResultType.h>
#include <configuration-manager/MiningSchema.h>
#include "faceted_types.h"
#include "ontology.h"
#include "ontology_searcher.h"
#include <document-manager/DocumentManager.h>
#include <index-manager/IndexManager.h>
#include <idmlib/util/idm_analyzer.h>
#include <idmlib/util/file_object.h>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/mpl/and.hpp>
#include <mining-manager/taxonomy-generation-submanager/LabelManager.h>
#include <ir/id_manager/IDManager.h>
#include <la-manager/LAManager.h>
#include <search-manager/ANDDocumentIterator.h>
#include <search-manager/ORDocumentIterator.h>
#include <search-manager/TermDocumentIterator.h>
#include <ir/index_manager/index/LAInput.h>
NS_FACETED_BEGIN
using namespace izenelib::ir::idmanager;
using namespace izenelib::ir::indexmanager;
typedef std::pair<uint32_t, uint32_t> id2count_t;
class OntologyManager {

public:
  OntologyManager(const std::string& container, idmlib::util::IDMAnalyzer* analyzer);
  OntologyManager(const std::string& container, const boost::shared_ptr<DocumentManager>& document_manager,idmlib::util::IDMAnalyzer* analyzer, const std::string& tgpath);
  OntologyManager(const std::string& container, const boost::shared_ptr<DocumentManager>& document_manager, const std::vector<std::string>& properties, idmlib::util::IDMAnalyzer* analyzer);

  OntologyManager( std::string& container,  boost::shared_ptr<DocumentManager>& document_manager,
		  idmlib::util::IDMAnalyzer* analyzer,
		  boost::shared_ptr<IndexManager>& indexManager,
		  LabelManager* labelManager,
		  boost::shared_ptr<IDManager>& idManager,
		  LabelManager::LabelDistributeSSFType::ReaderType* reader,
		  boost::shared_ptr<LAManager>& laManager);


  OntologyManager(
  		 std::string& facetedPath,
  		 std::string& tgLabelPath,
  		//collectionid_t collectionId,
  		 std::string& collectionName,
                const schema_type& schema,
                const MiningSchema& mining_schema_,
  		 boost::shared_ptr<DocumentManager>& document_manager,
  		// std::vector<std::string>& properties,
  		idmlib::util::IDMAnalyzer* analyzer,
  		boost::shared_ptr<IndexManager>& index_manager,
  		//LabelManager* labelManager,
  		boost::shared_ptr<LabelManager>& labelManager,
  		boost::shared_ptr<IDManager>& idManager,
  		LabelManager::LabelDistributeSSFType::ReaderType* reader,
  		boost::shared_ptr<LAManager>& laManager
  		);
  ~OntologyManager();
  
  bool Open();
  bool close();
  bool ProcessCollection(bool rebuild = false);
  bool ProcessDocuments(bool rebuild = false);
  bool ProcessCollectionWithSmooth(bool rebuild = false);
  bool GetXML(std::string& xml);
  
  bool SetXML(const std::string& xml);
  bool SetXMLNew(const std::string& xml);
  Ontology* GetOntology()
  {
    return service_;
  }
  
  OntologySearcher* GetSearcher()
  {
    return searcher_;
  }
  
  bool DefineDocCategory(const std::vector<ManmadeDocCategoryItem>& items);
  typedef izenelib::ir::indexmanager::termid_t izene_termid_t;
  int getCoOccurence(uint32_t labelTermId,uint32_t topicTermId);
private:
  void  calculateCoOccurenceByIndexManager(Ontology* ontology);
  uint32_t GetProcessedMaxId_();
  bool ProcessCollection_(bool rebuild = false);
  bool ProcessDocuments_(bool rebuild = false);
  //void readDocsFromFile(const std::string& file, vector<Document> &docs);
  int getCollectionTermsCount();
  int getTFInDoc(izenelib::util::UString& label,Document& doc );
  int getTFInCollection(izenelib::util::UString& label);
  int getDocTermsCount(Document& doc);
  bool ProcessCollectionWithSmooth_(bool rebuild);
  void getDocumentFrequency(boost::shared_ptr<DocumentManager>& document_manager,
  						uint32_t last_docid,Ontology* ontology,
  						std::map<uint32_t, uint32_t> &topicDF,
  						std::map<std::pair<uint32_t, izenelib::util::UString>,uint32_t> &coDF);

  void  calculateCoOccurenceBySearchManager(Ontology* ontology);

private:
  std::string container_;
  collectionid_t collectionId_;
  std::string collectionName_;
  schema_type schema_;
  MiningSchema mining_schema_;
  boost::shared_ptr<DocumentManager> document_manager_;
  idmlib::util::IDMAnalyzer* analyzer_;
  Ontology* service_;
  OntologySearcher* searcher_;
  ManmadeDocCategory* manmade_;
  boost::shared_ptr<IndexManager> index_manager_;
  boost::shared_ptr<LabelManager> labelManager_;
  boost::shared_ptr<IDManager> idManager_;
  LabelManager::LabelDistributeSSFType::ReaderType* reader_;
  boost::shared_ptr<LAManager> laManager_;
  idmlib::util::FileObject<uint32_t> max_docid_file_;
  idmlib::util::FileObject<std::vector<std::list<uint32_t> > > docItemListFile_;
  //idmlib::util::FileObject<std::map<std::pair<izene_termid_t, izene_termid_t>, int> > cooccurenceFile_;
  boost::mutex mutex_;
  std::string scdpath_;
  std::string tgPath_;
  izenelib::am::rde_hash<std::string, bool> faceted_properties_;
  Ontology* edit_;
  uint32_t maxDocid_;
  std::vector<uint32_t> property_ids_;
  std::vector<std::string> property_names_;
  IndexSDB<unsigned int, unsigned int,izenelib::util::ReadWriteLock>* docRuleTFSDB_;
    
};
NS_FACETED_END
#endif 

