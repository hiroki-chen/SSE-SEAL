/*
 Copyright (c) 2021 Haobin Chen

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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

void ServerStorage::setCapacity(const int& totalNumOfBuckets)
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

Bucket ServerStorage::ReadBucket(const int& position)
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

void ServerStorage::WriteBucket(const int& position, const Bucket& bucket_to_write)
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