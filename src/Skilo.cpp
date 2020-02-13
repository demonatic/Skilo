#include "Skilo.h"

namespace Skilo {

using Rinx::HttpStatusCode;

#define BIND_SKILO_CALLBACK(__handler__) std::bind(&SkiloServer::handle_request,this,\
                                            std::placeholders::_1,std::placeholders::_2,\
                                                static_cast<SkiloReqHandler>(std::bind(&__handler__,this,\
                                                   std::placeholders::_1,std::placeholders::_2,std::placeholders::_3))))

SkiloServer::SkiloServer(const std::string &db_path):_collection_manager(db_path)
{
    nanolog::initialize(nanolog::GuaranteedLogger(),"/tmp/","rinx_log",20);
    Rinx::RxProtocolHttp1Factory http1;
    this->init_http_route(http1);
    _server.listen("127.0.0.1",80,http1);
}

void SkiloServer::skilo_create_collection(const SegmentBuf &json,Status &res,QueryContext &)
{
    CollectionMeta collection_meta(json);
    res=_collection_manager.create_collection(collection_meta);
}

void SkiloServer::skilo_add_document(const SegmentBuf &json,Status &res,QueryContext &context)
{
    Document new_doc(json);
    std::string collection_name=this->extract_collection_name(context.req);
    res=_collection_manager.add_document(collection_name,new_doc);
}

void SkiloServer::skilo_query_collection(const SegmentBuf &json,Status &res,QueryContext &context)
{
    std::string collection_name=this->extract_collection_name(context.req);
    Query query(collection_name,json);
    res=_collection_manager.search(query);
}

std::string SkiloServer::extract_collection_name(const HttpRequest *req) const
{
    std::string_view uri=req->uri();
}

void SkiloServer::init_http_route(Rinx::RxProtocolHttp1Factory &http1)
{
    http1.GET("/",BIND_SKILO_CALLBACK(SkiloServer::skilo_create_collection);
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
    resp.status_code(HttpStatusCode(status.code));
    resp.body()<<status.description;
}

} //namespace Skilo
