#ifndef CLIENT_RUNNER_H
#define CLIENT_RUNNER_H

#include <client/Client.h>
#include <proto/seal.grpc.pb.h>
#include <proto/seal.pb.h>

#include <string>

class ClientRunner : public Seal::Service {
private:
    std::unique_ptr<Seal::Stub> stub_;

    std::unique_ptr<SEAL::Client> client;

    using Seal::Service::setup;

    void setup(const int& bucket_size, const int& block_number, const int& block_size, const int& oram_block_size);

public:
    ClientRunner(const std::string& address);

    /**
     * @brief JUST A WRAPPER FOR SEAL-CLIENT.
     */
    ClientRunner(const int& bucket_size, const int& block_number,
        const int& block_size, const int& odict_size,
        const size_t& max_size, const unsigned int& alpha,
        const unsigned int& x, std::string_view password,
        std::string_view connection_info, const int& oram_block_size,
        const char* address = "127.0.0.1:4567");

    ~ClientRunner();

    void test_add_node(const unsigned int& number);

    void test_adj(std::string_view file_path);

    using Seal::Service::search;
    
    std::string search(std::string_view keyword);
};

#endif