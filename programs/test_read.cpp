#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <random>

#include <map>

#include "ReferenceIndex.h"
#include "ReferenceFactory.h"
#include "NanoTimer.h"

#include "CoderBlocks.h"
#include "CoderBlocksRelz.h"

#include "DecoderBlocks.h"
#include "DecoderBlocksRelz.h"

#include "Compressor.h"
#include "CompressorSingleBuffer.h"

using namespace std;

int main(int argc, char* argv[]){

	if(argc != 5){
		cout << "\nUsage: test_read serialized_reference compressed_file length_read n_tests\n";
		cout << "The program reads the files \"compressed_file\" and the text from \"serialized_reference\".\n";
		cout << "It writes the text in \"output_text\" using a decompression block of length \"buffer_size\".\n";
		cout << "\n";
		return 0;
	}
	
	const char *serialized_reference = argv[1];
	const char *compressed_file = argv[2];
	unsigned int len_read = atoi(argv[3]);
	unsigned int n_tests = atoi(argv[4]);
	
	cout << "Start (reference \"" << serialized_reference << "\", compressed \"" << compressed_file << "\", len_read " << len_read << ", n_tests " << n_tests << ")\n";
	
	cout << "Loading reference text\n";
	NanoTimer timer;
	
	char *reference_text = ReferenceFactory::loadText(serialized_reference);
	
	cout << "Testing text[0]: " << string(reference_text, 20) << "\n";
	cout << "Testing text[100]: " << string(reference_text + 100, 20) << "\n";
	cout << "Testing text[1000]: " << string(reference_text + 1000, 20) << "\n";
	cout << "Testing text[10000]: " << string(reference_text + 10000, 20) << "\n";
	
	//Usando el Compressor
	CompressorSingleBuffer compressor(compressed_file, NULL, new DecoderBlocksRelz(reference_text));
	
//	compressor.decompress(output_text, buffer_size);
//	cout << "Decompression finished in "<<timer.getMilisec()<<" ms\n";
	
	unsigned long long size = compressor.getTextSize();
	char *buff = new char[len_read + 1];
	
	random_device dev;
	mt19937 gen(dev());
	uniform_int_distribution<unsigned long long> pos_dist(0, size - 1);
	unsigned long long total_bytes = 0;
	
	timer.reset();
	cout << "Starting Read Test\n";
	for(unsigned int i = 0; i < n_tests; ++i){
		unsigned long long pos = pos_dist(gen);
//		cout << "test " << i << " - pos: " << pos << "\n";
		unsigned long long bytes = compressor.read(pos, len_read, buff);
		total_bytes += bytes;
//		cout << "test " << i << " - bytes: " << bytes << "\n";
	}
	double milisec = timer.getMilisec();
	cout << "Read Speed: " << (total_bytes / ((milisec/1000) * 1024*1024)) << " MB/s (total_bytes: " << total_bytes << ", milisec: " << milisec << ")\n";
	
	delete [] buff;
	delete [] reference_text;
	
	cout << "End\n";
	
}

