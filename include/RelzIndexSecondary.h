#ifndef _RELZ_INDEX_SECONDARY_H
#define _RELZ_INDEX_SECONDARY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <algorithm>
#include <vector>

#include "NanoTimer.h"
#include "ReferenceIndex.h"
#include "DecoderBlocksRelz.h"

using namespace std;

class RelzIndexSecondary{

protected: 

	ReferenceIndex *reference;
	
	// vector< pair<ini, text> >
	vector< pair<unsigned int, string> > segments;
	
public: 
	
	RelzIndexSecondary();
	
	RelzIndexSecondary(ReferenceIndex *_reference);
	
	virtual ~RelzIndexSecondary();
	
	void search(const char *text, unsigned int size, vector<unsigned int> &res) const;
	
	// Metodo de indexaminto de factores (ya sea por tablas o guardando el texto secondario)
	// Almacena solo los textos de los factores MENORES a min_len
	// Los factores de largo MAYOR O IGUAL a min_len son puntos de corte entre segmentos
	void indexFactors(DecoderBlocksRelz &decoder, unsigned int min_len);
	
};







#endif //_RELZ_INDEX_SECONDARY_H





