#ifndef _COMMUNICATION_UTILS_H
#define _COMMUNICATION_UTILS_H

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;

// Funciones basicas de comunicacion de bajo nivel

// Escribe chars de buff hasta escribir size o fallar
// Retoran el numero de bytes escritos
unsigned int writeBytes(int sock_fd, const char *buff, unsigned int size);

// Lee chars hasta completar size (o fallar), escribiendolos en buff
// Asume que buff tiene al menos size bytes
// Notar que este metodo NO agrega el '\0' final para un c-string valido
// Retorna el numero de bytes leidos
unsigned int readBytes(int sock_fd, char *buff, unsigned int size);

// Envia un c-string
// Primero envia el largo (strlen) en 4 bytes, luego los chars
// Retorna true en exito
bool writeString(int sock_fd, const char *buff);

// Lee un c-string
// Primero lee el largo en 4 bytes, luego los chars (y los escribe en buff)
// ASUME que el buff tiene espacio suficiente (notar que esta es una llamada insegura)
// SOLO DEBE SER USADA SI SE CONOCE A PRIORI EL LARGO MAXIMO para entregarle un buffer adecuado
// Retorna true en exito
bool readStringSimple(int sock_fd, char *buff, unsigned int buff_size);

// Lee un c-string
// Primero lee el largo en 4 bytes, luego los chars
// Escribe los chars leidos en buff, hasta un maximo de (buff_size - 1)
// Finalmente agrega el '\0' final
// Notar que esta llamada lee pero omite chars por sobre (buff_size - 1), trunca el string
// Retorna true en exito
bool readStringSimple(int sock_fd, char *buff, unsigned int buff_size, char *inner_buff, unsigned int inner_buff_size);

// Objetos para facilitar envios

class RequestID {
public:
	
	unsigned int user_id;
	unsigned char type;
	char md5[17];
	
	RequestID(){
		user_id = 0;
		type = 0;
		md5[0] = 0;
	}
	
	// Asume que _md5 tiene al menos 16 bytes validos
	RequestID(unsigned int _user_id, unsigned char _type, char *_md5){
		user_id =  _user_id;
		type = _type;
		memcpy(md5, _md5, 16);
		md5[16] = 0;
	}
	
	~RequestID(){}
	
	bool send(int sock_fd){
		cout<<"RequestID::send - Inicio ("<<user_id<<", "<<(unsigned int)type<<", \""<<md5<<"\")\n";
		if( writeBytes(sock_fd, (char*)&user_id, sizeof(int)) != sizeof(int) ){
			cout<<"RequestID::send - Error en write user_id\n";
			return false;
		}
		if( writeBytes(sock_fd, (char*)&type, 1) != 1 ){
			cout<<"RequestID::send - Error en write type\n";
			return false;
		}
		if( writeBytes(sock_fd, md5, 16) != 16 ){
			cout<<"RequestID::send - Error en write md5\n";
			return false;
		}
		cout<<"RequestID::send - Fin\n";
		return true;
	}
	
	bool receive(int sock_fd){
		cout<<"RequestID::receive - Inicio\n";
		if( readBytes(sock_fd, (char*)&user_id, sizeof(int)) != sizeof(int) ){
			return false;
		}
		if( readBytes(sock_fd, (char*)&type, 1) != 1 ){
			return false;
		}
		if( readBytes(sock_fd, md5, 16) != 16 ){
			return false;
		}
		md5[16] = 0;
		cout<<"RequestID::receive - Fin (user_id: "<<user_id<<", type: "<<(unsigned int)type<<", md5: \""<<md5<<"\")\n";
		return true;
	}
	
};

// Esta clase fija el largo maximo de nombre de archivos a 255 (trunca en caso de ser mayor)
class FileHeader {
public:
	
	unsigned int file_size;
	char file_name[256];
	char md5[17];
	
	FileHeader(){
		file_size = 0;
		file_name[0] = 0;
		md5[0] = 0;
	}
	
	// Asume que _file_name es un c-string valido (terminado en '\0')
	// Asume que _md5 tiene al menos 16 bytes validos
	FileHeader(unsigned int _file_size, char *_file_name, char *_md5){
		file_size = _file_size;
		unsigned int size = strlen(_file_name);
		if( size > 255 ){
			size = 255;
		}
		memcpy( file_name, _file_name, size );
		file_name[size] = 0;
		memcpy( md5, _md5, 16 );
		md5[16] = 0;
	}
	
	~FileHeader(){}
	
	bool send(int sockfd){
		if( writeBytes(sockfd, (char*)&file_size, sizeof(int)) != sizeof(int) ){
			return false;
		}
		if( ! writeString(sockfd, file_name) ){
			return false;
		}
		if( writeBytes(sockfd, md5, 16) != 16 ){
			return false;
		}
		return true;
	}
	
	bool receive(int sockfd){
		if( readBytes(sockfd, (char*)&file_size, sizeof(int)) != sizeof(int) ){
			return false;
		}
		if( ! readStringSimple(sockfd, file_name, 256) ){
			return false;
		}
		if( readBytes(sockfd, md5, 16) != 16 ){
			return false;
		}
		md5[16] = 0;
		return true;
	}
};










#endif //_COMMUNICATION_UTILS_H
