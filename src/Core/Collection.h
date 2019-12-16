#ifndef COLLECTION_H
#define COLLECTION_H
#include <string>

class Collection
{
public:
    Collection();
    bool add_document(const std::string &json_str); //TODO 这一层用json object还是json str作为参数？
};

#endif // COLLECTION_H
