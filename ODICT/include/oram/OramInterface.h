#ifndef PORAM_ORAMINTERFACE_H
#define PORAM_ORAMINTERFACE_H

#include <vector>
#include <string>

#include "Block.h"

class OramInterface {
public:
    enum Operation {
        READ,
        WRITE
    };

    virtual std::string access(Operation op, const unsigned int& blockIndex, const std::string& newdata) { return 0; };

    virtual std::string access_direct(Operation op, const std::string& newdata) { return 0; }

    virtual int P(int leaf, int level) { return 0; };

    virtual int* getPositionMap() { return 0; };

    virtual std::vector<Block> getStash() { return std::vector<Block>(); };

    virtual int getStashSize() { return 0; };

    virtual int getNumLeaves() { return 0; };

    virtual int getNumLevels() { return 0; };

    virtual int getNumBlocks() { return 0; };

    virtual int getNumBuckets() { return 0; };
};

#endif //PORAM_ORAMINTERFACE_H
