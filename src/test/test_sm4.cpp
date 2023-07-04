#include <crypto/sm4.h>

#include <iostream>

int main(int argc, const char** argv)
{
    std::string plain;
    std::cin >> plain;
    std::string ciphertext = encrypt_SM4_EBC(plain, "123");
    std::string plaintext = decrypt_SM4_EBC(ciphertext, "123");

    std::cout << ciphertext << ", " << plaintext << std::endl;
}