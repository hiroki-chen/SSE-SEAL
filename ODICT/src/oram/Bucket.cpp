#include <oram/Bucket.h>

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

bool Bucket::is_init = false;
int Bucket::max_size = -1;

Bucket::Bucket()
{
    if (!is_init) {
        std::cout << "triggered by me? Uninit?\n";
        throw new runtime_error("Please set bucket size before creating a bucket");
    }
    blocks = vector<Block>();
}

//Copy constructor
Bucket::Bucket(Bucket* other)
{
    if (other == NULL) {
        std::cout << "triggered by me? NULL check\n";
        throw new runtime_error("the other bucket is not malloced.");
    }
    blocks = vector<Block>(max_size);
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
        std::cout << "triggered by me?\n";
        throw new runtime_error("Max Bucket Size was already set");
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