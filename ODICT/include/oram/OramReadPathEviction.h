#ifndef PORAM_ORAMREADPATHEVICTION_H
#define PORAM_ORAMREADPATHEVICTION_H

#include <cmath>
#include <map>

#include "OramInterface.h"
#include "RandForOramInterface.h"
#include "UntrustedStorageInterface.h"

class OramReadPathEviction : public OramInterface {
private:
    int* access_handler(Operation op, int blockIndex, int oldLeaf, int newLeaf, int* newdata);

public:
    UntrustedStorageInterface* storage;
    RandForOramInterface* rand_gen;

    int bucket_size;
    int num_levels;
    int num_leaves;
    int num_blocks;
    int num_buckets;
    int block_size;

    std::map<int, int> position_map; //array
    vector<Block> stash;

    OramReadPathEviction(UntrustedStorageInterface* storage,
        RandForOramInterface* rand_gen, int bucket_size, int num_blocks, int block_size = Block::BLOCK_SIZE);
    int* access(Operation op, int blockIndex, int newdata[]);
    int* access_direct(Operation op, int newdata[]);
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
