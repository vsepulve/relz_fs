#ifndef _SERVER_CONNECTION_H
#define _SERVER_CONNECTION_H

#include "Communicator.h"

using namespace std;

//class ServerConnection;
//typedef ServerConnection connection;

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

class ServerConnection : public Communicator {
private:
//	int sock_fd;

	unsigned int user_id;
	
	int connectServer(const char *host, int port);
	
public:
	
	ServerConnection();
	
	ServerConnection(const char *host, int port, unsigned int _user_id);
	
	virtual ~ServerConnection();
	
	// Envia un mensaje de request al server
	// Retorna true en exito
	bool sendRequest(unsigned char req);
	
};








#endif //_SERVER_CONNECTION_H
