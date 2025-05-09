#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <optional>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rc4.h>

#include "apie/singleton/threadsafe_singleton.h"

namespace apie {
namespace crypto {

class Utility {
public:
	static std::string hex(const std::string& src);
	static std::string hex(const uint8_t* src, size_t srcLen);

	static std::string sha1(const std::string& src);
	static std::string md5(const std::string& src);

	static std::string decode_rc4(const std::string& sharedKey, const std::string& data);
	static std::string encode_rc4(const std::string& sharedKey, const std::string& data);
};

class RSAUtility
{
public:
	~RSAUtility();

	bool init(const std::string &pubFile, const std::string &priFile, std::string &errInfo);

	bool encrypt(const std::string& plainMsg, std::string *encryptedMsg);
	bool decrypt(const std::string& encryptedMsg, std::string* decryptedMsg);
	std::optional<int> rsaSize();

	static bool encryptByPub(RSA* ptrPubKey, const std::string& plainMsg, std::string *encryptedMsg);

private:
	FILE *m_pub_file = nullptr;
	RSA *m_pub_key = nullptr;

	FILE *m_pri_file = nullptr;
	RSA *m_pri_key = nullptr;
};

using RSAUtilitySingleton = ThreadSafeSingleton<RSAUtility>;

} // namespace Crypto
} // namespace APie
