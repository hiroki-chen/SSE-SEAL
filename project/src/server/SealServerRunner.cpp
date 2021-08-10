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

#include <server/SealServerRunner.h>
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>

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
    server_builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    server_builder.RegisterService(service.get());
    server = server_builder.BuildAndStart();

    std::cout << "The server starts.\n";
    server.get()->Wait();
}

SealServerRunner::~SealServerRunner()
{
}