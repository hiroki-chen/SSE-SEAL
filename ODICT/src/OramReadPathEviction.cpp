#include <OramReadPathEviction.h>
#include <utils.h>

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <strings.h>

OramReadPathEviction::OramReadPathEviction(UntrustedStorageInterface* storage, RandForOramInterface* rand_gen,
    int bucket_size, int num_blocks, int block_size)
{
    this->storage = storage;
    this->block_size = block_size;
    this->rand_gen = rand_gen;
    this->bucket_size = bucket_size;
    this->num_blocks = num_blocks;
    this->num_levels = ceil(log10(num_blocks) / log10(2)) + 1;
    this->num_buckets = pow(2, num_levels) - 1;
    if (this->num_buckets * this->bucket_size < this->num_blocks) //deal with precision loss
    {
        throw new runtime_error("Not enough space for the acutal number of blocks.");
    }
    this->num_leaves = pow(2, num_levels - 1);
    Bucket::resetState();
    Bucket::setMaxSize(bucket_size);
    this->rand_gen->setBound(num_leaves);
    this->storage->setCapacity(num_buckets);
    this->stash = vector<Block>();

    for (int i = 0; i < this->num_blocks; i++) {
        position_map[i] = rand_gen->getRandomLeaf();
    }

    for (int i = 0; i < num_buckets; i++) {

        Bucket init_bkt = Bucket();
        for (int j = 0; j < bucket_size; j++) {
            init_bkt.addBlock(Block());
        }
        storage->WriteBucket(i, Bucket(init_bkt));
    }
}

int* OramReadPathEviction::access_handler(Operation op, int blockIndex, int oldLeaf, int newLeaf, int* newdata)
{
    int targetPos = 0;
    int* data = new int[block_size];

    for (int i = 0; i < num_levels; i++) {
        vector<Block> blocks = storage->ReadBucket(OramReadPathEviction::P(oldLeaf, i)).getBlocks();
        for (Block b : blocks) {
            if (b.index != -1) {
                stash.push_back(Block(b));
            }
        }
    }

    Block* targetBlock = nullptr;

    for (unsigned int i = 0; i < stash.size(); i++) {
        Block b = stash[i];
        if (b.index == blockIndex) {
            targetBlock = &b;
            targetPos = i;
            break;
        }
    }

    if (op == Operation::WRITE) {
        if (targetBlock == nullptr) {
            Block newBlock = Block(newLeaf, blockIndex, newdata);
            newBlock.data = new int[block_size];
            memcpy(newBlock.data, newdata, block_size);
            stash.push_back(newBlock);
        } else {
            targetBlock->data = new int[block_size];
            memcpy(targetBlock->data, newdata, block_size);
        }
    } else {
        if (targetBlock == nullptr) {
            data = nullptr;
        } else {
            memcpy(data, targetBlock->data, block_size);
        }
    }

    // Eviction steps: write to the same path that was read from.
    for (int l = num_levels - 1; l >= 0; l--) {

        vector<int> bid_evicted = vector<int>();
        Bucket bucket = Bucket();
        int Pxl = P(oldLeaf, l);
        int counter = 0;

        for (Block b_instash : stash) {

            if (counter >= bucket_size) {
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

        while (counter < bucket_size) {
            bucket.addBlock(Block()); //dummy block
            counter++;
        }
        storage->WriteBucket(Pxl, bucket);
    }

    return data;
}

int* OramReadPathEviction::access_direct(Operation op, int* newdata)
{
    /*if (!instanceof<ODict::Node, int>(newdata))
    {
        throw std::runtime_error("Cannot access directly with a non-node data array!");
    }*/

    ODict::Node* node = transform<ODict::Node, int>(newdata);
    int blockIndex = node->id;
    int oldLeaf = position_map[blockIndex];
    int newLeaf = node->pos_tag;

    if (op == Operation::READ) {
        newLeaf = rand_gen->getRandomLeaf();
    }

    return access_handler(op, blockIndex, oldLeaf, newLeaf, newdata);
}

int* OramReadPathEviction::access(Operation op, int blockIndex, int* newdata)
{
    int oldLeaf = position_map[blockIndex];
    position_map[blockIndex] = rand_gen->getRandomLeaf();

    return access_handler(op, blockIndex, oldLeaf, position_map[blockIndex], newdata);
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
