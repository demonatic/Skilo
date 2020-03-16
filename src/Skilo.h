#ifndef SKILO_H
#define SKILO_H

#include <functional>
#include <memory>
#include "Rinx/Server/Server.h"
#include "Rinx/Protocol/HTTP/ProtocolHttp1.h"
#include "core/CollectionManager.h"
#include "utility/LogSink.h"

namespace Skilo {

using HttpRequest=Rinx::HttpRequest;
using HttpResponse=Rinx::HttpResponse;

class SkiloServer
{
    struct QueryContext{
        HttpRequest *req;
        HttpResponse *resp;
    };
    using SkiloReqHandler=std::function<void(const SegmentBuf &json,Status &status,QueryContext &context)>;

public:
    SkiloServer(const SkiloConfig &config,const bool debug=false);
    bool listen();

private:
    ///Route: POST /collections
    void skilo_create_collection(const SegmentBuf &json,Status &status,QueryContext &context);

    ///Route: POST /collections/<collection_name>/documents
    void skilo_add_document(const SegmentBuf &json,Status &status,QueryContext &context);

    ///Route: GET /collections/<collection_name>/documents
    void skilo_query_collection(const SegmentBuf &json,Status &status,QueryContext &context);

private:
    std::string extract_collection_name(const HttpRequest *req) const;

    void init_http_route(Rinx::RxProtocolHttp1Factory &http1);

    void handle_request(HttpRequest &req,HttpResponse &resp,const SkiloReqHandler handler);

private:
    const SkiloConfig &_config;
    std::unique_ptr<g3::LogWorker> _log_worker;

    Rinx::RxServer _server;
    CollectionManager _collection_manager;
};


} //namespace Skilo


#endif // SKILO_H
