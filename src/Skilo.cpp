#include "Skilo.h"

namespace Skilo {

using Rinx::HttpStatusCode;

#define BIND_SKILO_CALLBACK(__handler__) std::bind(&SkiloServer::handle_request,this,\
                                            std::placeholders::_1,std::placeholders::_2,\
                                                static_cast<SkiloReqHandler>(std::bind(&__handler__,this,\
                                                   std::placeholders::_1,std::placeholders::_2,std::placeholders::_3))))

SkiloServer::SkiloServer(const SkiloConfig &config):_config(config),_collection_manager(config)
{
    nanolog::initialize(nanolog::GuaranteedLogger(),config.get_log_dir(),"log",20);
}

bool SkiloServer::listen()
{
    Rinx::RxProtocolHttp1Factory http1;
    this->init_http_route(http1);
    return _server.listen(_config.get_listen_address(),_config.get_listen_port(),http1);
}

void SkiloServer::skilo_create_collection(const SegmentBuf &json,Status &status,QueryContext &)
{
    CollectionMeta collection_meta(json);
    status=_collection_manager.create_collection(collection_meta);
}

void SkiloServer::skilo_add_document(const SegmentBuf &json,Status &status,QueryContext &context)
{
    DocumentBase base(json);
    std::string collection_name=this->extract_collection_name(context.req);
    if(base.contain_key("docs")){
        DocumentBatch doc_batch(base);
        status=_collection_manager.add_document_batch(collection_name,doc_batch);
    }
    else{
        Document new_doc(base);
        status=_collection_manager.add_document(collection_name,new_doc);
    }
}

void SkiloServer::skilo_query_collection(const SegmentBuf &json,Status &status,QueryContext &context)
{
    std::string collection_name=this->extract_collection_name(context.req);
    Query query(collection_name,json);
    status=_collection_manager.search(query);
}

std::string SkiloServer::extract_collection_name(const HttpRequest *req) const
{
    std::string_view uri=req->uri();
    uri.remove_prefix(1);
    size_t name_start=1+uri.find_first_of('/');
    size_t name_end=uri.find_last_of('/');
    assert(name_start!=uri.npos);
    assert(name_end!=uri.npos);
    return std::string(uri.substr(name_start,name_end-name_start));
}

void SkiloServer::init_http_route(Rinx::RxProtocolHttp1Factory &http1)
{
    http1.POST("^\\/collections$",BIND_SKILO_CALLBACK(SkiloServer::skilo_create_collection);
    http1.POST("^\\/collections\\/[a-zA-Z_\\$][a-zA-Z\\d_]*\\/documents$",BIND_SKILO_CALLBACK(SkiloServer::skilo_add_document);
    http1.GET("^\\/collections\\/[a-zA-Z_\\$][a-zA-Z\\d_]*\\/documents$",BIND_SKILO_CALLBACK(SkiloServer::skilo_query_collection);
}

void SkiloServer::handle_request(HttpRequest &req, HttpResponse &resp, const SkiloReqHandler handler)
{
    Status status;
    try {
          SegmentBuf json_buf=req.body().get_data();
          QueryContext ctx;
          ctx.req=&req; ctx.resp=&resp;
          handler(json_buf,status,ctx);
    }  catch (const InvalidFormatException &invalid_json_err) {
        status.code=RetCode::BAD_REQUEST;
        status.description=invalid_json_err.what();
    }
    std::string body_length=std::to_string(status.description.length());
    resp.status_code(HttpStatusCode(status.code)).headers("Content-Length",std::move(body_length));
    resp.body()<<status.description;
}

} //namespace Skilo
