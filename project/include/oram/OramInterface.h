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
