#include <Client.h>
#include <utils.h>
#include <plog/Log.h>

#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>

#include <cstring>
#include <cassert>
#include <cmath>

// TODO: FIX RIGHT ROTATE.
void SEAL::Client::init_dummy_data()
{
    PLOG(plog::info) << "Initializing dummy data!";

    ODict::Node *test_root = new ODict::Node(0, -1);
    unsigned char *data = transform<unsigned char, ODict::Node>(test_root);
    oramAccessController.get()->oblivious_access(ORAM_ACCESS_WRITE, 0, data);
}

int SEAL::Client::find_pos_by_id(const int &id)
{
    PLOG(plog::info) << "Finding position_tag for " << id;

    int pos_tag = -1;
    for (auto node : clientCache)
    {
        if (node->left_id == id)
        {
            pos_tag = node->childrenPos[0].pos_tag;
            break;
        }
        else if (node->right_id == id)
        {
            pos_tag = node->childrenPos[1].pos_tag;
            break;
        }
    }

    PLOG(plog::info) << "Found position tag " << (pos_tag != -1 ? pos_tag : root_pos);
    return root_pos;
}

void SEAL::Client::ODS_access(ODict::Operation &op)
{
    ODict::Node *node = transform<ODict::Node, char>(op.data);

    const int id = op.id;

    switch (op.op)
    {
    case ORAM_ACCESS_READ:
    {
        PLOG(plog::info) << "Read node " << id << " from Oblivious Data Structure";
        read_count += 1;
        ODict::Node *ret = cache->get(id);
        if (ret != nullptr)
        {
            PLOG(plog::info) << "Found node in cache: " << ret->id;
            memcpy(node, ret, block_size);
        }
        else
        {
            PLOG(plog::info) << "Node not found in cache! Fetch from ORAM";
            cache_helper(id, node);
            node->old_tag = node->pos_tag;
            cache->put(id, node);
        }

        return;
    }

    case ORAM_ACCESS_WRITE:
    {
        PLOG(plog::info) << "Write node " << node->id << " to the Oblivious Data Structure";
        write_count += 1;
        ODict::Node *ret = cache->get(node->id);

        if (ret != nullptr)
        {
            PLOG(plog::info) << "Found node in cache: " << ret->id;
            memcpy(ret, node, block_size);
        }
        else
        {
            PLOG(plog::info) << "Node not found in cache! Fetch from ORAM";
            ODict::Node *tmp = new ODict::Node();
            cache_helper(id, tmp);
            memcpy(tmp, node, block_size);
            cache->put(id, tmp);
        }

        return;
    }

    // TODO.
    case ORAM_ACCESS_DELETE:
    {
        PLOG(plog::info) << "Delete node " << node->id << " from Oblivious Data Structure";
        ODict::Node *ret = cache->get(node->id);
        if (ret != nullptr)
        {
            PLOG(plog::info) << "Found node in cache: " << ret->id;
            // clientCache.erase(iter);
        }
        else
        {
            PLOG(plog::info) << "Node not found in cache! Fetch from ORAM";
            cache_helper(id, node);
        }

        return;
    }

    case ORAM_ACCESS_INSERT:
    {
        PLOG(plog::info) << "Insert a new node " << node->id << " into cache";
        cache->put(node->id, node);
        // clientCache.push_back(node);

        return;
    }
    }
}

void SEAL::Client::cache_helper(const int &id, ODict::Node *const ret)
{
    int pos = find_pos_by_id(id);
    unsigned char *buffer = new unsigned char[block_size];
    memcpy(buffer, new ODict::Node(id, pos), block_size);
    oramAccessController.get()->oblivious_access_direct(ORAM_ACCESS_READ, buffer);
    memcpy(ret, buffer, block_size);
}

void SEAL::Client::ODS_start()
{
    clientCache.clear();
}

void SEAL::Client::ODS_access(std::vector<ODict::Operation> &ops)
{
    for (auto op : ops)
    {
        ODS_access(op);
    }
}

