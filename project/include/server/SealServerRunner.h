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

#ifndef SEAL_SERVER_RUNNER_H
#define SEAL_SERVER_RUNNER_H

#include <string>
#include <memory>

#include <grpc++/server.h>
#include <grpc++/server_builder.h>

class SealServerRunner {
private:
    std::unique_ptr<grpc::Server> server;
public:
    SealServerRunner(const std::string& address);
    ~SealServerRunner();
};

#endif