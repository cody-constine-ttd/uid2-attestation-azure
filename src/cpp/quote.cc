#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <sgx_quote.h>
#include <sgx_quote_3.h>
#include <sgx_report.h>

#include <openssl/sha.h>
#include <cstring>
#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "quote.h"

using namespace std::chrono_literals;

static std::stringstream gLogger;

template<typename... Ts>
void LOG(const Ts&... args)
{
	(gLogger << ... << args) << std::endl;
}

#define SGXIOC_GET_DCAP_QUOTE_SIZE  _IOR('s', 7, uint32_t)
#define SGXIOC_GEN_QUOTE            _IOWR('s', 8, EnclaveQuoteArgs)

const char* const kSgxDeviceName = "/dev/sgx";

struct EnclaveQuoteArgs
{
	sgx_report_data_t*     mReportData; // input
	uint32_t*              mQuoteSize;  // input/output
	union
	{
		uint8_t*       mAsBuf;
		sgx_quote_t*   mAsQuote;
		sgx_quote3_t*  mAsQuote3;
	}                      mQuote;      // output
};

static std::string GetErrorString(int err)
{
	char buf[1024];
	strerror_r(err, buf, sizeof(buf));
	return buf;
}

static int SgxDeviceIoctl(unsigned long request, void* data)
{
	const int fd = open(kSgxDeviceName, O_RDONLY);
	if(fd < 0)
	{
		LOG("Failed to open ", kSgxDeviceName, ": ", errno, ", ", GetErrorString(errno));
		return -1;
	}

	int ret = 0;
	int count = 3;
	while(count--)
	{
		if(ioctl(fd, request, data) == 0)
		{
			break;
		}
		else if (errno != EAGAIN)
		{
			LOG("ioctl() failed for ", kSgxDeviceName, ": ", errno, ", ", GetErrorString(errno));
			ret = -1;
			break;
		}
		else
		{
			LOG(kSgxDeviceName, " is temporarily busy. Try again after 1s.");
			std::this_thread::sleep_for(1000ms);
		}
	}

	close(fd);
	return ret;
}

static int SgxDeviceGetQuote(EnclaveQuoteArgs* args)
{
	if(!args->mQuote.mAsBuf || (args->mQuoteSize == nullptr) || (*args->mQuoteSize == 0))
	{
		LOG("Invalid quote buffer or len");
		return -1;
	}

	return SgxDeviceIoctl(SGXIOC_GEN_QUOTE, args);
}

static int SgxDeviceGetQuoteSize(uint32_t* quoteSize)
{
	return SgxDeviceIoctl(SGXIOC_GET_DCAP_QUOTE_SIZE, quoteSize);
}

static const char* FormatHex(char* out, const uint8_t* data, size_t size)
{
	for(size_t i = 0; i < size; ++i)
	{
		sprintf(&out[i*2], "%02X", data[i]);
	}
	out[size*2+1] = '\0';
	return out;
}

static void Sha256(uint8_t* out, const uint8_t* data, uint32_t size)
{
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, data, size);
	SHA256_Final(out, &sha256);
}

static bool GetSgxQuoteImpl(const std::vector<std::uint8_t>& nonce, EnclaveQuote& out)
{
	EnclaveQuoteArgs quoteArgs;
	bzero(&quoteArgs, sizeof(quoteArgs));

	uint32_t quoteSize = 0;
	if(SgxDeviceGetQuoteSize(&quoteSize) != 0)
	{
		LOG("Failed to get quote size");
		return false;
	}

	out.mQuoteData.resize(quoteSize);
	quoteArgs.mQuote.mAsBuf = out.mQuoteData.data();
	quoteArgs.mQuoteSize = &quoteSize;

	sgx_report_data_t hash;
	bzero(&hash, sizeof(hash));
	Sha256(hash.d, nonce.data(), nonce.size());
	quoteArgs.mReportData = &hash;

	if(SgxDeviceGetQuote(&quoteArgs) != 0)
	{
		LOG("Failed to get quote");
		return false;
	}

	sgx_quote_t* quote = quoteArgs.mQuote.mAsQuote;
	sgx_report_body_t& report_body = quote->report_body;

	char hexBuffer[1024*64];
	out.mMrEnclaveHex = FormatHex(hexBuffer, report_body.mr_enclave.m, SGX_HASH_SIZE);
	out.mMrSignerHex = FormatHex(hexBuffer, report_body.mr_signer.m, SGX_HASH_SIZE);
	out.mProductId = (uint16_t)report_body.isv_prod_id;
	out.mSecurityVersion = (uint16_t)report_body.isv_svn;

	return true;
}

bool GetSgxQuote(const std::vector<std::uint8_t>& nonce, EnclaveQuote& out, std::string& errorDetails)
{
	gLogger = std::stringstream();

	const bool res = GetSgxQuoteImpl(nonce, out);
	if(!res)
	{
		errorDetails = gLogger.str();
	}

	return res;
}