void SEAL::Client::ODS_finalize(const int &pad_val)
{
    // Initialize a random device for randomly generating the postion tag for each node in the cache.
    std::map<int, int> position_tag;
    for (auto node : clientCache)
    {
        // Generate a random position tag.
        position_tag[node->id] = oramAccessController.get()->random_new_pos();

        if (node->old_tag == 0)
        {
            node->old_tag = position_tag[node->id];
        }

        node->pos_tag = position_tag[node->id];
        // std::cout << "created pos tag " << node->pos_tag << " for node " << node->id;
    }

    for (auto node : clientCache)
    {
        // Reallocate the position tag for each child node.
        node->childrenPos[0].pos_tag = position_tag[node->left_id];
        node->childrenPos[1].pos_tag = position_tag[node->right_id];
    }

    // Update rootPos based on rootId.
    root_pos = position_tag[root_id];

    // Pad the operation_cache.

    for (int i = pad_val - read_count; i <= pad_val; i++)
    {
        // dummy operation.
        unsigned char *data = new unsigned char[block_size];
        oramAccessController.get()->oblivious_access(ORAM_ACCESS_READ, 0, data);
    }

    // Evict the cache
    while (!clientCache.empty())
    {
        ODict::Node *node = clientCache.back();
        clientCache.pop_back();

        // We store a node as a char array.
        unsigned char *buffer = new unsigned char[block_size];
        memcpy(buffer, node, block_size);
        oramAccessController.get()->oblivious_access_direct(ORAM_ACCESS_WRITE, buffer);
    }

    // Pad add to padVal.
    for (int i = pad_val - write_count; i <= pad_val; i++)
    {
        // dummy operation.
        unsigned char *data = new unsigned char[block_size];
        oramAccessController.get()->oblivious_access(ORAM_ACCESS_WRITE, 0, data);
    }

    write_count = read_count = 0;
}

ODict::Node *SEAL::Client::find(const int &key)
{
    ODS_start();
    ODict::Node *node = find_priv(key, root_id);
    ODS_finalize((int)(3 * 1.44 * log(node_count)));

    return node;
}

std::map<int, ODict::Node *> SEAL::Client::find(const std::vector<int> &keys)
{
    ODS_start();

    std::map<int, ODict::Node *> ans;
    for (unsigned int i = 0; i < keys.size(); i++)
    {
        ODict::Node *node = find_priv(keys[i], root_id);
        ans[keys[i]] = node;
    }

    ODS_finalize(int(3 * 1.44 * log(node_count)));

    return ans;
}

ODict::Node *SEAL::Client::find_priv(const int &key, const int &root_id)
{
    ODict::Node *root = nullptr;
    if (root_id == 0)
    {
        return root;
    }

    char *data = new char[block_size];
    ODict::Operation *const op = new ODict::Operation(root_id, data, ORAM_ACCESS_READ);
    ODS_access(*op);

    root = transform<ODict::Node, char>(data);

    if (root->key == key)
    {
        return root;
    }
    else if (root->key < key)
    {
        return find_priv(key, root->right_id);
    }
    else
    {
        return find_priv(key, root->left_id);
    }
}

ODict::Node *SEAL::Client::insert(ODict::Node *node)
{
    ODict::Node *res = insert_priv(node, root_id);

    if (this->root_id == 0)
    {
        this->root_id = node->id;
    }

    return res;
}

void SEAL::Client::insert(const std::vector<ODict::Node *> &nodes)
{
    ODS_start();

    for (auto node : nodes)
    {
        PLOG(plog::info) << "Inserting " << node->id;
        this->root_id = insert(node)->id;
    }

    ODS_finalize((int)1.44 * 3 * log(node_count));
}

