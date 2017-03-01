#ifndef _REFERENCE_INDEX_BASIC_H
#define _REFERENCE_INDEX_BASIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <algorithm>
#include <vector>
#include <set>

#include <thread>
#include <mutex>

#include "NanoTimer.h"
#include "ReferenceIndex.h"
#include "ComparatorUtils.h"

using namespace std;
	
class ReferenceIndexBasic : public ReferenceIndex{

private:
	//Largo (producto de strlen) del string referencia
	unsigned int largo;
	//String referencia. Podria contener largo ceros adicionales (para simplificar operaciones)
	unsigned char *ref;
	//Arreglo de sufijos explicito
	unsigned int *arr;

	//Contadores para la prueba de uso (notar que es del mismo largo que arr)
	unsigned int *arr_uso;
	
public: 
	
	void printArrUso(){
		for(unsigned int i = 0 ; i < largo; ++i){
			cout<<"arr_uso "<<i<<" "<<arr_uso[i]<<"\n";
		}
	}
	
	ReferenceIndexBasic();
	
	ReferenceIndexBasic(const char *_referencia, unsigned int n_threads = 4);
	
	virtual ~ReferenceIndexBasic();
	
	//Busca el "text" en el arreglo de sufijos
	//Guarda la posicio y el largo del mayor prefijo comun
	virtual void find(const char *text, unsigned int size, unsigned int &position, unsigned int &length) const;
	
	//Metodos de save para carga sin construccion
	virtual void save(const char *ref_file);
	
	// Lee y retorna SOLO el texto de la referencia
	// Pide memoria para retornar el texto, independiente del tamaño
	// Esto puede implicar generar ese texto si se almacena comprimido
	// NO REQUIERE INSTANCIA (es static)
	// PUEDE RETORNAR NULL (en falla)
	static char *loadText(const char *ref_file);
	
	//Metodos de carga sin construccion
	virtual void load(const char *ref_file);
	
	unsigned int getLength(){
		return largo;
	}
	
	const char *getText() const{
		return (const char*)ref;
	}
	
	// Similar al anterior, pero solo permite sufijos en un rango determinado (de ref, no de arr)
	// Tambien considera un cierto min_length para considerar las busquedas validas
	// Si no encuentra un sufijo comun de largo min_length o mas, omite el resultado
	// Mientras busca el sufijo, comienza a probar el rango valido solo si ya tiene min_length o mas encontrado
	virtual void find(const char *text, unsigned int size, unsigned int &position, unsigned int &length, unsigned int min_length, bool *arr_valid) const;
	
	// De forma similar a find, busca "text" de largo "size" en el arreglo de sufijos
	// Esta version busca el texto completo y almacena todas las ocurrencias en "res", ordenadas crecientemente
	virtual void search(const char *text, unsigned int size, vector<unsigned int> &res) const;
	
	
};


#endif //_REFERENCE_INDEX_BASIC_H











