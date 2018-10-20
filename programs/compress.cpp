#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <map>
#include <vector>

#include "ReferenceIndexBasic.h"
#include "ReferenceIndexRR.h"
#include "NanoTimer.h"
#include "BitsUtils.h"

#include "CoderBlocks.h"
#include "CoderBlocksRelz.h"

#include "DecoderBlocks.h"
#include "DecoderBlocksRelz.h"

#include "Compressor.h"
#include "CompressorSingleBuffer.h"

#include "TextFilter.h"
#include "TextFilterBasic.h"
#include "TextFilterFull.h"

using namespace std;

int main(int argc, char* argv[]){
	
	if(argc != 7){
		cout<<"\nUsage: compress serialized_reference sequence_text output block_size n_threads use_metadata?\n";
		cout<<"If use_metadata? == 1, the program stores data about case and newlines. In any other case, the program OMITS case and newlines.\n";
		cout<<"\n";
		return 0;
	}
	
	const char *serialized_reference = argv[1];
	const char *sequence_text = argv[2];
	const char *output = argv[3];
	unsigned int block_size = atoi(argv[4]);
	unsigned int n_threads = atoi(argv[5]);
	bool use_metadata = (atoi(argv[6]) == 1);
	//Dejo la base de golomb por defecto (2^6)
	
	cout << "Start (reference \"" << serialized_reference << "\", sequence \"" << sequence_text << "\", output \"" << output << "\")\n";
	
	BitsUtils utils;
	NanoTimer timer;
	
	cout<<"Loading Reference\n";
	ReferenceIndex *reference = new ReferenceIndexBasic();
	reference->load(serialized_reference);
	cout<<"Reference loaded in "<<timer.getMilisec()<<" ms\n";
	
	//Usando el Compressor
	CompressorSingleBuffer compressor(
		output, 
		new CoderBlocksRelz(reference), 
		new DecoderBlocksRelz(reference->getText()), 
		new TextFilterFull()
		);
		
	timer.reset();
	compressor.compress(sequence_text, n_threads, block_size, use_metadata);
	cout<<"Compression finished in "<<timer.getMilisec()<<" ms\n";
	
	delete reference;
	
	cout<<"End\n";
	
}

















