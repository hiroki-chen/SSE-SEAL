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

#ifndef PORAM_SERVERSTORAGE_H
#define PORAM_SERVERSTORAGE_H

#include <cmath>

#include "OramInterface.h"
#include "RandForOramInterface.h"
#include "UntrustedStorageInterface.h"

#include <proto/seal.grpc.pb.h>
#include <proto/seal.pb.h>

/**
 * This design is shit, bullshit.
 * 
 * We need to reconstruct the code completely because server storage is constructed on the server side,
 * rather than the client side.
 * 
 * TODO: 1. Provide Intefaces to connect to the server.
 * TODO: 2. Remove oramPathEivction class from the server.
 * TODO: 3. Totally rewrite the ORAM access interface provided by the server.
 */

/**
 * The ServerStorage class provided interfaces to communicate with the server.
 * 
 * @note ServerStorage 
 */
class ServerStorage : public UntrustedStorageInterface {
private:
    Seal::Stub* const stub_;

    const unsigned int oram_id;

    const bool is_odict;

    const std::string key;

public:

    /**
     * @brief The constructor for the ServerStorage class.
     * 
     * @param oram_id The id of the oram structure to which it belongs.
     * @param is_odict Oblivioud data structure is a little bit different.
     * @param key Used to look up the storage on the server side.
     * @param stub_ Connection to the server.
     */
    ServerStorage(const unsigned int& oram_id, const bool& is_odict, const std::string& key, Seal::Stub * stub_);

    void setCapacity(const int& total_number_of_buckets);

    Bucket ReadBucket(const int& position);

    void WriteBucket(const int& position, const Bucket& bucket_to_write);

private:
    int capacity;
};

#endif //PORAM_ORAMREADPATHEVICTION_H
