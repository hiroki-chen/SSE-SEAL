#ifndef PORAM_BUCKET_H
#define PORAM_BUCKET_H

#include <stdexcept>
#include <vector>

#include <Block.h>

using namespace std;

class Bucket {

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
