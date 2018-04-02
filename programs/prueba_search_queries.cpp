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

using namespace std;



int main(int argc, char* argv[]){

	if(argc != 6){
		cout<<"\nModo de Uso: prueba_search_queries ref_file seq_file f_min_len f_blocksize query_file\n";
		cout<<"\n";
		return 0;
	}
//	
	const char *ref_file = argv[1];
	const char *seq_file = argv[2];
	unsigned int f_min_len = atoi(argv[3]);
	unsigned int f_blocksize = atoi(argv[4]);
	const char *query_file = argv[5];
	
	cout<<"Inicio (ref: "<<ref_file<<", seq: "<<seq_file<<", min_len: "<<f_min_len<<", blocksize: "<<f_blocksize<<")\n";
	
	
	cout<<"Cargando Referencia\n";
	ReferenceIndexBasic *referencia = new ReferenceIndexBasic();
	referencia->load(ref_file);
	
	cout<<"Cargando Decoder\n";
	DecoderBlocksRelz decoder(referencia->getText());
	decoder.load(seq_file);
	
	cout<<"Indexando\n";
	RelzIndexPrimary index(referencia);
	index.indexFactors(decoder, f_min_len, f_blocksize);
	
	
	cout<<"Leyendo Queries...\n";
	fstream lector(query_file, fstream::in);
	if(! lector.good() ){
		cerr<<"Error en lectura\n";
		return 0;
	}
	unsigned int buff_size = 1024*1024;
	char buff[buff_size];
//	vector<size_t> res;
	vector<unsigned int> res;
	vector<string> arr_queries;
	
	while(lector.good()){
		lector.getline(buff, buff_size);
		unsigned int read_len = lector.gcount();
		if(read_len > 0 || strlen(buff) > 0){
			arr_queries.push_back(string(buff));
		}// if... linea valida
	}// while... linea
	lector.close();
	
	// Prueba de Tiempo
	cout<<"Calculando tiempo de "<<arr_queries.size()<<" queries\n";
	NanoTimer timer;
	unsigned int total = 0;
	for(unsigned int i = 0; i < arr_queries.size(); ++i){
		const char *query = arr_queries[i].data();
		unsigned int len = strlen(query);
		index.search(query, len, res);
		
		cout<<"Resultados: "<<res.size()<<"\n";
		for(unsigned int i = 0; i < res.size() && i < 3; ++i){
			cout<<"res["<<i<<"]: "<<res[i]<<"\n";
		}
		if(res.size() > 3){
			cout<<"...\n";
		}
		cout<<"-----     -----\n";
		total += res.size();
		
		res.clear();
	}
	cout<<"Tiempo: "<<timer.getMilisec()<<" ms para "<<total<<" ocurrencias totales ("<<(timer.getMilisec() / total)<<" ms/oc)\n";
	
	arr_queries.clear();
	
	
	cout<<"Fin\n";
	
}

