// we suppose that node->id is pre-set.
ODict::Node *SEAL::Client::insert_priv(ODict::Node *node, const int &root_id)
{
    // If there is yet no root, assign node as the root node.
    if (root_id == 0)
    {
        ODict::Node *ret = new ODict::Node();
        memcpy(ret, node, block_size);
        char *data = transform<char, ODict::Node>(ret);
        ODict::Operation *const op = new ODict::Operation(node->id, data, ORAM_ACCESS_INSERT); // insert into cache.
        ODS_access(*op);

        return ret;
    }

    // Read the current root node.
    char *data = new char[block_size];
    ODict::Operation *const op = new ODict::Operation(root_id, data, ORAM_ACCESS_READ);
    ODS_access(*op);
    ODict::Node *root = transform<ODict::Node, char>(data);

    PLOG(plog::info) << "Reading " << root->id;

    if (root->key < node->key)
    {
        ODict::Node *cur = insert_priv(node, root->right_id);
        root->right_id = cur->id;
        root->right_height = get_height(cur);

        char *buffer = transform<char, ODict::Node>(root);
        ODict::Operation *const op2 = new ODict::Operation(root_id, buffer, ORAM_ACCESS_WRITE);
        ODS_access(*op2);
    }
    else if (root->key > node->key)
    {
        ODict::Node *cur = insert_priv(node, root->left_id);
        root->left_id = cur->id;
        root->left_height = get_height(cur);

        char *buffer = transform<char, ODict::Node>(root);
        ODict::Operation *const op2 = new ODict::Operation(root_id, buffer, ORAM_ACCESS_WRITE);
        ODS_access(*op2);
    }

    return balance(root_id);
}

ODict::Node *SEAL::Client::balance(const int &root_id)
{
    char *data = new char[block_size];
    ODict::Operation *const op = new ODict::Operation(root_id, data, ORAM_ACCESS_READ);
    ODS_access(*op);
    ODict::Node *root = static_cast<ODict::Node *>(static_cast<void *>(op->data));

    int left_height = root->left_height, right_height = root->right_height;

    // LL / LR
    if (left_height - right_height > 1)
    {
        op->id = root->left_id;
        ODS_access(*op);
        ODict::Node *left = transform<ODict::Node, char>(op->data);
        // case 1: LL. Single Rotation. RightRotate.
        if (left->left_height >= left->right_height)
        {
            return right_rotate(root_id);
        }
        // case 2: LR. Double Rotation. LeftAndRightRotate.
        else
        {
            return left_right_rotate(root_id);
        }
    }
    else if (right_height - left_height > 1)
    {
        op->id = root->right_id;
        ODS_access(*op);
        ODict::Node *right = transform<ODict::Node, char>(op->data);
        // case 3: RR. Single Rotation. LeftRotate
        if (right->right_height >= right->left_height)
        {
            return left_rotate(root_id);
        }
        // case 4: RL. Double Rotation. RightAndLeftRotate.
        else
        {
            return right_left_rotate(root_id);
        }
    }

    return root;
}

ODict::Node *SEAL::Client::right_rotate(const int &root_id)
{
    PLOG(plog::info) << "Doing right rotating";

    char *data = new char[block_size];
    ODict::Operation *const op = new ODict::Operation(root_id, data, ORAM_ACCESS_READ);
    ODS_access(*op);
    ODict::Node *root = new ODict::Node();
    memcpy(root, op->data, block_size);

    op->id = root->left_id;
    ODS_access(*op);
    ODict::Node *left = new ODict::Node();
    memcpy(left, op->data, block_size); // Deep Copy! Or you will lose information.

    root->left_id = left->right_id;
    root->childrenPos[0].pos_tag = left->childrenPos[1].pos_tag;
    root->left_height = left->right_height;
    char *data2 = transform<char, ODict::Node>(root);
    ODict::Operation *const op2 = new ODict::Operation(root->id, data2, ORAM_ACCESS_WRITE);
    ODS_access(*op2);

    left->right_height = get_height(root);
    left->right_id = root_id;
    left->childrenPos[1].pos_tag = root->pos_tag;
    char *data3 = transform<char, ODict::Node>(left);
    ODict::Operation *const op3 = new ODict::Operation(left->id, data3, ORAM_ACCESS_WRITE);
    ODS_access(*op3);

    return left;
}

ODict::Node *SEAL::Client::left_rotate(const int &root_id)
{
    PLOG(plog::info) << "Doing left rotating";

    char *data = new char[block_size];
    ODict::Operation *const op = new ODict::Operation(root_id, data, ORAM_ACCESS_READ);
    ODS_access(*op);
    ODict::Node *root = new ODict::Node();
    memcpy(root, op->data, block_size);

    op->id = root->right_id;
    ODS_access(*op);
    ODict::Node *right = new ODict::Node();
    memcpy(right, op->data, block_size); // Deep Copy! Or you will lose information.

    root->right_id = right->left_id;
    root->childrenPos[1].pos_tag = right->childrenPos[0].pos_tag;
    root->right_height = right->left_height;
    char *data2 = transform<char, ODict::Node>(root);
    ODict::Operation *const op2 = new ODict::Operation(root->id, data2, ORAM_ACCESS_WRITE);
    ODS_access(*op2);

    right->left_height = get_height(root);
    right->left_id = root_id;
    right->childrenPos[0].pos_tag = root->pos_tag;
    char *data3 = transform<char, ODict::Node>(right);
    ODict::Operation *const op3 = new ODict::Operation(right->id, data3, ORAM_ACCESS_WRITE);
    ODS_access(*op3);

    return right;
}

