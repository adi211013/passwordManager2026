//
// Created by adi on 3/19/26.
//

#include "Crypto.h"
#include <sodium.h>
#include <stdexcept>
#include <jwt-cpp/traits/kazuho-picojson/defaults.h>

void Crypto::init()
{
    if (sodium_init()<0)
        throw std::runtime_error("nie udalo sie zaincjalizowac biblioteki libsodium");
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
            throw std::runtime_error("blad podczas hashowania");
    return std::string(hash);
}
std::string Crypto::generateToken(const std::string& login)
{
    auto token=jwt::create().set_issuer("passwordManager2026")
        .set_type("JWS")
        .set_payload_claim("login",jwt::claim(login))
        .set_issued_at(std::chrono::system_clock::now())
        .set_expires_at(std::chrono::system_clock::now()+std::chrono::hours{1})
        .sign(jwt::algorithm::hs256{"xddd"});
    return token;
}

bool Crypto::verifyPassword(const std::string& password, const std::string& hash)
{
    if (crypto_pwhash_str_verify(hash.c_str(),password.c_str(),password.length())==0)
        return true;
    return false;
}
