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
		cout<<"It the writes the text in \"output_text\" using a decompression block of length \"buffer_size\".\n";
		cout<<"\n";
		return 0;
	}
	
	const char *referencia_serializada = argv[1];
	const char *entrada_comprimida = argv[2];
	const char *nombre_salida = argv[3];
	unsigned int buffer_size = atoi(argv[4]);
	
	cout<<"Inicio (referencia \""<<referencia_serializada<<"\", comprimida \""<<entrada_comprimida<<"\", salida \""<<nombre_salida<<"\", block_size "<<buffer_size<<")\n";
	
	cout<<"Cargando texto referencia\n";
	NanoTimer timer;
	
	char *reference_text = ReferenceIndexBasic::loadText(referencia_serializada);
	
	//Usando el Compressor
	CompressorSingleBuffer compressor(entrada_comprimida, NULL, new DecoderBlocksRelz(reference_text));
	
	timer.reset();
	compressor.decompress(nombre_salida, buffer_size);
	cout<<"Descompresion terminada en "<<timer.getMilisec()<<" ms\n";
	
	cout<<"Borrando decoder...\n";
	delete [] reference_text;
	
	cout<<"Fin\n";
	
}

















