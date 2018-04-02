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
#include "NanoTimer.h"
#include "BitsUtils.h"

#include "PositionsCoderBlocks.h"
#include "LengthsCoderBlocks.h"
#include "BlockHeaders.h"


using namespace std;

int main(int argc, char* argv[]){

	if(argc != 4){
		cout<<"\nModo de Uso: construccion_histograma referencia_serializada entrada_comprimir salida_histograma\n";
		cout<<"\n";
		return 0;
	}
	
	const char *entrada_referencia = argv[1];
	const char *entrada_comprimir = argv[2];
	const char *salida_histograma = argv[3];
	
	cout<<"Inicio (referencia \""<<entrada_referencia<<"\", procesar \""<<entrada_comprimir<<"\")\n";
	
	BitsUtils utils;
	NanoTimer timer;
	
	cout<<"Cargando Referencia\n";
	timer.reset();
	ReferenceIndex *referencia = new ReferenceIndexBasic();
	referencia->load(entrada_referencia);
	cout<<"Cargada en "<<timer.getMilisec()<<" ms\n";
	
	cout<<"Cargando Texto a comprimir\n";
	
	timer.reset();
	fstream lector(entrada_comprimir, fstream::in);
	
	//Tomo el largo para cargarlo completo
	lector.seekg (0, lector.end);
	unsigned int file_size = lector.tellg();
	lector.seekg (0, lector.beg);
	
	//arreglo del tamaño del archivo
	//Notar que se cargaran MENOS bytes (por el filtrado)
	char *text = new char[file_size + 1];
	
	unsigned int max_read = 10000000;
	char *buff = new char[max_read];
	unsigned int text_size = 0;
	for(unsigned int i = 0; i < file_size / max_read; ++i){
		lector.read(buff, max_read);
		for(unsigned int j = 0; j < max_read; ++j){
			char c = toupper(buff[j]);
//			if(c == 'A' || c == 'T' || c == 'C' || c == 'G'){
			if(c == 'A' || c == 'T' || c == 'C' || c == 'G' || c == 'N'){
				text[text_size++] = c;
			}
		}
	}
	//lectura final (el largo es 0 cuando no hay lectura final)
	unsigned int lectura_final = file_size % max_read;
	if(lectura_final > 0){
		lector.read(buff, lectura_final);
		for(unsigned int j = 0; j < lectura_final; ++j){
			char c = toupper(buff[j]);
//			if(c == 'A' || c == 'T' || c == 'C' || c == 'G'){
			if(c == 'A' || c == 'T' || c == 'C' || c == 'G' || c == 'N'){
				text[text_size++] = c;
			}
		}
	}
	
	lector.close();
	text[text_size] = 0;
	cout<<"Caracteres cargados: "<<text_size<<" en "<<timer.getMilisec()<<" ms\n";
	
	unsigned int n_factores = 0;
	unsigned int max_pos = 0;
	unsigned int max_len = 0;
	unsigned int compressed_text = 0;
	unsigned int pos_prefijo, largo_prefijo;
	map<unsigned int, unsigned int> histograma;
	
	cout<<"Preparando histograma\n";
	
	timer.reset();
	
	while(compressed_text < text_size){
		
		referencia->find(text + compressed_text, text_size - compressed_text, pos_prefijo, largo_prefijo);
		
		compressed_text += largo_prefijo;
		if(pos_prefijo > max_pos){
			max_pos = pos_prefijo;
		}
		if(largo_prefijo > max_len){
			max_len = largo_prefijo;
		}
//		cout<<"("<<pos_prefijo<<", "<<largo_prefijo<<")\n";
		histograma[largo_prefijo]++;
		n_factores++;

	}//while... cada factor

	cout<<"Proceso terminado en "<<timer.getMilisec()<<" ms (n_factores: "<<n_factores<<", max_len: "<<max_len<<", max_pos: "<<max_pos<<")\n";
	
	//Escribir histograma
	fstream escritor(salida_histograma, fstream::trunc | fstream::out);
	map<unsigned int, unsigned int>::iterator it_histograma;
	for(it_histograma = histograma.begin(); it_histograma != histograma.end(); it_histograma++){
		sprintf(buff, "%d\t%d\n", it_histograma->first, it_histograma->second);
		escritor.write(buff, strlen(buff));
	}
	
	cout<<"Borrando\n";
	delete referencia;
	delete [] buff;
	
	
	cout<<"Fin\n";
	
}

















