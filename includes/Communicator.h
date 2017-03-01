#ifndef _COMMUNICATOR_H
#define _COMMUNICATOR_H

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <ctime>

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "CommunicationUtils.h"

using namespace std;

class Communicator{
private:

protected:
	int sock_fd;
	
public:
	
	Communicator();
	
	virtual ~Communicator();
	
	// Retorna true en conexion correcta
	// De momento solo verifica (sock_fd >= 0)
	virtual bool good();
	
	// ----- Metodos de Lectura -----
	
	// Lee un int (4 bytes) y lo guarda en num
	// Retorna true en lectura exitosa
	virtual bool readInt(int &num);
	
	// Lee un unsigned int (4 bytes) y lo guarda en num
	// Retorna true en lectura exitosa
	virtual bool readUInt(unsigned int &num);
	
	// Lee un long long (8 bytes) y lo guarda en num
	// Retorna true en lectura exitosa
	virtual bool readLong(long long &num);
	
	// Lee un unsigned long long (8 bytes) y lo guarda en num
	// Retorna true en lectura exitosa
	virtual bool readULong(unsigned long long &num);
	
	// Lee un unsigned char (1 bytes) y lo guarda en num
	// Retorna true en lectura exitosa
	virtual bool readByte(unsigned char &num);
	
	// Lee un c-string
	// Primero lee el largo en 4 bytes, luego los chars
	// Escribe los chars leidos en buff, hasta un maximo de (buff_size - 1)
	// Finalmente agrega el '\0' final
	// Notar que esta llamada lee pero OMITE chars por sobre (buff_size - 1), trunca el string
	// Retorna true en exito
	virtual bool readString(char *buff, unsigned int buff_size);
	
	// Lectura directa de bytes
	virtual bool readData(char *buff, unsigned int size);
	
	// ----- Metodos de Escritura -----
	
	// Envia el int (4 bytes) de num
	// Retorna true en lectura exitosa
	virtual bool writeInt(int num);
	
	// Envia el unsigned int (4 bytes) de num
	// Retorna true en lectura exitosa
	virtual bool writeUInt(unsigned int num);
	
	// Envia el long long (8 bytes) de num
	// Retorna true en lectura exitosa
	virtual bool writeLong(long long num);
	
	// Envia el unsigned long long (8 bytes) de num
	// Retorna true en lectura exitosa
	virtual bool writeULong(unsigned long long num);
	
	// Envia el unsigned char (1 bytes) de num
	// Retorna true en lectura exitosa
	virtual bool writeByte(unsigned char num);
	
	// Envia un c-string
	// Primero envia el largo (strlen) en 4 bytes, luego los chars
	// Retorna true en exito
	virtual bool writeString(const char *buff);
	
	// Escritura directa de bytes
	virtual bool writeData(const char *buff, unsigned int size);
	
};








#endif //_COMMUNICATOR_H
