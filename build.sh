#!/bin/bash

set -ex

version=1.0.0
container_name=dev.docker.adsrvr.org/uid2/occlum-build
container_version=${version}.1
work_dir="${PWD}"
tty_arg=
if [[ -t 0 ]]; then
	tty_arg="-it"
fi

docker_run()
{
	docker run ${tty_arg} -w "${PWD}" -v "${work_dir}:${work_dir}" -u $(id -u ${USER}):$(id -g ${USER}) $container_name:dev "$@"
}

build_openssl()
{
	pushd target
	rm -rf openssl
	wget -O openssl.tar.gz https://github.com/openssl/openssl/archive/OpenSSL_1_1_1k.tar.gz
	tar xzf openssl.tar.gz
	mv openssl-OpenSSL_* openssl
	pushd openssl
	CC=occlum-gcc ./config \
		--prefix=${PWD}/target \
		--openssldir=${PWD}/target \
		--with-rand-seed=rdcpu \
		no-zlib no-async no-tests no-ssl2 no-ssl3 no-comp no-idea no-psk no-srp no-ec2m no-weak-ssl-ciphers no-dtls no-dtls1 no-shared
	docker_run make -j
	docker_run make install
	popd
	popd
}

# Container with build environment as required by occlum
docker build -f Dockerfile.build -t ${container_name}:dev -t ${container_name}:${container_version} .

mvn package
mvn deploy:deploy-file -Durl=file://$PWD/example/repo -DgroupId='com.uid2' -DartifactId='attestation-azure' -Dpackaging=jar -Dfile="./target/attestation-azure-${version}.jar" -Dversion=${version}

test -f target/openssl/target/lib/libcrypto.a || build_openssl

rm -rf target/bin
mkdir -p target/bin
CXXFLAGS="-ggdb -std=c++17 -I/opt/intel/sgxsdk/include -Itarget/openssl/target/include"
JNI_CXXFLAGS="-fPIC -Itarget/headers -I/usr/lib/jvm/java-11-openjdk-amd64/include -I/usr/lib/jvm/java-11-openjdk-amd64/include/linux"
LDFLAGS="-Ltarget/openssl/target/lib"
docker_run occlum-g++ ./src/cpp/{quote,quote_app}.cc -o target/bin/sgx_quote $CXXFLAGS $LDFLAGS -lcrypto
docker_run occlum-g++ ./src/cpp/{quote,quote_jni}.cc -shared -o target/bin/libazure-attestation.so $CXXFLAGS $JNI_CXXFLAGS $LDFLAGS -lcrypto -z noexecstack
