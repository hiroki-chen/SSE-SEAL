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

#include <client/Objects.h>
#include <oram/OramReadPathEviction.h>
#include <oram/RandomForOram.h>
#include <oram/ServerStorage.h>
#include <plog/Log.h>
#include <server/SealService.h>
#include <utils.h>

SealService::SealService()
{
    PLOG_(1, plog::info) << "Server starts.";
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
    std::cout << "The server is setting the capacity of oblivioud ram!" << std::endl;
    const unsigned int total_number_of_buckets = message->number_of_buckets();
    const bool is_odict = message->is_odict();

    if (is_odict == true) {
        odict_storage.assign(total_number_of_buckets, Bucket());
    } else {
        const unsigned int oram_id = message->oram_id();
        if (oram_id == oram_storage.size()) {
            std::vector<Bucket> new_storage(total_number_of_buckets, Bucket());
            oram_storage.push_back(new_storage);
        } else if (oram_id < oram_storage.size()) {
            oram_storage[oram_id].assign(total_number_of_buckets, Bucket());
        } else {
            const std::string error_message = "The ORAM ID is not correct because"
                                              "it exceeds the maximum allowed bound!";
            return grpc::Status(grpc::FAILED_PRECONDITION, error_message);
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
    PLOG_(1, plog::info) << "The server is reading the bucket!";

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
    PLOG_(1, plog::info) << "The server is writing the bucket!";

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
    } catch (const std::exception& e) {
        PLOG_(1, plog::error) << e.what();
        std::cout << e.what() << std::endl;
    }

    return grpc::Status::OK;
}