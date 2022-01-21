# Azure SGX Attestation Support for UID2 Services

The repo provides helper facilities for obtaining an attestation request on Azure/SGX platform.

# Building

The build is only possible on Azure confidential computing virtual machines. It has been tested with a Standard_DC8_v2 instance.
Only Ubuntu 18.04 LTS (server) is supported at this stage.

Once you get a build VM, make sure it has all the necessary software by running:

```
./setup_build_vm.sh
```

Before running the build, make sure you obtain a GitHub personal access token with permissions to clone UID2 repos.

To build the project:

```
export GITHUB_ACCESS_TOKEN=<your-token>
./setup_dependencies.sh
./build.sh
```

# Testing

Running `./test.sh` will build and execute the example inside an SGX container.

# Example

You can verify that the build worked successfully by trying out the basic example:

```
cd example
./build.sh
```

If all goes well this should print out base64-encoded attestation quote similar to what would be required
to pass attestation with the UID2 Core service.
