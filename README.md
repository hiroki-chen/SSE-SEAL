# SSE-SEAL
Searchable Encryption with Adjustable Leakage
<br>
* The codes are implementation for the SEAL model proposed by Demertzis et al. [SEAL: Attack Mitigation for Encrypted Databases via Adjustable Leakage](https://www.usenix.org/system/files/sec20fall_demertzis_prepub.pdf). The model is based on the oblivious ram model called PathORAM and the oblivious data structures (oblivious dictionary). The codes contain the implementation for Oblivious Data Structures (AVL Tree-based Dictionary) as well as a oblivious access controller, serving as a wrapper.
* The codes will need to communicate with a relational database (e.g., MySQL, PostgreSQL, openGauss...) to see if such an SSE scheme could be used in real-world scenarios. For the connection to the remote database, please be sure the relevant libararies (e.g., mysql-connector libs) are installed correctly.

# The structure of the model
For simplicity, this model is a client-server model. We construct a server which handles remote process call and also serves as the oblivious storage for the client. The server contains a relational database connector so the client could interact with the client. To search for an item, the client first looks up the position map locally and then sends the position and the bucket to the remote server (or fetch a bucket from the server).

# The structure of the project:
```
├── build                           Object files and binary executable files can be found here
│   ├── client
│   ├── crypto
│   ├── executable
│   ├── oram
│   ├── protos
│   ├── server
│   └── test
├── include
│   ├── cereal
│   ├── client
│   ├── crypto
│   ├── oram
│   ├── plog
│   ├── proto
│   └── server
├── input
├── log
├── protos                           Protobuf files can be found here.
└── src                              Source files can be found here.
    ├── client
    ├── crypto
    ├── oram
    ├── protos
    ├── server
    └── test
```

# Usage and Prerequisites
* GCC Compiler (or equivalent compilers) version 7.3.0 or higher;
* The compiler must support C++ 2017 standard (to enable the use of `std::string_view`)
* libcereal for object serialization to `std::string`; this is included in `include` directory. For more information, please refer to the [website](https://github.com/USCiLab/cereal)
* <del>mysql-connector-c++ 8.0 library</del>(not used) libpqxx for PostgreSQL-based openGauss database system.
  <br>
  For macOS users, you can install libpqxx by homebrew:
  ```shell
  brew install libpqxx
  ```
  which will automatically install all the required libraries and the headers in the homebrew path and link the libararies to /usr/local/lib.
  <br>
  For Ubuntu users, `apt-get` suffices.
  ```shell
  sudo apt-get install libpqxx-dev
  ```
* libsodium(github page is [here](https://github.com/jedisct1/libsodium))
  <br>
  Sodium library provides with cryptographically secure random generators, and it also gives us some public-key or private-key encryption interfaces.

* gRPC for remote process call. This is for the communication with the remote database. (We first issue request to remote processes rather than the remote database directly.)
  ```shell
  brew install grpc
  ```
  ```shell
  sudo apt-get install libgrpc-dev
  ```
* Dependencies on which gRPC relies. These may include `autoconf`, `automake`, `openssl`, `libtool`, `pkg-config`, etc. For more information, please refer to gRPC's official website: https://grpc.io.
* Make sure that gRPC plugin `grpc_cpp_plugin` for generating protobuf-based cpp files is correctly installed in the `grpc/bin/` path. If your gRPC dirctory is different with the one specified in `Makefile`, you should modify it manually.
* Make sure that your `pkgconfig` directory (this may located in `/usr/local/lib` directory) contains these three pkgconfig files: `libcrypto.pc`, `libssl.pc` and `openssl.pc`. If not, link the original `.pc` file (can be found in openssl directory) to the path or manually export the path:
```shell
export PKG_CONFIG_PATH=/path/to/openssl/lib/pkgconfig;
```

1. To compile the project, please first locate to the directory by
```shell
cd <path/to/SEAL/directory>;
```
2. Then make the project by
```shell
make all;
```
3. It will create the build directory, and object files and executable files are in the directory. You could run the server by
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

# Notice
* The implementation codes are still under construction.
* Robustness for the project is not guaranteed: use with care!
