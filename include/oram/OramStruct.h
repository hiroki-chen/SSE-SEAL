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