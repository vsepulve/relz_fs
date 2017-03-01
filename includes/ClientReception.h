#ifndef _CLIENT_RECEPTION_H
#define _CLIENT_RECEPTION_H

#include "ConcurrentLogger.h"
#include "Communicator.h"
#include "CheckUser.h"

#include <atomic>

using namespace std;

// Asumo por el momento que TODAS las interacciones tienen la forma:
//  - Conexion
//  - Request (que genera un thread en el server para el servicio)
//  - Envio de datos del cliente (pregunta)
//  - Envio de datos del server (respuesta)
//  - (otras preguntas/respuestas pueden seguir siempre en pares, detalles aun sin determinar)
//  - Señal de termino del cliente (un byte particular)
//  - close del cliente
//  - Recepcion de señal del server
//  - close del server (y termino del thread de servicio)
// Notar que siempre espero una pregunta del cliente, una respuesta del server
// ...y una señal final de cliente antes del close

class ClientReception : public Communicator {
private:
//	int sock_fd;
	RequestID request;
	
	static atomic<unsigned int> next_id;
	unsigned int instance_id;
	
public:
	
	ClientReception();
	
	// Constructor accept (inicializa el sock_fd llamando a accept con los parametros directamente)
	ClientReception(int listen_socket, struct sockaddr *addr, socklen_t *addrlen);
	
	// Constructor de copia (NO copia instance_id, genera uno nuevo)
	ClientReception(const ClientReception &original);
	
	virtual ~ClientReception();
	
	bool receiveRequest(CheckUser *users);
	
	int getSocket(){
		return sock_fd;
	}
	
	void setSocket(int _sock_fd){
		sock_fd = _sock_fd;
	}
	
	unsigned int getUser(){
		return request.user_id;
	}
	
	unsigned char getType(){
		return request.type;
	}
	
	
};








#endif //_CLIENT_RECEPTION_H
