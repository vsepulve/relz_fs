
#include <thread>
#include <mutex>

#include <iostream>
#include <sstream>
#include <fstream>

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include <map>

#include "CheckUser.h"
#include "ConcurrentLogger.h"

#include "Configuration.h"

#include "CommunicationUtils.h"
#include "ServerThreads.h"
#include "ClientReception.h"

using namespace std;

// Constantes (estas podrian ser constantes verdaras, o variables estaticas de algun objeto de configuracion)
unsigned int THREADS_PROCESO = 16;
unsigned int BLOCK_SIZE = 1000000;

int main(int argc, char* argv[]){
	
	if(argc != 2){
		cout << "uso: server config_file\n";
		return -1;
	}
	
	string config_file = argv[1];
	Configuration config;
	config.loadConfiguration(config_file);
	
//	int port = atoi(argv[1]);
//	const char *reference_file = argv[2];
	
	// Para usar el logger, primero debo agregar al user 0 (de sistema)
	// Esto deberia hacerse internamente en la clase y de modo estatico
	ConcurrentLogger::addUserLock(0);
//	ConcurrentLogger::setLogBase(config.log_base);
	
	logger()<<"------------------------------\n";
	logger()<<"Server::main - Inicio (port: "<< config.port <<")\n";
	
	CheckUser users("db/Users");
	
	// Inicio de enlace del puerto
	logger()<<"Server::main - Enlazando puerto "<< config.port <<"\n";
	logger()<<"Server::main - Creando Socket\n";
	int sock_servidor = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_servidor < 0){
		logger()<<"Server::main - Error al crear socket.\n";
		return 0;
	}
	// Datos para conexion de server
	struct sockaddr_in serv_addr;
	memset((char*)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(config.port);
	logger()<<"Server::main - Enlazando.\n";
	if (bind(sock_servidor, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		logger()<<"Server::main - Error en bind.\n";
		return 0;
	}

	logger()<<"Server::main - Iniciando listen.\n";
	listen(sock_servidor, 100);
	// Datos para conexion de cliente
	int sock_cliente = -1;
	// Esto es innecesario en nuestro caso
//	struct sockaddr_in cli_addr;
//	socklen_t clilen = sizeof(cli_addr);
	// Inicio de proceso
	bool procesar = true;
	while( procesar ){
		
		logger()<<"Server::main - Esperando clientes.\n";
		// La idea es convertir el accept en la construccion de un objeto adecuado para la comunicacion
		// Este objeto, sin embargo, NO puede ser pasado al thread y al mismo tiempo encargarse del cerrado del socket
		// Esto es porque al iniciarse el thread, se realiza MAS de una copia.
		// Las dos opciones son usar un puntero o pasar los valores para que el thread construya un nuevo objeto de conexion propio
		// En esta version uso esa opcion, le quito el socket al objeto local y creare otro en el thread.
//		ClientReception conexion(sock_servidor, (struct sockaddr *)&cli_addr, &clilen);
		ClientReception conexion(sock_servidor, NULL, NULL);
		logger()<<"Server::main - Cliente aceptado.\n";
		
		if( ! conexion.receiveRequest(&users) ){
			logger()<<"Server::main - Error al recibir Request.\n";
			continue;
		}
		
		// Antes de iniciar un thread detached, hay que ajustar el objeto de conexion local para que no cierre la conexion.
		// Eso queda en manos del thread que reciba los datos (sea por copia del objeto, o de su contenido)
		// SI DEBE CERRAR la conexion cuando ningun thread es iniciado
		
		logger()<<"Server::main - user "<<conexion.getUser()<<", request "<<(unsigned int)conexion.getType()<<"\n";
		ConcurrentLogger::addUserLock(conexion.getUser());
		switch( conexion.getType() ){
			case 0:
				logger()<<"Server::main - Request vacio, ignorando.\n";
				break;
			case REMOTE_STAT:
				logger()<<"Server::main - Creando thread_stat para user "<<conexion.getUser()<<".\n";
				sock_cliente = conexion.getSocket();
				conexion.setSocket(-1);
				thread( thread_stat, sock_cliente, conexion.getUser(), &config ).detach();
				break;
			case REMOTE_ACCESS:
				logger()<<"Server::main - Creando thread_access para user "<<conexion.getUser()<<".\n";
				sock_cliente = conexion.getSocket();
				conexion.setSocket(-1);
				thread( thread_access, sock_cliente, conexion.getUser(), &config ).detach();
				break;
			case REMOTE_READDIR:
				logger()<<"Server::main - Creando thread_readdir para user "<<conexion.getUser()<<".\n";
				sock_cliente = conexion.getSocket();
				conexion.setSocket(-1);
				thread( thread_readdir, sock_cliente, conexion.getUser(), &config ).detach();
				break;
			case REMOTE_MKNOD:
				logger()<<"Server::main - Creando thread_mknod para user "<<conexion.getUser()<<".\n";
				sock_cliente = conexion.getSocket();
				conexion.setSocket(-1);
				thread( thread_mknod, sock_cliente, conexion.getUser(), &config ).detach();
				break;
			case REMOTE_MKDIR:
				logger()<<"Server::main - Creando thread_mkdir para user "<<conexion.getUser()<<".\n";
				sock_cliente = conexion.getSocket();
				conexion.setSocket(-1);
				thread( thread_mkdir, sock_cliente, conexion.getUser(), &config ).detach();
				break;
			case REMOTE_UNLINK:
				logger()<<"Server::main - Creando thread_unlink para user "<<conexion.getUser()<<".\n";
				sock_cliente = conexion.getSocket();
				conexion.setSocket(-1);
				thread( thread_unlink, sock_cliente, conexion.getUser(), &config ).detach();
				break;
			case REMOTE_RMDIR:
				logger()<<"Server::main - Creando thread_rmdir para user "<<conexion.getUser()<<".\n";
				sock_cliente = conexion.getSocket();
				conexion.setSocket(-1);
				thread( thread_rmdir, sock_cliente, conexion.getUser(), &config ).detach();
				break;
			case REMOTE_RENAME:
				logger()<<"Server::main - Creando thread_rename para user "<<conexion.getUser()<<".\n";
				sock_cliente = conexion.getSocket();
				conexion.setSocket(-1);
				thread( thread_rename, sock_cliente, conexion.getUser(), &config ).detach();
				break;
			case REMOTE_CHMOD:
				logger()<<"Server::main - Creando thread_chmod para user "<<conexion.getUser()<<".\n";
				sock_cliente = conexion.getSocket();
				conexion.setSocket(-1);
				thread( thread_chmod, sock_cliente, conexion.getUser(), &config ).detach();
				break;
			case REMOTE_CHOWN:
				logger()<<"Server::main - Creando thread_chown para user "<<conexion.getUser()<<".\n";
				sock_cliente = conexion.getSocket();
				conexion.setSocket(-1);
				thread( thread_chown, sock_cliente, conexion.getUser(), &config ).detach();
				break;
			case REMOTE_TRUNCATE:
				logger()<<"Server::main - Creando thread_truncate para user "<<conexion.getUser()<<".\n";
				sock_cliente = conexion.getSocket();
				conexion.setSocket(-1);
				thread( thread_truncate, sock_cliente, conexion.getUser(), &config ).detach();
				break;
			case REMOTE_STATFS:
				logger()<<"Server::main - Creando thread_statfs para user "<<conexion.getUser()<<".\n";
				sock_cliente = conexion.getSocket();
				conexion.setSocket(-1);
				thread( thread_statfs, sock_cliente, conexion.getUser(), &config ).detach();
				break;
			case REMOTE_FALLOCATE:
				logger()<<"Server::main - Creando thread_fallocate para user "<<conexion.getUser()<<".\n";
				sock_cliente = conexion.getSocket();
				conexion.setSocket(-1);
				thread( thread_fallocate, sock_cliente, conexion.getUser(), &config ).detach();
				break;
			case REMOTE_RECEIVE:
				logger()<<"Server::main - Creando thread_receive para user "<<conexion.getUser()<<".\n";
				sock_cliente = conexion.getSocket();
				conexion.setSocket(-1);
				thread( thread_receive, sock_cliente, conexion.getUser(), &config ).detach();
				break;
			case KILL_SERVER:
				logger()<<"Server::main - Cerrando Server.\n";
				close( sock_servidor );
				procesar = false;
				break;
			default:
				logger()<<"Server::main - Handler NO definido para usuario "<<conexion.getUser()<<", ignorando.\n";
				break;
		}
		
	}//while... true
	
	//Fin conversacion con cliente
	logger()<<"Server::main - Finzalizando.\n";
	
}



