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

#include <client/OramAccessController.h>
#include <oram/OramReadPathEviction.h>
#include <oram/RandomForOram.h>
#include <oram/ServerStorage.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Log.h>
#include <utils.h>

#include <iostream>
#include <strings.h>

OramAccessController::OramAccessController(
    const int& bucket_size,
    const int& block_number,
    const int& block_size,
    const int& oram_id,
    const bool& is_odict,
    const std::string& key,
    Seal::Stub* stub_)
    : oram_id(oram_id)
    , block_size(block_size)
    , is_odict(is_odict)
    , stub_(stub_)
{
    auto logger = plog::get<0>();
    if (logger == NULL) {
        plog::init(plog::debug, "log/log.txt");
    }

    PLOG(plog::info) << "Warming up OramAccessController...\n";
    Bucket::setMaxSize(bucket_size);

    storage = new ServerStorage(oram_id, is_odict, key, stub_);
    random = RandomForOram::get_instance();
    oram = new OramReadPathEviction(storage, random, bucket_size, block_number, block_size);
}

void OramAccessController::oblivious_access(OramAccessOp op, const int& address, std::string& data)
{
    OramInterface::Operation operation = deduct_operation(op);
    data = oram->access(operation, address, data);
}

void OramAccessController::oblivious_access_direct(OramAccessOp op, std::string& data)
{
    OramInterface::Operation operation = deduct_operation(op);
    data = oram->access_direct(operation, data);
}

int OramAccessController::random_new_pos()
{
    return random->getRandomLeaf();
}

RandForOramInterface*
OramAccessController::get_random_engine()
{
    return random;
}

void OramAccessController::set_stub(Seal::Stub * stub_)
{
    this->stub_ = stub_;
}