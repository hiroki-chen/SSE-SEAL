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

#ifndef PORAM_UNTRUSTEDSTORAGEINTERFACE_H
#define PORAM_UNTRUSTEDSTORAGEINTERFACE_H

#include "Bucket.h"

/**
 * @brief This is a public interface for any inheritance of server storage.
 */
class UntrustedStorageInterface {
public:
    /**
     * @brief Set the capacity of buckets.
     * @param total_num_of_buckets
     */ 
    virtual void setCapacity(const int& total_num_of_buckets) {};

    /**
     * @brief Write to the bucket, called by the oram implementation.
     * @param position the position of the bucket in the bucket array. @see Bucket.h
     * @param bucket_to_write
     */
    virtual void WriteBucket(const int& position, const Bucket& bucket_to_write) {};

    /**
     * @brief Read the bucket from the bucket array.
     * @param position
     */ 
    virtual Bucket ReadBucket(const int& position) { return Bucket(); };
};

#endif //PORAM_UNTRUSTEDSTORAGEINTERFACE_H
