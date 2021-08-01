#ifndef STASHSIZESIMULATOR_H
#define STASHSIZESIMULATOR_H

#include <vector>

#include <OramInterface.h>
#include <OramReadPathEviction.h>
#include <ServerStorage.h>
#include <UntrustedStorageInterface.h>

class StashSizeSimulator {
public:
    StashSizeSimulator(int bSize, int nBlocks, int nAccesses, int sNumber, int t);
    int* sampleData(int i);
    void runSimulation();
    void writeSimulation();
    void updateStashSizes(int recorded, int times);
    int* recordedSizes;
    int* stashSizes;
    int numBlocks;
    int bucketSize;
    int numAccesses;
    int simNumber;
    int type;
};

#endif