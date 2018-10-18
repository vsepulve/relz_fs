# README #

Prototype of a compressed filesystem for genomic data, using Relative Lempel-Ziv and FUSE library (https://github.com/libfuse/libfuse). This prototype also makes usage of LZMA library. It includes some test programs and the basic implementation of the filesystem.

Dependecies
-------------------
To install FUSE
```
tar -xzf utils/fuse-2.9.4.tar.gz
cd fuse-2.9.4/
./configure
make
sudo make install
```
If necessary, to install XZ for LZMA
```
tar -xzf utils/xz-5.2.4.tar.gz
cd xz-5.2.4/
./configure
make
sudo make install
```

The main programs of this version of the code are prepare\_reference\_text, compress, decompress, and daemon\_fuse\_hybrid.

Compile
-------------------
```
mkdir build
cd build
cmake ..
make
```

Preparing the Reference
-------------------
In cases where there is no appropriate sequence to be used as a reference (for example, when dealing with a collection of reads), this program can extract part of the text from the collection to build a reference. It receives a text input, an output to write the reference, and the expected size for the reference in bytes.
An example could be:
```
./bin/prepare_reference_text ../data/reads_cut.fa ../data/reads_ref.txt 300000
./bin/compress ../data/reads_ref.txt ../data/reads_cut.fa ../data/reads_cut.relz 100000 4 ../data/reads_ref.bin 1 1
```
That would prepare the text data/reads\_ref.txt as a reference, and compress the data data/reads\_cut.txt to data/reads\_cut.relz, building the binary reference in data/reads\_ref.bin.

Compressing
-------------------
The bin/compress program is responsible for both the compressing test sequences and constructing the index of a reference text. The generated binary reference can then be reused without having to build it again. 
The parameters of the program include the reference text, the input sequence, the output compressed file, the blocksize, the number of threads to be used both in the index construction and the compression, the output to write the indexed reference, a flag indicating whether the reference should be built, and a flag indicating whether metadata should be included in the output compressed file.
An example could be:
```
./bin/compress ../data/tair10_cut.fa ../data/thaliana_cut.fa ../data/thaliana_cut.relz 100000 4 ../data/tair10_cut.bin 1 1
```
On the other hand, if the reference is already built, we can skip that step with a command like:
```
./bin/compress -- ../data/thaliana_cut.fa ../data/thaliana_cut.relz 100000 4 ../data/tair10_cut.bin 0 1
```

Decompression
-------------------
The bin/decompress program receives the indexed reference used for the compression, a compressed file as input, a path to write the output file, and the size of the buffer used for writing.
An example, followed by a verification, could be:
```
./bin/decompress ../data/tair10_cut.bin ../data/thaliana_cut.relz ../data/thaliana_output.fa 100000
diff ../data/thaliana_cut.fa ../data/thaliana_output.fa
```

Filesystem Daemon
-------------------
The bin/daemon\_fuse\_hybrid program works as a local file system. It receives optional arguments for fuse, the mounting point for the filesystem, and a configuration file for the daemon. In the following example we use "-d" and "-o big\_writes" as fuse arguments (debug, and the possibility of making large writings), we define "./test" as the mounting point, and use daemon\_fuse\_local.config to configure the daemon. First we create two directories, one to serve as the virtual mounting point, and another to store the real compressed files.
```
mkdir test test_real
./bin/daemon_fuse_hybrid -d -o big_writes ./test ../daemon_fuse_local.config
```
While the daemon is running, the virtual directory can be used as follows:
```
cp ../data/thaliana_cut.fa test/thaliana_cut.relz
ls -l test/ test_real/
mkdir test/reads
cp ../data/reads_cut.fa test/reads/reads_cut.relz
ls -l test/reads/ test_real/reads/
```




















