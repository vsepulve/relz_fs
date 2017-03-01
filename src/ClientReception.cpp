#include "ClientReception.h"

atomic<unsigned int> ClientReception::next_id(1);

ClientReception::ClientReception(){
	sock_fd = -1;
	instance_id = next_id++;
}

ClientReception::ClientReception(int listen_socket, struct sockaddr *addr, socklen_t *addrlen){
	sock_fd = accept(listen_socket, addr, addrlen);
	instance_id = next_id++;
}

ClientReception::ClientReception(const ClientReception &original){
	sock_fd = original.sock_fd;
	request = original.request;
	instance_id = next_id++;
	logger()<<"ClientReception - Copia (fd "<<original.sock_fd<<", id "<<instance_id<<")\n";
}

ClientReception::~ClientReception(){
	logger()<<"ClientReception::~ClientReception (id "<<instance_id<<")\n";
	//Cerrar socket
	if( sock_fd != -1 ){
		logger()<<"ClientReception::~ClientReception - cerrando fd "<<sock_fd<<", id "<<instance_id<<"\n";
		close(sock_fd);
	}
}

bool ClientReception::receiveRequest(CheckUser *users){
	logger()<<"ClientReception::receiveRequest (id "<<instance_id<<")\n";
	
	char user_ok = 1;
	if( ! request.receive(sock_fd) ){
		logger()<<"ClientReception::receiveRequest - Error al leer ID.\n";
		user_ok = 0;
	}
	if( (user_ok == 1) && (users != NULL) && ! users->valid(request.user_id, request.md5) ){
		logger()<<"Server::main - User invalido ("<<request.user_id<<", "<<request.md5<<").\n";
		user_ok = 0;
	}
	if( writeBytes(sock_fd, &user_ok, 1 ) != 1 ){
		logger()<<"Server::main - Error al confirmar usuario.\n";
		user_ok = 0;
	}
	return (user_ok == 1);
}

//int ClientReception::connectServer(const char *host, int port){
//	int res = 0;
//	struct hostent *server = NULL;
//	struct sockaddr_in serv_addr;
//	res = socket(AF_INET, SOCK_STREAM, 0);
//	if(res < 0){
//		cerr<<"ClientReception::connectServer - Error al crear socket.\n";
//		return -1;
//	}
//	server = gethostbyname(host);
//	if(server == NULL){
//		cerr<<"ClientReception::connectServer - Error al buscar host.\n";
//		return -1;
//	}
//	bzero( (char*)&serv_addr, sizeof(serv_addr) );
//	serv_addr.sin_family = AF_INET;
//	bcopy( (char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length );
//	serv_addr.sin_port = htons(port);
//	if( connect(res, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0 ){
//		cerr<<"ClientReception::connectServer - Error al conectar.\n";
//		close(res);
//		return -1;
//	}
//	return res;	
//}

// Envia un mensaje de request al server
// Retorna true en exito
//bool ClientReception::sendRequest(unsigned char req){
//	// Creacion y envio de request
//	RequestID request;
//	request.user_id = user_id;
//	request.type = req;
//	memset( request.md5, '0', 16 );
//	request.md5[16] = 0;
//	if( ! request.send(sock_fd) ){
//		return false;
//	}
//	// Recepcion de byte de aprobacion
//	char ok = 0;
//	if( readBytes(sock_fd, &ok, 1) != 1 || ok == 0 ){
//		return false;
//	}
//	return true;
//}




