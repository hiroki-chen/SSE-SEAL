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

#ifndef PORAM_ORAMREADPATHEVICTION_H
#define PORAM_ORAMREADPATHEVICTION_H

#include <cmath>
#include <map>

#include "OramInterface.h"
#include "RandForOramInterface.h"
#include "UntrustedStorageInterface.h"

class OramReadPathEviction : public OramInterface {
private:
    std::string access_handler(
        Operation op, const unsigned int& blockIndex,
        const int& oldLeaf, const int& newLeaf, const std::string& newdata);

public:
    UntrustedStorageInterface* storage;

    RandForOramInterface* rand_gen;

    unsigned int bucket_size;

    unsigned int num_levels;

    unsigned int num_leaves;

    unsigned int num_blocks;

    unsigned int num_buckets;

    std::map<unsigned int, unsigned int> position_map; //array

    std::vector<Block> stash;

    OramReadPathEviction(
        UntrustedStorageInterface* storage,
        RandForOramInterface* rand_gen, const unsigned int& bucket_size, 
        const unsigned int& num_blocks, const unsigned int& block_size = BLOCK_SIZE);

    std::string access(Operation op, const unsigned int& blockIndex, const std::string& new_data);

    std::string access_direct(Operation op, const std::string& new_data);

    int P(int leaf, int level);

    int* getPositionMap();

    vector<Block> getStash();

    int getStashSize();

    int getNumLeaves();

    int getNumLevels();

    int getNumBlocks();

    int getNumBuckets();
};

#endif
