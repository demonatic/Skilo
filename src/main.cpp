#include "Skilo.h"

using namespace std;
using namespace Skilo;

int main()
{
    SkiloConfig config;
    SkiloServer server(config,true);
    server.listen();
    return 0;
}
