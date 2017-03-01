#include "LocalBuffer.h"

mutex LocalBuffer::global_mutex;
// maximo de bloques reservados permitidos (const por ahora)
//unsigned int LocalBuffer::max_blocks;
// estructura para almacenar bloques estaticamente
// vector o lista? creo que lista
list< pair< unsigned int, char* > > LocalBuffer::blocks;
pair<unsigned int, char*> LocalBuffer::arr_blocks[];
unsigned int LocalBuffer::used_blocks = 0;

LocalBuffer::LocalBuffer(unsigned int _size){
//	cout<<"LocalBuffer - Inicio ("<<_size<<")\n";
	
	// lock del bloque estatico
	// revisar cada bloque en busca del que cumpla
	// si al final tengo uno valido, lo saco de la estructura
	// si no, creo uno nuevo
	// entrego el bloque a la instancia y termino
	
	lock_guard<mutex> lock(global_mutex);
	/*
	// Version con lista
	list< pair< unsigned int, char* > >::iterator it_blocks;
	list< pair< unsigned int, char* > >::iterator best_block = blocks.end();
	unsigned int min_size = 0xffffffff;
	for( it_blocks = blocks.begin(); it_blocks != blocks.end(); it_blocks++ ){
		if( it_blocks->first >= _size ){
			if( it_blocks->first < min_size ){
				min_size = it_blocks->first;
				best_block = it_blocks;
			}
		}
	}
	if( best_block == blocks.end() ){
		// crear nuevo block
		mem_size = real_size = _size;
		cout<<"LocalBuffer - Reservando memoria real ("<<mem_size<<")\n";
		mem = new char[mem_size];
	}
	else{
//		cout<<"LocalBuffer - Reusando memoria ("<<best_block->first<<")\n";
		mem_size = _size;
		real_size = best_block->first;
		mem = best_block->second;
		blocks.erase(best_block);
	}
	*/
	
	// Version con arreglo
//	unsigned int min_size = 0xffffffff;
//	unsigned int pos_block = 0;
//	for(unsigned int i = 0; i < used_blocks; ++i){
//		if( (arr_blocks[i].first >= _size) && (arr_blocks[i].first < min_size) ){
//			min_size = arr_blocks[i].first;
//			pos_block = i;
//		}
//	}
//	if( min_size == 0xffffffff ){
//		// crear nuevo block
//		mem_size = real_size = _size;
//		cout<<"LocalBuffer - Reservando memoria real ("<<mem_size<<")\n";
//		mem = new char[mem_size];
//	}
//	else{
////		cout<<"LocalBuffer - Reusando memoria ("<<min_size<<")\n";
//		mem_size = _size;
//		real_size = arr_blocks[pos_block].first;
//		mem = arr_blocks[pos_block].second;
//		// para eliminarlo, ubico en esta posicion el ultimo bloque y reduzco el total
//		arr_blocks[pos_block].first = arr_blocks[used_blocks - 1].first;
//		arr_blocks[pos_block].second = arr_blocks[used_blocks - 1].second;
//		--used_blocks;
//	}
	
	unsigned int pos_block = 0;
	for(; pos_block < used_blocks; ++pos_block){
		if( arr_blocks[pos_block].first >= _size ){
			break;
		}
	}
	if( pos_block < used_blocks ){
//		cout<<"LocalBuffer - Reusando memoria ("<<arr_blocks[pos_block].first<<")\n";
		mem_size = _size;
		real_size = arr_blocks[pos_block].first;
		mem = arr_blocks[pos_block].second;
		// para eliminarlo, ubico en esta posicion el ultimo bloque y reduzco el total
		arr_blocks[pos_block].first = arr_blocks[used_blocks - 1].first;
		arr_blocks[pos_block].second = arr_blocks[used_blocks - 1].second;
		--used_blocks;
	}
	else{
		// crear nuevo block
		mem_size = real_size = _size;
		cout<<"LocalBuffer - Reservando memoria real ("<<mem_size<<")\n";
		mem = new char[mem_size];
	}
	
}

LocalBuffer::~LocalBuffer(){
//	cout<<"~LocalBuffer - Inicio\n";
	
	// lock del bloque estatico
	// tomo el bloque de la instancia
	// reviso la estructura estatica en busca de espacio
	// si no hay, escogo el mas pequeño y lo elimino
	// almaceno el bloque
	// anulo el bloque de la instancia y termino
	lock_guard<mutex> lock(global_mutex);
	/*
	// Version con Lista
	if( blocks.size() >= max_blocks ){
		// blocks lleno, busco el menor para borrar
		list< pair< unsigned int, char* > >::iterator it_blocks;
		list< pair< unsigned int, char* > >::iterator best_block = blocks.end();
		unsigned int min_size = 0xffffffff;
		for( it_blocks = blocks.begin(); it_blocks != blocks.end(); it_blocks++ ){
			if( it_blocks->first < min_size ){
				min_size = it_blocks->first;
				best_block = it_blocks;
			}
		}
		if( best_block != blocks.end() ){
			cout<<"~LocalBuffer - Borrando memoria real ("<<best_block->first<<")\n";
			delete [] best_block->second;
			blocks.erase(best_block);
		}
	}
	// espacio seguro
	blocks.push_back( pair<unsigned int, char*>(real_size, mem) );
	mem = NULL;
	mem_size = real_size = 0;
	*/
	
	// Version con arreglo
	if( used_blocks >= max_blocks ){
		// blocks lleno, busco el menor para borrar
		unsigned int min_size = 0xffffffff;
		unsigned int pos_block = 0;
		for(unsigned int i = 0; i < used_blocks; ++i){
			if( arr_blocks[i].first < min_size ){
				min_size = arr_blocks[i].first;
				pos_block = i;
			}
		}
		if( min_size < real_size ){
			cout<<"~LocalBuffer - Borrando memoria real ("<<min_size<<")\n";
			delete [] arr_blocks[pos_block].second;
			arr_blocks[pos_block].first = real_size;
			arr_blocks[pos_block].second = mem;
			mem = NULL;
			mem_size = real_size = 0;
		}
		else{
			// No guardar, borrar memoria local
			cout<<"~LocalBuffer - Borrando mem ("<<real_size<<")\n";
			delete [] mem;
			mem = NULL;
			mem_size = real_size = 0;
		}
	}
	else{
		// espacio seguro
		arr_blocks[used_blocks].first = real_size;
		arr_blocks[used_blocks].second = mem;
		++used_blocks;
		mem = NULL;
		mem_size = real_size = 0;
	}
	
}

char *LocalBuffer::memory(){
	return mem;
}

unsigned int LocalBuffer::size(){
	return mem_size;
}

void LocalBuffer::clearBlocks(){
	lock_guard<mutex> lock(global_mutex);
	list< pair< unsigned int, char* > >::iterator it_blocks;
	for( it_blocks = blocks.begin(); it_blocks != blocks.end(); it_blocks++ ){
		delete [] it_blocks->second;
	}
	blocks.clear();
}





