package com.uid2.example;

import java.util.Base64;
import com.uid2.attestation.azure.AzureAttestationProvider;

public class Main {
	public static void main(String[] args) {
		try {
			AzureAttestationProvider provider = new AzureAttestationProvider();
			byte[] nonce = {0, 1, 2, 3, 4, 5, 6, 7};
			byte[] request = provider.getAttestationRequest(nonce);
			System.out.println("Quote Data (base64): " + Base64.getEncoder().encodeToString(request));
			System.out.println("Enclave Held Data (base64): " + Base64.getEncoder().encodeToString(nonce));
		} catch (Exception e) {
			System.out.println(e.getMessage());
		}
	}
}
