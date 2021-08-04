#include <oram/OramReadPathEviction.h>
#include <oram/RandomForOram.h>
#include <oram/ServerStorage.h>
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
    std::cout << "The remote service is warming up." << std::endl;
    try {
        // connector = std::make_unique<SEAL::Connector>(PSQL_CONNECTION_INFORMATION);

        int bucket_size = request->bucket_size();
        int block_number = request->block_number();
        int block_size = request->block_size();
        this->block_size = request->block_size();
        this->oram_block_size = request->oram_block_size();

        // Initialize the oblivious data structure
        oram_for_odict = std::make_unique<OramStruct>();
        oram_for_odict.get()->random = new RandomForOram();
        oram_for_odict.get()->storage = new ServerStorage();
        oram_for_odict.get()->oram = new OramReadPathEviction(
            oram_for_odict.get()->storage, oram_for_odict.get()->random, bucket_size, block_number, block_size);
        std::cout << "Initialized Oblivious dictionary!" << std::endl;

    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return grpc::Status(grpc::FAILED_PRECONDITION, e.what());
    }

    return grpc::Status::OK;
}