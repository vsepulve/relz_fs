#include "RelzIndexPrimary.h"

RelzIndexPrimary::RelzIndexPrimary(){
	reference = NULL;
	table_size = 0;
	factors_table = NULL;
//	factors_positions = NULL;
}

RelzIndexPrimary::RelzIndexPrimary(ReferenceIndex *_reference){
	reference = _reference;
	table_size = 0;
	factors_table = NULL;
//	factors_positions = NULL;
}

RelzIndexPrimary::~RelzIndexPrimary(){
	reference = NULL;
	if( factors_table != NULL){
		delete [] factors_table;
		factors_table = NULL;
		table_size = 0;
	}
//	if( factors_positions != NULL){
//		delete [] factors_positions;
//		factors_positions = NULL;
//	}
	factors_positions.clear();
	
	arr_factors.clear();
	factors_blocks.clear();
	
}

void RelzIndexPrimary::search(const char *text, unsigned int size, vector<unsigned int> &res) const{
	
	if(reference == NULL){
		cerr<<"RelzIndexPrimary::search - Error, reference NULL\n";
		return;
	}
	
	// Primero se buscan las posiciones en la referencia
	// Luego se pasan esas posiciones por el indice de factores
	// Luego se pasan esos resultados por los archivos comprimidos
	// Luego se ajustan las posiciones en cada archivo
	
	// En una primera version, considero un unico archivo (aun asi un factor puede estar replicado)
	// El indice de factores puede generarse por esta misma clase y guardarse en binario
	// En ese archivo puede ir el identificador de la referencia para asegurar que siempre se usa la misma
	// Otra opcion es que el indice de factores sea una clase a parte dedicada a ello
	// Su interfaz seria algo como:
	// "bool realPosition(int ref_position, int size, int &real_pos)" que retorna si es valido y guarda la posicion real
	
	vector<unsigned int> res_prev;
	reference->search(text, size, res_prev);
//	unsigned int real_pos = 0;
	cout<<"RelzIndexPrimary::search - Buscando posiciones reales de "<<res_prev.size()<<" pos de ref\n";
	for(unsigned int i = 0; i < res_prev.size(); ++i){
//		if( factors_index->realPosition(res_prev[i], size, real_pos) ){
//			res.push_back(real_pos);
//		}
		// Version local que simplemente agrega (res.push_back) todas las posiciones reales de la posicion en ref i
		getIndexedPosition(res_prev[i], size, res);
	}
	cout<<"RelzIndexPrimary::search - Ordenando "<<res.size()<<" resultados finales\n";
	sort(res.begin(), res.end());
	
	// Eliminacion de resultados con overlap
	// Notar que esto no es totalmente util o necesario, lo agrego para simular los resultados de la busqueda lineal actual
	unsigned int last = 0xffffffff;
	vector<unsigned int> res_final;
	for(unsigned int i = 0; i < res.size(); ++i){
		if(last == 0xffffffff || ( res[i] - last ) >= size ){
			res_final.push_back(res[i]);
			last = res[i];
		}
	}
	res = res_final;
	
	// Revision de resultados
//	cout<<"RelzIndexPrimary::search - Resultados:\n";
//	for(unsigned int i = 0; i < res.size(); ++i){
//		cout<<"RelzIndexPrimary::search - "<<res[i]<<"\n";
//	}
	
}

unsigned int RelzIndexPrimary::getIndexedPosition(unsigned int text_pos, unsigned int text_size, vector<unsigned int> &real_res) const{
	
	// Calcular bloque
	// Iterar por factores del bloque
	// para cada uno, calcular agregarlo a res si esta en el rango correcto
	
	unsigned int block_ini = text_pos / factors_block_size;
	unsigned int block_fin = (text_pos + text_size) / factors_block_size;
	if(block_fin >= factors_blocks.size()){
		block_fin = factors_blocks.size()-1;
	}
//	cout<<"RelzIndexPrimary::getIndexedPosition - Inicio (text_pos: "<<text_pos<<", blocks: "<<block_ini<<"..."<<block_fin<<")\n";
	// Con el limite de block_fin, la condicion siguiente no es necesaria
//	if(block >= factors_blocks.size()){
//		cout<<"RelzIndexPrimary::getIndexedPosition - Error, block fuera de rango (n_blocks: "<<factors_blocks.size()<<")\n";
//		return 0;
//	}
	
	unsigned int n_res = 0;
	unsigned int pos = 0;
	for(unsigned int block = block_ini; block <= block_fin; ++block){
//		cout<<"RelzIndexPrimary::getIndexedPosition - Revisando "<<factors_blocks[block].size()<<" factores de bloque "<<block<<"\n";
		for(unsigned int i = 0; i < factors_blocks[block].size(); ++i){
			// i-esimo factor de la lista del bloque block
			FactorData factor = arr_factors[factors_blocks[block][i]];
//			cout<<"RelzIndexPrimary::getIndexedPosition - Factor "<<factors_blocks[block][i]<<" ("<<factor.ref_pos<<", "<<factor.len<<", "<<factor.seq_pos<<")\n";
			if( text_pos >= factor.ref_pos 
				&& (text_pos + text_size) <= (factor.ref_pos + factor.len) ){
				++n_res;
	//			pos = factor.seq_pos + (factor.ref_pos - text_pos);
				pos = factor.seq_pos + (text_pos - factor.ref_pos);
//				cout<<"RelzIndexPrimary::getIndexedPosition - Agregando pos "<<pos<<"\n";
				real_res.push_back(pos);
			}
		}// for... cada factor de block
	}// for... cada block
	
//	cout<<"RelzIndexPrimary::getIndexedPosition - Fin (n_res: "<<n_res<<")\n";
	return n_res;
	
}

