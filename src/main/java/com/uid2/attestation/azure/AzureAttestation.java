package com.uid2.attestation.azure;

class AzureAttestation {
	static {
		System.loadLibrary("azure-attestation");
	}

	public static native EnclaveQuoteResult generateAttestationQuote(byte[] nonce) throws EnclaveException;
}
