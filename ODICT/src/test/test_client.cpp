#include <client/ClientRunner.h>

int main(int argc, const char** argv)
{
    ClientRunner client(256, 256, sizeof(ODict::Node), 1024, INT_MAX, 2, 2, "123", PSQL_CONNECTION_INFORMATION, 8);
    //ClientRunner client("127.0.0.1:4567");
}