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
#include <server/SealService.h>
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>

SealServerRunner::SealServerRunner(const std::string& address)
{
    plog::init(plog::error, "log/server.txt");
    SealService service;
    grpc::ServerBuilder server_builder;
    server_builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    server_builder.RegisterService(&service);
    server = server_builder.BuildAndStart();

    std::cout << "The server starts.\n";
    server.get()->Wait();
}

SealServerRunner::~SealServerRunner()
{
}