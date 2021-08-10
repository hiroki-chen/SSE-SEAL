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

#ifndef PORAM_RANDOMFORORAM_H
#define PORAM_RANDOMFORORAM_H

#include <cmath>
#include <random>
#include <stdlib.h>
#include <vector>

#include "RandForOramInterface.h"
#include <csprng.hpp>

using namespace std;

class RandomForOram : public RandForOramInterface {
public:
    static RandomForOram* random;
    static bool is_initialized;
    static int bound;
    vector<int> rand_history;
    RandomForOram();
    void RandomForOramMT();
    void RandomForOramLCG();
    int getRandomLeaf();
    int getRandomLeafMT();
    int getRandomLeafLCG();
    void setBound(int totalNumOfLeaves);
    void resetState();
    void clearHistory();

    static RandomForOram* get_instance();
    vector<int> getHistory();
    linear_congruential_engine<unsigned long, 25214903917, 11, 281474976710656> rnd_generator;
    std::mt19937 mt_generator;
    long seed = 0L;
    duthomhas::csprng rng;
};

//static int RandomForOram::bound = -1;

#endif
