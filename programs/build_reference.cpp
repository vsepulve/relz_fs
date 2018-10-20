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
	
	if(argc != 4){
		cout<<"\nUsage: build_reference reference_text binary_output n_threads\n";
		cout<<"\n";
		return 0;
	}
	
	const char *reference_text = argv[1];
	const char *serialized_reference = argv[2];
	unsigned int n_threads = atoi(argv[3]);
	//Dejo la base de golomb por defecto (2^6)
	
	cout << "Start (reference \"" << reference_text << "\", output \"" << serialized_reference << "\")\n";
	
	BitsUtils utils;
	NanoTimer timer;
	
	ReferenceIndex *reference = NULL;
	TextFilter *filter = new TextFilterFull();
	
	char *text = NULL;
	unsigned int text_size = 0;
	
	fstream reader(reference_text, fstream::in);
	if( ! reader.good() ){
		cerr<<"Error reading Reference\n";
		return 0;
	}
	reader.seekg (0, reader.end);
	unsigned int text_len = reader.tellg();
	reader.seekg (0, reader.beg);
	reader.close();
	
	text = new char[text_len + 1];
	
	// The first version only loads ACGT
//	text_size += filter->readReference(reference_text, text + text_size);
	text_size += filter->readReferenceFull(reference_text, text + text_size);
	
	// Additional text could be added (N's, the full alphabet, etc)
	// ...
	
	cout << "Building Reference with " << text_size << " chars\n";
	timer.reset();
	reference = new ReferenceIndexBasic(text, n_threads);
	cout<<"Built in "<<timer.getMilisec()<<" ms\n";
	timer.reset();
	
	reference->save(serialized_reference);
	cout<<"Saved in "<<timer.getMilisec()<<" ms\n";
	
	delete [] text;
	delete reference;
	
	cout << "End\n";
	
}

















