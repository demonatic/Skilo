#include "Skilo.h"
#include "Rinx/Protocol/HTTP/HttpResponse.h"
#include "Rinx/Protocol/HTTP/ProtocolHttp1.h"
#include "g3log/loglevels.hpp"
#include "g3log/g3log.hpp"

namespace Skilo {

using std::string_view;
using Rinx::HttpStatusCode;
using Rinx::HttpRequest;
using Rinx::HttpResponse;
using Rinx::RxProtocolHttp1Factory;
using MakeAsync=Rinx::MakeAsync;

#define BIND_SKILO_CALLBACK(__handler__) std::bind(&SkiloServer::handle_request,this,\
                                            std::placeholders::_1,std::placeholders::_2,\
                                                static_cast<SkiloReqHandler>(std::bind(&__handler__,this,\
                                                   std::placeholders::_1,std::placeholders::_2))))

SkiloServer::SkiloServer(const SkiloConfig &config,const bool debug)
    :_debug(debug),_config(config)
{
    nanolog::initialize(nanolog::GuaranteedLogger(),config.get_log_dir(),"log",20);
    _log_worker = g3::LogWorker::createLogWorker();
    if(debug){
        _log_worker->addSink(std::make_unique<CustomLogSink>(),&CustomLogSink::ReceiveLogMessage);
    }
    else{
        g3::log_levels::setHighest(INFO);
        _log_file_handle =_log_worker->addDefaultLogger("skilo_log", config.get_log_dir());
        _log_worker->addSink(std::make_unique<CustomLogSink>(),&CustomLogSink::ReceiveLogMessage);
    }
    g3::initializeLogging(_log_worker.get());
    _collection_manager=std::make_unique<CollectionManager>(config);
}

bool SkiloServer::listen()
{
    LOG(INFO)<<"Initiating collections...";
    _collection_manager->init_collections();
    RxProtocolHttp1Factory http1;
    LOG(INFO)<<"Initiating server Http route";
    this->init_http_route(http1);

    const string &addr=_config.get_listen_address();
    _server.on_signal(SIGINT,[this](int){
        this->stop();
    });

    uint16_t port=_config.get_listen_port();
    LOG(INFO)<<"Start listening on address "<<addr<<":"<<port;
    return _server.listen(addr,port,http1);
}

void SkiloServer::stop()
{
    LOG(INFO)<<"Stopping Skilo server...";
    _server.stop();
    LOG(INFO)<<"Server has stopped";
}

void SkiloServer::skilo_collection_summary(SkiloServer::QueryContext &context, string &response)
{
    string collection_name=this->extract_collection_name(context.req->uri());
    response=_collection_manager->collection_summary(collection_name);
}

void SkiloServer::skilo_get_document(SkiloServer::QueryContext &context, string &response)
{
    string collection_name=extract_collection_name(context.req->uri());
    uint32_t doc_id=extract_document_id(context.req->uri());
    response=_collection_manager->get_document(collection_name,doc_id);
}

void SkiloServer::skilo_get_document_in_seq(SkiloServer::QueryContext &context, string &response)
{
    bool in_seq=true;
    const string &uri=context.req->uri();
    string collection_name=extract_collection_name(uri);

    uint32_t seq_id=stoul(uri.substr(uri.find_last_of('/')+1)); //since regex matches, wouldn't throw
    response=_collection_manager->get_document(collection_name,seq_id,in_seq);
}

void SkiloServer::skilo_overall_summary([[maybe_unused]]SkiloServer::QueryContext &context,string &response)
{
    response=_collection_manager->overall_summary();
}

void SkiloServer::skilo_create_collection(QueryContext &context,string &response)
{
    CollectionMeta collection_meta(context.req->body().get_data());
    response=_collection_manager->create_collection(collection_meta);
}

void SkiloServer::skilo_drop_collection(SkiloServer::QueryContext &context,string &response)
{
    string collection_name=this->extract_collection_name(context.req->uri());
    response=_collection_manager->drop_collection(collection_name);
}

void SkiloServer::skilo_add_document(QueryContext &context,string &response)
{
    DocumentBase base(context.req->body().get_data());
    string collection_name=this->extract_collection_name(context.req->uri());
    if(base.contain_key("docs")){
        DocumentBatch doc_batch(base);
        response=_collection_manager->add_document_batch(collection_name,doc_batch);
    }
    else{
        Document new_doc(base);
        response=_collection_manager->add_document(collection_name,new_doc);
    }
}

void SkiloServer::skilo_remove_document(SkiloServer::QueryContext &context, string &response)
{
    string collection_name=extract_collection_name(context.req->uri());
    uint32_t doc_id=extract_document_id(context.req->uri());
    response=_collection_manager->remove_document(collection_name,doc_id);
}

