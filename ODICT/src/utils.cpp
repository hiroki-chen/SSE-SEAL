#include <utils.h>

#include <cstring>
#include <random>
#include <regex>
#include <stdexcept>

void copy(ODict::Node *const dst, const ODict::Node *const src)
{
    dst->data = new char[strlen(src->data) + 1];
    strcpy(dst->data, src->data);
    dst->left_height = src->left_height;
    dst->right_height = src->right_height;
    dst->childrenPos[0] = src->childrenPos[0];
    dst->childrenPos[1] = src->childrenPos[1];
    dst->id = src->id;
    dst->left_id = src->left_id;
    dst->right_id = src->right_id;
    dst->pos_tag = src->pos_tag;
}

int get_height(const ODict::Node *const node)
{
    return MAX(node->left_height, node->right_height) + 1;
}

std::string random_string(const int &len)
{
    std::string tmp_s;
    std::string alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::random_device rd;
    std::mt19937 e(rd());
    std::uniform_int_distribution<int> dist(0, alphanum.size() - 1);

    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i)
        tmp_s += alphanum[dist(e)];

    return tmp_s;
}

OramInterface::Operation deduct_operation(OramAccessOp op)
{
    switch (op)
    {
    case ORAM_ACCESS_READ:
        return OramInterface::Operation::READ;
    case ORAM_ACCESS_WRITE:
        return OramInterface::Operation::WRITE;
    default:
        throw std::runtime_error("This operation is not supported!");
    }
}

std::vector<std::string> split(const std::string &input, const std::string &regex)
{
    // passing -1 as the submatch index parameter performs splitting
    std::regex re(regex);
    std::sregex_token_iterator
        first{input.begin(), input.end(), re, -1},
        last;
    return {first, last};
}