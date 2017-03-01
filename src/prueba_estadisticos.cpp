
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <thread>

#include "NanoTimer.h"
#include "ReferenceIndexBasic.h"
#include "DecoderBlocksRelz.h"
#include "BlockHeaders.h"
#include "BlockHeadersRelz.h"


using namespace std;


int main(int argc, char *argv[]){
	
	const char *reference_file = "referencia_y01.bin";
	const char *master_file = "yeast30_r01.relz";
	const char *archivo_salida = "salida_estadisticos.txt";
	
	// 28
	unsigned int n_files = 28;
	const char *arr_files[] = {"/home2/cebib/datos/yeast/sacCer1.txt.relz",
							"/home2/cebib/datos/yeast/sacCer2.txt.relz",
							"/home2/cebib/datos/yeast/sacCer3.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R01.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R10.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R10_up.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R11.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R12.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R13.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R14.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R15.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R16.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R17.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R18.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R19.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R20.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R20_up.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R21.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R22.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R23.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R24.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R25.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R26.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R27.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R28.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R29.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R30.txt.relz",
							"/home2/cebib/datos/yeast/yeast_R30_up.txt.relz"};
	
	ReferenceIndexBasic referencia;
	referencia.load(reference_file);
	unsigned int *arr_uso = new unsigned int[ referencia.getLength() + 1 ];
	memset(arr_uso, 0, referencia.getLength() * sizeof(int) );
	
	unsigned int buff_size = 1000000 + 1;
	char *buff = new char[buff_size];
	
	DecoderBlocksRelz decoder( referencia.getText() );
	BlockHeadersRelz *headers = NULL;
	unsigned int n_blocks = 0;
	unsigned int *buff_pos = NULL;
	unsigned int *buff_len = NULL;
	
	for(unsigned int file = 0; file < n_files; ++file){
		
		master_file = arr_files[file];
	
		// Inicio proceso del master
	
		cout<<"----- Inicio master_file["<<file<<"]: \""<<master_file<<"\" -----\n";
		decoder.load(master_file);
	
		if( (headers = dynamic_cast<BlockHeadersRelz*>( decoder.getHeaders() )) == NULL ){
			cerr<<"Error - Headers de tipo incorrecto\n";
			return 0;
		}
	
		n_blocks = headers->getNumBlocks();
		cout<<"n_blocks: "<<n_blocks<<"\n";
		unsigned int n_factors = 0;
		for(unsigned int i = 0; i < n_blocks; ++i){
			n_factors = headers->getFactors(i);
			cout<<"block["<<i<<"] - n_factors: "<<n_factors<<"\n";
			decoder.decodeBlock(i, buff);
			buff_pos = decoder.getBuffPos();
			buff_len = decoder.getBuffLen();
			unsigned int largo_total = 0;
			unsigned int pos, len;
			for(unsigned int j = 0; j < n_factors; ++j){
				// suma de largo total (estadistico general del bloque)
				pos = buff_pos[j];
				len = buff_len[j];
				largo_total += len;
				// incrementar arreglo desde pos, hasta (pos + len)
				if( pos + len > referencia.getLength() ){
					cout<<"Error\n";
				}
				for(unsigned int k = 0; k < len; ++k){
					++(arr_uso[ pos + k ]);
				}
			}
			cout<<"largo_total: "<<largo_total<<"\n";
		}
	
		// Fin de proceso del master
	
	
	}// for... cada master
	
	cout<<"Revisando arr_uso (guardando en "<<archivo_salida<<")\n";
	fstream escritor(archivo_salida, fstream::trunc | fstream::out);
	char linea[1024];
	unsigned int ini = 0;
	unsigned int val = 0;
	unsigned int num = 0;
	for(unsigned int i = 0; i < referencia.getLength(); ++i){
		if( arr_uso[i] == val ){
			++num;
		}
		else{
			// guardar (ini, num, val)
			// iniciar ini = i, num = 1, val = arr[i]
			if( num > 0 ){
//				cout<<"histo\t"<<ini<<"\t"<<num<<"\t"<<val<<"\n";
				sprintf(linea, "%d\t%d\t%d\t\n", ini, num, val);
				escritor.write(linea, strlen(linea));
			}
			ini = i;
			num = 1;
			val = arr_uso[i];
		}
	}
	escritor.close();
	
	cout<<"Fin\n";
	
	return 0;
}