void SkiloServer::skilo_query_collection(QueryContext &context,string &response)
{
    LOG(DEBUG)<<"@skilo_query_collection";
    string collection_name=extract_collection_name(context.req->uri());
    if(context.req->body().empty()){
        throw InvalidFormatException("missing query body");
    }
    Query query(collection_name,context.req->body().get_data());
    response=_collection_manager->search(query);
}

void SkiloServer::skilo_auto_suggest(SkiloServer::QueryContext &context,string &response)
{
    LOG(DEBUG)<<"@skilo_auto_suggest";
    string decoded_uri=context.req->decode_uri();
    string collection_name=this->extract_collection_name(decoded_uri);

    size_t query_index=decoded_uri.find_first_of('=');
    string query_prefix=decoded_uri.substr(query_index+1);
    response=_collection_manager->auto_suggest(collection_name,query_prefix);
}

string SkiloServer::extract_collection_name(string_view uri) const
{
    uri.remove_prefix(1);
    size_t name_begin=uri.find_first_of('/')+1; //since uri match regex, this '/' must exists
    size_t name_end=uri.find_first_of('/',name_begin);
    if(name_end==string_view::npos){
        name_end=uri.length();
    }
    if(name_begin==name_end){
        throw InvalidFormatException("bad uri: collection name is empty");
    }
    return string(uri.substr(name_begin,name_end-name_begin));
}

uint32_t SkiloServer::extract_document_id(string_view uri) const
{
    auto doc_id_str=uri.substr(uri.find_last_of('/')+1);
    return std::stoul(string(doc_id_str)); //since regex matches, wouldn't throw
}

void SkiloServer::init_http_route(Rinx::RxProtocolHttp1Factory &http1)
{
    http1.GET(R"(^\/collections$)",BIND_SKILO_CALLBACK(SkiloServer::skilo_overall_summary);

    http1.GET(R"(^\/collections\/[a-zA-Z_\$][a-zA-Z\d_]*$)",BIND_SKILO_CALLBACK(SkiloServer::skilo_collection_summary);
    http1.POST(R"(^\/collections$)",MakeAsync(BIND_SKILO_CALLBACK(SkiloServer::skilo_create_collection));
    http1.DELETE(R"(^\/collections\/[a-zA-Z_\$][a-zA-Z\d_]*$)",MakeAsync(BIND_SKILO_CALLBACK(SkiloServer::skilo_drop_collection));

    http1.POST(R"(^\/collections\/[a-zA-Z_\$][a-zA-Z\d_]*$)",MakeAsync(BIND_SKILO_CALLBACK(SkiloServer::skilo_add_document));
    http1.GET(R"(^\/collections\/[a-zA-Z_\$][a-zA-Z\d_]*\/[0-9]+$)",BIND_SKILO_CALLBACK(SkiloServer::skilo_get_document);
    http1.GET(R"(^\/collections\/[a-zA-Z_\$][a-zA-Z\d_]*\/seq_id\/[0-9]+$)",BIND_SKILO_CALLBACK(SkiloServer::skilo_get_document_in_seq);
    http1.DELETE(R"(^\/collections\/[a-zA-Z_\$][a-zA-Z\d_]*\/[0-9]+$)",MakeAsync(BIND_SKILO_CALLBACK(SkiloServer::skilo_remove_document));

    http1.GET(R"(^\/collections\/[a-zA-Z_\$][a-zA-Z\d_]*\/documents$)",BIND_SKILO_CALLBACK(SkiloServer::skilo_query_collection);
    http1.POST(R"(^\/collections\/[a-zA-Z_\$][a-zA-Z\d_]*\/documents$)",BIND_SKILO_CALLBACK(SkiloServer::skilo_query_collection);  //in case some clients doesn't support GET with body

    http1.GET(R"(^\/collections\/[a-zA-Z_\$][a-zA-Z\d_]*\/auto_suggestion\?q=[\w\W]*$)",BIND_SKILO_CALLBACK(SkiloServer::skilo_auto_suggest);

    http1.default_handler([](HttpRequest &req,HttpResponse &resp){
          resp.send_status(HttpStatusCode::FORBIDDEN);
          req.close_connection();
    });

    if(_debug){
        http1.head_filter(R"([\s\S]*)",[](HttpRequest &,Rinx::HttpResponseHead &head,Rinx::Next next){
            head.header_fields.add("Access-Control-Allow-Origin","*");
            next();
        });
    }
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
    string body_length=std::to_string(status.description.length());
    resp.status_code(HttpStatusCode(status.code)).headers("Content-Length",std::move(body_length));
    resp.body()<<status.description;
}

} //namespace Skilo
