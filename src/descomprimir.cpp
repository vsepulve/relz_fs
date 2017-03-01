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
		cout<<"\nModo de Uso: descomprimir referencia_serializada entrada_comprimida salida_texto buffer_size\n";
		cout<<"Lee archivo \"entrada_comprimida\" y el texto de la \"referencia_serializada\"\n";
		cout<<"Escribe el texto en \"salida_texto\" usando un bloque de descompresion de largo \"buffer_size\"\n";
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

















