/**
 * @file process/controllers/CollectionHandler.cpp
 * @author Ian Yang
 * @date Created <2011-01-25 18:40:12>
 */
#include <process/common/XmlConfigParser.h>
#include <process/common/CollectionManager.h>

#include <bundles/index/IndexSearchService.h>
#include <bundles/index/IndexTaskService.h>
#include <bundles/mining/MiningSearchService.h>
#include <bundles/recommend/RecommendTaskService.h>
#include <bundles/recommend/RecommendSearchService.h>
#include <aggregator-manager/GetRecommendWorker.h>
#include <aggregator-manager/UpdateRecommendWorker.h>

#include "CollectionHandler.h"
#include "DocumentsGetHandler.h"
#include "DocumentsSearchHandler.h"

#include <common/Keys.h>
#include <common/JobScheduler.h>

#include <boost/bind.hpp>

namespace sf1r
{

using driver::Keys;

using namespace izenelib::driver;
using namespace izenelib::osgi;

CollectionHandler::CollectionHandler(const string& collection)
        : collection_(collection)
        , indexSearchService_(0)
        , indexTaskService_(0)
        , productSearchService_(0)
        , productTaskService_(0)
        , miningSearchService_(0)
        , recommendTaskService_(0)
        , recommendSearchService_(0)
        , getRecommendWorker_(0)
        , updateRecommendWorker_(0)
{
}


CollectionHandler::~CollectionHandler()
{
}

void CollectionHandler::search(::izenelib::driver::Request& request, ::izenelib::driver::Response& response)
{
    DocumentsSearchHandler handler(request,response,*this);
    handler.search();
}

void CollectionHandler::get(::izenelib::driver::Request& request, ::izenelib::driver::Response& response)
{
    DocumentsGetHandler handler(request, response,*this);
    handler.get();
}

void CollectionHandler::similar_to(::izenelib::driver::Request& request, ::izenelib::driver::Response& response)
{
    DocumentsGetHandler handler(request, response,*this);
    handler.similar_to();
}

void CollectionHandler::duplicate_with(::izenelib::driver::Request& request, ::izenelib::driver::Response& response)
{
    DocumentsGetHandler handler(request, response,*this);
    handler.duplicate_with();
}

void CollectionHandler::similar_to_image(::izenelib::driver::Request& request, ::izenelib::driver::Response& response)
{
    DocumentsGetHandler handler(request, response,*this);
    handler.similar_to_image();
}

bool CollectionHandler::create(const ::izenelib::driver::Value& document)
{
    task_type task = boost::bind(&IndexTaskService::createDocument, indexTaskService_, document);
    JobScheduler::get()->addTask(task, collection_);
    return true;
}

bool CollectionHandler::update(const ::izenelib::driver::Value& document)
{
    task_type task = boost::bind(&IndexTaskService::updateDocument, indexTaskService_, document);
    JobScheduler::get()->addTask(task, collection_);
    return true;
}

bool CollectionHandler::destroy(const ::izenelib::driver::Value& document)
{
    task_type task = boost::bind(&IndexTaskService::destroyDocument, indexTaskService_, document);
    JobScheduler::get()->addTask(task, collection_);
    return true;
}

} // namespace sf1r
