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

#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Log.h>
#include <server/SealServerRunner.h>
#include <utils.h>

#include <grpc++/security/server_credentials.h>

void SealServerRunner::sig_handler(int t)
{
    std::cout << "Received CTRL+C signal call! Stop the server now..." << std::endl;
    service.get()->print_oram_blocks();
}

void SealServerRunner::run(const std::string& address)
{
    plog::init(plog::error, "log/server.txt");
    service = std::make_unique<SealService>();
    grpc::ServerBuilder server_builder;
    const std::string servercert = read_keycert("keys/server.crt");
    const std::string serverkey = read_keycert("keys/server.key");

    grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp;
    pkcp.private_key = serverkey;
    pkcp.cert_chain = servercert;
    grpc::SslServerCredentialsOptions ssl_options;
    ssl_options.pem_root_certs = "";
    ssl_options.pem_key_cert_pairs.push_back(pkcp);
    std::shared_ptr<grpc::ServerCredentials> credentials = grpc::SslServerCredentials(ssl_options);

    server_builder.AddListeningPort(address, credentials);
    server_builder.RegisterService(service.get());
    server = server_builder.BuildAndStart();

    std::cout << "The server starts.\n";
    server.get()->Wait();
}

SealServerRunner::~SealServerRunner()
{
}