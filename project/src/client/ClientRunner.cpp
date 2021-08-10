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

#include <client/ClientRunner.h>
#include <oram/RandomForOram.h>

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
    : stub_(Seal::NewStub(std::shared_ptr<grpc::Channel>(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()))))
    , client(std::make_unique<SEAL::Client>(bucket_size, block_number, block_size,
          odict_size, max_size, alpha, x,
          password, connection_info, stub_.get()))
{
    std::cout << "In ClientRunner!" << std::endl;

    client.get()->set_stub(stub_);
    client.get()->init_dummy_data();
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

    grpc::Status status = stub_.get()->setup(&context, message, &e);

    if (!status.ok()) {
        throw std::runtime_error(status.error_message());
    }
}

void ClientRunner::test_add_node(const unsigned int&number)
{
    client.get()->add_node(number);
}

void ClientRunner::test_adj(std::string_view file_path)
{
    client.get()->test_adj(file_path);
}

std::vector<std::string> ClientRunner::search(std::string_view keyword)
{
    return client.get()->search(keyword);
}