#include "ServerConnection.h"

ServerConnection::ServerConnection(){
	user_id = 0;
	sock_fd = -1;
}

ServerConnection::ServerConnection(const char *host, int port, unsigned int _user_id){
	user_id = _user_id;
	sock_fd = connectServer(host, port);
}

ServerConnection::~ServerConnection(){
	//Envio de mensaje de termino
	writeInt(1);
	//Cerrar socket
	close(sock_fd);
}

int ServerConnection::connectServer(const char *host, int port){
	int res = 0;
	struct hostent *server = NULL;
	struct sockaddr_in serv_addr;
	res = socket(AF_INET, SOCK_STREAM, 0);
	if(res < 0){
		cerr<<"ServerConnection::connectServer - Error al crear socket.\n";
		return -1;
	}
	server = gethostbyname(host);
	if(server == NULL){
		cerr<<"ServerConnection::connectServer - Error al buscar host.\n";
		return -1;
	}
	bzero( (char*)&serv_addr, sizeof(serv_addr) );
	serv_addr.sin_family = AF_INET;
	bcopy( (char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length );
	serv_addr.sin_port = htons(port);
	if( connect(res, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0 ){
		cerr<<"ServerConnection::connectServer - Error al conectar.\n";
		close(res);
		return -1;
	}
	return res;	
}

// Envia un mensaje de request al server
// Retorna true en exito
bool ServerConnection::sendRequest(unsigned char req){
	// Creacion y envio de request
	RequestID request;
	request.user_id = user_id;
	request.type = req;
	memset( request.md5, '0', 16 );
	request.md5[16] = 0;
	if( ! request.send(sock_fd) ){
		return false;
	}
	// Recepcion de byte de aprobacion
	char ok = 0;
	if( readBytes(sock_fd, &ok, 1) != 1 || ok == 0 ){
		return false;
	}
	return true;
}










