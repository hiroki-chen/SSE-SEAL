#include <oram/ServerStorage.h>
#include <plog/Log.h>
#include <utils.h>

#include <iostream>
#include <sstream>
#include <string>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <grpc++/client_context.h>
#include <grpc/grpc.h>

ServerStorage::ServerStorage(const unsigned int& oram_id, const bool& is_odict, Seal::Stub* stub_)
    : stub_(stub_)
    , oram_id(oram_id)
    , is_odict(is_odict)
{
    PLOG(plog::info) << "The server storage interface class is initialized.";
}

void ServerStorage::setCapacity(int totalNumOfBuckets)
{
    capacity = totalNumOfBuckets;

    grpc::ClientContext context;
    google::protobuf::Empty e;
    BucketSetMessage message;
    message.set_is_odict(is_odict);
    message.set_oram_id(oram_id);
    message.set_number_of_buckets(totalNumOfBuckets);

    grpc::Status status = stub_->set_capacity(&context, message, &e);

    if (!status.ok()) {
        throw std::runtime_error(status.error_message());
    }
}

Bucket ServerStorage::ReadBucket(int position)
{
    if (position >= this->capacity || position < 0) {
        throw std::runtime_error(
            "You are trying to access Bucket " + to_string(position) + ", but this Server contains only " + to_string(this->capacity) + " buckets.");
    }

    grpc::ClientContext context;
    BucketReadResponse response;
    BucketReadMessage message;
    message.set_is_odict(is_odict);
    message.set_oram_id(oram_id);
    message.set_position(position);

    grpc::Status status = stub_->read_bucket(&context, message, &response);
    if (!status.ok()) {
        throw std::runtime_error(status.error_message());
    }

    return deserialize<Bucket>(response.buffer());
}

void ServerStorage::WriteBucket(int position, const Bucket& bucket_to_write)
{   
    if (position >= this->capacity || position < 0) {
        throw std::runtime_error(
            "You are trying to access Bucket " + to_string(position) + ", but this Server contains only " + to_string(this->capacity) + " buckets.");
    }

    grpc::ClientContext context;
    BucketWriteMessage message;
    google::protobuf::Empty e;
    message.set_is_odict(is_odict);
    message.set_oram_id(oram_id);
    message.set_position(position);
    message.set_buffer(serialize<Bucket>(bucket_to_write));

    grpc::Status status = stub_->write_bucket(&context, message, &e);
    if (!status.ok()) {
        throw std::runtime_error(status.error_message());
    }
}