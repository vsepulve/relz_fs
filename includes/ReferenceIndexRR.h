#ifndef _REFERENCE_INDEX_RR_H
#define _REFERENCE_INDEX_RR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <algorithm>
#include <vector>

#include <thread>
#include <mutex>

#include "NanoTimer.h"
#include "ReferenceIndex.h"
#include "ComparatorUtils.h"
#include "CompactSequence.h"

using namespace std;
	
class ReferenceIndexRR : public ReferenceIndex{

private:
	//Largo (producto de strlen) del string referencia
	unsigned int largo;
	//String referencia. Podria contener largo ceros adicionales (para simplificar operaciones)
	unsigned char *ref;
	//Largo del arreglo (<= largo)
	unsigned int largo_arr;
	//Arreglo de sufijos explicito
	unsigned int *arr;
	
public: 
	
	ReferenceIndexRR();
	
	//Notar que esta version recibe una referencia completa serializada (ya construida)
	//Escoge 1 de cada "distancia" enteros del arreglo (pero el texto completo)
	ReferenceIndexRR(const char *ref_file, unsigned int distancia);
	
	//Esta version recibe un archivo especial para la RR que guarda el texto compacto
	//Tampoco requiere distancia, esta version ya esta pasada por muestreo
	ReferenceIndexRR(const char *ref_file_rr);
	
	virtual ~ReferenceIndexRR();
	
	//Busca el "text" en el arreglo de sufijos
	//Guarda la posicio y el largo del mayor prefijo comun
	virtual void find(const char *text, unsigned int size, unsigned int &position, unsigned int &length) const;
	
	//Metodos de save para carga sin construccion
	virtual void save(const char *ref_file);
	
	//Metodos de carga sin construccion
	virtual void load(const char *ref_file);
	
};


#endif //_REFERENCE_INDEX_BASIC_H











