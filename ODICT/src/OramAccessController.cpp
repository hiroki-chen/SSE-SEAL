#include <OramAccessController.h>
#include <ServerStorage.h>
#include <OramReadPathEviction.h>
#include <RandomForOram.h>
#include <OramDeterministic.h>
#include <utils.h>
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>

#include <iostream>
#include <strings.h>

OramAccessController::OramAccessController(const int &bucket_size, const int &block_number, const int &block_size)
{
    plog::init(plog::debug, "log/log.txt");
    PLOG(plog::info) << "Warming up OramAccessController...\n";
    Bucket::setMaxSize(bucket_size);
    this->block_size = block_size;

    storage = new ServerStorage();
    random = new RandomForOram();
    oram = new OramReadPathEviction(storage, random, bucket_size, block_number, block_size);
}

void OramAccessController::oblivious_access(OramAccessOp op, const int &address, unsigned char *data)
{
    int *new_data = new int[block_size];
    memcpy(new_data, transform<int, unsigned char>(data), block_size);
    OramInterface::Operation operation = deduct_operation(op);
    new_data = oram->access(operation, address, new_data);
    memcpy(data, transform<unsigned char, int>(new_data), block_size);
}

void OramAccessController::oblivious_access_direct(OramAccessOp op, unsigned char *data)
{
    int *new_data = new int[block_size];
    memcpy(new_data, transform<int, unsigned char>(data), block_size);
    OramInterface::Operation operation = deduct_operation(op);
    new_data = oram->access_direct(operation, new_data);
    memcpy(data, transform<unsigned char, int>(new_data), block_size);
}

int OramAccessController::random_new_pos()
{
    return random->getRandomLeaf();
}