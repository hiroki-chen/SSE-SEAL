#include <client/OramAccessController.h>
#include <oram/OramDeterministic.h>
#include <oram/OramReadPathEviction.h>
#include <oram/RandomForOram.h>
#include <oram/ServerStorage.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Log.h>
#include <utils.h>

#include <iostream>
#include <strings.h>

OramAccessController::OramAccessController(const int& bucket_size, const int& block_number, const int& block_size)
{
    auto logger = plog::get<0>();
    if (logger == NULL) {
        plog::init(plog::debug, "log/log.txt");
    }

    PLOG(plog::info) << "Warming up OramAccessController...\n";
    Bucket::setMaxSize(bucket_size);
    this->block_size = block_size;

    storage = nullptr;
    random = RandomForOram::get_instance();
    oram = nullptr;
}

// TODO: Add an overloaded version for gRPC.
void OramAccessController::oblivious_access(OramAccessOp op, const int& address, unsigned char* data)
{
    int* new_data = new int[block_size];
    memcpy(new_data, transform<int, unsigned char>(data), block_size);
    OramInterface::Operation operation = deduct_operation(op);
    new_data = oram->access(operation, address, new_data);
    memcpy(data, transform<unsigned char, int>(new_data), block_size);
}

void OramAccessController::oblivious_access(
    OramAccessOp op,
    const int& address,
    unsigned char* data,
    const int& oram_id,
    Seal::Stub* stub_)
{
    grpc::ClientContext context;
    OramAccessMessage message;
    OramAccessResponse response;
    message.set_id(address);
    message.set_is_odict(false);
    message.set_oram_id(oram_id);
    message.set_operation(op == OramAccessOp::ORAM_ACCESS_READ ? false : true);
    message.set_buffer(data, block_size);

    grpc::Status status = stub_->oram_access(&context, message, &response);

    if (!status.ok()) {
        PLOG(plog::error) << "Cannot access the remote server.";
        return;
    }
    const std::string ans = response.buffer();
    memcpy(data, ans.c_str(), block_size);
}

void OramAccessController::oblivious_access_direct(
    OramAccessOp op,
    unsigned char* data,
    Seal::Stub* stub_)
{
    ODict::Node* node = transform<ODict::Node, unsigned char>(data);
    grpc::ClientContext context;
    OramAccessMessage message;
    OramAccessResponse response;
    message.set_id(node->id);
    message.set_is_odict(true);
    message.set_oram_id(0);
    message.set_operation(op == OramAccessOp::ORAM_ACCESS_READ ? false : true);
    message.set_buffer(data, block_size);
    grpc::Status status = stub_->oram_access(&context, message, &response);

    if (!status.ok()) {
        PLOG(plog::error) << "Cannot access the remote server.";
        return;
    }
    const std::string ans = response.buffer();
    memcpy(data, ans.c_str(), block_size);
}

void OramAccessController::oblivious_access_direct(OramAccessOp op, unsigned char* data)
{
    int* new_data = new int[block_size];
    memcpy(new_data, transform<int, unsigned char>(data), block_size);
    OramInterface::Operation operation = deduct_operation(op);
    new_data = oram->access_direct(operation, new_data);
    memcpy(data, transform<unsigned char, int>(new_data), block_size);
}

int OramAccessController::random_new_pos()
{
    return random->getRandomLeaf();
}

RandForOramInterface* OramAccessController::get_random_engine()
{
    return random;
}