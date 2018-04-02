
// Este programa es un demonio de fuse, pero de uso directo de metodos
// Lo unico que hace es asociar un directorio fisico a un punto de montaje
// Interviene todas las funciones necesarias traduciendo las rutas del punto de montaje (agregando el directorio base)
// Esta version no guarda la estructura de directorios en memoria, todo lo hace directamente

//Definicion e includes necesarios para FUSE
#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <errno.h>
#include <fcntl.h>

//Otros includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

//Para directorios y stat archivos
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
//unistd es para pathconf
#include <unistd.h>
//stddef es para offsetof
#include <stddef.h>

#include <mutex>

#include "NanoTimer.h"
#include "ConcurrentLogger.h"

using namespace std;

// Prueba de FUSE

// Se usa como (el directorio de montaje debe estar creado, el -d es para debug):
// > ./bin/demonio_fuse_direct /cebib -d

// Recordar cerrarlo con (o usarlo si el programa se cae para desmontar el directorio):
// > fusermount -u /tmp/fuse

//Estructura para enlazar las funciones
static struct fuse_operations my_oper;
static char *base_path = NULL;

//Metodo generico para juntar dos paths (deberia pertenecer a algun tipo de "utils")
//Retorna un c-string valido (vacio si ambas entradas son NULL, o con un path que siempre empieza por '/')
//Este metodo ASUME que buff tiene al menos strlen(path1) + strlen(path2) + 2 chars
void joint_path(const char *path1, const char *path2, char *buff){
	//Verificacion de NULL (ambos, el primero, o el segundo)
	//Notar que se puede evitar un par de comparaciones anidando en ambos sentidos
	//Lo dejo por legibilidad
	if(path1 == NULL && path2 == NULL){
		buff[0] = 0;
		return;
	}
	if(path1 == NULL){
		if(path2[0] == '/'){
			strcpy(buff, path2);
		}
		else{
			buff[0] = '/';
			strcpy(buff + 1, path2);
		}
		return;
	}
	if(path2 == NULL){
		if(path1[0] == '/'){
			strcpy(buff, path1);
		}
		else{
			buff[0] = '/';
			strcpy(buff + 1, path1);
		}
		return;
	}
	//Aqui ninguna ruta es NULL
	if( path1[strlen(path1) - 1] != '/' && path2[0] != '/'){
		//Ninguno tiene '/'
		sprintf(buff, "%s/%s", path1, path2);
	}
	else if( path1[strlen(path1) - 1] == '/' && path2[0] == '/'){
		//Ambos tienen '/' (omito el primer char de path2)
		sprintf(buff, "%s%s", path1, path2 + 1);
	}
	else{
		//Solo uno lo tiene (no importa cual)
		sprintf(buff, "%s%s", path1, path2);
	}
	return;
}

void colorRed(){
	cout<<"\033[1;31m";
}

void colorGreen(){
	cout<<"\033[1;32m";
}

void colorYellow(){
	cout<<"\033[1;33m";
}

void colorBlue(){
	cout<<"\033[1;34m";
}

void colorReset(){
	cout<<"\033[0m";
}


static int my_getattr(const char *path, struct stat *stbuf) {
	cout<<" ---> my_getattr - \""<<path<<"\" \n";
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path( base_path, path, real_path );
	int res = lstat(real_path, stbuf);
	if (res == -1){
		return -errno;
	}
	return 0;
}

static int my_access(const char *path, int mask){
	cout<<" ---> my_access - \""<<path<<"\" \n";
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path( base_path, path, real_path );
	int res = access(real_path, mask);
	if (res == -1){
		return -errno;
	}
	return 0;
}

static int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *flags) {
	(void) offset;
	(void) flags;
	cout<<" ---> my_readdir - \""<<path<<"\" \n";
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path( base_path, path, real_path );
	//Esto es usando readdir_r para que sea thread-safe, pero usa malloc/free
	DIR *directory = NULL;
	dirent *child_dir = NULL;
	dirent *test_dir = NULL;
	//burocracia para construir el struct con memoria pedida
	int name_max = pathconf(real_path, _PC_NAME_MAX);
	//Si el limite no esta definido, adivino
	if (name_max == -1){
		name_max = 255;
	}
	int len = offsetof(struct dirent, d_name) + name_max + 1;
	child_dir = (struct dirent*)malloc(len);
	directory = opendir(real_path);
	if (directory == NULL){
		free(child_dir);
		return -ENOENT;
	}
	while(true){
		readdir_r( directory, child_dir, &test_dir );
		if(test_dir == NULL){
			break;
		}
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = child_dir->d_ino;
		st.st_mode = child_dir->d_type << 12;
		//filler CON stat (notar el mode)
		if( filler(buf, child_dir->d_name, &st, 0) ){
			break;
		}
	}
	free(child_dir);
	closedir(directory);
	return 0;
}

