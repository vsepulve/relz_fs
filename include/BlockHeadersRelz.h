#ifndef _BLOCK_HEADERS_RELZ_H
#define _BLOCK_HEADERS_RELZ_H

#include <vector>

#include "BlockHeaders.h"
#include "BlockHeadersFactory.h"

using namespace std;

class BlockHeadersRelz : public BlockHeaders {

public:
	class HeaderRelz : public BlockHeaders::Header{
	public:
		unsigned int n_factores;
		unsigned int bytes_pos;
		unsigned int bytes_len;
		HeaderRelz(){
			n_factores = 0;
			bytes_pos = 0;
			bytes_len = 0;
		}
		virtual ~HeaderRelz(){}
		HeaderRelz(unsigned int _n_factores, unsigned int _bytes_pos, unsigned int _bytes_len){
			n_factores = _n_factores;
			bytes_pos = _bytes_pos;
			bytes_len = _bytes_len;
		}
		virtual unsigned int size(){
			return 3 * sizeof(int);
		}
		virtual void save(fstream *file){
			if( file == NULL || (! file->good()) ){
				return;
			}
			file->write((char*)&n_factores, sizeof(int));
			file->write((char*)&bytes_pos, sizeof(int));
			file->write((char*)&bytes_len, sizeof(int));
		}
		virtual void load(fstream *file){
			if( file == NULL || (! file->good()) ){
				return;
			}
			//Por ahora omito el tipo
			file->read((char*)&n_factores, sizeof(int));
			file->read((char*)&bytes_pos, sizeof(int));
			file->read((char*)&bytes_len, sizeof(int));
		}
	};
	
private: 
	
//	//Variables globales
//	unsigned int block_size;
//	unsigned long long text_size;
//	unsigned int bytes_total_initial
//	unsigned int unprepared_block;
	
	//Uso un vector de HeaderRelz* para usar cast SOLO en el add
	//vector<Header*> headers;
	vector<HeaderRelz*> headers;
	
	//Encapsula el borrado de los headers
	void clearHeaders();
	
public: 

	BlockHeadersRelz();

	BlockHeadersRelz(unsigned long long _text_size, unsigned int _block_size, Metadata *_metadata);
	
	virtual ~BlockHeadersRelz();
	
	//Agrega los datos de un header particular a este objeto
	//Verifica que el tipo del header sea correcto
	virtual void addBlock(Header *header);
	
	//Carga un los datos de un header del fstream
	//Para ello crea un nuevo header del tipo correcto, lo carga con load
	//Luego agrega los datos de ese header a este objeto
	virtual void loadBlock(fstream *reader, unsigned int bytes);
	
	//Similar al anterior, pero reemplaza el bloque de pos si existe (si no, lo agrega)
	virtual void reloadBlock(fstream *reader, unsigned int bytes, unsigned int pos);
	
	//Guarda los datos de este BlockHeaders en un fstream
	//Esto incluye todos los datos propios incluyendo metadatos
	//Sin embargo NO almacena el tipo de headers (eso deberia hacerlo el factory)
	virtual unsigned int save(fstream *writer);
	
	//Carga todos los datos guardados por un save
	virtual void load(fstream *reader);
	
	//Igual al anterior, pero desde un BytesReader
	virtual void load(BytesReader *reader);
	
	//Version de lectura usada previamente en el Recoder
	//Queda en espera mientras no se reconstruya ese objeto	
//	void loadBytes(char *bytes, unsigned int byte_ini = 0);
	
	//Retorna el numero (logico) de bloques
	virtual unsigned int getNumBlocks();
	
	//Prepara headers recien cargados para un save (incluyendo calculo de data_pos)
	//Acumula o ajusta las posiciones y agrega el bloque ficticio final (asume que dicho bloque no existe aun)
	virtual void prepare();
	
	//Deshace la preparacion desde un cierpo block_ini incluyendo la eliminacion del bloque ficticio
	//Des-acumula los valores desde ese bloque en adelante
	//...guardando la posicion de ese bloque y su byte inicial para el proximo prepare
	virtual void unprepare(unsigned int block_ini = 0);
	
	//Retorna la posicion absoluta de inicio de un bloque en el archivo terminado
	//Esto se usa en el write, para saber los bytes que deben preservarse
	//Notar que el inicio de un bloque se usa tambien para el final del anterior
	virtual unsigned int getBlockPosition(unsigned int block);
	
	//Propios de Relz
	unsigned int getFactors(unsigned int block);
	unsigned int getBytesPos(unsigned int block);
	unsigned int getBytesLen(unsigned int block);
	
};







#endif //_BLOCK_HEADERS_RELZ_H





