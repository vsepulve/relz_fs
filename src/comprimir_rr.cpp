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

	if(argc != 8){
		cout<<"\nModo de Uso: comprimir_rr referencia_serializada entrada_comprimir nombre_salida block_size n_threads distancia usar_metadata?\n";
		cout<<"Utiliza una referencia RR, tomando 1 / distancia posiciones\n";
		cout<<"Si la distancia es 0, carga directamente la referencia compactada\n";
		cout<<"Si usar_metadata? == 1, los usa. De otro modo, DESCARTA lowcase y newlines.\n";
		cout<<"\n";
		return 0;
	}
	
	const char *referencia_serializada = argv[1];
	const char *entrada_comprimir = argv[2];
	const char *nombre_salida = argv[3];
	unsigned int block_size = atoi(argv[4]);
	unsigned int n_threads = atoi(argv[5]);
	unsigned int distancia = atoi(argv[6]);
	bool use_metadata = (atoi(argv[7]) == 1);
	//Dejo la base de golomb por defecto (2^6)
	
	cout<<"Inicio (referencia \""<<referencia_serializada<<"\", procesar \""<<entrada_comprimir<<"\", salida \""<<nombre_salida<<"\")\n";
	
	BitsUtils utils;
	NanoTimer timer;
	
	ReferenceIndex *referencia = NULL;
	
	cout<<"Cargando Referencia\n";
	if(distancia > 0){
		referencia = new ReferenceIndexRR(referencia_serializada, distancia);
		referencia->save("ref_tmp.bin");
	}
	else{
		referencia = new ReferenceIndexRR();
		referencia->load(referencia_serializada);
	}
	cout<<"Referencia Cargada en "<<timer.getMilisec()<<" ms\n";
	
	//Usando el Compressor
	CompressorSingleBuffer compressor(
		nombre_salida, 
		new CoderBlocksRelz(referencia), 
		new DecoderBlocksRelz(referencia->getText()), 
		new TextFilterFull()
		);
		
	timer.reset();
	compressor.compress(entrada_comprimir, n_threads, block_size, use_metadata);
	cout<<"Compresion terminada en "<<timer.getMilisec()<<" ms\n";
	
	cout<<"Borrando\n";
	delete referencia;
	
	cout<<"Fin\n";
	
}

















