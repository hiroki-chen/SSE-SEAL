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

grpc::Status
SealService::setup(
    grpc::ServerContext* context,
    const SetupMessage* request,
    google::protobuf::Empty* e)
{
    return grpc::Status::OK;
}

grpc::Status
SealService::set_capacity(
    grpc::ServerContext* context,
    const BucketSetMessage* message,
    google::protobuf::Empty* e)
{
    const unsigned int total_number_of_buckets = message->number_of_buckets();
    const bool is_odict = message->is_odict();

    if (is_odict == true) {
        odict_storage.assign(total_number_of_buckets, Bucket());
    } else {
        const unsigned int oram_id = message->oram_id();
        std::vector<Bucket> new_storage(total_number_of_buckets, Bucket());
        oram_storage.push_back(new_storage);
        if (oram_storage.size() - 1 != oram_id) {
            return grpc::Status(grpc::FAILED_PRECONDITION, "The ORAM ID is not correct!");
        }
    }

    return grpc::Status::OK;
}

grpc::Status
SealService::read_bucket(
    grpc::ServerContext* context,
    const BucketReadMessage* message,
    BucketReadResponse* response)
{
    std::cout << "The server is reading the bucket!" << std::endl;
    const unsigned int position = message->position();
    const unsigned int oram_id = message->oram_id();
    const bool is_odict = message->is_odict();

    Bucket* bucket = nullptr;

    if (is_odict == true) {
        bucket = &(odict_storage.at(position));
    } else {
        bucket = &(oram_storage[oram_id].at(position));
    }

    response->set_buffer(serialize<Bucket>(*bucket));
    return grpc::Status::OK;
}

grpc::Status
SealService::write_bucket(
    grpc::ServerContext* context,
    const BucketWriteMessage* message,
    google::protobuf::Empty* e)
{
    std::cout << "The server is writing the bucket!" << std::endl;
    const unsigned int position = message->position();
    const unsigned int oram_id = message->oram_id();
    const bool is_odict = message->is_odict();
    const std::string buffer = message->buffer();

    try {
        if (is_odict == true) {
            odict_storage[position] = deserialize<Bucket>(buffer);
        } else {
            oram_storage[oram_id][position] = deserialize<Bucket>(buffer);
        }
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    }

    return grpc::Status::OK;
}