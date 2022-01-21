#include "quote.h"

#include <iostream>

int main()
{
	std::vector<uint8_t> nonce{0, 1, 2, 3};
	std::string errors;
	EnclaveQuote quote;
	if(!GetSgxQuote(nonce, quote, errors))
	{
		std::cerr << "Failed to get SGX quote. Errors:\n" << errors << std::endl;
		return 1;
	}

	std::cout << "MRENCLAVE   = " << quote.mMrEnclaveHex << "\n";
	std::cout << "MRSIGNER    = " << quote.mMrSignerHex << "\n";
	std::cout << "PRODUCT ID  = " << quote.mProductId << "\n";
	std::cout << "SEC VERSION = " << quote.mSecurityVersion << "\n";
	std::cout << "QUOTE SIZE  = " << quote.mQuoteData.size() << "\n";

	return 0;
}
