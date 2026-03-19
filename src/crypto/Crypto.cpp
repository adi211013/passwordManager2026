//
// Created by adi on 3/19/26.
//

#include "Crypto.h"
#include <sodium.h>
#include <stdexcept>

void Crypto::init()
{
    if (sodium_init()<0)
    {
        throw std::runtime_error("nie udalo sie zaincjalizowac biblioteki libsodium");
    }
}

std::string Crypto::hashPassword(const std::string& password)
{
    char hash[crypto_pwhash_strbytes()];

    if (crypto_pwhash_str(
        hash,
        password.c_str(),
        password.length(),
        crypto_pwhash_OPSLIMIT_INTERACTIVE,
        crypto_pwhash_MEMLIMIT_INTERACTIVE)!=0)
    {
        throw std::runtime_error("blad podczas hashowania");
    }
    return std::string(hash);
}
