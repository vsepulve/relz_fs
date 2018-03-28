SRCDIR = src
BINDIR = bin
LIBDIR = lib
DATADIR = data
TYPE ?= -O3
# CPPFLAGS = $(TYPE) -Wall -I./includes/ -L./lib/ -std=c++0x -lrt
### Notar el -pthread para g++ 5.1 (no era necesario en el 4.6, bastaba con -std=c++0x)
CPPFLAGS = $(TYPE) -Wall -I./includes/ -L./lib/ -std=c++0x -lrt -pthread
### Flags para FUSE (dependiendo del linker deben ir DESPUES del .cpp)
# FUSEFLAGS = `pkg-config fuse --cflags --libs`
### pkg-config Puede fallar en ciertso sistemas (por ejemplo, en WT)
### Se puede agregar el path al .pc a mano, pero por el momento prefiero dejarlo explicito
### Notar que la ruta "/usr/local/include/fuse" puede ser diferente
FUSEFLAGS = -D_FILE_OFFSET_BITS=64 -I/usr/local/include/fuse  -pthread -L/usr/local/lib -lfuse
### Por alguna razon, en WT el sistema busca libfuse en "/lib", no en "/usr/local/lib/"
### Podria haber ajustado variables de sesion, pero preferi copiar la biblioteca
### Es decir, hice "> sudo cp /usr/local/lib/libfuse.so.2.9.4 /lib/libfuse.so.2"

CPP = g++

CREATED_OBJECTS = MutationsFile.o  NanoTimer.o ComparatorUtils.o BitsUtils.o BytesReader.o
CREATED_OBJECTS += CommunicationUtils.o ServerThreads.o RemoteFunctions.o
CREATED_OBJECTS += Communicator.o ServerConnection.o ClientReception.o
CREATED_OBJECTS += ReferenceIndex.o ReferenceIndexBasic.o ReferenceIndexBasicTest.o
CREATED_OBJECTS += ReferenceIndexRR.o ReferenceIndexRRCompact.o 
CREATED_OBJECTS += PositionsCoderBlocksBytes.o LengthsCoderBlocksBytes.o DecoderBlocksRelzBytes.o 
CREATED_OBJECTS += PositionsCoderBlocks.o LengthsCoderBlocks.o  
CREATED_OBJECTS += BlockHeaders.o BlockHeadersRelz.o 
CREATED_OBJECTS += BlockHeadersFactory.o Metadata.o 
CREATED_OBJECTS += CoderBlocks.o CoderBlocksRelz.o 
CREATED_OBJECTS += DecoderBlocks.o DecoderBlocksRelz.o 
CREATED_OBJECTS += Compressor.o CompressorSingleBuffer.o Recompressor.o 
CREATED_OBJECTS += CompactSequence.o
CREATED_OBJECTS += ConcurrentLogger.o LocalBuffer.o
CREATED_OBJECTS += CheckUser.o dirList.o FilesTree.o
CREATED_OBJECTS += TextFilter.o TextFilterBasic.o TextFilterFull.o TextFilterNoNL.o
CREATED_OBJECTS += RelzIndexPrimary.o RelzIndexSecondary.o LinearPatternMatching.o
#CREATED_OBJECTS += DecoderBlocksCompact.o 
#CREATED_OBJECTS += DecoderBlocksBytes.o RecoderBlocks.o

OBJECTS = $(CREATED_OBJECTS) 
OBJECTS_LOC = $(OBJECTS:%.o=$(LIBDIR)/%.o)

.PHONY: all clean

all: directories compress decompress daemon_fuse_hybrid

directories: 
	mkdir -p bin/ lib/ 

%.o: 
	make -C src $@
	
%: $(OBJECTS)
	$(CPP) $(OBJECTS_LOC) $(CPPFLAGS) -o $(BINDIR)/$@ $(SRCDIR)/$@.cpp $(FUSEFLAGS)
	
clean:
	rm -f bin/* lib/*
	
