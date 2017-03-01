#include "CommunicationUtils.h"

// Escribe chars de buff hasta escribir size o fallar
// Retoran el numero de bytes escritos
unsigned int writeBytes(int sock_fd, const char *buff, unsigned int size){
	unsigned int total = 0;
	unsigned int res = 0;
	while( total < size ){
		res = write(sock_fd, buff + total, size - total);
		if(res < 0){
			cout<<"ServerConnection::writeBytes - Error en write.\n";
			break;
		}
		total += res;
	}
	return total;
}

// Lee chars hasta completar size (o fallar), escribiendolos en buff
// Asume que buff tiene al menos size bytes
// Notar que este metodo NO agrega el '\0' final para un c-string valido
// Retorna el numero de bytes leidos
unsigned int readBytes(int sock_fd, char *buff, unsigned int size){
//	cout<<"ServerConnection::readBytes - Inicio (sock_fd: "<<sock_fd<<", size: "<<size<<")\n";
	unsigned int total = 0;
	unsigned int res = 0;
	while( total < size ){
		res = read(sock_fd, buff + total, size - total);
		if(res < 0){
			cout<<"ServerConnection::readBytes - Error en read.\n";
			break;
		}
		total += res;
	}
//	cout<<"ServerConnection::readBytes - Fin (total: "<<total<<")\n";
	return total;
}

// Envia un c-string
// Primero envia el largo (strlen) en 4 bytes, luego los chars
// Retorna true en exito
bool writeString(int sock_fd, const char *buff){
	unsigned int size = 0;
	if( buff != NULL ){
		size = strlen(buff);
	}
	if( writeBytes(sock_fd, (char*)&size, sizeof(int)) != sizeof(int) ){
		return false;
	}
	if( writeBytes(sock_fd, buff, size) != size ){
		return false;
	}
	return true;
}

// Lee un c-string
// Primero lee el largo en 4 bytes, luego los chars (y los escribe en buff)
// Falla si el buffer es insuficiente
// Retorna true en exito
bool readStringSimple(int sock_fd, char *buff, unsigned int buff_size){
	if( buff == NULL ){
		return false;
	}
	unsigned int size = 0;
	if( readBytes(sock_fd, (char*)&size, sizeof(int)) != sizeof(int) ){
		return false;
	}
	if( buff_size - 1 < size ){
		return false;
	}
	if( readBytes(sock_fd, buff, size) != size ){
		return false;
	}
	buff[size] = 0;
	return true;
}

// Lee un c-string
// Primero lee el largo en 4 bytes, luego los chars
// Escribe los chars leidos en buff, hasta un maximo de (buff_size - 1)
// Finalmente agrega el '\0' final
// Notar que esta llamada lee pero omite chars por sobre (buff_size - 1), trunca el string
// Retorna true en exito
bool readStringSimple(int sock_fd, char *buff, unsigned int buff_size, char *inner_buff, unsigned int inner_buff_size){
	if( buff == NULL || inner_buff == NULL ){
		return false;
	}
	unsigned int size = 0;
	if( readBytes(sock_fd, (char*)&size, sizeof(int)) != sizeof(int) ){
		return false;
	}
	// En este caso reescribo la lectura de bajo nivel para manipular el uso del buffer
	// Posicion en el buffer real para memcpy
	unsigned int buff_pos = 0;
	// Lo que sigue es para asegurar la ultima posicion para el '\0'
	--buff_size;
	// Control de la lectura real
	unsigned int total = 0;
	unsigned int res = 0;
	while( total < size ){
		// read y verificacion de lectura
		res = read(sock_fd, inner_buff, inner_buff_size);
		if(res < 0){
			cout<<"ServerConnection::readString - Error en read.\n";
			break;
		}
		total += res;
		// Verificacion y escritura en buff
		if( buff_pos < buff_size ){
			if( (buff_size - buff_pos) < res ){
				res = buff_size - buff_pos;
			}
			memcpy( buff + buff_pos, inner_buff, res );
			buff_pos += res;
		}
	}
	//0 final
	buff[buff_pos] = 0;
	//verificacion de error (solo si no se leyo todo lo esperado)
	if( total < size ){
		return false;
	}
	return true;
}



