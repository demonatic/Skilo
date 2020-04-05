#include "Skilo.h"
#include "Rinx/Protocol/HTTP/HttpResponse.h"
#include "Rinx/Protocol/HTTP/ProtocolHttp1.h"

namespace Skilo {

using Rinx::HttpStatusCode;
using Rinx::HttpRequest;
using Rinx::HttpResponse;
using Rinx::RxProtocolHttp1Factory;
using MakeAsync=Rinx::MakeAsync;

#define BIND_SKILO_CALLBACK(__handler__) std::bind(&SkiloServer::handle_request,this,\
                                            std::placeholders::_1,std::placeholders::_2,\
                                                static_cast<SkiloReqHandler>(std::bind(&__handler__,this,\
                                                   std::placeholders::_1,std::placeholders::_2))))

SkiloServer::SkiloServer(const SkiloConfig &config,const bool debug):_config(config),_collection_manager(config)
{
    nanolog::initialize(nanolog::GuaranteedLogger(),config.get_log_dir(),"log",20);
    _log_worker = g3::LogWorker::createLogWorker();
    if(debug){
        _log_worker->addSink(std::make_unique<CustomLogSink>(),&CustomLogSink::ReceiveLogMessage);
    }
    else{
        auto handle = _log_worker->addDefaultLogger("skilo_log", config.get_log_dir());
    }
    g3::initializeLogging(_log_worker.get());
}

bool SkiloServer::listen()
{
    RxProtocolHttp1Factory http1;
    this->init_http_route(http1);
    return _server.listen(_config.get_listen_address(),_config.get_listen_port(),http1);
}

void SkiloServer::skilo_create_collection(QueryContext &context,std::string &response)
{
    CollectionMeta collection_meta(context.req->body().get_data());
    response=_collection_manager.create_collection(collection_meta);
}

void SkiloServer::skilo_add_document(QueryContext &context,std::string &response)
{
    DocumentBase base(context.req->body().get_data());
    std::string collection_name=this->extract_collection_name(context.req->uri());
    if(base.contain_key("docs")){
        DocumentBatch doc_batch(base);
        response=_collection_manager.add_document_batch(collection_name,doc_batch);
    }
    else{
        Document new_doc(base);
        response=_collection_manager.add_document(collection_name,new_doc);
    }
}

void SkiloServer::skilo_query_collection(QueryContext &context,std::string &response)
{
    std::string collection_name=extract_collection_name(context.req->uri());
    if(context.req->body().empty()){
        throw InvalidFormatException("missing query body");
    }
    Query query(collection_name,context.req->body().get_data());
    response=_collection_manager.search(query);
}

void SkiloServer::skilo_auto_suggest(SkiloServer::QueryContext &context,std::string &response)
{
    std::string decoded_uri=context.req->decode_uri();
    std::string collection_name=this->extract_collection_name(decoded_uri);

    size_t query_index=decoded_uri.find_first_of('?');
    std::string query_prefix=decoded_uri.substr(query_index+1);
    response=_collection_manager.auto_suggest(collection_name,query_prefix);
}

std::string SkiloServer::extract_collection_name(std::string_view uri) const
{
    uri.remove_prefix(1);
    size_t name_start=1+uri.find_first_of('/');
    size_t name_end=uri.find_last_of('/');
    if(name_start==uri.npos||name_end==uri.npos){
        throw InvalidFormatException("missing \"collection name\" in uri");
    }
    return std::string(uri.substr(name_start,name_end-name_start));
}

void SkiloServer::init_http_route(Rinx::RxProtocolHttp1Factory &http1)
{
    http1.POST(R"(^\/collections$)",MakeAsync(BIND_SKILO_CALLBACK(SkiloServer::skilo_create_collection));
    http1.POST(R"(^\/collections\/[a-zA-Z_\$][a-zA-Z\d_]*\/documents$)",MakeAsync(BIND_SKILO_CALLBACK(SkiloServer::skilo_add_document));
    http1.GET(R"(^\/collections\/[a-zA-Z_\$][a-zA-Z\d_]*\/documents$)",BIND_SKILO_CALLBACK(SkiloServer::skilo_query_collection);
    http1.GET(R"(^\/collections\/[a-zA-Z_\$][a-zA-Z\d_]*\/auto_suggestion?q=[\w\W]*$)",BIND_SKILO_CALLBACK(SkiloServer::skilo_auto_suggest);
}

void SkiloServer::handle_request(HttpRequest &req, HttpResponse &resp, const SkiloReqHandler handler)
{
    Status status;
    try {
          QueryContext ctx;
          ctx.req=&req; ctx.resp=&resp;
          handler(ctx,status.description);
    }  catch (const InvalidFormatException &err) {
        status.code=RetCode::BAD_REQUEST;
        status.description=err.what();
    }
    catch(const InternalServerException &err){
        status.code=RetCode::INTERNAL_SERVER_ERROR;
        status.description=err.what();

    }catch(const NotFoundException &err){
        status.code=RetCode::NOT_FOUND;
        status.description=err.what();

    }catch(const ConflictException &err){
        status.code=RetCode::CONFLICT;
        status.description=err.what();

    }catch(const std::exception &err){
        status.code=RetCode::INTERNAL_SERVER_ERROR;
        status.description=err.what();
    }
    std::string body_length=std::to_string(status.description.length());
    resp.status_code(HttpStatusCode(status.code)).headers("Content-Length",std::move(body_length));
    resp.body()<<status.description;
}

struct CustomSink {
  enum FG_Color {YELLOW = 33, RED = 31, GREEN=32, WHITE = 97};

  FG_Color GetColor(const LEVELS level) const {
     if (level.value == WARNING.value) { return YELLOW; }
     if (level.value == DEBUG.value) { return GREEN; }
     if (g3::internal::wasFatal(level)) { return RED; }

     return WHITE;
  }

  void ReceiveLogMessage(g3::LogMessageMover logEntry) {
     auto level = logEntry.get()._level;
     auto color = GetColor(level);

     std::cout << "\033[" << color << "m"
       << logEntry.get().toString() << "\033[m" << std::endl;
  }
};

} //namespace Skilo
