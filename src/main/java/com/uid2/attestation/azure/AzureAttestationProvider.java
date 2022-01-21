package com.uid2.attestation.azure;

import com.uid2.enclave.AttestationException;
import com.uid2.enclave.IAttestationProvider;
import java.util.Arrays;

public class AzureAttestationProvider implements IAttestationProvider {
	@Override
	public byte[] getAttestationRequest(byte[] publicKey) throws AttestationException {
		try {
			byte nonce[] = publicKey;
			EnclaveQuoteResult result = AzureAttestation.generateAttestationQuote(nonce);
			return result.quote;
		} catch (EnclaveException e) {
			throw new AttestationException(e);
		}
	}
}
