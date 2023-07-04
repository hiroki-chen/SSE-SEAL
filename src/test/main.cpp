#include <Connector.h>
#include <client/Client.h>
#include <utils.h>

#include <client/ClientRunner.h>
#include <oram/Bucket.h>
#include <oram/ServerStorage.h>
#include <proto/seal.pb.h>

#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc/grpc.h>

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
    //std::unique_ptr<Seal::Stub> stub_ = Seal::NewStub(std::shared_ptr<grpc::Channel>(grpc::CreateChannel("127.0.0.1:4567", grpc::InsecureChannelCredentials())));
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

    try {
        ClientRunner client(256, 256, sizeof(ODict::Node), 1024, INT_MAX, 2, 2, "123456789", PSQL_CONNECTION_INFORMATION, sizeof(unsigned int), 6, "test", "localhost:4567");
        client.test_adj("input/test.csv");
        // Currently the keyword is defined as <file_path>_<column_name>_<value>...
        std::vector<SEAL::Document> ans = client.search_range("input/test.csvkwd1", "3", "5");
        for (auto item : ans) {
            item.print();
        }
        //Range::Node* root = client.get_t1_root("input/test.csvkwd1");
        //Range::Node* single = single_range_cover(root, 3, 5);

    } catch (const std::runtime_error& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}