# SSE-SEAL
Searchable Encryption with Adjustable Leakage
<br>
* The codes are implementation for the SEAL model proposed by Demertzis et al. [SEAL: Attack Mitigation for Encrypted Databases via Adjustable Leakage](https://www.usenix.org/system/files/sec20fall_demertzis_prepub.pdf). The model is based on the oblivious ram model called PathORAM and the oblivious data structures (oblivious dictionary). The codes contain the implementation for Oblivious Data Structures (AVL Tree-based Dictionary) as well as a oblivious access controller, serving as a wrapper.
* The codes will need to communicate with a relational database (e.g., MySQL, PostgreSQL, openGauss...) to see if such an SSE scheme could be used in real-world scenarios. For the connection to the remote database, please be sure the relevant libararies (e.g., mysql-connector libs) are installed correctly.

# Usage
* GCC Compiler (or equivalent compilers) version 7.3.0 or higher;
* -std=c++17
* mysql-connector-c++ 8.0 library
* libsodium(github page is [here](https://github.com/jedisct1/libsodium))

1. To compile the project, please first locate to the directory by
```shell
cd <path/to/SEAL/directory>;
```
2. Then make the project by
```shell
make all;
```
3. It will create the build directory, and object files and executable files are in the directory. You could run the program by
```shell
./build/run
```

# Notice
* The implementation codes are still under construction.
