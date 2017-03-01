#ifndef _REFERENCE_INDEX_BASIC_TEST_H
#define _REFERENCE_INDEX_BASIC_TEST_H

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

#include "ReferenceIndex.h"
#include "ComparatorUtils.h"

using namespace std;
	
class ReferenceIndexBasicTest : public ReferenceIndex{

private:
	//Largo (producto de strlen) del string referencia
	unsigned int largo;
	//Largo del arreglo (puede ser menor a largo)
	unsigned int largo_arr;
	//String referencia. Podria contener largo ceros adicionales (para simplificar operaciones)
	unsigned char *ref;
	//Arreglo de sufijos explicito
	unsigned int *arr;
	
public: 

	ReferenceIndexBasicTest();
	
	ReferenceIndexBasicTest(const char *_referencia);
	
	virtual ~ReferenceIndexBasicTest();

	//Busca el "text" en el arreglo de sufijos
	//Guarda la posicio y el largo del mayor prefijo comun
	virtual void find(const char *text, unsigned int size, unsigned int &position, unsigned int &length) const;
	
	//Metodos de save para carga sin construccion
	virtual void save(const char *ref_file);
	
	//Metodos de carga sin construccion
	virtual void load(const char *ref_file);
	
};


#endif //_REFERENCE_INDEX_BASIC_H











