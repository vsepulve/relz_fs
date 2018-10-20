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
	
	if(argc != 5){
		cout<<"\nUsage: build_reference reference_text binary_output n_threads force_ranges?\n";
		cout<<"If force_ranges == 1, it assumes the reference and sequences are of the same type, matching a positional range, increasing compression.\n";
		cout<<"\n";
		return 0;
	}
	
	const char *reference_text = argv[1];
	const char *serialized_reference = argv[2];
	unsigned int n_threads = atoi(argv[3]);
	bool force_ranges = (atoi(argv[4]) == 1);
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
		cerr << "Error reading Reference\n";
		return 0;
	}
	reader.seekg (0, reader.end);
	unsigned int text_len = reader.tellg();
	reader.seekg (0, reader.beg);
	reader.close();
	
	// Adding N's and the full alphabet
	unsigned int ns_len = 1024;
	vector<char> alphabet;
	filter->getAlphabet( &alphabet );
	unsigned int type_flags = 0;
	
	if( force_ranges && text_len > 256*1024*1024 ){
		unsigned int range = 1024*1024;
		if( text_len < 512*1024*1024 ){
			range *= 128;
			type_flags = 1;
			ns_len = 128;
		}
		else if( text_len < 1024*1024*1024 ){
			range *= 256;
			type_flags = 2;
			ns_len = 256;
		}
		else if( text_len < 2ull*1024*1024*1024 ){
			range *= 512;
			type_flags = 3;
			ns_len = 512;
		}
		else{
			range *= 1024;
			type_flags = 4;
			ns_len = 1024;
		}
		cout << "Positional Range of " << range << " bytes, type_flags: " << type_flags << "\n";
		
		unsigned int n_adds = text_len / range;
		if( n_adds * range < text_len ){
			++n_adds;
		}
		
		cout << "Adding " << n_adds << " voc extensions (" << text_len << " / " << range << ")\n";
		char *text_original = new char[text_len + 1];
		filter->readReferenceFull(reference_text, text_original);
		text = new char[text_len + 1 + n_adds * (ns_len + alphabet.size())];
		
		unsigned int read_pos = 0;
		unsigned int total_copied = 0;
		while( total_copied < text_len ){
			unsigned int copy_len = range;
			if( total_copied + copy_len > text_len ){
				copy_len = text_len - total_copied;
			}
			for(unsigned int i = 0; i < ns_len; ++i){
				text[total_copied++] = 'N';
			}
			for(unsigned int i = 0; i < alphabet.size(); ++i){
				text[total_copied++] = alphabet[i];
			}
			memcpy(text + total_copied, text_original + read_pos, copy_len);
			total_copied += copy_len;
			read_pos += copy_len;
		}
		
		delete [] text_original;
		text_size = total_copied;
		text[text_size] = 0;
	}
	else{
		// text = new char[text_len + 1];
		text = new char[text_len + ns_len + alphabet.size() + 1];
		
		for(unsigned int i = 0; i < ns_len; ++i){
			text[text_size++] = 'N';
		}
		for(unsigned int i = 0; i < alphabet.size(); ++i){
			text[text_size++] = alphabet[i];
		}
		
		// The first version only loads ACGT
		// text_size += filter->readReference(reference_text, text + text_size);
		text_size += filter->readReferenceFull(reference_text, text + text_size);
	}
	
	cout << "Building Reference with " << text_size << " chars\n";
	timer.reset();
	reference = new ReferenceIndexBasic(text, n_threads, type_flags);
	cout<<"Built in "<<timer.getMilisec()<<" ms\n";
	timer.reset();
	
	reference->save(serialized_reference);
	cout<<"Saved in "<<timer.getMilisec()<<" ms\n";
	
	delete [] text;
	delete reference;
	
	cout << "End\n";
	
}

















