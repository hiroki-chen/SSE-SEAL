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

#ifndef CLIENT_RUNNER_H
#define CLIENT_RUNNER_H

#include <client/Client.h>
#include <proto/seal.grpc.pb.h>
#include <proto/seal.pb.h>

#include <string>

class ClientRunner : public Seal::Service {
private:
    std::unique_ptr<Seal::Stub> stub_;

    std::unique_ptr<SEAL::Client> client;

    using Seal::Service::setup;

    void setup(std::string_view connection_inforamtion, std::string_view table_name, const size_t& column_number);

public:
    ClientRunner(const std::string& address);

    /**
     * @brief JUST A WRAPPER FOR SEAL-CLIENT.
     */
    ClientRunner(const int& bucket_size, const int& block_number,
        const int& block_size, const int& odict_size,
        const size_t& max_size, const unsigned int& alpha,
        const unsigned int& x, std::string_view password,
        std::string_view connection_info, const int& oram_block_size,
        const size_t& column_number, std::string_view table_name,
        const char* address = "127.0.0.1:4567");

    ~ClientRunner();

    void test_add_node(const unsigned int& number);

    void test_adj(std::string_view file_path);

    Range::Node* get_t1_root(const std::string& map_key);

    std::vector<SEAL::Document> search(std::string_view keyword);

    std::vector<SEAL::Document> search_range(std::string_view map_key, std::string_view lower, std::string_view upper);
};

#endif