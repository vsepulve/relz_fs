#ifndef _RELZ_INDEX_PRIMARY_H
#define _RELZ_INDEX_PRIMARY_H

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

class RelzIndexPrimary{

protected: 
	
	class FactorData{
		public: 
		// El id lo considro implicito por su posicion
		// unsigned int id;
		unsigned int ref_pos;
		unsigned int len;
		unsigned int seq_pos;
		FactorData(){
			ref_pos = 0;
			len = 0;
			seq_pos = 0;
		}
		FactorData(unsigned int _ref_pos, unsigned int _len, unsigned int _seq_pos){
			ref_pos = _ref_pos;
			len = _len;
			seq_pos = _seq_pos;
		}
		~FactorData(){}
	};

	ReferenceIndex *reference;
	
	// Primer enfoque del indice de factores (tablas directas y descomprimidas)
	// Tabla de acceso general, de tamaño igual a la referencia (para cada posicion, una posicion en la siguiente tabla)
	unsigned int *factors_table;
	unsigned int table_size;
	// Tabla de posiciones indexadas (tamaño variable, tuplas con n_ocur y ese numero de posiciones, el primer 0 se usa varias veces)
//	unsigned int *factors_positions;
	// Lo cambio a vector pues, en esta version, los datos son agregados con push_back
	vector<unsigned int> factors_positions;
	
	// Version de indice de factores por bloques
	vector<FactorData> arr_factors;
	vector< vector<unsigned int> > factors_blocks;
	unsigned int factors_block_size;
	
public: 
	
	RelzIndexPrimary();
	
	RelzIndexPrimary(ReferenceIndex *_reference);
	
	virtual ~RelzIndexPrimary();
	
	void search(const char *text, unsigned int size, vector<unsigned int> &res) const;
	
	// Busca la posicion real de una secuencia en la referencia (pos, size) en los factores indexados
	// Retorna true si (pos, size) esta contenido en algun factor
	// Almacena las posiciones reales de los factores
	// Notar que esto puede implicar varias posiciones
	// ...pues varios factores pueden contener (pos, size) incluso si es una sola secuencia indexada
	// En ese caso quizas sea mejor retornar el numero de apariciones (quizas el vector se esta reusando para todos los resultados)
	// Eso hay que considerarlo para el indice de factores, no basta una permutacion directa tipo tabla
	// El indice podria ser en dos pasos, una tabla directa a un arreglo de tamaño variable con n_ocur seguido de ese numero de posiciones
//	bool getIndexedPosition(unsigned int pos, unsigned int size, unsigned int &real_pos);
	unsigned int getIndexedPosition(unsigned int text_pos, unsigned int text_size, vector<unsigned int> &real_res) const;
	
	// Metodo de indexaminto de factores (llenado de las tablas)
	// Solo considera factores de largo MAYOR O IGUAL a min_len
	// Lee un archivo relz, crea un DecoderBlocksRelz directamente (necesita referencia o un decoder ya creado)
	// ...descomprime cada bloque y toma los buffers del decoder para iterar por los factores 
//	void indexFactors(const char *relz_file, unsigned int min_len);
	void indexFactors(DecoderBlocksRelz &decoder, unsigned int min_len, unsigned int _factors_block_size = 1024);
	
};







#endif //_RELZ_INDEX_PRIMARY_H





