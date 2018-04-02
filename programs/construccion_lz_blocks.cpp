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

	if(argc != 6){
		cout<<"\nModo de Uso: construccion_lz entrada_referencia entrada_comprimir nombre_salida potencia_base block_size\n";
		cout<<"Crea archivos \"nombre_salida.position.bin\" y \"nombre_salida.length.bin\"\n";
		cout<<"Tambien creara un archivo temporal que borra al final \"nombre_salida.tmp\"\n";
		cout<<"base de golob = 2 ^ potencia_base\n";
		cout<<"\n";
		return 0;
	}
	
	const char *entrada_referencia = argv[1];
	const char *entrada_comprimir = argv[2];
	const char *nombre_salida = argv[3];
	unsigned int potencia_base = atoi(argv[4]);
	unsigned int block_size = atoi(argv[5]);
	
	cout<<"Inicio (referencia \""<<entrada_referencia<<"\", procesar \""<<entrada_comprimir<<"\", salida \""<<nombre_salida<<"\")\n";
	
	BitsUtils utils;
	NanoTimer timer;
	
	unsigned int max_linea = 1024*1024*10;
	char *linea = new char[max_linea];
	unsigned int text_size = 0;
	unsigned int contador = 0;
	
	cout<<"Cargando string\n";
	
	char *text = NULL;
	unsigned int largo_text = 0;
	fstream lector;
	
	char *archivo_salida_ref = new char[1024];
	sprintf(archivo_salida_ref, "%s.ref.bin", nombre_salida);
	
	//----- INICIO CONSTRUCCION REFERENCIA -----
	
	timer.reset();
	lector.open(entrada_referencia, fstream::in);
	lector.seekg (0, lector.end);
	largo_text = lector.tellg();
	lector.seekg (0, lector.beg);
	
	text = new char[largo_text+1];
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
				text[text_size] = c;
				++text_size;
			}
		}
		
//		if(text_size > 1000){
//			break;
//		}
	}
	lector.close();
	text[text_size] = 0;
	cout<<"Caracteres cargados: "<<text_size<<" en "<<timer.getMilisec()<<" ms\n";
	
	cout<<"Construyendo Referencia\n";
	timer.reset();
	ReferenceIndexBasic *referencia = new ReferenceIndexBasic(text);
//	ReferenceIndexBasicTest *referencia = new ReferenceIndexBasicTest(text);
	cout<<"Construido en "<<timer.getMilisec()<<" ms\n";
	
	timer.reset();
	referencia->save(archivo_salida_ref);
	cout<<"Guardado en "<<timer.getMilisec()<<" ms\n";
	
	//----- FIN CONSTRUCCION REFERENCIA -----
	
//	cout<<"Cargando Referencia\n";
//	timer.reset();
//	ReferenceIndexBasic *referencia = new ReferenceIndexBasic();
//	referencia->load(archivo_salida_ref);
//	cout<<"Cargada en "<<timer.getMilisec()<<" ms\n";
	
	delete [] text;
	
	cout<<"Cargando Texto a comprimir\n";
	
	timer.reset();
	lector.open(entrada_comprimir, fstream::in);
	lector.seekg (0, lector.end);
	largo_text = lector.tellg();
	lector.seekg (0, lector.beg);
	
	text = new char[largo_text+1];
	text_size = 0;
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
				text[text_size] = c;
				++text_size;
			}
		}
		
//		if(text_size > 1000){
//			break;
//		}
	}
	text[text_size] = 0;
	unsigned int largo_real = text_size;
	cout<<"Caracteres cargados: "<<text_size<<" en "<<timer.getMilisec()<<" ms\n";
	
	//Generacion de archivo para simplificar pruebas
	//Esto es solo para debug
	//Escribo (con trunc) el archivo "salida.tmp"
	//Fijo el largo de linea en 70
	fstream escritor("salida.tmp", fstream::trunc | fstream::out);
	unsigned int largo_escritura = 70;
	for(unsigned int i = 0; i < text_size; i += 70){
		if(text_size - i < 70){
			largo_escritura = text_size - i;
		}
		memcpy(linea, text+i, largo_escritura);
		linea[largo_escritura] = 0;
		escritor.write(linea, largo_escritura);
		sprintf(linea, "\n");
		escritor.write(linea, strlen(linea));
	}
	escritor.close();
	
	cout<<"Comprimiendo Texto\n";
	timer.reset();
	
