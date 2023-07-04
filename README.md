# SSE-SEAL
Searchable Encryption with Adjustable Leakage

* This repo contains the code for the reference implementation of the paper [SEAL: Attack Mitigation for Encrypted Databases via Adjustable Leakage](https://www.usenix.org/system/files/sec20fall_demertzis_prepub.pdf). SEAL is a protocol of searchable encryption which is built atop the oblivious data structures (oblivious dictionary) proposed by Xiao Shaun Wang et al. at CCS'15. The codebase contains an Oblivious Data Structure (AVL Tree-based Dictionary) as well as a oblivious access controller, serving as a wrapper.

## The layout of the implementation
For simplicity, the code implements the single-client-single-server model. We construct a server which handles remote process call (RPC) and also serves as the oblivious storage for the client. To search for an element on the server, the client first looks up the position map locally and then sends the position and the bucket to the remote server (or fetch a bucket from the server).

## Compatibility

The code was tested on macOS 10.14.6 High Sierra with Clang 10 and Ubuntu 20.04 LTS with GCC 9.3.

## Prerequisites
* GCC Compiler (or equivalent compilers) version 7.3.0 or higher that supports C++17;
* libsodium (see [here](https://github.com/jedisct1/libsodium)). The library provides with cryptographically secure random generators, and it also gives us some public-key or private-key encryption interfaces. You can install via:

  ```shell
  ./configure;
  make && make check && sudo make install;
  ```
* CMake:

  ```shell
  sudo apt-get install cmake;
  ```
* gRPC for remote process call. It is recommended that gRPC is installed via git clone.
  ```shell
  git clone https://github.com/grpc/grpc.git;
  cd grpc;
  git submodule update --init;
  mkdir -p cmake/build;
  cd cmake/build;
  cmake ../..;
  make && sudo make install;
  ```
* Dependencies for gRPC. These include `autoconf`, `automake`, `openssl`, `libtool`, `pkg-config`, etc. For more information, please refer to gRPC's official website: https://grpc.io.
* Make sure that gRPC plugin `grpc_cpp_plugin` for generating protobuf-based cpp files is correctly installed in `$PATH`. If your gRPC dirctory is different with the one specified in `Makefile`, you should modify it manually.
* Make sure that your `pkgconfig` directory (this may located in `/usr/local/lib` directory) contains these three pkgconfig files: `libcrypto.pc`, `libssl.pc` and `openssl.pc`. If not, link the original `.pc` file (can be found in openssl directory) to the path or manually export the path:
  ```shell
  export PKG_CONFIG_PATH=/path/to/openssl/lib/pkgconfig;
  ```
  For Ubuntu users, you can install ssl via `apt-get`:
  ```shell
  sudo apt-get install libssl-dev;
  ```

# Compilation
`make -j` should work. It will create the build directory, and object files and executable files are in the directory. You could run the server by

```shell
./build/executable/server;
```

and run the client by

```shell
./build/executable/client;
```

If you have any modification on the file, just remake the project:
```shell
make client;
make server;
```

# SSL Key Generation
If there is a need to generate the ssl key on your own, one can execute the following commands (make sure that openssl is correctly installed):
```shell
export mypass=<password>;
openssl genrsa -passout pass:$pass -des3 -out server.key 4096;
openssl req -passin pass:$mypass -new -key server.key -out server.csr -subj  "/C=US/ST=CA/L=SanFrancisco/O=Google/OU=youtube/CN=localhost";
openssl x509 -req -passin pass:$mypass -days 365 -in server.csr -signkey server.key -set_serial 01 -out server.crt;
openssl rsa -passin pass:$mypass -in server.key -out server.key
rm -rf server.csr;
mv server.crt server.key keys;
```
