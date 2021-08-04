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