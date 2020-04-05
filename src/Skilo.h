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
    using SkiloReqHandler=std::function<void(QueryContext &context,std::string &response)>;

public:
    SkiloServer(const SkiloConfig &config,const bool debug=false);
    bool listen();

private:
    ///Route: POST /collections
    void skilo_create_collection(QueryContext &context,std::string &response);

    ///Route: POST /collections/<collection_name>/documents
    void skilo_add_document(QueryContext &context,std::string &response);

    ///Route: GET /collections/<collection_name>/documents
    void skilo_query_collection(QueryContext &context,std::string &response);

    ///Route: GET /collections/<collection_name>/auto_suggestion?q=<query_prefix>
    void skilo_auto_suggest(QueryContext &context,std::string &response);
    
private:
    std::string extract_collection_name(std::string_view uri) const;

    void init_http_route(Rinx::RxProtocolHttp1Factory &http1);

    void handle_request(HttpRequest &req,HttpResponse &resp,const SkiloReqHandler handler);

private:
    const SkiloConfig &_config;
    std::unique_ptr<g3::LogWorker> _log_worker;
    std::unique_ptr<g3::FileSinkHandle> _log_file_handle;

    Rinx::RxServer _server;
    CollectionManager _collection_manager;
};

enum class RetCode{
    OK=200,
    CREATED=201,
    NOT_CONTENT=204,
    BAD_REQUEST=400,
    FORBIDDEN=403,
    NOT_FOUND=404,
    METHOD_NOT_ALLOWED=405,
    CONFLICT=409,
    INTERNAL_SERVER_ERROR=500,
    UNDEFINED=0
};
struct Status{
    RetCode code=RetCode::OK;
    std::string description;
};


} //namespace Skilo


#endif // SKILO_H
