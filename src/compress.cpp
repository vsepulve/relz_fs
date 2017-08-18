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
		cout<<"\nUsage: compress reference_text sequence_text output block_size n_threads serialized_reference build_reference? use_metadata?\n";
		cout<<"If build_reference? == 1, the program builds it (creating serialized_reference). In any other case, the program reads serialized_reference (and omits reference_text).\n";
		cout<<"If use_metadata? == 1, the program stores data about case and newlines. In any other case, the program OMITS case and newlines.\n";
		cout<<"\n";
		return 0;
	}
	
	const char *reference_text = argv[1];
	const char *sequence_text = argv[2];
	const char *output = argv[3];
	unsigned int block_size = atoi(argv[4]);
	unsigned int n_threads = atoi(argv[5]);
	const char *serialized_reference = argv[6];
	bool build_reference = (atoi(argv[7]) == 1);
	bool use_metadata = (atoi(argv[8]) == 1);
	//Dejo la base de golomb por defecto (2^6)
	
	cout<<"Start (reference \""<<reference_text<<"\", sequence \""<<sequence_text<<"\", output \""<<output<<"\")\n";
	
	BitsUtils utils;
	NanoTimer timer;
	
	ReferenceIndex *reference = NULL;
//	TextFilter *filter = new TextFilterBasic();
	TextFilter *filter = new TextFilterFull();
	
	if(build_reference){
	
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
		
//		//Agregacion de N's y del alfabeto valido completo
		unsigned int ns_len = 1024;
		vector<char> alphabet;
		filter->getAlphabet( &alphabet );
		
		text = new char[text_len + ns_len + alphabet.size() + 1];
//		text = new char[text_len + ns_len + 1];
		
		//Agregacion de N's y del alfabeto valido completo
		for(unsigned int i = 0; i < ns_len; ++i){
			text[text_size++] = 'N';
		}
		for(unsigned int i = 0; i < alphabet.size(); ++i){
			text[text_size++] = alphabet[i];
		}
		
		//Carga del texto de la referencia (metodo especial para cargar bases solamente)
		text_size += filter->readReference(reference_text, text + text_size);
		
		cout<<"Building Reference with "<<text_size<<" chars\n";
		timer.reset();
		reference = new ReferenceIndexBasic(text, n_threads);
		cout<<"Built in "<<timer.getMilisec()<<" ms\n";
		
		timer.reset();
		reference->save(serialized_reference);
		cout<<"Saved in "<<timer.getMilisec()<<" ms\n";
		
		delete [] text;
	}
	else{
		cout<<"Loading Reference\n";
		timer.reset();
		
		reference = new ReferenceIndexBasic();
		reference->load(serialized_reference);
		cout<<"Reference loaded in "<<timer.getMilisec()<<" ms\n";
	}
	
	//Usando el Compressor
	CompressorSingleBuffer compressor(
		output, 
		new CoderBlocksRelz(reference), 
		new DecoderBlocksRelz(reference->getText()), 
		filter
		);
		
	timer.reset();
	compressor.compress(sequence_text, n_threads, block_size, use_metadata);
	cout<<"Compression finished in "<<timer.getMilisec()<<" ms\n";
	
	delete reference;
	
	cout<<"End\n";
	
}

















