#include "quote.h"

#include "com_uid2_attestation_azure_AzureAttestation.h"

#include <cstdlib>
#include <iostream>

JNIEXPORT jobject JNICALL Java_com_uid2_attestation_azure_AzureAttestation_generateAttestationQuote(JNIEnv* env, jclass thisObject, jbyteArray nonce)
{
	jboolean isCopy = JNI_FALSE;
	const int nonceSize = env->GetArrayLength(nonce);
	jbyte* nonceData = env->GetByteArrayElements(nonce, &isCopy);
	std::vector<std::uint8_t> nonceVec((const std::uint8_t*)nonceData, (const std::uint8_t*)nonceData + nonceSize);
	env->ReleaseByteArrayElements(nonce, nonceData, 0);

	EnclaveQuote quote;
	std::string errorDetails;
	if(!GetSgxQuote(nonceVec, quote, errorDetails))
	{
		jclass exClass = env->FindClass("com/uid2/attestation/azure/EnclaveException");
		if(!exClass)
		{
			std::cerr << "Failed to find EnclaveException class" << std::endl;
			abort();
		}

		env->ThrowNew(exClass, errorDetails.c_str());
		return nullptr;
	}

	jclass resultClass = env->FindClass("com/uid2/attestation/azure/EnclaveQuoteResult");
	if(!resultClass)
	{
		std::cerr << "Failed to find EnclaveQuoteResult class" << std::endl;
		abort();
	}

	jmethodID method = env->GetMethodID(resultClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;SS[B)V");
	if(!method)
	{
		std::cerr << "Failed to find EnclaveQuoteResult constructor" << std::endl;
		abort();
	}

	jbyteArray quoteData = env->NewByteArray(quote.mQuoteData.size());
	env->SetByteArrayRegion(quoteData, 0, quote.mQuoteData.size(), (const jbyte*)quote.mQuoteData.data());

	return env->NewObject(resultClass, method,
		env->NewStringUTF(quote.mMrEnclaveHex.c_str()),
		env->NewStringUTF(quote.mMrSignerHex.c_str()),
		quote.mProductId, quote.mSecurityVersion,
		quoteData);
}
