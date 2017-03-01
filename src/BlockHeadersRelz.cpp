#include "BlockHeadersRelz.h"

BlockHeadersRelz::BlockHeadersRelz()
 : BlockHeaders()
{
}

BlockHeadersRelz::BlockHeadersRelz(unsigned long long _text_size, unsigned int _block_size, Metadata *_metadata)
 : BlockHeaders(_text_size, _block_size, _metadata)
{
}

BlockHeadersRelz::~BlockHeadersRelz(){
	clearHeaders();
}

void BlockHeadersRelz::clearHeaders(){
	for(unsigned int i = 0; i < headers.size(); ++i){
		if(headers[i] != NULL){
			delete headers[i];
		}
	}
	headers.clear();
}

void BlockHeadersRelz::addBlock(Header *header){
	HeaderRelz *header_relz = NULL;
	if( (header_relz = dynamic_cast<HeaderRelz*>(header)) == NULL ){
		cerr<<"BlockHeadersRelz::addBlock - Error, header de tipo incorrecto\n";
		return;
	}
	cout<<"BlockHeadersRelz::addBlock - ["<<headers.size()<<"] ("<<header_relz->n_factores<<", "<<header_relz->bytes_pos<<", "<<header_relz->bytes_len<<")\n";
	headers.push_back( header_relz );
}

void BlockHeadersRelz::loadBlock(fstream *lector, unsigned int bytes){
	if( lector == NULL || (! lector->good()) ){
		cerr<<"BlockHeadersRelz::loadHeader - Error en lector\n";
		return;
	}
	//Por ahora omito bytes, podria ser util para algun otro tipo de header
	HeaderRelz *header = new HeaderRelz();
	header->load( lector );
	headers.push_back( header );
}

void BlockHeadersRelz::reloadBlock(fstream *lector, unsigned int bytes, unsigned int pos){
	if( lector == NULL || (! lector->good()) ){
		cerr<<"BlockHeadersRelz::reloadBlock - Error en lector\n";
		return;
	}
	// Simplemente creo nuevos bloques vacios mientras sea necesario
	// Asi, al final es seguro simplemente recargarlo
	for(unsigned int i = headers.size(); i <= pos; ++i){
		headers.push_back( new HeaderRelz() );
	}
	headers[pos]->load( lector );
}

unsigned int BlockHeadersRelz::save(fstream *escritor){
	unsigned int size = headers.size();
	
	cout<<"BlockHeadersRelz::save - Guardando "<<size<<" bloques (block_size: "<<block_size<<")\n";
	
	if( escritor == NULL || (! escritor->good()) ){
		cerr<<"BlockHeadersRelz::save - Error en escritor\n";
		return 0;
	}
	
	//Metadatos
	escritor->write((char*)(&size), sizeof(int));
	escritor->write((char*)(&text_size), sizeof(long long));
	escritor->write((char*)(&block_size), sizeof(int));
	escritor->write((char*)(&data_pos), sizeof(int));
	
//	cout<<"BlockHeadersRelz::save - Datos iniciales escritos, guardando Metadata\n";
	
	if( metadata == NULL ){
		cout<<"BlockHeadersRelz::save - metadata NULL, agregando marca\n";
		unsigned char marca = 0;
		escritor->write((char*)&marca, 1);
	}
	else{
		metadata->save(escritor);
	}
//	cout<<"BlockHeadersRelz::save - Metadata guardado, escribiendo bloques\n";
	
	//Headers
	unsigned int n_factores, bytes_pos, bytes_len;
	for(unsigned int i = 0; i < size; ++i){
//		cout<<"BlockHeadersRelz::save - Escribiendo Meta-Bloque "<<i<<" / "<<size<<"\n";
		n_factores = headers[i]->n_factores;
		bytes_pos = headers[i]->bytes_pos;
		bytes_len = headers[i]->bytes_len;
//		cout<<"BlockHeadersRelz::save - write...\n";
		escritor->write((char*)(&n_factores), sizeof(int));
		escritor->write((char*)(&bytes_pos), sizeof(int));
		escritor->write((char*)(&bytes_len), sizeof(int));
//		cout<<"BlockHeadersRelz::save - Ok\n";
	}
	
	cout<<"BlockHeadersRelz::save - Bloques escritos, terminando\n";
	
	return escritor->tellp();
	
}

void BlockHeadersRelz::load(fstream *lector){
	
	cout<<"BlockHeadersRelz::load - inicio \n";
	
	if( lector == NULL || (! lector->good()) ){
		cerr<<"BlockHeadersRelz::load - Error en lector\n";
		return;
	}
	
	//Metadatos
	unsigned int size = 0;
	text_size = 0;
	block_size = 0;
	lector->read((char*)(&size), sizeof(int));
	lector->read((char*)(&text_size), sizeof(long long));
	lector->read((char*)(&block_size), sizeof(int));
	lector->read((char*)(&data_pos), sizeof(int));
	
	if( metadata != NULL ){
		delete metadata;
	}
	metadata = new Metadata();
	metadata->load(lector);
	
	cout<<"BlockHeadersRelz::load - Cargando "<<size<<" bloques en total (text_size: "<<text_size<<", block_size: "<<block_size<<", data_pos: "<<data_pos<<")\n";
	
	//Headers
	clearHeaders();
	HeaderRelz *header = NULL;
	unsigned int n_factores, bytes_pos, bytes_len;
	for(unsigned int i = 0; i < size; ++i){
		lector->read((char*)(&n_factores), sizeof(int));
		lector->read((char*)(&bytes_pos), sizeof(int));
		lector->read((char*)(&bytes_len), sizeof(int));
		header = new HeaderRelz(n_factores, bytes_pos, bytes_len);
		headers.push_back( header );
	}
	
}

