#include "Communicator.h"

Communicator::Communicator(){
	sock_fd = -1;
}

Communicator::~Communicator(){
}

// Retorna true en conexion correcta
// De momento solo verifica (sock_fd >= 0)
bool Communicator::good(){
	return (sock_fd >= 0);
}

// Envia un c-string
// Primero envia el largo (strlen) en 4 bytes, luego los chars
// Retorna true en exito
bool Communicator::writeString(const char *buff){
	unsigned int size = 0;
	if( buff != NULL ){
		size = strlen(buff);
	}
	if( ! writeUInt(size) ){
		return false;
	}
	if( writeBytes(sock_fd, buff, size) != size ){
		return false;
	}
	return true;
}

// Lee un c-string
// Primero lee el largo en 4 bytes, luego los chars
// Escribe los chars leidos en buff, hasta un maximo de (buff_size - 1)
// Finalmente agrega el '\0' final
// Notar que esta llamada lee pero omite chars por sobre (buff_size - 1), trunca el string
// Retorna true en exito
bool Communicator::readString(char *buff, unsigned int buff_size){
	// Buffer interno para la lectura
	// Notar que este buffer se creara en CADA readString
	// Esto ES INEFICIENTE, pero es claro para esta interface particular
	// Para evitar esto, podria usarse una interface que reciba el buffer intermedio adicial al de la salida
	unsigned int inner_buff_size = 512;
	char inner_buff[inner_buff_size];
	if( ! readStringSimple(sock_fd, buff, buff_size, inner_buff, inner_buff_size) ){
		return false;
	}
	return true;
}
	
// Envia el int (4 bytes) de num
// Retorna true en lectura exitosa
bool Communicator::writeInt(int num){
	if( writeBytes(sock_fd, (char*)&num, sizeof(int)) != sizeof(int) ){
		return false;
	}
	return true;
}

// Envia el unsigned int (4 bytes) de num
// Retorna true en lectura exitosa
bool Communicator::writeUInt(unsigned int num){
	if( writeBytes(sock_fd, (char*)&num, sizeof(int)) != sizeof(int) ){
		return false;
	}
	return true;
}

// Envia el long long (8 bytes) de num
// Retorna true en lectura exitosa
bool Communicator::writeLong(long long num){
	if( writeBytes(sock_fd, (char*)&num, sizeof(long long)) != sizeof(long long) ){
		return false;
	}
	return true;
}

// Envia el unsigned long long (8 bytes) de num
// Retorna true en lectura exitosa
bool Communicator::writeULong(unsigned long long num){
	if( writeBytes(sock_fd, (char*)&num, sizeof(long long)) != sizeof(long long) ){
		return false;
	}
	return true;
}

// Envia el unsigned char (1 bytes) de num
// Retorna true en lectura exitosa
bool Communicator::writeByte(unsigned char num){
	if( writeBytes(sock_fd, (char*)&num, 1) != 1 ){
		return false;
	}
	return true;
}

// Lee un int (4 bytes) y lo guarda en num
// Retorna true en lectura exitosa
bool Communicator::readInt(int &num){
	if( readBytes(sock_fd, (char*)&num, sizeof(int)) != sizeof(int) ){
		return false;
	}
	return true;
}

// Lee un unsigned int (4 bytes) y lo guarda en num
// Retorna true en lectura exitosa
bool Communicator::readUInt(unsigned int &num){
	if( readBytes(sock_fd, (char*)&num, sizeof(int)) != sizeof(int) ){
		return false;
	}
	return true;
}

// Lee un long long (8 bytes) y lo guarda en num
// Retorna true en lectura exitosa
bool Communicator::readLong(long long &num){
	if( readBytes(sock_fd, (char*)&num, sizeof(long long)) != sizeof(long long) ){
		return false;
	}
	return true;
}

// Lee un unsigned long long (8 bytes) y lo guarda en num
// Retorna true en lectura exitosa
bool Communicator::readULong(unsigned long long &num){
	if( readBytes(sock_fd, (char*)&num, sizeof(long long)) != sizeof(long long) ){
		return false;
	}
	return true;
}

// Lee un unsigned char (1 bytes) y lo guarda en num
// Retorna true en lectura exitosa
bool Communicator::readByte(unsigned char &num){
	if( readBytes(sock_fd, (char*)&num, 1) != 1 ){
		return false;
	}
	return true;
}

bool Communicator::writeData(const char *buff, unsigned int size){
	return (writeBytes(sock_fd, buff, size) == size);
}

bool Communicator::readData(char *buff, unsigned int size){
	return (readBytes(sock_fd, buff, size) == size);
}












