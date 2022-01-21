#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct EnclaveQuote
{
	std::string mMrEnclaveHex;
	std::string mMrSignerHex;
	std::uint16_t mProductId;
	std::uint16_t mSecurityVersion;
	std::vector<std::uint8_t> mQuoteData;
};

extern bool GetSgxQuote(const std::vector<std::uint8_t>& nonce, EnclaveQuote& out, std::string& errorDetails);
