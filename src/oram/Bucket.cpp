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

#include <oram/Bucket.h>

#include <iostream>
#include <sstream>
#include <string>

bool Bucket::is_init = false;
int Bucket::max_size = -1;

Bucket::Bucket()
{
}

//Copy constructor
Bucket::Bucket(Bucket* other)
{
    if (other == NULL) {
        std::cout << "triggered by me? NULL check\n";
        throw std::runtime_error("the other bucket is not malloced.");
    }
    blocks = std::vector<Block>(max_size);
    for (int i = 0; i < max_size; i++) {
        blocks[i] = Block(other->blocks[i]);
    }
}

//Get block object with matching index
Block Bucket::getBlockByIndex(int index)
{
    Block* copy_block = NULL;
    for (Block b : blocks) {
        if (b.index == index) {
            copy_block = new Block(b);
            break;
        }
    }
    return *copy_block;
}

void Bucket::addBlock(Block new_blk)
{
    if (blocks.size() < (unsigned)max_size) {
        Block toAdd = Block(new_blk);
        blocks.push_back(toAdd);
    }
}

bool Bucket::removeBlock(Block rm_blk)
{
    bool removed = false;
    for (unsigned int i = 0; i < blocks.size(); i++) {
        if (blocks[i].index == rm_blk.index) {
            blocks.erase(blocks.begin() + i);
            removed = true;
            break;
        }
    }
    return removed;
}

// Return a shallow copy.
vector<Block> Bucket::getBlocks()
{
    return this->blocks;
}

void Bucket::setMaxSize(int maximumSize)
{
    // For convenience, we can re-init it.
    if (is_init == true && maximumSize != max_size) {
        throw std::runtime_error("Max Bucket Size was already set");
    }
    max_size = maximumSize;
    is_init = true;
}

int Bucket::getMaxSize()
{
    return max_size;
}

void Bucket::resetState()
{
    is_init = false;
}

void Bucket::printBlocks()
{
    for (Block b : blocks) {
        b.printBlock();
    }
}
