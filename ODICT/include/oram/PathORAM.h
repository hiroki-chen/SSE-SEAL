#ifndef PATHORAM_PATHORAM_H
#define PATHORAM_PATHORAM_H

typedef enum {
    ORAM_ACCESS_READ,
    ORAM_ACCESS_WRITE,
    ORAM_ACCESS_DELETE,
    ORAM_ACCESS_INSERT // insert to cache.
} OramAccessOp;

#endif //PATHORAM_PATHORAM_H