ODict::Node *SEAL::Client::left_right_rotate(const int &root_id)
{
    PLOG(plog::info) << "Doing left right rotating";

    char *data = new char[block_size];
    ODict::Operation *const op = new ODict::Operation(root_id, data, ORAM_ACCESS_READ);
    ODS_access(*op);
    ODict::Node *root = new ODict::Node();
    memcpy(root, op->data, block_size);

    ODict::Node *left = left_rotate(root->left_id);
    root->left_id = left->id;
    root->left_height = get_height(left);
    char *data2 = new char[block_size];
    memcpy(data2, root, block_size);
    ODict::Operation *const op2 = new ODict::Operation(root->id, data2, ORAM_ACCESS_WRITE);
    ODS_access(*op2);

    return right_rotate(root_id);
}

ODict::Node *SEAL::Client::right_left_rotate(const int &root_id)
{
    PLOG(plog::info) << "Doing right left rotating";

    char *data = new char[block_size];
    ODict::Operation *const op = new ODict::Operation(root_id, data, ORAM_ACCESS_READ);
    ODS_access(*op);
    ODict::Node *root = new ODict::Node();
    memcpy(root, op->data, block_size);

    ODict::Node *right = right_rotate(root->right_id);
    root->right_id = right->id;
    root->right_height = get_height(right);
    char *data2 = new char[block_size];
    memcpy(data2, root, block_size);
    ODict::Operation *const op2 = new ODict::Operation(root->id, data2, ORAM_ACCESS_WRITE);
    ODS_access(*op2);

    return left_rotate(root_id);
}

/**
 * @brief A test function, you can add whatever you like.
 */
const char *SEAL::Client::add_node(const int &number)
{
    insert(create_test_cases(number));

    std::vector<int> keys;
    for (int i = 0; i < number; i++)
    {
        keys.push_back(i % 2 == 0 ? i : number - i);
    }

    auto begin = std::chrono::high_resolution_clock::now();
    auto ans = find(keys);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - begin;
    PLOG(plog::info) << "Read finished, time elapsed: " << elapsed.count() << "s";
    std::cout << "Read finished, time elapsed: " << elapsed.count() << "s" << std::endl;

    for (auto iter = ans.begin(); iter != ans.end(); iter++)
    {
        ODict::Node *res = iter->second;
        if (res != nullptr)
        {
            PLOG(plog::info) << "Read " << res->key << ": " << res->data << ", " << res->left_id << ", " << res->right_id;
        }
        else
        {
            PLOG(plog::info) << "Not found";
        }
    }

    return "ok";
}

std::vector<ODict::Node *> SEAL::Client::create_test_cases(const int &number)
{
    std::vector<ODict::Node *> vec;

    for (int i = 0; i < number; i++)
    {
        const std::string information = random_string(16);
        ODict::Node *const test_root = new ODict::Node();
        test_root->id = node_count++;
        test_root->key = (i % 2 == 0 ? test_root->id : number - test_root->id);
        test_root->data = new char[information.size() + 1];
        strcpy(test_root->data, information.c_str());
        vec.push_back(test_root);
    }

    return vec;
}

SEAL::Client::Client(const int &bucket_size, const int &block_number, const int &block_size, const int &odict_size, const size_t &max_size)
    : block_size(block_size), odict_size(odict_size), block_number(block_number),
      root_id(0), root_pos(-1),
      oramAccessController(std::unique_ptr<OramAccessController>(new OramAccessController(bucket_size, block_number, block_size))),
      node_count(1), read_count(0), write_count(0), cache(new Cache<ODict::Node>(max_size, oramAccessController.get()))
{
    init_dummy_data();
}