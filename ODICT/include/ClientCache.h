#ifndef CLIENT_CACHE_H
#define CLIENT_CACHE_H

#include <cstddef>
#include <deque>
#include <unordered_map>

#include <plog/Log.h>
#include <OramAccessController.h>

namespace SEAL
{
    template <typename T>
    class Cache
    {
    private:
        const size_t max_size;

        std::unordered_map<int, T> cache_items;

        std::deque<int> lru_table;

        OramAccessController *const oramAccessController;

    public:
        Cache(const size_t &max_size, OramAccessController *const oramAccessController);

        T *get(const int &id);

        T *put(const int &id, const T *const item);
    };

    template <typename T>
    SEAL::Cache<T>::Cache(const size_t &max_size, OramAccessController *const oramAccessController)
        : max_size(max_size), oramAccessController(oramAccessController)
    {
        PLOG(plog::info) << "Client Cache started.";
    }

    template <typename T>
    T *SEAL::Cache<T>::get(const int &id)
    {
        auto iter = cache_items.find(id);

        if (iter == cache_items.end())
        {
            return nullptr;
        }
        else
        {
            auto it = std::find_if(lru_table.begin(), lru_table.end(), [id](const int &item_id)
                                   { return item_id == id; });

            int item_id = *it;
            lru_table.erase(it);
            lru_table.push_front(item_id);
            return &(iter->second);
        }
    }

    template <typename T>
    T *SEAL::Cache<T>::put(const int &id, const T *const item)
    {
        auto iter = cache_items.find(id);
        auto it = std::find_if(lru_table.begin(), lru_table.end(), [id](const int &item_id)
                               { return item_id == id; });
        lru_table.push_front(id);

        if (iter != cache_items.end() && it != lru_table.end())
        {
            cache_items.erase(iter);
            lru_table.erase(it);
        }

        cache_items[id] = *item;

        // evict the item
        if (lru_table.size() > max_size)
        {
            int back = lru_table.back();
            lru_table.pop_back();
            iter = cache_items.find(back);

            T *ret = &iter->second;
            cache_items.erase(iter);

            // Write to the oram.
            if (std::is_same_v<T, ODict::Node>)
            {
                ret->pos_tag = oramAccessController->random_new_pos();

                for (auto iter = cache_items.begin(); iter != cache_items.end(); iter++)
                {
                    if (iter->second.left_id == ret->id)
                    {
                        iter->second.childrenPos[0].pos_tag = ret->pos_tag;
                    }
                    else if (iter->second.right_id == ret->id)
                    {
                        iter->second.childrenPos[1].pos_tag = ret->pos_tag;
                    }
                }

                unsigned char *buffer = new unsigned char[sizeof(T)];
                memcpy(buffer, ret, sizeof(T));
                oramAccessController->oblivious_access_direct(ORAM_ACCESS_WRITE, buffer);
            }

            return ret;
        }

        return nullptr;
    }

} // namespace SEAL

#endif
