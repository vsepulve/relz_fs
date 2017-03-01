#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include "ReferenceIndexBasic.h"
#include "ReferenceIndexBasicTest.h"
#include "NanoTimer.h"

using namespace std;

int main(int argc, char* argv[]){

	if(argc != 2){
		cout<<"\nModo de Uso: prueba_construccion entrada_texto\n";
		cout<<"\n";
		return 0;
	}
	
	const char *entrada_texto = argv[1];
	
	cout<<"Inicio (leyendo desde "<<entrada_texto<<")\n";
	
//	const char *text = "banananani";
//	const char *text = "CCATTGCNNNACGTCAAT";
//	const char *text = "TANNGANANACAA";

	unsigned int pos = 0;
	
	cout<<"Cargando string\n";
	
	unsigned int max_linea = 1024;
	char *linea = new char[max_linea];
	char *text = new char[1000000000];
	unsigned int contador = 0;
	
	fstream lector(entrada_texto, fstream::in);
	while(true){
		lector.getline(linea, max_linea);
		if(!lector.good() || strlen(linea) < 1){
			break;
		}
		
		if(++contador % 1000000 == 0){
			cout<<"Linea "<<contador<<" ("<<strlen(linea)<<" chars)\n";
		}
		
		//En esta prueba voy a FILTRAR lo que no sea A, T, C o G
		for(unsigned int i = 0; i < strlen(linea); ++i){
			char c = toupper(linea[i]);
			if(c == 'A' || c == 'T' || c == 'C' || c == 'G'){
				text[pos] = c;
				++pos;
			}
		}
			
	}
	text[pos] = 0;
	cout<<"Caracteres cargados: "<<pos<<"\n";
	
	
	NanoTimer timer;
//	ReferenceIndexBasic *referencia = new ReferenceIndexBasic(text);
	ReferenceIndexBasicTest *referencia = new ReferenceIndexBasicTest(text);
	double milisec = timer.getMilisec();
	
	cout<<"construido en "<<milisec<<" ms\n";
	
//	const char *query = "NANANO";
	const char *query = "ACCTGCATTTACGGACAA";
	unsigned int largo;
	cout<<"Consultando "<<query<<" (largo "<<strlen(query)<<")\n";
	referencia->find(query, strlen(query), pos, largo);
	cout<<"Resultado: "<<pos<<", "<<largo<<" ("<<string((const char*)(text+pos), largo)<<")\n";
	
	cout<<"Fin\n";
	
}

















