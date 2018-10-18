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






Test (from build/ directory)
-------------------
```
./bin/compress ../data/ref.txt ../data/seq.txt ../data/seq.relz 100000 4 ../data/ref.bin 1 1
./bin/decompress ../data/ref.bin ../data/seq.relz ../data/seq_decomp.txt 100000
mkdir test test_real
./bin/daemon_fuse_hybrid -d ./test ../daemon_fuse.config
cp ../data/seq.txt test/seq.relz
ls -l test/ test_real/
```

Note that in the example, the test/ folder is the mounting point (the virtual directory) while test_real/ is the folder that stores the compressed data. Note also that the system only compresses files if the extension of the target is "relz", so they will appear uncompressed in test/ while they are actually stored in compressed form in the real directory.
