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
#include "RelzIndexSecondary.h"


using namespace std;

int main(int argc, char* argv[]){

	if(argc != 5){
		cout<<"\nModo de Uso: prueba_search ref_bin seq_relz f_min_len f_blocksize\n";
		cout<<"\n";
		return 0;
	}
//	
	const char *ref_bin = argv[1];
	const char *seq_relz = argv[2];
	unsigned int f_min_len = atoi(argv[3]);
	unsigned int f_blocksize = atoi(argv[4]);
	
	cout<<"Inicio (ref: "<<ref_bin<<", seq: "<<seq_relz<<", min_len: "<<f_min_len<<", blocksize: "<<f_blocksize<<")\n";
	
//	ReferenceIndexBasic *referencia = new ReferenceIndexBasic();
//	referencia->load(referencia_serializada);
	
	/*
	const char *ref_text = "abracadabra\0-----\0";
	ReferenceIndexBasic referencia(ref_text, 1);
	
	char text[1024];
	vector<unsigned int> res;
	
	sprintf(text, "abracadabra");
	referencia.search(text, strlen(text), res);
	cout<<"results de \""<<text<<"\": "<<res.size()<<"\n";
	for(unsigned int i = 0; i < res.size(); ++i){
		cout<<"res["<<i<<"]: "<<res[i]<<" ("<<string(ref_text+res[i], strlen(text))<<"["<<string(ref_text+res[i]+strlen(text), 3)<<"])\n";
	}
	cout<<"-----\n";
	res.clear();
	
	sprintf(text, "abraca");
	referencia.search(text, strlen(text), res);
	cout<<"results de \""<<text<<"\": "<<res.size()<<"\n";
	for(unsigned int i = 0; i < res.size(); ++i){
		cout<<"res["<<i<<"]: "<<res[i]<<" ("<<string(ref_text+res[i], strlen(text))<<"["<<string(ref_text+res[i]+strlen(text), 3)<<"])\n";
	}
	cout<<"-----\n";
	res.clear();
	
	sprintf(text, "abra");
	referencia.search(text, strlen(text), res);
	cout<<"results de \""<<text<<"\": "<<res.size()<<"\n";
	for(unsigned int i = 0; i < res.size(); ++i){
		cout<<"res["<<i<<"]: "<<res[i]<<" ("<<string(ref_text+res[i], strlen(text))<<"["<<string(ref_text+res[i]+strlen(text), 3)<<"])\n";
	}
	cout<<"-----\n";
	res.clear();
	
	sprintf(text, "ab");
	referencia.search(text, strlen(text), res);
	cout<<"results de \""<<text<<"\": "<<res.size()<<"\n";
	for(unsigned int i = 0; i < res.size(); ++i){
		cout<<"res["<<i<<"]: "<<res[i]<<" ("<<string(ref_text+res[i], strlen(text))<<"["<<string(ref_text+res[i]+strlen(text), 3)<<"])\n";
	}
	cout<<"-----\n";
	res.clear();
	
	sprintf(text, "a");
	referencia.search(text, strlen(text), res);
	cout<<"results de \""<<text<<"\": "<<res.size()<<"\n";
	for(unsigned int i = 0; i < res.size(); ++i){
		cout<<"res["<<i<<"]: "<<res[i]<<" ("<<string(ref_text+res[i], strlen(text))<<"["<<string(ref_text+res[i]+strlen(text), 3)<<"])\n";
	}
	cout<<"-----\n";
	res.clear();
	
	sprintf(text, "abrir");
	referencia.search(text, strlen(text), res);
	cout<<"results de \""<<text<<"\": "<<res.size()<<"\n";
	for(unsigned int i = 0; i < res.size(); ++i){
		cout<<"res["<<i<<"]: "<<res[i]<<" ("<<string(ref_text+res[i], strlen(text))<<"["<<string(ref_text+res[i]+strlen(text), 3)<<"])\n";
	}
	cout<<"-----\n";
	res.clear();
	
	sprintf(text, "abrab");
	referencia.search(text, strlen(text), res);
	cout<<"results de \""<<text<<"\": "<<res.size()<<"\n";
	for(unsigned int i = 0; i < res.size(); ++i){
		cout<<"res["<<i<<"]: "<<res[i]<<" ("<<string(ref_text+res[i], strlen(text))<<"["<<string(ref_text+res[i]+strlen(text), 3)<<"])\n";
	}
	cout<<"-----\n";
	res.clear();
	*/
	
	cout<<"Cargando Referencia\n";
	ReferenceIndexBasic *referencia = new ReferenceIndexBasic();
	referencia->load(ref_bin);
	
	
	cout<<"Cargando Decoder\n";
	DecoderBlocksRelz decoder(referencia->getText());
	decoder.load(seq_relz);
	
	cout<<"Indexando\n";
	RelzIndexPrimary index(referencia);
	index.indexFactors(decoder, f_min_len, f_blocksize);
	
	vector<unsigned int> res;
//	index.search("AAA", 3, res);
//	index.search("AAT", 3, res);
	index.search("TAG", 3, res);
//	index.search("CTATAAGGCCATAGTCACCAAAACAGCATGGTACTGGTATAAAAATAGGCATATAGACCAATGGAATAGA", 70, res);
	res.clear();
	
	cout<<"Indexando (Secundario)\n";
	RelzIndexSecondary index_seq(referencia);
	index_seq.indexFactors(decoder, f_min_len);
	index_seq.search("TAG", 3, res);
	res.clear();

	
	/*
	cout<<"Cargando Decoder\n";
	DecoderBlocksRelz decoder(referencia->getText());
	decoder.load("seq_test_v2.relz");
	
	cout<<"Cargando Indexando\n";
	RelzIndexPrimary index(referencia);
	index.indexFactors(decoder, 0);
	
	vector<unsigned int> res;
	index.search("ATT", 3, res);
	*/
	
	
	cout<<"Fin\n";
	
}

















