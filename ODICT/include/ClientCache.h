#ifndef CLIENT_CACHE_H
#define CLIENT_CACHE_H

#include <cstddef>
#include <deque>
#include <iostream>
#include <map>
#include <unordered_map>

#include <Objects.h>
#include <OramAccessController.h>
#include <plog/Log.h>

namespace SEAL {
template <typename T>
class Cache {
private:
    const size_t max_size;

    std::unordered_map<int, T> cache_items;

    std::deque<int> lru_table;

    OramAccessController* const oramAccessController;

public:
    Cache(const size_t& max_size, OramAccessController* const oramAccessController);

    T* get(const int& id);

    T* get();

    T* put(const int& id, const T* const item);

    int update_pos(const int& root_id);

    void clear();

    void pop();

    bool empty();
};

template <typename T>
SEAL::Cache<T>::Cache(const size_t& max_size, OramAccessController* const oramAccessController)
    : max_size(max_size)
    , oramAccessController(oramAccessController)
{
    PLOG(plog::info) << "Client Cache started.";
}

template <typename T>
T* SEAL::Cache<T>::get(const int& id)
{
    auto iter = cache_items.find(id);

    if (iter == cache_items.end()) {
        return nullptr;
    } else {
        auto it = std::find_if(lru_table.begin(), lru_table.end(), [id](const int& item_id) { return item_id == id; });

        int item_id = *it;
        lru_table.erase(it);
        lru_table.push_front(item_id);
        return &(iter->second);
    }
}

template <typename T>
T* SEAL::Cache<T>::get()
{
    return &(cache_items.begin()->second);
}

template <typename T>
T* SEAL::Cache<T>::put(const int& id, const T* const item)
{
    auto iter = cache_items.find(id);
    auto it = std::find_if(lru_table.begin(), lru_table.end(), [id](const int& item_id) { return item_id == id; });
    lru_table.push_front(id);

    if (iter != cache_items.end() && it != lru_table.end()) {
        cache_items.erase(iter);
        lru_table.erase(it);
    }

    cache_items[id] = *item;

    // evict the item
    if (cache_items.size() > max_size) {
        int back = lru_table.back();
        lru_table.pop_back();
        iter = cache_items.find(back);

        T* ret = new T();
        memcpy(ret, &(iter->second), sizeof(T));
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

    return nullptr;
}

template <typename T>
int SEAL::Cache<T>::update_pos(const int& root_id)
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
void SEAL::Cache<T>::clear()
{
    cache_items.clear();
    lru_table.clear();
}

template <typename T>
void SEAL::Cache<T>::pop()
{
    cache_items.erase(cache_items.begin());
}

template <typename T>
bool SEAL::Cache<T>::empty()
{
    return cache_items.empty();
}

} // namespace SEAL

#endif