void RelzIndexPrimary::indexFactors(DecoderBlocksRelz &decoder, unsigned int min_len, unsigned int _factors_block_size){
	
	if( factors_table != NULL){
		delete [] factors_table;
		factors_table = NULL;
	}
	factors_positions.clear();
	factors_block_size = _factors_block_size;
	
	unsigned int n_blocks = decoder.getNumBlocks();
	unsigned int block_size = decoder.getBlockSize();
	
	cout<<"RelzIndexPrimary::indexFactors - Inicio ("<<n_blocks<<" blocks de largo "<<block_size<<", factors_block_size: "<<factors_block_size<<")\n";
	
	char *buff = new char[block_size + 1];
	unsigned int *buff_pos = NULL;
	unsigned int *buff_len = NULL;
	unsigned int n_factors = 0;
	unsigned int pos, len;
	
	/*
	// Primera revision para determinar el tamaño de la tabla (es decir, maximo texto indexado)
	cout<<"RelzIndexPrimary::indexFactors - Buscando table_size...\n";
	table_size = 0;
	for(unsigned int i = 0; i < n_blocks; ++i){
		decoder.decodeBlock(i, buff);
		// Notar que los buffers internos pueden pedirse en decodeBlock
		// Por esto los punteros SOLO son validos hasta la llamada siguiente, hay que volver a pedirlos
		buff_pos = decoder.getBuffPos();
		buff_len = decoder.getBuffLen();
		n_factors = decoder.getNumFactors();
		cout<<"RelzIndexPrimary::indexFactors - Block "<<i<<", "<<n_factors<<" factores\n";
		for(unsigned int j = 0; j < n_factors; ++j){
			pos = buff_pos[j];
			len = buff_len[j];
			if( (len >= min_len) && (pos + len > table_size) ){
				table_size = pos + len;
			}
		}
	}
	cout<<"RelzIndexPrimary::indexFactors - table_size: "<<table_size<<"\n";
	factors_table = new unsigned int [table_size];
	
	// Segunda iteracion para llenar las tablas
	// Primero agrego el 0 inicial que quedara como defecto para posiciones no consideradas
	memset(factors_table, 0, table_size*sizeof(int));
	factors_positions.push_back(0);
	unsigned int next_position = 1;
	
	for(unsigned int i = 0; i < n_blocks; ++i){
		decoder.decodeBlock(i, buff);
		buff_pos = decoder.getBuffPos();
		buff_len = decoder.getBuffLen();
		n_factors = decoder.getNumFactors();
//		cout<<"RelzIndexPrimary::indexFactors - Block "<<i<<", "<<n_factors<<" factores\n";
		for(unsigned int j = 0; j < n_factors; ++j){
			pos = buff_pos[j];
			len = buff_len[j];
			if( (len >= min_len) ){
				// agregar posiciones a las tablas
				
			}
		}
	}
	*/
	
	// Version de indice de factores por bloque
	// Primera pasada para buscar factores
	cout<<"RelzIndexPrimary::indexFactors - Buscando factores...\n";
	unsigned int seq_pos = 0;
	for(unsigned int i = 0; i < n_blocks; ++i){
		decoder.decodeBlock(i, buff);
		buff_pos = decoder.getBuffPos();
		buff_len = decoder.getBuffLen();
		n_factors = decoder.getNumFactors();
//		cout<<"RelzIndexPrimary::indexFactors - Block "<<i<<", "<<n_factors<<" factores\n";
		for(unsigned int j = 0; j < n_factors; ++j){
			pos = buff_pos[j];
			len = buff_len[j];
			if( (len >= min_len) ){
				// agregar factor
//				cout<<"RelzIndexPrimary::indexFactors - Factor ("<<pos<<", "<<len<<", "<<seq_pos<<")\n";
				arr_factors.push_back( FactorData(pos, len, seq_pos) );
			}
			seq_pos += len;
		}
	}
	
	// Iteracion por los factores para indexarlos en bloques
	// Llenado inicial de bloques vacios
	factors_blocks.resize( 1 + (reference->getLength() / factors_block_size) );
//	cout<<"RelzIndexPrimary::indexFactors - factors_blocks.size: "<<factors_blocks.size()<<" (ref de largo "<<reference->getLength()<<")\n";
	for(unsigned int i = 0; i < arr_factors.size(); ++i){
		pos = arr_factors[i].ref_pos;
		len = arr_factors[i].len;
		seq_pos = arr_factors[i].seq_pos;
		unsigned int first_block = pos / factors_block_size;
		unsigned int last_block = (pos + len) / factors_block_size;
		for(unsigned int j = first_block; j <= last_block; ++j){
//			cout<<"RelzIndexPrimary::indexFactors - Factor "<<i<<" a bloque de factores "<<j<<"\n";
			factors_blocks[j].push_back(i);
		}
	}
	
	// Revision de resultados
//	for(unsigned int i = 0; i < factors_blocks.size(); ++i){
//		cout<<"RelzIndexPrimary::indexFactors - Bloque "<<i<<" ("<<factors_blocks[i].size()<<" factores)\n";
//		cout<<"f_block\t"<<i<<"\t"<<factors_blocks[i].size()<<"\n";
//		for(unsigned int j = 0; j < factors_blocks[i].size(); ++j){
//			cout<<""<<factors_blocks[i][j]<<"\n";
//		}
//	}
	
	
	delete [] buff;
}








