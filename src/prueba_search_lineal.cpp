#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <iostream>

#include <map>
#include <vector>

#include "ReferenceIndex.h"
#include "ReferenceIndexBasic.h"
#include "NanoTimer.h"

#include "CoderBlocks.h"
#include "CoderBlocksRelz.h"

#include "DecoderBlocks.h"
#include "DecoderBlocksRelz.h"

#include "Compressor.h"
#include "CompressorSingleBuffer.h"

#include "RelzIndexPrimary.h"
#include "LinearPatternMatching.h"

using namespace std;



int main(int argc, char* argv[]){

	if(argc != 4){
		cout<<"\nModo de Uso: prueba_search_lineal seq_file query_file results_file\n";
		cout<<"results_file == 0 => no se almacenan resultados (solo medicion de tiempo)\n";
		cout<<"\n";
		return 0;
	}
//	
	const char *seq_file = argv[1];
	const char *query_file = argv[2];
	const char *results_file = argv[3];
	LinearPatternMatching matching;
	
	cout<<"Inicio (archivo "<<seq_file<<", query_file "<<query_file<<")\n";
	
	fstream lector(seq_file, fstream::in);
	if(! lector.good() ){
		cerr<<"Error en lectura\n";
		return 0;
	}
	// Tomo el largo para cargarlo completo (notar que text_length sera menor a file_size por el filtrado)
	lector.seekg (0, lector.end);
	size_t file_size = lector.tellg();
	lector.seekg (0, lector.beg);
	char *text = new char[file_size + 1];
	
	cout<<"Leyendo texto completo (file_size "<<file_size<<")\n";
	lector.read(text, file_size);
	lector.close();
	text[file_size] = 0;
	
	/*
	cout<<"Filtrando (solo dejo [A..Z])\n";
	unsigned int eliminados = 0;
	for(unsigned int i = 0; i < file_size; ++i){
		if(text[i] < 'A' || text[i] > 'Z'){
			++eliminados;
		}
		else{
			text[i-eliminados] = text[i];
		}
	}
	file_size += eliminados;
	text[file_size] = 0;
	cout<<"Caracteres eliminados: "<<eliminados<<", largo final: "<<file_size<<"\n";
	*/
	
	cout<<"Creando string...\n";
	string str_text(text);
	
	cout<<"Leyendo Queries...\n";
	lector.open(query_file, fstream::in);
	unsigned int buff_size = 1024*1024;
	char buff[buff_size];
	char buff_res[buff_size];
	
	// -----
	// ----- TEMPORALMENTE LIMITADO A 32 BITS -----
	// -----
	//vector<unsigned int> res;
	vector<size_t> res;
	size_t pos = 0;
	
//	unsigned int max_res_show = 10;
	unsigned int n_queries = 0;
	vector<string> arr_queries;
	
	fstream escritor;
	bool escribir_resultados = (strlen(results_file) > 0 && results_file[0] != '0');
	if( escribir_resultados ){
		escritor.open(results_file, fstream::out);
		if(! escritor.good() ){
			cerr<<"Error en escritura\n";
			return 0;
		}
	}
	
	while(lector.good()){
		lector.getline(buff, buff_size);
		unsigned int read_len = lector.gcount();
		if(read_len > 0 || strlen(buff) > 0){
			arr_queries.push_back(string(buff));
			
			if(escribir_resultados){
				cout<<"Buscando posiciones de query "<<arr_queries.size()<<" \""<<buff<<"\"\n";
				
				// Version 1: directo
				while( true ){
					pos = str_text.find(buff, pos + strlen(buff));
					if(pos == string::npos || pos > file_size){
						break;
					}
					else{
						// cout<<"res: "<<pos<<"\n";
						res.push_back(pos);
					}
				}// while... next pos
				
				
				// Version 2: Boyer Moore
				//matching.search((unsigned char*)str_text.data(), (unsigned int)str_text.length(), (unsigned char*)buff, strlen(buff), res);
				//matching.searchNO((unsigned char*)str_text.data(), (unsigned int)str_text.length(), (unsigned char*)buff, strlen(buff), res);
			
				cout<<"Resultados: "<<res.size()<<"\n";
				// query ID TEXT NRES
				sprintf(buff_res, "query %d %s %u\n", n_queries++, buff, (unsigned int)(res.size()));
				escritor.write(buff_res, strlen(buff_res));
				for(unsigned int i = 0; i < res.size(); ++i){
//					if(i < max_res_show){
//						cout<<"res["<<i<<"]: "<<res[i]<<"\n";
//					}
					// res ID POS
					sprintf(buff_res, "res %d %llu\n", i, (unsigned long long)(res[i]));
					escritor.write(buff_res, strlen(buff_res));
				}
//				if(res.size() >= max_res_show){
//					cout<<"...\n";
//				}
//				cout<<"-----     -----\n";
				res.clear();
			}
			
		}// if... linea valida
	}// while... linea
	
	lector.close();
	if(escribir_resultados){
		escritor.close();
	}
	else{
	
		// Prueba de Tiempo
		cout<<"Calculando tiempo de "<<arr_queries.size()<<" queries\n";
		NanoTimer timer;
		unsigned int n_res = 0;
		for(unsigned int i = 0; i < arr_queries.size(); ++i){
			const char *query = arr_queries[i].data();
			unsigned int len = strlen(query);
			// cout<<"Query["<<i<<"]: "<<query<<" ("<<len<<")\n";
			
			// Version 1: directo
			while( true ){
				pos = str_text.find(query, pos + len);
				if(pos == string::npos || pos > file_size){
					break;
				}
				else{
					// cout<<"res: "<<pos<<"\n";
					res.push_back(pos);
				}
			}// while... next pos
			
			// Version 2: Boyer Moore
			//matching.search((unsigned char*)str_text.data(), (unsigned int)str_text.length(), (unsigned char*)query, len, res);
			//matching.searchNO((unsigned char*)str_text.data(), (unsigned int)str_text.length(), (unsigned char*)query, len, res);
		
			n_res += res.size();
			res.clear();
		}
		double milis = timer.getMilisec();
		cout<<"Tiempo Total: "<<milis<<" ms\n";
		cout<<"Tiempo/Query: "<<milis/arr_queries.size()<<" ms\n";
		cout<<"Tiempo/Occ: "<<milis/n_res<<" ms\n";
		
	}
	
	arr_queries.clear();
	
	
	cout<<"Fin\n";
	
}

















