#include <iostream>
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include "Skilo.h"
using namespace std;
using namespace g3;
using namespace Skilo;

int main()
{
    SkiloConfig config;
    SkiloServer server(config,true);
    server.listen();
    return 0;
}
