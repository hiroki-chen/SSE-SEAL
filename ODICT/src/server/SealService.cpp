#include <server/SealService.h>

SealService::SealService()
{
}

SealService::~SealService()
{
}

grpc::Status SealService::setup(
    grpc::ServerContext* context,
    const SetupMessage* request,
    google::protobuf::Empty* e)
{
    std::cout << "The remote server is warming up." << std::endl;
    connector = std::make_unique<SEAL::Connector>(PSQL_CONNECTION_INFORMATION);
    std::cout << request->block_size() << std::endl;

    return grpc::Status::OK;
}