#ifndef _REFERENCE_INDEX_H
#define _REFERENCE_INDEX_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <algorithm>
#include <vector>

using namespace std;

class ReferenceIndex{

private: 

public: 
	ReferenceIndex();
	virtual ~ReferenceIndex();

	// Busca el "text" de largo "size" en el arreglo de sufijos
	// Guarda la posicion y el largo ("position" y "length") del mayor prefijo comun
	virtual void find(const char *text, unsigned int size, unsigned int &position, unsigned int &length) const;
	
	// Metodos de save para carga sin construccion
	virtual void save(const char *ref_file);
	
	// Metodos de carga sin construccion
	virtual void load(const char *ref_file);
	
	// Lee y retorna SOLO el texto de la referencia
	// Pide memoria para retornar el texto, independiente del tamaño
	// Esto puede implicar generar ese texto si se almacena comprimido
	// NO REQUIERE INSTANCIA (es static)
	// PUEDE RETORNAR NULL (en falla)
	static char *loadText(const char *ref_file);
	
	// Metodos para acceder al texto
	// No es claro que todas las referencias sean capaces de hacer esto
	virtual unsigned int getLength();
	
	virtual const char *getText() const;
	
	// De forma similar a find, busca "text" de largo "size" en el arreglo de sufijos
	// Esta version busca el texto completo y almacena todas las ocurrencias en "res", ordenadas crecientemente
	virtual void search(const char *text, unsigned int size, vector<unsigned int> &res) const;
	
};

#endif //_REFERENCE_INDEX_H

