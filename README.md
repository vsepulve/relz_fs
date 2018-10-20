# README #

Prototype of a compressed filesystem for genomic data, using Relative Lempel-Ziv and FUSE library (https://github.com/libfuse/libfuse). This prototype is a proof of concept to show that relative compression specifically designed for genomic data can be applied in a filesystem. It was designed to use RLZ for sequence data, but the capability of separating metadata text from fasta files and compressing it with LZMA was later added. It includes some test programs and the basic implementation of the filesystem.

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

Preparing the Reference Text
-------------------
In cases where there is no appropriate sequence to be used as a reference (for example, when dealing with a collection of reads), this program can extract part of the text from the collection to build a reference. It receives a text input, an output to write the reference, and the expected size for the reference in bytes.
An example could be:
```
./bin/prepare_reference_text ../data/reads_cut.fa ../data/reads_ref.txt 300000
./bin/build_reference ../data/reads_ref.txt ../data/reads_ref.bin 4
./bin/compress ../data/reads_ref.bin ../data/reads_cut.fa ../data/reads_cut.relz 100000 4 1
```
That would prepare the text data/reads\_ref.txt as a reference, and compress the data data/reads\_cut.txt to data/reads\_cut.relz, building the binary reference in data/reads\_ref.bin.

Building (indexing) the Reference
-------------------
The first step to use the system is the indexing of the reference text to generate binary references used for the rest of the processes. The program bin/build\_reference performs the indexing, and its arguments are the reference text file, the output to write the binary and indexed reference, and the number of threads used for construction.
An example could be:
```
./bin/build_reference ../data/tair10_cut.fa ../data/tair10_cut.bin 4
```

Compressing
-------------------
The bin/compress program is responsible for both the compressing test sequences and constructing the index of a reference text. The generated binary reference can then be reused without having to build it again. 
The parameters of the program include the reference text, the input sequence, the output compressed file, the blocksize, the number of threads to be used both in the index construction and the compression, the output to write the indexed reference, a flag indicating whether the reference should be built, and a flag indicating whether metadata should be included in the output compressed file.
An example could be:
```
./bin/compress ../data/tair10_cut.bin ../data/thaliana_cut.fa ../data/thaliana_cut.relz 100000 4 1
```

Decompression
-------------------
The bin/decompress program receives the indexed reference used for the compression, a compressed file as input, a path to write the output file, and the size of the buffer used for writing.
An example, followed by a verification, could be:
```
./bin/decompress ../data/tair10_cut.bin ../data/thaliana_cut.relz ../data/thaliana_output.fa 100000
diff ../data/thaliana_cut.fa ../data/thaliana_output.fa
```

Sampling and Compression
-------------------
The program bin/compress\_rr performs the sampling of the reference's index and compresses a sequence with the newly generated sampled reference.
The parameters of the program are the indexed (full) reference, the output for the sampled reference, the sequence to be compressed, the output for the compressed sequence, the blocksize, number of threads, the samplig rate and the flag that defines if metadata should be considered.
An example could be:
```
./bin/compress_rr ../data/tair10_cut.bin ../data/tair10_cut_rr10.bin ../data/thaliana_cut.fa ../data/thaliana_cut_rr10.relz 100000 4 10 1
./bin/compress_rr ../data/reads_ref.bin ../data/reads_ref_rr10.bin ../data/reads_cut.fa ../data/reads_cut_rr10.relz 100000 4 10 1
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
The configuration file daemon\_fuse\_local.config explains in comments the parameters used. The main part of the configuration is the definition of references associated with different subdirectories in the virutal filesystem. First, the number of references is defined, and then each one of them is associated with a path. The first references is associated to the "/" path, relative to the mounting point (so, in the examples, "/" actually refers to the "./test/" directory).
The system uses the references of the most precise subdirectory defined in the references association block from the configuration. If a subdirectory has no reference associated with it, the one from its parent directory is used (so the first reference, associated with "/" will be used if no other reference is defined).

For the client/server version, the program bin/daemon\_server should be used and configured in the same way as a local filesystem (using bin/daemon\_fuse\_hybrid), and the bin/daemon\_fuse\_client serves as the client filesystem, configured with a very similar daemon\_fuse\_client.config configuration file. In that case, the variables "host" and "port" defined in both configuration files are used for the communication, so they must match.

















