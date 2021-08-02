#include <Client.h>
#include <Connector.h>
#include <utils.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <random>

/**
 * Bug: We cannot evict the cache when it is full because evicting a node means to not only update pos_tag for it,
 *      but also update its parent's pos_tags.
 */
int main(int argc, const char** args)
{
    // Every time the client will sample a new key from the password.
    SEAL::Client* client = new SEAL::Client(256, 256, sizeof(ODict::Node), 1024, INT_MAX, 5, 4, "123456789", PSQL_CONNECTION_INFORMATION);

    client->test_adj("./input/test.txt");
    
    std::cout << "sql: " << std::endl;
    std::string s;
    getline(std::cin, s);
    client->test_sql(s);
    return 0;
}