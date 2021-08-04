#include <Client.h>
#include <Connector.h>
#include <utils.h>

#include <server/SealServerRunner.h>
#include <proto/seal.pb.h>

#include <grpc/grpc.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

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
    /*
    try {
        SEAL::Client* client = new SEAL::Client(256, 256, sizeof(ODict::Node), 1024, INT_MAX, 2, 2, "123456789", PSQL_CONNECTION_INFORMATION);
        client->test_adj("./input/test.txt");

        std::cout << "sql: " << std::endl;
        std::string s;
        getline(std::cin, s);
        client->test_sql(s);
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << '\n';
    }
    //auto ans = get_bits((unsigned int)0b0, 3);
    //std::cout << ans.first << "," << ans.second << std::endl;
    */

    SEAL::SealServerRunner* runner = new SEAL::SealServerRunner("127.0.0.1:4567");
    return 0;
}