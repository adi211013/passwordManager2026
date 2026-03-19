//
// Created by adi on 3/19/26.
//

#include "Crypto.h"
#include <sodium.h>
#include <stdexcept>
#include <jwt-cpp/traits/kazuho-picojson/defaults.h>
const unsigned char SERVER_MASTER_KEY[crypto_aead_xchacha20poly1305_ietf_KEYBYTES] = "123456789123456789123456789xddd";
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

std::string Crypto::verifyToken(const std::string& token)
{
    try
    {
        auto decoded =jwt::decode(token);
        auto verifier=jwt::verify().allow_algorithm(jwt::algorithm::hs256{"xddd"}).with_issuer("passwordManager2026");
        verifier.verify(decoded);
        return decoded.get_payload_claim("login").as_string();
    }catch (const std::exception&e )
    {
        std::cerr<<"Blad weryfikacji"<<e.what()<<std::endl;
        return "";
    }
}

#include <vector>
#include <cstring>


std::string Crypto::encrypt(const std::string& plaintext) {
    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
    randombytes_buf(nonce, sizeof nonce);

    std::vector<unsigned char> ciphertext(plaintext.length() + crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned long long ciphertext_len;

    crypto_aead_xchacha20poly1305_ietf_encrypt(
        ciphertext.data(), &ciphertext_len,
        reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.length(),
        nullptr, 0, nullptr, nonce, SERVER_MASTER_KEY
    );

    std::vector<unsigned char> combined;
    combined.insert(combined.end(), nonce, nonce + sizeof nonce);
    combined.insert(combined.end(), ciphertext.data(), ciphertext.data() + ciphertext_len);

    std::vector<char> hex_output(combined.size() * 2 + 1);
    sodium_bin2hex(hex_output.data(), hex_output.size(), combined.data(), combined.size());

    return std::string(hex_output.data());
}

std::string Crypto::decrypt(const std::string& ciphertextHex) {
    std::vector<unsigned char> combined(ciphertextHex.length() / 2);
    if (sodium_hex2bin(combined.data(), combined.capacity(), ciphertextHex.c_str(), ciphertextHex.length(), nullptr, nullptr, nullptr) != 0) {
        throw std::runtime_error("Blad dekodowania HEX!");
    }

    if (combined.size() < crypto_aead_xchacha20poly1305_ietf_NPUBBYTES + crypto_aead_xchacha20poly1305_ietf_ABYTES) {
        throw std::runtime_error("Zaszyfrowany tekst jest za krotki!");
    }

    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
    std::memcpy(nonce, combined.data(), sizeof nonce);

    const unsigned char* c = combined.data() + sizeof nonce;
    unsigned long long clen = combined.size() - sizeof nonce;

    std::vector<unsigned char> decrypted(clen - crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned long long decrypted_len;

    if (crypto_aead_xchacha20poly1305_ietf_decrypt(
        decrypted.data(), &decrypted_len,
        nullptr, c, clen,
        nullptr, 0, nonce, SERVER_MASTER_KEY) != 0) {
        throw std::runtime_error("Falszowanie danych! Certyfikat MAC nie pasuje.");
    }

    return std::string(reinterpret_cast<char*>(decrypted.data()), decrypted_len);
}