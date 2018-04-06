#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <iostream>

#include <map>

#include "NanoTimer.h"
#include "DecoderBlocks.h"
//#include "DecoderBlocksCompact.h"
#include "ReferenceIndexRR.h"
#include "CompactSequence.h"

using namespace std;

int main(int argc, char* argv[]){

	if(argc != 5){
		cout<<"\nModo de Uso: descomprimir_compact referencia_rr entrada_comprimida salida_texto largo_linea\n";
		cout<<"Lee archivo \"entrada_comprimida\" y el texto de la \"referencia_serializada\"\n";
		cout<<"Escribe el texto en \"salida_texto\" con \"largo_linea\" chars por linea\n";
		cout<<"\n";
		return 0;
	}
	
	const char *referencia_serializada = argv[1];
	const char *entrada_comprimida = argv[2];
	const char *nombre_salida = argv[3];
	unsigned int largo_linea = atoi(argv[4]);
	
	cout<<"Inicio (referencia \""<<referencia_serializada<<"\", comprimida \""<<entrada_comprimida<<"\", salida \""<<nombre_salida<<"\", largo de linea "<<largo_linea<<")\n";
	
	cout<<"Cargando texto referencia\n";
	NanoTimer timer;

	ReferenceIndex *referencia = new ReferenceIndexRR();
	referencia->load(referencia_serializada);
	
	//FALTA LA BASE DE GOLOMB
	//Por ahora SOLO funciona con base 2^6 (la por defecto)
	DecoderBlocks *decoder = new DecoderBlocks(entrada_comprimida, referencia->getText());
	decoder->decodeFullText(nombre_salida, largo_linea);
	
	cout<<"Borrando decoder...\n";
	delete decoder;
	delete referencia;
	
	cout<<"Fin\n";
	
}

















