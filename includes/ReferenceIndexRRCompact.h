#ifndef _REFERENCE_INDEX_RR_COMPACT_H
#define _REFERENCE_INDEX_RR_COMPACT_H

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
	
class ReferenceIndexRRCompact : public ReferenceIndex{

private:
	//Largo (producto de strlen) del string referencia
//	unsigned long long largo;
	unsigned long long largo_arr;
	//String referencia. Podria contener largo ceros adicionales (para simplificar operaciones)
//	unsigned char *ref;
	//Arreglo de sufijos explicito
	unsigned int *arr;
	
	CompactSequence *texto;
	
public: 
	
	ReferenceIndexRRCompact();
	
	//Notar que esta version recibe una referencia completa serializada (ya construida)
	//Escoge 1 de cada distancia enteros del arreglo (pero el texto completo)
	ReferenceIndexRRCompact(const char *ref_file, unsigned int distancia);
	
	//Esta version recibe un archivo especial para la RR que guarda el texto compacto
	//Tampoco requiere distancia, esta version ya esta pasada por muestreo
	ReferenceIndexRRCompact(const char *ref_file_rr);
	
	virtual ~ReferenceIndexRRCompact();
	
	//Busca el "text" en el arreglo de sufijos
	//Guarda la posicio y el largo del mayor prefijo comun
	virtual void find(const char *text, unsigned int size, unsigned int &position, unsigned int &length) const;
	
	//Metodos de save para carga sin construccion
	virtual void save(const char *ref_file);
	
	//Metodos de carga sin construccion
	virtual void load(const char *ref_file);
	
	virtual CompactSequence *getCompactText(){
		return texto;
	}
	
};


#endif //_REFERENCE_INDEX_BASIC_H











