#ifndef _SERVER_THREADS_H
#define _SERVER_THREADS_H

#include <thread>
#include <mutex>

#include <iostream>
#include <sstream>
#include <fstream>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>

//Para directorios y stat archivos
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
//unistd es para pathconf
#include <unistd.h>
//stddef es para offsetof
#include <stddef.h>
// para el struct statvfs
#include <sys/statvfs.h>

#include "ConcurrentLogger.h"
#include "CommunicationUtils.h"
#include "ClientReception.h"

#include "Configuration.h"

using namespace std;

// tipos de request soportados por el server
// Valores validos en [1, 255 (0xff)]
enum{
	REMOTE_STAT = 1, 
	REMOTE_ACCESS = 2, 
	REMOTE_READDIR = 3, 
	REMOTE_MKNOD = 4, 
	REMOTE_MKDIR = 5, 
	REMOTE_UNLINK = 6, 
	REMOTE_RMDIR = 7, 
	REMOTE_RENAME = 8, 
	REMOTE_CHMOD = 9, 
	REMOTE_CHOWN = 10, 
	REMOTE_TRUNCATE = 11, 
	REMOTE_STATFS = 12, 
	REMOTE_FALLOCATE = 13, 
	REMOTE_OPEN = 14, 
	REMOTE_RELEASE = 15, 
	REMOTE_READ = 16, 
	REMOTE_WRITE = 17, 
	REMOTE_RECEIVE = 18, 
	REMOTE_SEND = 19, 
	REMOTE_INITIALIZE = 50, 
	KILL_SERVER = 100
};

// Por ahora defino estas funcionalidades como funcion global
// Quizas podrian ser metodos estaticos de una clase ServerThreads o algo similar

// Funciones para generar ruta real localmente
// Estos podrian ser metodos de algun objeto apropiado, de configuracion del server quizas
unsigned int real_path_size(const char *base_path, const char *path);
void create_real_path(char *buff, const char *base_path, unsigned int user, const char *path);

void thread_stat(int sock_cliente, unsigned int user_id, Configuration *config);

void thread_access(int sock_cliente, unsigned int user_id, Configuration *config);

void thread_readdir(int sock_cliente, unsigned int user_id, Configuration *config);

void thread_mknod(int sock_cliente, unsigned int user_id, Configuration *config);

void thread_mkdir(int sock_cliente, unsigned int user_id, Configuration *config);

void thread_unlink(int sock_cliente, unsigned int user_id, Configuration *config);

void thread_rmdir(int sock_cliente, unsigned int user_id, Configuration *config);

void thread_rename(int sock_cliente, unsigned int user_id, Configuration *config);

void thread_chmod(int sock_cliente, unsigned int user_id, Configuration *config);

void thread_chown(int sock_cliente, unsigned int user_id, Configuration *config);

void thread_truncate(int sock_cliente, unsigned int user_id, Configuration *config);

void thread_statfs(int sock_cliente, unsigned int user_id, Configuration *config);

void thread_fallocate(int sock_cliente, unsigned int user_id, Configuration *config);

void thread_receive(int sock_cliente, unsigned int user_id, Configuration *config);

void thread_send(int sock_cliente, unsigned int user_id, Configuration *config);


// Thread de carga inicial
// Luego de recibir la peticion, el server leera todos los archivos del directorio del usuario
// Envia cada archivo, primero la ruta, luego el contenido, para que el cliente inicie
// Quzias sea necesario enviar directorios por separado
void thread_initialize(int sock_cliente, unsigned int user_id, Configuration *config);














#endif //_SERVER_THREADS_H
