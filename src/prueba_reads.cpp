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
		cout<<"\nModo de Uso: prueba_reads referencia_serializada entrada_comprimida read_size sequential?\n";
		cout<<"\n";
		return 0;
	}
	
	const char *referencia_serializada = argv[1];
	const char *entrada_comprimida = argv[2];
	unsigned int line_size = atoi(argv[3]);
	bool sequential = (atoi(argv[4]) == 1);
	
	cout<<"Inicio (referencia \""<<referencia_serializada<<"\", comprimida \""<<entrada_comprimida<<"\")\n";
	
	cout<<"Cargando texto referencia\n";
	
	ReferenceIndexBasic *referencia = new ReferenceIndexBasic();
	referencia->load(referencia_serializada);
	
	//Usando el Compressor
	CompressorSingleBuffer compressor(entrada_comprimida, new CoderBlocksRelz(referencia), new DecoderBlocksRelz(referencia->getText()));
	
	char line[line_size + 1];
	unsigned int n_tests = 10000;
	unsigned int text_size = compressor.getTextSize();
	unsigned int pos = 0;
	long double total_bytes = 0;
	
	NanoTimer timer;
	
	if( sequential ){
		cout<<"Realizando Prueba Sequential (Total)\n";
//		unsigned int next_pos = 100000000;
//		double last_time = timer.getMilisec();
		for(pos = 0; pos < text_size; pos += line_size){
//			if( pos > next_pos ){
//				cout<<" -> "<<(100.0*pos/text_size)<<"% (time: "<<(timer.getMilisec()-last_time)<<")\n";
//				last_time = timer.getMilisec();
//				next_pos += 100000000;
//			}
			compressor.read( pos, line_size, line );
		}
		total_bytes = text_size;
	}
	else{
		cout<<"Realizando Prueba Rand\n";
		for(unsigned int i = 0; i < n_tests; ++i){
			pos = rand() % text_size;
			compressor.read( pos, line_size, line );
		}
		total_bytes = line_size * n_tests;
	}
	
	double milisec = timer.getMilisec();
	cout<<"Tiempo read: "<<(milisec/n_tests)<<" ("<<milisec<<" total)\n";
	total_bytes /= (1024*1024);
	total_bytes /= (milisec/1000);
	cout<<"MB/s "<<total_bytes<<"\n";
	
	cout<<"Borrando...\n";
	delete referencia;
	
	cout<<"Fin\n";
	
}

















