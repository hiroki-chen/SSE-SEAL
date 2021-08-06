#include <utils.h>

#include <cmath>
#include <cstring>
#include <regex>
#include <sodium.h>
#include <stdexcept>

void copy(ODict::Node* const dst, const ODict::Node* const src)
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

int get_height(const ODict::Node* const node)
{
    return node == nullptr ? 0 : MAX(node->left_height, node->right_height) + 1;
}

std::string random_string(const int& len, std::string_view secret_key)
{
    std::string tmp_s;
    std::string alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    unsigned int random_value[len];
    randombytes_buf_deterministic(random_value, sizeof(random_value), (unsigned char*)std::to_string(randombytes_random()).c_str());

    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i)
        tmp_s += alphanum[random_value[i] % alphanum.size()];

    return tmp_s;
}

OramInterface::Operation deduct_operation(OramAccessOp op)
{
    switch (op) {
    case ORAM_ACCESS_READ:
        return OramInterface::Operation::READ;
    case ORAM_ACCESS_WRITE:
        return OramInterface::Operation::WRITE;
    default:
        throw std::runtime_error("This operation is not supported!");
    }
}

std::vector<std::string> split(const std::string& input, const std::string& regex)
{
    // passing -1 as the submatch index parameter performs splitting
    std::regex re(regex);
    std::sregex_token_iterator
        first { input.begin(), input.end(), re, -1 },
        last;
    return { first, last };
}

/**
 * The randombytes_buf() function fills size bytes starting at buf with an unpredictable sequence of bytes.
 * The randombytes_buf_deterministic function stores size bytes into buf indistinguishable
 * from random bytes without knowing seed.
 * For a given seed, this function will always output the same sequence. size can be up to 2^38 (256 GB).
 */
std::vector<unsigned int>
pseudo_random_permutation(const size_t& value_size,
    std::string_view secret_key)
{
    /* Calculate the base digit number. log2 n */
    unsigned int base = std::ceil(log(value_size) / log(2));
    unsigned int interval = (unsigned int)(pow(2, base));
    unsigned int permutation[interval];
    std::vector<unsigned int> ans;
    for (unsigned int i = 0; i < interval; i++) {
        ans.push_back(i);
    }

    randombytes_buf_deterministic(permutation, sizeof(permutation), (unsigned char*)secret_key.data());

    for (unsigned int i = 0; i < interval - 1; i++) {
        // j := random integer such that i ≤ j < n
        unsigned int j = i + permutation[i] % (interval - i);
        std::swap(ans[i], ans[j]);
    }

    return ans;
}

std::pair<unsigned int, unsigned int> get_bits(const unsigned int& base, const unsigned int& number, const unsigned int& alpha)
{
    unsigned int most = number >> (base - alpha);
    unsigned int rest = number & (unsigned int)(pow(2, base - alpha) - 1);
    std::cout << most << "," << rest << std::endl;
    return { most, rest };
}

/*
std::string encrypt_message(std::string_view key,
    std::string_view message,
    const unsigned char* nonce)
{
    unsigned int ciphertext_length = crypto_secretbox_MACBYTES + message.size();
    unsigned char ciphertext[ciphertext_length];
    crypto_secretbox_easy(
        ciphertext, (unsigned char*)message.data(), message.size(), nonce, (unsigned char*)key.data());

    return std::string((char*)(ciphertext), ciphertext_length);
}

std::string decrypt_message(
    std::string_view key,
    std::string_view ciphertext,
    const unsigned char* nonce,
    const size_t& raw_length)
{
        unsigned int ciphertext_length = crypto_secretbox_MACBYTES + message.size();
        unsigned char nonce[crypto_secretbox_NONCEBYTES];
        unsigned char plaintext[raw_length];
        crypto_secretbox_easy(
            plaintext, (unsigned char*)ciphertext.data(), ciphertext.size(), nonce, (unsigned char*)key.data());

        return std::string((char*)(plaintext), raw_length);
}
*/