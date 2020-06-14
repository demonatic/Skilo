#include "Skilo.h"

using namespace std;
using namespace Skilo;

int main()
{
    SkiloConfig config;
    SkiloServer server(config,false);
    server.listen();
    return 0;
}
