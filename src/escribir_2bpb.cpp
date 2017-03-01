#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <algorithm>
#include <vector>

#include<sys/types.h>
#include<iostream>

using namespace std;

int main(int argc, char* argv[]){

	if(argc != 3){
		cout<<"Modo de Uso: escribir_2bpb entrada_texto salida_bin\n";
		return 0;
	}
	
	const char *entrada_texto = argv[1];
	const char *salida_bin = argv[2];
	
	
	unsigned int max_linea = 1024;
	char *linea = new char[max_linea];
	unsigned char *buff = new unsigned char[max_linea];
	
	memset(buff, 0, max_linea);
	
	unsigned char mask_char;
	unsigned int pos_relativo = 0;
	unsigned int pos_buff = 0;
	
	fstream lector(entrada_texto, fstream::in);
	
	fstream escritor(salida_bin, fstream::trunc | fstream::out | fstream::binary);
	
	unsigned int contador = 0;
	
	while(true){
		lector.getline(linea, max_linea);
		if(!lector.good() || strlen(linea) < 1){
			break;
		}
		
		if(++contador % 1000000 == 0){
			cout<<"Linea "<<contador<<" ("<<strlen(linea)<<" chars)\n";
		}
		
//		for(unsigned int i = 0; i < strlen(linea); ++i){
//			cout<<""<<(linea[i])<<" ";	
//		}
//		cout<<"\n";
		
		for(unsigned int i = 0; i < strlen(linea); ++i){
		
			//agregar bits al buff
			switch(linea[i]){
				case 'A' : 
					mask_char = 0x0;
					break;
				case 'T' :
					switch(pos_relativo){
						case 0:
							mask_char = 0x40;
							break;
						case 1:
							mask_char = 0x10;
							break;
						case 2:
							mask_char = 0x04;
							break;
						case 3:
							mask_char = 0x01;
							break;
					}
					break;
				case 'C' :
					switch(pos_relativo){
						case 0:
							mask_char = 0x80;
							break;
						case 1:
							mask_char = 0x20;
							break;
						case 2:
							mask_char = 0x08;
							break;
						case 3:
							mask_char = 0x02;
							break;
					}
					break;
				case 'G' :
					switch(pos_relativo){
						case 0:
							mask_char = 0xc0;
							break;
						case 1:
							mask_char = 0x30;
							break;
						case 2:
							mask_char = 0x0c;
							break;
						case 3:
							mask_char = 0x03;
							break;
					}
					break;
				default :
					mask_char = 0;
					break;
			}
		
			buff[pos_buff] |= mask_char;

			if(++pos_relativo == 4){
				pos_relativo = 0;
				++pos_buff;
			}
			
			//si se ha escrito el numero adecuado de bits, escribir el buff
		
			if(pos_buff == max_linea){
//				cout<<"Escribiendo buffer\n";
//				for(unsigned int j = 0; j < 10; ++j){
//					cout<<(unsigned int)(buff[j])<<" ";
//				}
//				cout<<"\n";
//				return 0;
				
				escritor.write((char*)buff, pos_buff);
				
				//reset buffer
				memset(buff, 0, max_linea);
				pos_buff = 0;
			}
		
		}//for... cada letra de la linea
		
	}
	
	//si hay bits en el buff, agregar padding y escribirlo
	
	if(pos_buff > 0){
		escritor.write((char*)buff, pos_buff);
	}
	
	lector.close();
	escritor.close();
	
	delete [] linea;
	delete [] buff;
	
}

















