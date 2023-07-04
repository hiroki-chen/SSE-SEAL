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

#ifndef PORAM_BUCKET_H
#define PORAM_BUCKET_H

#include <stdexcept>
#include <vector>

#include <cereal/access.hpp>
#include <cereal/types/vector.hpp>

#include "Block.h"

using namespace std;

class Bucket {
    friend class cereal::access;
    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(is_init, max_size, blocks);
    }

public:
    Bucket();
    Bucket(Bucket* other);
    Block getBlockByIndex(int index);
    void addBlock(Block new_blk);
    bool removeBlock(Block rm_blk);
    vector<Block> getBlocks();
    static void setMaxSize(int maximumSize);
    static void resetState();
    static int getMaxSize();
    void printBlocks();

private:
    static bool is_init; //should be initially false
    static int max_size; //should be initially -1
    vector<Block> blocks;
};

#endif //PORAM_BUCKET_H
