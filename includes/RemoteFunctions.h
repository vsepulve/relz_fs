#ifndef _REMOTE_FUNCTIONS_H
#define _REMOTE_FUNCTIONS_H

#include <iostream>
#include <sstream>
#include <fstream>

// Para directorios y stat archivos
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
// unistd es para pathconf
#include <unistd.h>
// stddef es para offsetof
#include <stddef.h>
// para el struct statvfs
#include <sys/statvfs.h>

#include "ServerConnection.h"
#include "ServerThreads.h"

using namespace std;

class RemoteFunctions {
private: 
	// URL del server
	const char *host;
	// Puerto del server
	unsigned int port;
	// ID del usuario virtual de este cliente
	unsigned int user_id;

public:
	
	RemoteFunctions();
	
	RemoteFunctions(const char *_host, unsigned int _port, unsigned int _user_id);
	
	virtual ~RemoteFunctions();
	
	void setParameters(const char *_host, unsigned int _port, unsigned int _user_id);
	
	int stat(const char *path, struct stat *stbuf);
	
	int access(const char *path, int mask);
	
	// Este es especial, retorna un char* pedido con los datos serializados
	// El llamador debe encargarse de liberar la memoria
	// Retorna NULL en caso de error o problema de algun tipo
	char *readdir(const char *path);
	
	int mknod(const char *path, mode_t mode, dev_t dev);
	
	int mkdir(const char *path, mode_t mode);
	
	int unlink(const char *path);
	
	int rmdir(const char *path);
	
	int rename(const char *from, const char *to);
	
	int chmod(const char *path, mode_t mode);
	
	int chown(const char *path, uid_t uid, gid_t gid);
	
	int truncate(const char *path, off_t size);
	
	int statfs(const char *path, struct statvfs *stbuf);
	
	int fallocate(const char *path, int mode, off_t offset, off_t length, int flags);
	
	// Notar que recibo flags interno (no el de fuse, omito el fd)
	// En el nuevo modelo, este metodo (o su llamador, demonio::my_open) deberia asegurar el archivo local
	int open(const char *path, int flags);
	
	// Notar que recibo flags interno (no el de fuse, omito el fd)
	// En el nuevo modelo, este metodo  (o su llamador, demonio::my_release) deberia enviar el archivo al server
	int release(const char *path, int flags);
	
	// Notar que recibo flags interno (no el de fuse, omito el fd)
	// En el nuevo modelo, este metodo puede NO SER NECESARIO (read local)
	int read(const char *path, char *buf, size_t size, off_t offset, int flags);
	
	// Notar que recibo flags interno (no el de fuse, omito el fd)
	// En el nuevo modelo, este metodo puede NO SER NECESARIO (write local)
	int write(const char *path, const char *buf, size_t size, off_t offset, int flags);
	
	// Metodo para copiar un archivo del server a una ruta local
	// La idea es que open llame a este metodo y lo abra para read / write local
	int receiveFile(const char *path, const char *path_local);
	
	// Metodo para enviar un archivo al server desde una ruta local
	// La idea es que release llame a este metodo para dejar el archivo en el server
	int sendFile(const char *path, const char *path_local);
	
};





#endif //_SERVER_THREADS_H
