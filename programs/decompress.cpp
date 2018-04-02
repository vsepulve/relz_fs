#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <iostream>

#include <map>

#include "ReferenceIndex.h"
#include "ReferenceIndexBasic.h"
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
		cout<<"\nUsage: decompress serialized_reference compressed_file output_text buffer_size\n";
		cout<<"The program reads the files \"compressed_file\" and the text from \"serialized_reference\".\n";
		cout<<"It writes the text in \"output_text\" using a decompression block of length \"buffer_size\".\n";
		cout<<"\n";
		return 0;
	}
	
	const char *serialized_reference = argv[1];
	const char *compressed_file = argv[2];
	const char *output_text = argv[3];
	unsigned int buffer_size = atoi(argv[4]);
	
	cout<<"Start (reference \""<<serialized_reference<<"\", compressed \""<<compressed_file<<"\", output \""<<output_text<<"\", block_size "<<buffer_size<<")\n";
	
	cout<<"Loading reference text\n";
	NanoTimer timer;
	
	char *reference_text = ReferenceIndexBasic::loadText(serialized_reference);
	
	//Usando el Compressor
	CompressorSingleBuffer compressor(compressed_file, NULL, new DecoderBlocksRelz(reference_text));
	
	timer.reset();
	compressor.decompress(output_text, buffer_size);
	cout<<"Decompression finished in "<<timer.getMilisec()<<" ms\n";
	
	delete [] reference_text;
	
	cout<<"End\n";
	
}

















