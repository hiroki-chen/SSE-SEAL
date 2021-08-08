#ifndef ORAM_STRUCT_H
#define ORAM_STRUCT_H

#include <oram/OramReadPathEviction.h>
#include <oram/RandForOramInterface.h>
#include <oram/UntrustedStorageInterface.h>

struct OramStruct {
    UntrustedStorageInterface* storage;

    OramInterface* oram;

    RandForOramInterface* random;
};

#endif