//Crear el archivo (vacio) en path
//Creo que por ahora se puede omitir mode y dev
static int my_mknod(const char *path, mode_t mode, dev_t dev){
	colorYellow();
	cout<<" ---> my_mknod - \""<<path<<"\" (mode: "<<mode<<")\n";
	colorReset();
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path(base_path, path, real_path);
	//Creacion del archivo real
	//En teoria basta con un mknod, pero esto es mas portable y seguro
	int res = -1;
	if( S_ISREG(mode) ){
		cout<<" ---> my_mknod - S_ISREG \n";
		res = open(real_path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if(res >= 0){
			cout<<" ---> my_mknod - Open Ok \n";
			res = close(res);
		}
	}
	else if( S_ISFIFO(mode) ){
		cout<<" ---> my_mknod - S_ISFIFO \n";
		res = mkfifo(real_path, mode);
	}
	else{
		cout<<" ---> my_mknod - ELSE \n";
		res = mknod(real_path, mode, dev);
	}
	//Revision de estado
	if(res == -1){
		cout<<" ---> my_mknod - Error al crear archivo real \n";
		return -errno;
	}
	return 0;
}

static int my_mkdir(const char *path, mode_t mode){
	cout<<" ---> my_mkdir - \""<<path<<"\" \n";
	//Crear directorio real
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path(base_path, path, real_path);
	int res = mkdir(path, mode);
	if (res == -1){
		return -errno;
	}

	return 0;
}

//Eliminar el archivo de path
static int my_unlink(const char *path){
	cout<<" ---> my_unlink - \""<<path<<"\" \n";
	//Borrar archivo real
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path( base_path, path, real_path );
	int res = unlink(real_path);
	if (res == -1){
		return -errno;
	}
	return 0;
}

//Eliminar el directorio de path
static int my_rmdir(const char *path){
	cout<<" ---> my_rmdir - \""<<path<<"\" \n";
	//Borrar directorio real
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path( base_path, path, real_path );
	int res = rmdir(real_path);
	if (res == -1){
		return -errno;
	}
	return 0;
}

static int my_rename(const char *from, const char *to){
	cout<<" ---> my_rename - \""<<from<<"\" -> \""<<to<<"\" \n";
	char real_from[ strlen(base_path) + strlen(from) + 2 ];
	joint_path( base_path, from, real_from );
	char real_to[ strlen(base_path) + strlen(to) + 2 ];
	joint_path( base_path, to, real_to );
	int res = rename(real_from, real_to);
	if (res == -1){
		return -errno;
	}
	return 0;
}

static int my_chmod(const char *path, mode_t mode){
	cout<<" ---> my_chmod - \""<<path<<"\" \n";
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path( base_path, path, real_path );
	int res = chmod(real_path, mode);
	if (res == -1){
		return -errno;
	}
	return 0;
}

static int my_chown(const char *path, uid_t uid, gid_t gid){
	cout<<" ---> my_chown - \""<<path<<"\" \n";
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path( base_path, path, real_path );
	int res = lchown(real_path, uid, gid);
	if (res == -1){
		return -errno;
	}
	return 0;
}

static int my_truncate(const char *path, off_t size){
	colorYellow();
	cout<<" ---> my_truncate - Inicio (\""<<path<<"\", offset "<<size<<")\n";
	colorReset();
	//truncate del archivo real
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path( base_path, path, real_path );
	int res = truncate(real_path, size);
	if (res == -1){
		return -errno;
	}
	return 0;
}

static int my_utimens(const char *path, const struct timespec ts[2]){
	cout<<" ---> my_utimens - Inicio (\""<<path<<"\")\n";
//	char real_path[ strlen(base_path) + strlen(path) + 2 ];
//	joint_path( base_path, path, real_path );
//	int res = utimensat(0, real_path, ts, AT_SYMLINK_NOFOLLOW);
//	cout<<" ---> my_utimens - res: "<<res<<" (de \""<<real_path<<"\")\n";
//	if (res == -1){
//		return -errno;
//	}
	return 0;
}

static int my_open(const char *path, struct fuse_file_info *flags) {
	colorRed();
	cout<<" ---> my_open - \""<<path<<"\" ("<<( (flags->flags & O_WRONLY)?"-- W --":"-- R --" )<<")\n";
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path( base_path, path, real_path );
	int fd = -1;
	fd = open(real_path, flags->flags);
	if (fd == -1){
		colorReset();
		return -errno;
	}
	flags->fh = fd;
	cout<<" ---> my_open - Archivo abierto en fd: "<<flags->fh<<"\n";
	colorReset();
	return 0;
}

static int my_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *flags) {
	colorGreen();
	cout<<" ---> my_read - Inicio (\""<<path<<"\", "<<size<<" bytes desde "<<offset<<")\n";
	int fd = -1;
	bool close_fd = false;
	if(flags == NULL || flags->fh == 0){
		//abrir el archivo localmente
		char real_path[ strlen(base_path) + strlen(path) + 2 ];
		joint_path( base_path, path, real_path );
		fd = open( real_path, flags->flags );
		close_fd = true;
		if (fd == -1){
			colorReset();
			return -errno;
		}
	}
	else{
		fd = flags->fh;
	}
	int res = pread(fd, buf, size, offset);
	if(res == -1){
		res = -errno;
	}
	if(close_fd){
		close(fd);
	}
	cout<<" ---> my_read - Fin ("<<res<<" bytes)\n";
	colorReset();
	return res;
}

