#include <client/ClientRunner.h>

#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include <grpc/grpc.h>

ClientRunner::ClientRunner(const std::string& address)
    : stub_(Seal::NewStub(grpc::CreateChannel(address, grpc::InsecureChannelCredentials())))
{
    init(address);
}

ClientRunner::ClientRunner(const int& bucket_size, const int& block_number,
    const int& block_size, const int& odict_size,
    const size_t& max_size, const unsigned int& alpha,
    const unsigned int& x, std::string_view password,
    std::string_view connection_info, const char *address)
    : client(std::make_unique<SEAL::Client>(bucket_size, block_number, block_size,
        odict_size, max_size, alpha, x,
        password, connection_info))
{
    init(address);
}

ClientRunner::~ClientRunner()
{
}