void BlockHeadersRelz::load(BytesReader *lector){
	
	cout<<"BlockHeadersRelz::load - inicio (BytesReader)\n";
	
	if( lector == NULL || (! lector->good()) ){
		cerr<<"BlockHeadersRelz::load - Error en lector\n";
		return;
	}
	
	//Metadatos
	unsigned int size = 0;
	text_size = 0;
	block_size = 0;
	lector->read((char*)(&size), sizeof(int));
	lector->read((char*)(&text_size), sizeof(long long));
	lector->read((char*)(&block_size), sizeof(int));
	lector->read((char*)(&data_pos), sizeof(int));
	
	if( metadata != NULL ){
		delete metadata;
	}
	metadata = new Metadata();
	metadata->load(lector);
	
	cout<<"BlockHeadersRelz::load - Cargando "<<size<<" bloques en total (block_size: "<<block_size<<")\n";
	
	//Headers
	clearHeaders();
	HeaderRelz *header = NULL;
	unsigned int n_factores, bytes_pos, bytes_len;
	for(unsigned int i = 0; i < size; ++i){
		lector->read((char*)(&n_factores), sizeof(int));
		lector->read((char*)(&bytes_pos), sizeof(int));
		lector->read((char*)(&bytes_len), sizeof(int));
		header = new HeaderRelz(n_factores, bytes_pos, bytes_len);
		headers.push_back( header );
	}
	
}

//void BlockHeadersRelz::loadBytes(char *bytes, unsigned int byte_ini){
//}

unsigned int BlockHeadersRelz::getNumBlocks(){
	//Resta el bloque ficticio final
	if( headers.size() == 0 ){
		return 0;
	}
	return headers.size() - 1;
}

void BlockHeadersRelz::prepare(){
	unsigned int bytes_pos = 0;
	unsigned int bytes_len = 0;
	// El total es un valor guardado del ultimo unprepare, o 0 (inicialmente)
	unsigned int bytes_total = bytes_total_initial;
	for(unsigned int i = unprepared_block; i < headers.size(); ++i){
		bytes_pos = headers[i]->bytes_pos;
		bytes_len = headers[i]->bytes_len;
		headers[i]->bytes_pos = bytes_total;
		bytes_total += bytes_pos;
		headers[i]->bytes_len = bytes_total;
		bytes_total += bytes_len;
	}
	HeaderRelz *header = new HeaderRelz(0, bytes_total, 0);
	headers.push_back( header );
	//preparo data_pos
	//Marca del factory
	data_pos = BlockHeadersFactory::typeSize();
	
	
//	//Metadatos (4 enteros mas metadata)
//	data_pos += 4 * sizeof(int) + metadata->size();
	//Metadatos (3 enteros + 1 long long mas metadata)
	data_pos += 3 * sizeof(int) + sizeof(long long) + metadata->size();
	
	
	//headers (N * 3 enteros)
	data_pos += headers.size() * 3 * sizeof(int);
	cout<<"BlockHeadersRelz::prepare - data_pos: "<<data_pos<<" (type: "<<BlockHeadersFactory::typeSize()<<", meta: "<<(4 * sizeof(int) + metadata->size())<<", headers: "<<(headers.size() * 3 * sizeof(int))<<")\n";
}

void BlockHeadersRelz::unprepare(unsigned int block_ini){
	if( block_ini >= headers.size()-1 ){
		cerr<<"BlockHeadersRelz::unprepare - Advertencia, block_ini invalido ("<<block_ini<<" de "<<headers.size()<<")\n";
		return;
	}
	if( headers.back()->n_factores != 0 ){
		cerr<<"BlockHeadersRelz::unprepare - Advertencia, lista no preparada\n";
		return;
	}
	unprepared_block = block_ini;
	bytes_total_initial = headers[block_ini]->bytes_pos;
	unsigned int bytes_pos = 0;
	unsigned int bytes_len = 0;
	// Notar que omito el ultimo bloque para restar en forma segura
	// Esto asume que el ultimo bloque es ficticio
	for(unsigned int i = block_ini; i < headers.size() - 1; ++i){
		bytes_pos = headers[i]->bytes_len - headers[i]->bytes_pos;
		bytes_len = headers[i+1]->bytes_pos - headers[i]->bytes_len;
		headers[i]->bytes_pos = bytes_pos;
		headers[i]->bytes_len = bytes_len;
	}
	headers.pop_back();
}

unsigned int BlockHeadersRelz::getBlockPosition(unsigned int block){
	if( block >= headers.size() ){
		return 0;
	}
	return headers[block]->bytes_pos;
}

unsigned int BlockHeadersRelz::getFactors(unsigned int block){
	if(block >= headers.size()){
		return 0;
	}
//	cout<<"BlockHeadersRelz::getFactors - "<<(headers[block]->n_factores)<<"\n";
	return headers[block]->n_factores;
}

unsigned int BlockHeadersRelz::getBytesPos(unsigned int block){
	if(block >= headers.size()){
		return 0;
	}
	return headers[block]->bytes_pos;
}

unsigned int BlockHeadersRelz::getBytesLen(unsigned int block){
	if(block >= headers.size()){
		return 0;
	}
	return headers[block]->bytes_len;
}







