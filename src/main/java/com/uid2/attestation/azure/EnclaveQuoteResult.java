package com.uid2.attestation.azure;

class EnclaveQuoteResult {
	public String mrEnclave;
	public String mrSigner;
	public short productId;
	public short securityVersion;
	public byte[] quote;

	public EnclaveQuoteResult(String mrEnclave, String mrSigner, short productId, short securityVersion, byte[] quote) {
		this.mrEnclave = mrEnclave;
		this.mrSigner = mrSigner;
		this.productId = productId;
		this.securityVersion = securityVersion;
		this.quote = quote;
	}
}
