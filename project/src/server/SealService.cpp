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
}

SealService::~SealService()
{
}

grpc::Status
SealService::setup(
    grpc::ServerContext* context,
    const SetupMessage* message,
    google::protobuf::Empty* e)
{
    const std::string connection_info = message->connection_information();
    const std::string table_name = message->table_name();
    const int column_size = message->column_names_size();
    std::vector<std::string> column_names;
    for (int i = 0; i < column_size; i++) {
        column_names.push_back(message->column_names(i));
    }

    try {
        /* Create a new connector to the database. */
        connector = std::make_unique<SEAL::Connector>(connection_info);
        connector.get()->create_table_handler(table_name, column_names);
    } catch (const pqxx::sql_error& e) {
        const std::string error_message = "The connection information is not correct.";
        return grpc::Status(grpc::FAILED_PRECONDITION, error_message);
    }
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

grpc::Status
SealService::insert_handler(
    grpc::ServerContext* context,
    const InsertMessage* message,
    google::protobuf::Empty* e)
{
    const std::string table = message->table();
    const int value_size = message->values_size();

    std::vector<std::string> values;
    for (int i = 0; i < value_size; i++) {
        values.push_back(message->values(i));
    }

    try {
        connector.get()->insert_handler(table, values);
    } catch (const pqxx::sql_error& e) {
        return grpc::Status(grpc::FAILED_PRECONDITION, e.what());
    }

    return grpc::Status::OK;
}

grpc::Status
SealService::select_handler(
    grpc::ServerContext* context,
    const SelectMessage* message,
    SelectResult* reponse)
{
    const std::string table = message->table();
    const std::string where = message->document_id();
    const int column_size = message->columns_size();
    std::vector<std::string> columns;

    for (int i = 0; i < column_size; i++) {
        columns.push_back(message->columns(i));
    }

    try {
        connector.get()->select_handler(table, where, columns);
    } catch (pqxx::sql_error& e) {
        return grpc::Status(grpc::FAILED_PRECONDITION, e.what());
    }

    return grpc::Status::OK;
}

void SealService::print_oram_blocks()
{
    std::cout << "----------------- Oblivious Dictionary ----------------------" << std::endl;
    for (unsigned int i = 0; i < odict_storage.size(); i++) {
        odict_storage[i].printBlocks();
    }
    std::cout << "--------------------- Oblivious RAM -------------------------" << std::endl;
    for (unsigned int i = 0; i < oram_storage.size(); i++) {
        for (unsigned int j = 0; j < oram_storage[i].size(); j++) {
            oram_storage[i][j].printBlocks();
        }
    }
}