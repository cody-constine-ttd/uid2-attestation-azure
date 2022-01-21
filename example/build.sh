#!/bin/bash

set -ex

container_name=dev.docker.adsrvr.org/uid2/occlum-build:dev
occlum_glibc=/opt/occlum/glibc/lib
work_dir="${PWD}"
tty_arg=
if [[ -t 0 ]]; then
	tty_arg="-it"
fi

docker_run()
{
	docker run ${tty_arg} -w "${PWD}" -v "${work_dir}:${work_dir}" -u $(id -u ${USER}):$(id -g ${USER}) $container_name "$@"
}

docker_run_root()
{
	docker run ${tty_arg} -w "${PWD}" -v "${work_dir}:${work_dir}" --device /dev/sgx/enclave --device /dev/sgx/provision $container_name "$@"
}

mvn package

docker_run_root rm -rf target/occlum
mkdir -p target/occlum
pushd target/occlum

docker_run occlum init
mkdir -p image/usr/lib
mkdir -p image/app
mkdir -p image/$occlum_glibc

cp $work_dir/target/example-*-jar-with-dependencies.jar image/app/example.jar
cp $work_dir/../target/bin/libazure-attestation.so image/usr/lib
cp $work_dir/../target/bin/sgx_quote image/bin

docker_run cp $occlum_glibc/libdl.so.2 image/$occlum_glibc
docker_run cp $occlum_glibc/librt.so.1 image/$occlum_glibc
docker_run cp $occlum_glibc/libm.so.6 image/$occlum_glibc
docker_run cp -r /etc/java-11-openjdk image/etc
docker_run cp -r /usr/lib/jvm/java-11-openjdk-amd64 image/usr/lib/jvm
docker_run cp /lib/x86_64-linux-gnu/libz.so.1 image/lib

cp $work_dir/Occlum.json .

docker_run_root occlum build
docker_run_root bash -c "export AZDCAP_DEBUG_LOG_LEVEL=FATAL; /opt/occlum/start_aesm.sh; occlum run /bin/sgx_quote"
docker_run_root bash -c "export AZDCAP_DEBUG_LOG_LEVEL=FATAL; /opt/occlum/start_aesm.sh; occlum run /usr/lib/jvm/bin/java -XX:MaxHeapSize=512m -XX:InitialHeapSize=512m -XX:CompressedClassSpaceSize=256m -XX:MaxMetaspaceSize=256m -jar /app/example.jar"

popd
