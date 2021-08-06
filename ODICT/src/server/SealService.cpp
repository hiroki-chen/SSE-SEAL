#include <client/Objects.h>
#include <oram/OramReadPathEviction.h>
#include <oram/RandomForOram.h>
#include <oram/ServerStorage.h>
#include <server/SealService.h>
#include <utils.h>

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
        connector = std::make_unique<SEAL::Connector>(PSQL_CONNECTION_INFORMATION);

        int bucket_size = request->bucket_size();
        int block_number = request->block_number();
        int block_size = request->block_size();
        this->oram_block_size = request->oram_block_size();
        this->block_size = block_size;
        this->bucket_size = bucket_size;
        this->block_number = block_number;

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

grpc::Status SealService::oram_access(
    grpc::ServerContext* context,
    const OramAccessMessage* message,
    OramAccessResponse* response)
{
    std::cout << "Oram Accessing!\n";
    /* operation = 1 means a write operation. */
    const bool operation = message->operation();
    const bool is_odict = message->is_odict();
    const int address = message->id();
    const std::string buffer = message->buffer();

    ODict::Node* test = transform<ODict::Node, char>((char*)buffer.c_str());
    std::cout << "test->id: " << test->id << std::endl;

    int size = 0;
    OramStruct* oram_struct = nullptr;
    int* new_data = nullptr;
    /* Read operation */
    if (is_odict == true && address != 0) {
        size = block_size;
        new_data = new int[size];
        memcpy(new_data, buffer.c_str(), size);
        oram_struct = oram_for_odict.get();

        if (operation == false) {
            new_data = oram_for_odict.get()->oram->access_direct(OramInterface::Operation::READ, new_data);
            response->set_buffer(std::string((char*)new_data, size));
        } else {
            std::cout << "write test->id: " << test->id << std::endl;
            new_data = oram_for_odict.get()->oram->access_direct(OramInterface::Operation::WRITE, new_data);
        }
    } else {
        size = oram_block_size;
        new_data = new int[size];
        memcpy(new_data, buffer.c_str(), size);
        int oram_id = message->oram_id();
        oram_struct = oram_blocks[oram_id].get();

        if (operation == false) {
            new_data = oram_for_odict.get()->oram->access(OramInterface::Operation::READ, address, new_data);
            response->set_buffer(std::string((char*)new_data, size));
        } else {
            new_data = oram_for_odict.get()->oram->access(OramInterface::Operation::WRITE, address, new_data);
        }
    }

    return grpc::Status::OK;
}

grpc::Status
SealService::oram_init(
    grpc::ServerContext* context,
    const OramInitMessage* message,
    google::protobuf::Empty* reponse)
{
    std::cout << "The server is initializing each oram block..." << std::endl;

    const unsigned int oram_id = message->oram_id();
    const unsigned int size = message->block_size();

    if (size != oram_block_size) {
        return grpc::Status(grpc::FAILED_PRECONDITION, "Wrong oram block size!");
    }

    std::unique_ptr<OramStruct> oram_struct = std::make_unique<OramStruct>();
    oram_struct.get()->random = RandomForOram::get_instance();
    oram_struct.get()->storage = new ServerStorage();
    oram_struct.get()->oram = new OramReadPathEviction(
        oram_struct.get()->storage, oram_struct.get()->random, bucket_size, block_number, oram_block_size);\
    Bucket::setMaxSize(bucket_size);

    oram_blocks.push_back(std::move(oram_struct));
    
    if (oram_blocks.size() != oram_id - 1) {
        throw std::runtime_error("Oram block number does not correspond to oram id!");
    }

    return grpc::Status::OK;
}