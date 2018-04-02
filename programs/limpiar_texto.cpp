#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <map>
#include <vector>

#include "NanoTimer.h"

using namespace std;

int main(int argc, char* argv[]){

	if(argc != 4){
		cout<<"\nModo de Uso: limpiar_texto entrada salida largo_linea\n";
		cout<<"\n";
		return 0;
	}
	
	const char *entrada = argv[1];
	const char *salida = argv[2];
	unsigned int largo_linea = atoi(argv[3]);
	
	cout<<"Inicio";
	
	NanoTimer timer;
	
	unsigned int max_read = 10000000;
	if(largo_linea > max_read){
		//+2 pues al buffer se le agrega '\n' y '\0'
		max_read = largo_linea + 2;
	}
	char *buff = new char[max_read];
	
	timer.reset();
	fstream lector(entrada, fstream::in);
	
	//Tomo el largo para cargarlo completo
	lector.seekg (0, lector.end);
	unsigned long long file_size = lector.tellg();
	lector.seekg (0, lector.beg);
	
	char *text = new char[file_size + 1];
	
	//Total leido
	unsigned long long total = 0;
	//Total escrito (ya filtrado)
	unsigned long long text_size = 0;
	while( total < file_size && lector.good() ){
		lector.read(buff, max_read);
		unsigned int n_read = lector.gcount();
		total += n_read;
		for(unsigned int i = 0; i < n_read; ++i){
			char c = toupper(buff[i]);
//			if(c == 'A' || c == 'T' || c == 'C' || c == 'G'){
			if(c == 'A' || c == 'T' || c == 'C' || c == 'G' || c == 'N'){
//				text[text_size++] = c;
				text[text_size++] = buff[i];
			}
		}
	}
	text[text_size] = 0;
	lector.close();
	
	cout<<"Caracteres procesados: "<<text_size<<" en "<<timer.getMilisec()<<" ms\n";
	
	fstream escritor(salida, fstream::trunc | fstream::out);
	total = 0;
	while( total < text_size ){
		if( text_size - total < largo_linea){
			//ultima linea
			largo_linea = text_size - total;
		}
		memcpy(buff, text + total, largo_linea);
		total += largo_linea;
		//Agrego el final a parte
		buff[largo_linea] = '\n';
		buff[largo_linea + 1] = 0;
		escritor.write(buff, largo_linea + 1);
	}
	escritor.close();
	
	delete [] buff;
	delete [] text;
	
	cout<<"Fin\n";
	
}

















