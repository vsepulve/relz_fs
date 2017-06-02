# README #

Prototype of a compressed filesystem for genomic data, using Relative Lempel-Ziv and FUSE library (https://github.com/libfuse/libfuse). It includes some test programs and the basic implementation of the filesystem. It also includes a basic implementation of a search system for the compressed files.

Dependecies
-------------------
```
tar -xzf utils/fuse-2.9.4.tar.gz
cd fuse-2.9.4/
./configure
make
sudo make install
```

The main programs of this version of the code are compress, decompress, and daemon_fuse_hybrid, created by the main Makefile.

Test
-------------------
```
make
./bin/compress data/ref.txt data/seq.txt data/seq.relz 100000 4 data/ref.bin 1 1
./bin/decompress data/ref.bin data/seq.relz data/seq_decomp.txt 100000
mkdir test test_real
./bin/daemon_fuse_hybrid -d ./test
cp data/seq.txt test/seq.relz
ls -l test/ test_real/
```

Note that in the example, the test/ folder is the mounting point (the virtual directory) while test_real/ is the folder that stores the compressed data. Note also that the system only compresses files if the extension of the target is "relz", so they will appear uncompressed in test/ while they are actually stored in compressed form in the real directory.
