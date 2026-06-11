#pragma once
#include <string>
#include <fstream>
#include <openssl/evp.h>
#include <cstdio>

namespace hashing {
	inline std::string sha256File(const std::string& path)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file) return "";

		EVP_MD_CTX* ctx = EVP_MD_CTX_new();
		if (!ctx) return "";

		EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);

		char buffer[1024];

		while (file.read(buffer, sizeof(buffer))) {
			EVP_DigestUpdate(ctx, buffer, file.gcount());
		}

		if (file.gcount() > 0) {
			EVP_DigestUpdate(ctx, buffer, file.gcount());
		}

		unsigned char hash[EVP_MAX_MD_SIZE];
		unsigned int hashLen = 0;

		EVP_DigestFinal_ex(ctx, hash, &hashLen);
		EVP_MD_CTX_free(ctx);

		std::string result;
		char buf[3];

		for (unsigned int i = 0; i < hashLen; i++)
		{
			snprintf(buf, sizeof(buf), "%02x", hash[i]);
			result += buf;
		}

		return result;
	}
}
