#include <client/ClientRunner.h>

#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include <grpc/grpc.h>

ClientRunner::ClientRunner(const std::string& address)
    : stub_(Seal::NewStub(std::shared_ptr<grpc::Channel>(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()))))
{
}

ClientRunner::ClientRunner(const int& bucket_size, const int& block_number,
    const int& block_size, const int& odict_size,
    const size_t& max_size, const unsigned int& alpha,
    const unsigned int& x, std::string_view password,
    std::string_view connection_info, const int& oram_block_size,
    const char* address)
    : //client(std::make_unique<SEAL::Client>(bucket_size, block_number, block_size,
      //  odict_size, max_size, alpha, x,
      //  password, connection_info))
    stub_(Seal::NewStub(std::shared_ptr<grpc::Channel>(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()))))
{
    setup(bucket_size, block_number, block_size, oram_block_size);
}

ClientRunner::~ClientRunner()
{
}

void ClientRunner::setup(const int& bucket_size, const int& block_number, const int& block_size, const int& oram_block_size)
{
    grpc::ClientContext context;
    SetupMessage message;
    message.set_block_number(block_number);
    message.set_block_size(block_size);
    message.set_bucket_size(bucket_size);
    message.set_oram_block_size(oram_block_size);
    google::protobuf::Empty e;

    grpc::Status status = stub_->setup(&context, message, &e);

    if (!status.ok()) {
        throw std::runtime_error("Cannot setup the server!");
    }
}