#ifndef _BYTES_READER_H
#define _BYTES_READER_H

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

//Esta clase enmascara la lectura de bytes de un stream
//con la misma interface de read de fstream

//En esta primera version NO REALIZA NINGUNA verificacion de seguridad
//Asume memoria suficiente en la lectura y la copia de datos

//Notar que, idealmente, esto deberia heredar de un strema adecuado (basic_algo)
//Lamentablemente, hacer es es mucho mas complejo de lo que parece por la falta de definiciones virtuales

class BytesReader{
private:
	const char *bytes;
	const char *bytes_ini;
	
public:
	BytesReader(const char *_bytes);
	virtual ~BytesReader();
	
	void read(char *salida, unsigned int n_bytes);
	
	void seekg(unsigned int pos, unsigned int offset);
	
	void close();
	
	bool good() const;
	
	static const unsigned int beg = 0;
	
};


#endif //_BYTES_READER_H
