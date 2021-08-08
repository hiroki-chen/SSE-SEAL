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