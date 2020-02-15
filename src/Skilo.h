#ifndef SKILO_H
#define SKILO_H

#include <functional>
#include "Server/Server.h"
#include "Protocol/HTTP/ProtocolHttp1.h"
#include "core/CollectionManager.h"

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
    SkiloServer(const SkiloConfig &config);
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

    Rinx::RxServer _server;

    CollectionManager _collection_manager;
};

} //namespace Skilo


#endif // SKILO_H
