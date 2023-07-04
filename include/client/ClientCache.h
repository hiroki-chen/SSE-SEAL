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

#ifndef CLIENT_CACHE_H
#define CLIENT_CACHE_H

#include <cstddef>
#include <deque>
#include <iostream>
#include <map>
#include <unordered_map>

#include "Objects.h"
#include "OramAccessController.h"
#include <plog/Log.h>
#include <utils.h>

namespace SEAL {
template <typename T>
class Cache {
private:
    const size_t max_size;

    std::unordered_map<int, T> cache_items;

    std::deque<int> lru_table;

    OramAccessController* oramAccessController;

public:
    /**
     * @brief The constructor for client cache.
     * @param max_size the maximum size the cache can hold.
     * @param oramAccessController the oram controller needed to generate random position tag for the items in the cache.
     */
    Cache(const size_t& max_size, OramAccessController* const oramAccessController);

    /**
     * @brief Get an item by its id from the cache.
     * @param id the id of the desired item.
     */
    T get(const int& id);

    /**
     * @brief Used to get an arbitrary item from the tail of the cache.
     */
    T get();

    /**
     * @brief Put an item into the cache.
     * @param id the id of the item.
     * @param item the item passed by const reference.
     * @return An item evicted if  the cache is full.
     */
    T put(const int& id, const T& item);

    /**
     * @brief Find the position tag in the local cache.
     * @param id the id of the item.
     */
    int find_pos_by_id(const int& id);

    /**
     * @brief Update all the position tags locally.
     * @param root_id the id the root item (ODS)
     * @return The updated root position
     */
    int update_pos(const int& root_id);

    /**
     * @brief Clear all the items in the cache.
     */
    void clear();

    /**
     * @brief Pop an item from the cache.
     */
    void pop();

    /**
     * @brief Check if the cache is empty.
     */
    bool empty();
};

template <typename T>
inline SEAL::Cache<T>::Cache(const size_t& max_size, OramAccessController* const oramAccessController)
    : max_size(max_size)
    , oramAccessController(oramAccessController)
{
    PLOG(plog::info) << "Client Cache started.";
}

template <typename T>
inline T SEAL::Cache<T>::get(const int& id)
{
    auto iter = cache_items.find(id);

    if (iter == cache_items.end()) {
        return T(0, -1);
    } else {
        auto it = std::find_if(lru_table.begin(), lru_table.end(), [id](const int& item_id) { return item_id == id; });

        int item_id = *it;
        lru_table.erase(it);
        lru_table.push_front(item_id);
        return iter->second;
    }
}

template <typename T>
inline T SEAL::Cache<T>::get()
{
    return cache_items.begin()->second;
}

template <typename T>
inline T SEAL::Cache<T>::put(const int& id, const T& item)
{
    auto iter = cache_items.find(id);
    auto it = std::find_if(lru_table.begin(), lru_table.end(), [id](const int& item_id) { return item_id == id; });
    lru_table.push_front(id);

    if (iter != cache_items.end() && it != lru_table.end()) {
        cache_items.erase(iter);
        lru_table.erase(it);
    }

    cache_items[id] = item;

    // evict the item
    if (cache_items.size() > max_size) {
        int back = lru_table.back();
        lru_table.pop_back();
        iter = cache_items.find(back);

        T ret = iter->second;
        cache_items.erase(iter);

        PLOG(plog::info) << "Cache is full! Evict one element " << back << " to ORAM server";

        // Write to the oram.
        /*ret->pos_tag = oramAccessController->random_new_pos();

            std::cout << "ret->id " << ret->id << std::endl;

            for (auto it = cache_items.begin(); it != cache_items.end(); it++)
            {
                if (it->second.left_id == ret->id)
                {
                    it->second.childrenPos[0].pos_tag = ret->pos_tag;
                }
                else if (it->second.right_id == ret->id)
                {
                    it->second.childrenPos[1].pos_tag = ret->pos_tag;
                }
            }

            unsigned char *buffer = new unsigned char[sizeof(T)];
            memcpy(buffer, ret, sizeof(T));
            oramAccessController->oblivious_access_direct(ORAM_ACCESS_WRITE, buffer);*/

        return ret;
    }

    return T(0, -1);
}

template <typename T>
inline int SEAL::Cache<T>::update_pos(const int& root_id)
{
    std::map<int, int> position_tag;
    for (auto iter = cache_items.begin(); iter != cache_items.end(); iter++) {
        ODict::Node* node = &(iter->second);
        // Generate a random position tag.
        position_tag[node->id] = oramAccessController->random_new_pos();

        if (node->old_tag == 0) {
            node->old_tag = position_tag[node->id];
        }

        node->pos_tag = position_tag[node->id];
    }

    for (auto iter = cache_items.begin(); iter != cache_items.end(); iter++) {
        ODict::Node* node = &(iter->second);
        // Reallocate the position tag for each child node.
        node->childrenPos[0].pos_tag = position_tag[node->left_id];
        node->childrenPos[1].pos_tag = position_tag[node->right_id];
    }

    return position_tag[root_id];
}

template <typename T>
inline int SEAL::Cache<T>::find_pos_by_id(const int& id)
{
    for (auto iter = cache_items.begin(); iter != cache_items.end(); iter++) {
        auto node = transform<ODict::Node, T>(&(iter->second));
        if (node->left_id == id) {
            return node->childrenPos[0].pos_tag;
        } else if (node->right_id == id) {
            return node->childrenPos[1].pos_tag;
        }
    }

    return -1;
}

template <typename T>
inline void SEAL::Cache<T>::clear()
{
    cache_items.clear();
    lru_table.clear();
}

template <typename T>
inline void SEAL::Cache<T>::pop()
{
    cache_items.erase(cache_items.begin());
}

template <typename T>
inline bool SEAL::Cache<T>::empty()
{
    return cache_items.empty();
}

} // namespace SEAL

#endif