//	unsigned int n_factores = 0;
	
//	//archivo de largos de factor
	char *archivo_salida_fac = new char[1024];
	sprintf(archivo_salida_fac, "%s.length.bin", nombre_salida);
//	
//	//archivo de posiciones
	char *archivo_salida_pos = new char[1024];
	sprintf(archivo_salida_pos, "%s.positions.bin", nombre_salida);
//	
//	//archivo de headers
	char *archivo_salida_headers = new char[1024];
	sprintf(archivo_salida_headers, "%s.headers.bin", nombre_salida);
	
	LengthsCoderBlocks *lengths_coder = new LengthsCoderBlocks(archivo_salida_fac);
	lengths_coder->setGolombBase(potencia_base);
	PositionsCoderBlocks *positions_coder = new PositionsCoderBlocks(archivo_salida_pos);
	//Versiones de posiciones y largos?
	BlockHeaders *headers = new BlockHeaders(text_size, block_size);
	
	unsigned int n_blocks = largo_real / block_size;
	if(n_blocks * block_size < largo_real){
		++n_blocks;
	}
	
	cout<<"Preparando "<<n_blocks<<" bloques ("<<largo_real<<" chars en bloques de "<<block_size<<")\n";
	
	unsigned int *buff_pos = new unsigned int[block_size + 1];
	unsigned int *buff_largos = new unsigned int[block_size + 1];
	
	unsigned int bytes_pos = 0;
	unsigned int bytes_len = 0;
	
	unsigned int compressed_text = 0;
	unsigned int pos_prefijo, largo_prefijo;
	
	unsigned int largo_local = 0;
	for(unsigned int block = 0; block < n_blocks; ++block){
		
//		cout<<"Preparando bloque "<<block<<" (desde char "<<compressed_text<<")\n";
		
		//setear variables para el bloque actual, como el tamaño inicial
		if(largo_real-compressed_text < block_size){
			largo_local = largo_real-compressed_text;
		}
		else{
			largo_local = block_size;
		}
		
		unsigned int n_factores = 0;
		unsigned int max_pos = 0;
		while(largo_local > 0){
			referencia->find(text + compressed_text, largo_local, pos_prefijo, largo_prefijo);
			
			if(largo_prefijo == 0){
				cerr<<"Error - Prefijo de largo 0, saliendo\n";
				return 1;
			}
			
			largo_local -= largo_prefijo;
			compressed_text += largo_prefijo;
			if(pos_prefijo > max_pos){
				max_pos = pos_prefijo;
			}
			
//			cout<<"("<<pos_prefijo<<", "<<largo_prefijo<<")\n";

			buff_pos[n_factores] = pos_prefijo;
			buff_largos[n_factores] = largo_prefijo;
			++n_factores;
		}
		
		//Preparar header
		headers->addBlock(n_factores, bytes_pos, bytes_len);
		
		//escribir bloques de pos y de largos
		//Notar que los escritores retornan la POSICION en que queda (no los bytes escritos)
		
		bytes_pos = positions_coder->encodeBlockMaxBits(buff_pos, n_factores, utils.n_bits(max_pos));

//		bytes_pos = positions_coder->encodeBlockVarByte(buff_pos, n_factores);
		
		bytes_len = lengths_coder->encodeBlockGolomb(buff_largos, n_factores);
		
	}//for... cada bloque
	
	//Agrego un bloque mas con los finales de los archivos (para simplificar la lectura)
	headers->addBlock(0, bytes_pos, bytes_len);
	
	cout<<"Compresion terminada en "<<timer.getMilisec()<<" ms\n";
	
	cout<<"Cerrando escritores\n";
	positions_coder->close();
	lengths_coder->close();
	
	cout<<"Guardando Headers\n";
	headers->save(archivo_salida_headers);
	
	
	//Arreglo de usos por posicion
//	referencia->printArrUso();
	
	
	
	
	cout<<"Borrando\n";
	delete referencia;
	
	
	cout<<"Fin\n";
	
}

















