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
	
	const char *entrada_referencia = argv[1];
	const char *entrada_comprimir = argv[2];
	const char *nombre_salida = argv[3];
	unsigned int block_size = atoi(argv[4]);
	unsigned int n_threads = atoi(argv[5]);
	const char *referencia_serializada = argv[6];
	bool construir_referencia = (atoi(argv[7]) == 1);
	bool use_metadata = (atoi(argv[8]) == 1);
	//Dejo la base de golomb por defecto (2^6)
	
	cout<<"Inicio (referencia \""<<entrada_referencia<<"\", procesar \""<<entrada_comprimir<<"\", salida \""<<nombre_salida<<"\")\n";
	
	BitsUtils utils;
	NanoTimer timer;
	
	ReferenceIndex *referencia = NULL;
//	TextFilter *filter = new TextFilterBasic();
	TextFilter *filter = new TextFilterFull();
	
	if(construir_referencia){
	
		char *text = NULL;
		unsigned int text_size = 0;
		
		fstream lector(entrada_referencia, fstream::in);
		if( ! lector.good() ){
			cerr<<"Error leyendo Referencia\n";
			return 0;
		}
		lector.seekg (0, lector.end);
		unsigned int largo_text = lector.tellg();
		lector.seekg (0, lector.beg);
		lector.close();
		
//		//Agregacion de N's y del alfabeto valido completo
		unsigned int largo_ns = 1024;
		vector<char> alfabeto;
		filter->getAlphabet( &alfabeto );
		
		text = new char[largo_text + largo_ns + alfabeto.size() + 1];
//		text = new char[largo_text + largo_ns + 1];
		
		
		// TEMPORALMENTE COMENTADO MIENTRAS PRUEBO LA BUSQUEDA
		//Agregacion de N's y del alfabeto valido completo
		for(unsigned int i = 0; i < largo_ns; ++i){
			text[text_size++] = 'N';
		}
		for(unsigned int i = 0; i < alfabeto.size(); ++i){
			text[text_size++] = alfabeto[i];
		}
		
		
		//Carga del texto de la referencia (metodo especial para cargar bases solamente)
		text_size += filter->readReference(entrada_referencia, text + text_size);
		
		cout<<"Construyendo Referencia con texto de "<<text_size<<" chars\n";
		timer.reset();
		//ReferenceIndex *referencia = new ReferenceIndexBasic(text);
		referencia = new ReferenceIndexBasic(text, n_threads);
		cout<<"Construido en "<<timer.getMilisec()<<" ms\n";
		
		timer.reset();
		referencia->save(referencia_serializada);
		cout<<"Guardado en "<<timer.getMilisec()<<" ms\n";
		//----- FIN CONSTRUCCION REFERENCIA -----
		
		delete [] text;
	}
	else{
		cout<<"Cargando Referencia\n";
		timer.reset();
		
		referencia = new ReferenceIndexBasic();
		referencia->load(referencia_serializada);
		cout<<"Referencia Cargada en "<<timer.getMilisec()<<" ms\n";
	}
	
	//Usando el Compressor
	CompressorSingleBuffer compressor(
		nombre_salida, 
		new CoderBlocksRelz(referencia), 
		new DecoderBlocksRelz(referencia->getText()), 
		filter
		);
		
	timer.reset();
	compressor.compress(entrada_comprimir, n_threads, block_size, use_metadata);
	cout<<"Compresion terminada en "<<timer.getMilisec()<<" ms\n";
	
	cout<<"Borrando\n";
	delete referencia;
	
	cout<<"Fin\n";
	
}

