//Intenta escribir en el archivo de path.
//Agrega size bytes de buf, desde offset en el archivo objetivo.
//Debe retornar el numero real de bytes escritos.
//Retornar un numero != size puede ser considerado un error por el S.O.
static int my_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *flags){
	colorYellow();
	cout<<" ---> my_write - Inicio (\""<<path<<"\", "<<size<<" bytes desde "<<offset<<")\n";
	cout<<" ---> my_write - fd: "<<( (flags==NULL)?0:(flags->fh) )<<"\n";
	//Version C
	int fd = -1;
	bool close_fd = false;
	if(flags == NULL || flags->fh == 0){
		//abrir el archivo localmente
		char real_path[ strlen(base_path) + strlen(path) + 2 ];
		joint_path( base_path, path, real_path );
		fd = open(real_path, O_WRONLY);
		close_fd = true;
		if (fd == -1){
			colorReset();
			return -errno;
		}
	}
	else{
		fd = flags->fh;
	}
	int res = pwrite(fd, buf, size, offset);
	if (res == -1){
		res = -errno;
	}
	if(close_fd){
		close(fd);
	}
	cout<<" ---> my_write - Fin ("<<res<<" bytes)\n";
	colorReset();
	return res;
}

static int my_statfs(const char *path, struct statvfs *stbuf){
	cout<<" ---> my_statfs - \""<<path<<"\"\n";
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path( base_path, path, real_path );
	int res = statvfs(real_path, stbuf);
	if (res == -1){
		return -errno;
	}
	return 0;
}

//Flush is called on each close() of a file descriptor.
//The flush() method may be called more than once for each open().
//Esto implica que el metodo se llama varias veces en un mismo proceso de escritura.
//Por ejemplo, se llama despues de crear un archivo (antes de write) y despues de write (antes de release)
static int my_flush(const char *path, struct fuse_file_info *flags){
	cout<<" ---> my_flush - \""<<path<<"\"\n";
	..
	return 0;
}

//Release is called when there are no more references to an open file
//For every open() call there will be exactly one release()
//Solo el ultimo release implica que no hay mas lectura/escritura
static int my_release(const char *path, struct fuse_file_info *flags){
	colorRed();
	cout<<" ---> my_release - \""<<path<<"\"\n";
	//Omito los 0, 1 y 2 (los estandar)
	if(flags != NULL && flags->fh > 2){
		cout<<" ---> my_release - Cerrando fd "<<flags->fh<<" ("<<( (flags->flags & O_WRONLY)?"-- W --":"-- R --" )<<")\n";
		close(flags->fh);
		flags->fh = 0;
	}
	colorReset();
	return 0;
}

//Veo llamadas a este metodo con un simple "> touch /tmp/cebib/test.txt"
static int my_releasedir(const char *path, struct fuse_file_info *flags){
	cout<<" ---> my_releasedir - \""<<path<<"\"\n";
	
	return 0;
}

static int my_fallocate(const char *path, int mode, off_t offset, off_t length, struct fuse_file_info *fi) {
	(void) fi;
	cout<<" ---> my_fallocate - \""<<path<<"\"\n";
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path( base_path, path, real_path );
	if(mode){
		return -EOPNOTSUPP;
	}
	int fd = open(real_path, O_WRONLY);
	if (fd == -1){
		return -errno;
	}
	int res = -posix_fallocate(fd, offset, length);
	close(fd);
	return res;
}

int main(int argc, char *argv[]) {
	
	cout<<"Inicio - Preparando variables estaticas\n";
	
	//Enlace de funciones (a la C++) en el struct fuse_operations definido al inicio
	my_oper.getattr = my_getattr;
	my_oper.access = my_access;
	my_oper.readdir = my_readdir;
	my_oper.mknod = my_mknod;
	my_oper.mkdir = my_mkdir;
	my_oper.unlink = my_unlink;
	my_oper.rmdir = my_rmdir;
	my_oper.rename = my_rename;
	my_oper.chmod = my_chmod;
	my_oper.chown = my_chown;
	my_oper.truncate = my_truncate;
	my_oper.utimens = my_utimens;
	my_oper.open = my_open;
	my_oper.read = my_read;
	my_oper.write = my_write;
	my_oper.statfs = my_statfs;
	my_oper.flush = my_flush;
	my_oper.release = my_release;
	my_oper.releasedir = my_releasedir;
	my_oper.fallocate = my_fallocate;
	
	base_path = (char*)"test/";
	
	cout<<"Inicio - Iniciando fuse_main\n";
	
	return fuse_main(argc, argv, &my_oper, NULL);
}









