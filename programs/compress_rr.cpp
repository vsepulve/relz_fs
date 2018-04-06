#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <map>
#include <vector>

#include "ReferenceIndexBasic.h"
#include "ReferenceIndexBasicTest.h"
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

	if(argc != 9){
		cout<<"\nUsage: compress_rr serialized_reference compacted_reference sequence_text output block_size n_threads distance use_metadata?\n";
		cout<<"This program uses the Reference-RR, with 1 / distance positions from the SA\n";
		cout<<"If distance == 0, it loads directly an already reduced reference from compacted_reference\n";
		cout<<"In any other case, this program creates a new reduced reference and stores it in compacted_reference\n";
		cout<<"If use_metadata? == 1, the program stores data about case and newlines. In any other case, the program OMITS case and newlines.\n";
		cout<<"\n";
		return 0;
		
	}
	
	const char *serialized_reference = argv[1];
	const char *compacted_reference = argv[2];
	const char *sequence_text = argv[3];
	const char *output = argv[4];
	unsigned int block_size = atoi(argv[5]);
	unsigned int n_threads = atoi(argv[6]);
	unsigned int distance = atoi(argv[7]);
	bool use_metadata = (atoi(argv[8]) == 1);
	//Dejo la base de golomb por defecto (2^6)
	
	cout<<"Start (reference \""<<serialized_reference<<"\", procesar \""<<sequence_text<<"\", salida \""<<output<<"\")\n";
	
	BitsUtils utils;
	NanoTimer timer;
	
	ReferenceIndex *reference = NULL;
	
	cout<<"Cargando reference\n";
	if(distance > 0){
		reference = new ReferenceIndexRR(serialized_reference, distance);
		reference->save(compacted_reference);
	}
	else{
		reference = new ReferenceIndexRR();
		reference->load(compacted_reference);
	}
	cout<<"Reference Loaded in "<<timer.getMilisec()<<" ms\n";
	
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

















