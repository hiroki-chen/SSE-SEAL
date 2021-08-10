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

#include <oram/OramReadPathEviction.h>
#include <utils.h>

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <strings.h>

OramReadPathEviction::OramReadPathEviction(
    UntrustedStorageInterface* storage,
    RandForOramInterface* rand_gen, const unsigned int& bucket_size,
    const unsigned int& num_blocks, const unsigned int& block_size)
{
    this->storage = storage;
    this->rand_gen = rand_gen;
    this->bucket_size = bucket_size;
    this->num_blocks = num_blocks;
    this->num_levels = std::ceil(log10(num_blocks) / log10(2)) + 1;
    this->num_buckets = (unsigned int)std::pow(2, num_levels) - 1;

    if (this->num_buckets * this->bucket_size < this->num_blocks) //deal with precision loss
    {
        throw new runtime_error("Not enough space for the acutal number of blocks.");
    }

    this->num_leaves = (unsigned int)std::pow(2, num_levels - 1);
    Bucket::resetState();
    Bucket::setMaxSize(bucket_size);
    this->rand_gen->setBound(num_leaves);
    this->storage->setCapacity(num_buckets);

    for (unsigned int i = 0; i < num_blocks; i++) {
        position_map[i] = rand_gen->getRandomLeaf();
    }

    for (unsigned int i = 0; i < num_buckets; i++) {
        Bucket init_bkt = Bucket();
        for (unsigned int j = 0; j < bucket_size; j++) {
            init_bkt.addBlock(Block());
        }
        storage->WriteBucket(i, Bucket(init_bkt));
    }
}
std::string
OramReadPathEviction::access_handler(
    Operation op, const unsigned int& blockIndex,
    const int& oldLeaf, const int& newLeaf, const std::string& new_data)
{
    std::string data; // The data to be returned.

    for (unsigned int i = 0; i < num_levels; i++) {
        vector<Block> blocks = storage->ReadBucket(OramReadPathEviction::P(oldLeaf, i)).getBlocks();
        for (Block b : blocks) {
            if (b.index != -1) {
                stash.push_back(b);
            }
        }
    }

    auto iter = std::find_if(stash.begin(), stash.end(), [blockIndex](const Block& block) {
        return block.index == (int)blockIndex;
    });

    if (op == Operation::WRITE) {
        if (iter == stash.end()) {
            Block newBlock = Block(newLeaf, blockIndex, new_data);
            stash.push_back(newBlock);
        } else {
            (*iter).data = new_data;
        }
    } else {
        if (iter != stash.end()) {
            data = (*iter).data;
        }
    }

    // Eviction steps: write to the same path that was read from.
    for (int l = num_levels - 1; l >= 0; l--) {

        std::vector<int> bid_evicted;
        Bucket bucket = Bucket();
        int Pxl = P(oldLeaf, l);
        int counter = 0;

        for (Block b_instash : stash) {
            if ((unsigned)counter >= bucket_size) {
                break;
            }
            Block be_evicted = Block(b_instash);
            if (Pxl == P(position_map[be_evicted.index], l)) {
                bucket.addBlock(be_evicted);

                bid_evicted.push_back(be_evicted.index);
                counter++;
            }
        }

        //remove from the stash
        for (unsigned int i = 0; i < bid_evicted.size(); i++) {
            for (unsigned int j = 0; j < stash.size(); j++) {
                Block b_instash = stash.at(j);
                if (b_instash.index == bid_evicted.at(i)) {
                    this->stash.erase(this->stash.begin() + j);

                    break;
                }
            }
        }

        while ((unsigned)counter < bucket_size) {
            bucket.addBlock(Block()); //dummy block
            counter++;
        }
        storage->WriteBucket(Pxl, bucket);
    }

    return data;
}

std::string
OramReadPathEviction::access_direct(Operation op, const std::string& new_data)
{
    /*if (!instanceof<ODict::Node, int>(newdata))
    {
        throw std::runtime_error("Cannot access directly with a non-node data array!");
    }*/
    ODict::Node node = deserialize<ODict::Node>(new_data);
    int blockIndex = node.id;
    int oldLeaf = position_map[blockIndex];
    int newLeaf = node.pos_tag;

    if (op == Operation::READ || blockIndex == 0) {
        newLeaf = rand_gen->getRandomLeaf();
    }

    return access_handler(op, blockIndex, oldLeaf, newLeaf, new_data);
}

std::string
OramReadPathEviction::access(
    Operation op,
    const unsigned int& blockIndex,
    const std::string& new_data)
{
    int oldLeaf = position_map[blockIndex];
    position_map[blockIndex] = rand_gen->getRandomLeaf();

    return access_handler(op, blockIndex, oldLeaf, position_map[blockIndex], new_data);
}

int OramReadPathEviction::P(int leaf, int level)
{
    /*
    * This function should be deterministic. 
    * INPUT: leaf in range 0 to num_leaves - 1, level in range 0 to num_levels - 1. 
    * OUTPUT: Returns the location in the storage of the bucket which is at the input level and leaf.
    */
    return (1 << level) - 1 + (leaf >> (this->num_levels - level - 1));
}

/*
The below functions are to access various parameters, as described by their names.
INPUT: No input
OUTPUT: Value of internal variables given in the name.
*/

int* OramReadPathEviction::getPositionMap()
{
    return nullptr;
}

vector<Block> OramReadPathEviction::getStash()
{
    return this->stash;
}

int OramReadPathEviction::getStashSize()
{
    return (this->stash).size();
}

int OramReadPathEviction::getNumLeaves()
{
    return this->num_leaves;
}

int OramReadPathEviction::getNumLevels()
{
    return this->num_levels;
}

int OramReadPathEviction::getNumBlocks()
{
    return this->num_blocks;
}

int OramReadPathEviction::getNumBuckets()
{
    return this->num_buckets;
}
