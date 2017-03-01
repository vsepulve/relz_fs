#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <map>
#include <vector>

#include "ReferenceIndexBasic.h"
//#include "ReferenceIndexBasicTest.h"
//#include "ReferenceIndexRR.h"
#include "NanoTimer.h"
#include "BitsUtils.h"
#include "ConcurrentLogger.h"

#include "CoderBlocks.h"
#include "CoderBlocksRelz.h"

#include "DecoderBlocks.h"
#include "DecoderBlocksRelz.h"

#include "Compressor.h"
#include "Recompressor.h"

#include "TextFilter.h"
#include "TextFilterBasic.h"
#include "TextFilterFull.h"

using namespace std;

int main(int argc, char* argv[]){

	if(argc != 4){
		cout<<"\nModo de Uso: prueba_recoder referencia_1 referencia_2 entrada_comprimir\n";
		cout<<"\n";
		return 0;
	}
	
	const char *entrada_referencia_1 = argv[1];
	const char *entrada_referencia_2 = argv[2];
	const char *entrada_comprimir = argv[3];
	
	cout<<"Inicio (referencia 1 \""<<entrada_referencia_1<<"\", referencia 2 \""<<entrada_referencia_2<<"\", procesar \""<<entrada_comprimir<<"\")\n";
	
	NanoTimer timer;
	CoutColor amarillo(color_yellow);
	amarillo.reset();
	CoutColor rojo(color_red);
	rojo.reset();
	CoutColor verde(color_green);
	verde.reset();
	
	cout<<"Cargando Referencia 1\n";
	timer.reset();
	amarillo.init();
	ReferenceIndex *referencia_1 = new ReferenceIndexBasic();
	referencia_1->load(entrada_referencia_1);
	amarillo.reset();
	cout<<"Referencia Cargada en "<<timer.getMilisec()<<" ms\n";
	
//	CoderBlocks coder(referencia_1);
//	coder.compress(entrada_comprimir, "salida_precomp.relz.tmp", 16, 1000000);
	verde.init();
	Compressor compressor(
		"salida_precomp.relz.tmp", 
		new CoderBlocksRelz(referencia_1), 
		new DecoderBlocksRelz(referencia_1->getText()), 
		new TextFilterFull()
		);
	compressor.compress(entrada_comprimir, 1, 1000000);
	verde.reset();

	cout<<"Cargando bytes de archivo comprimido\n";
	fstream lector("salida_precomp.relz.tmp", fstream::binary | fstream::in);
	lector.seekg (0, lector.end);
	unsigned int file_size = lector.tellg();
	lector.seekg (0, lector.beg);
	
	char *bytes = new char[file_size + 1];
	unsigned int total_lectura = 0;
	while( total_lectura < file_size ){
		lector.read( bytes + total_lectura, file_size );
		total_lectura += lector.gcount();
	}
	bytes[file_size] = 0;
	
	cout<<"Bytes leidos: "<<file_size<<"\n";
	for(unsigned int i = 0; i < ((20<file_size)?20:file_size) ; ++i){
		cout<<"["<<(unsigned int)bytes[i]<<"]\n";
	}
	
	cout<<"Cargando Referencia Final\n";
	timer.reset();
	amarillo.init();
	ReferenceIndex *referencia_2 = new ReferenceIndexBasic();
	referencia_2->load(entrada_referencia_2);
	amarillo.reset();
	cout<<"Referencia Cargada en "<<timer.getMilisec()<<" ms\n";
	
//	RecoderBlocks recoder(referencia_2);
//	recoder.compress(bytes, file_size, "salida_final.relz.tmp", 16);
	verde.init();
	Recompressor recompressor(
		"salida_final.relz.tmp", 
		new CoderBlocksRelz(referencia_2), 
		new DecoderBlocksRelz(referencia_2->getText())
		);
	recompressor.recompress(bytes, file_size, 1);
	verde.reset();
	
	cout<<"Borrando\n";
	delete referencia_1;
	delete referencia_2;
	delete [] bytes;
	
	cout<<"Fin\n";
	
}

















