
//Definicion e includes necesarios para FUSE
//Notar que incluyo a errno para responder los errores apropiados

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

#include <mutex>

#include "ReferenceIndex.h"
#include "ReferenceIndexBasic.h"
#include "NanoTimer.h"
#include "DecoderBlocks.h"
#include "FilesTree.h"

using namespace std;

// Prueba de FUSE

// Se usa como (el directorio de montaje debe estar creado, el -d es para debug):
// > ./bin/prueba_fuse /tmp/cebib -d

// Recordar cerrarlo con (o usarlo si el programa se cae para desmontar el directorio):
// > fusermount -u /tmp/fuse

// Para el color en cout estoy usando:
// "\033[1;31m TEXTO \033[0m" donde el 31 es ROJO (32 VERDE, 34 AZUL, etc)
// "https://en.wikipedia.org/wiki/ANSI_escape_code#graphics" para mas detalles
// Otra forma es "\033[1;38;5;4m TEXTO \033[0m" donde "4" (entre "5;" y "m") es celeste (0-255 son los colores permitidos)

// Para las operaciones de fuse: http://fuse.sourceforge.net/doxygen/structfuse__operations.html

//Varialbes estaticas (OJO que no se deletea esto)
static const char *archivo_referencia = "datos/referencia_y01.bin";
static ReferenceIndexBasic *referencia;

//Estructura para enlazar las funciones
static struct fuse_operations my_oper;

//Estructura para arbol de archivos
static FilesTree *files_tree;

//Se realizan llamadas concurrentes a read (que usa al decoder)
//Esas llamadas deben ser exlusivas (no asi los metodos de solo lectura, como getTextSize)
static std::mutex global_mutex;
//Variable para debug de concurrencia
//static unsigned int next_id = 0;


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

//Este metodo se llama muy a menudo y debe ser muy rapido
//Si es necesario procesar algo, quizas deba ser cacheado
static int my_getattr(const char *path, struct stat *stbuf) {
	cout<<" ---> \033[1;31m my_getattr \033[0m - \""<<path<<"\" \n";
	memset(stbuf, 0, sizeof(struct stat));
	FileNode *node = files_tree->find(path);
	if(node == NULL){
		return -ENOENT;
	}
	else if( node->isDirectory() ){
		//Directorio y permisos
		stbuf->st_mode = S_IFDIR | 0755;
		//Directorio => 2
		stbuf->st_nlink = 2;
		return 0;
	}
	else{
		//Archivo regular y permisos
		stbuf->st_mode = S_IFREG | 0644;
		//archivo normal (sin links) => 1
		stbuf->st_nlink = 1;
		//Tamaño del archivo de algun modo
		if(node->decoder != NULL){
			//En el caso de archivo comprimido, agrego el '\n' final
			stbuf->st_size = node->getSize() + 1;
		}
		else{
			stbuf->st_size = node->getSize();
		}
		return 0;
	}
}

static int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *flags) {
	(void) offset;
	(void) flags;
	cout<<" ---> \033[1;31m my_readdir \033[0m - \""<<path<<"\" \n";
	//En esta version omito offset
	//Es decir, agrego todo el contenido del directorio al buf
	//Por esto le paso 0 al offset de filler, y omito su resultado
	//En otros casos, el retorno de filler indica si el buf esta lleno
	//Notar que agrego "." y ".." a mano
	//Obviamente falta una forma transparente de iterar por los hijos del nodo
	//Eso no es directo, pues debe ser resistente a concurrencia
	FileNode *node = files_tree->find(path);
	if( node != NULL && node->isDirectory() ){
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		FileNode *child = node->first_child;
		while(child != NULL){
			//El nombre debe contener al menos '/' (que se saca de la escritura)
			if(child->name.size() > 1){
				filler(buf, child->name.c_str() + 1, NULL, 0);
			}
			child = child->brother;
		}
		return 0;
	}
	return -ENOENT;
}

static int my_open(const char *path, struct fuse_file_info *flags) {
	cout<<" ---> \033[1;31m my_open \033[0m - \""<<path<<"\" \n";
	FileNode *node = files_tree->find(path);
	if( node == NULL || node->isDirectory() ){
		return -ENOENT;
	}
	//Verificar Permisos (por ahora asumo que todo es de escritura)
//	if ( (flags->flags & 3) != O_RDONLY ){
//		return -EACCES;
//	}
	//Preparar Recursos para la lectura o escritura
	if(files_tree->base_path == NULL || strlen(files_tree->base_path) == 0){
		cout<<" ---> \033[1;31m my_open \033[0m - Arbol con base_path null\n";
		return 0;
	}
	//Si hay descompresor, el deberia encargarse
	if( node->decoder != NULL ){
		cout<<" ---> \033[1;31m my_open \033[0m - Archivo comprimido, saliendo\n";
		return 0;
	}
	cout<<" ---> \033[1;31m my_open \033[0m - Abriendo archivo \""<<files_tree->base_path<<"\" + \""<<path<<"\"\n";
	//Path real (+2 por el potencial '/' y el 0 final)
	char real_path[ strlen(files_tree->base_path) + strlen(path) + 2 ];
	joint_path( files_tree->base_path, path, real_path );
	int fd = -1;
	fd = open(real_path, flags->flags);
	if (fd == -1){
		return -errno;
	}
	flags->fh = fd;
	cout<<" ---> \033[1;31m my_open \033[0m - Archivo abierto en fd: "<<flags->fh<<"\n";
	return 0;
	
}

static int my_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *flags) {
	cout<<"\033[1;31m";
	cout<<" ---> my_read - Inicio (\""<<path<<"\", "<<size<<" bytes desde "<<offset<<")\n";
	FileNode *node = files_tree->find(path);
	if( node == NULL || node->isDirectory() ){
		return -ENOENT;
	}
	if( node->decoder != NULL ){
		cout<<" ---> my_read - Archivo comprimido, usando decoder\n";
		//por ahora estoy usando un mutex GLOBAL
		//Esto deberia ser controlado por un mutex del decoder
		global_mutex.lock();
		unsigned int res = node->decoder->decodeText(offset, size, buf);
		global_mutex.unlock();
		//Verificacion para agregar el '\n' final
		//Solo si se escribio algo, y solo si se pidio MAS que el final del texto
		if( (res > 0) && (size > res) ) {
			buf[res++] = '\n';
		}
		cout<<" ---> my_read - Fin ("<<res<<" bytes leidos)\n";
		//Ojo con el cast, la interface espera un int
		cout<<"\033[0m";
		return (int)res;
	}
	cout<<" ---> my_read - Archivo normal, usando lector (fd "<<( (flags==NULL)?0:(flags->fh) )<<")\n";
	int fd = -1;
	bool close_fd = false;
	if(flags == NULL || flags->fh == 0){
		//abrir el archivo localmente
		char real_path[ strlen(files_tree->base_path) + strlen(path) + 2 ];
		joint_path( files_tree->base_path, path, real_path );
		fd = open(real_path, flags->flags);
		close_fd = true;
		if (fd == -1){
			cout<<"\033[0m";
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
	cout<<"\033[0m";
	return res;
	
}

//Crear el archivo (vacio) en path
//Creo que por ahora se puede omitir mode y dev
static int my_mknod(const char *path, mode_t mode, dev_t dev){
	cout<<"\033[1;32m";
	cout<<" ---> my_mknod - \""<<path<<"\" (mode: "<<mode<<")\n";
	//Path Real (+2 por el potencial '/' y el 0 final)
	char real_path[ strlen(files_tree->base_path) + strlen(path) + 2 ];
	joint_path(files_tree->base_path, path, real_path);
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
		cout<<"\033[0m";
		return -errno;
	}
	//Archivo real creado sin error, agregar a estructura (solo el path virtual)
	cout<<" ---> my_mknod - Agregando al arbol\n";
	files_tree->add(path);
	cout<<" ---> my_mknod - Actualizando Estado\n";
	FileNode *node = files_tree->find(path);
	node->updateNode(real_path);
	cout<<" ---> my_mknod - Fin \n";
	cout<<"\033[0m";
	return 0;
}

static int my_mkdir(const char *path, mode_t mode){
	cout<<" ---> \033[1;31m my_mkdir \033[0m - \""<<path<<"\" \n";
	//Crear directorio real
	char real_path[ strlen(files_tree->base_path) + strlen(path) + 2 ];
	joint_path(files_tree->base_path, path, real_path);
	int res = mkdir(path, mode);
	if (res == -1){
		return -errno;
	}
	//Crear directorio virtual (de momento estoy omitiendo mode)
	files_tree->find(path);
	return 0;
}

//Este metodo es equivalente a un mknod y un open
//Se supone que, si no esta implementado, se llama a ese par en su lugar
//Por este motivo o dejare SIN IMPLEMENTAR
//static int my_create(const char *path, mode_t mode, struct fuse_file_info *flags){
//	cout<<" ---> \033[1;31m my_create \033[0m - \""<<path<<"\" \n";
//	
//	return 0;
//}

//Eliminar el archivo de path
static int my_unlink(const char *path){
	cout<<" ---> \033[1;31m my_unlink \033[0m - \""<<path<<"\" \n";
	//Borrar archivo real
	char real_path[ strlen(files_tree->base_path) + strlen(path) + 2 ];
	joint_path( files_tree->base_path, path, real_path );
	int res = unlink(real_path);
	if (res == -1){
		return -errno;
	}
	//Borrar archivo virtual
	files_tree->remove(path);
	return 0;
}

//Eliminar el directorio de path
static int my_rmdir(const char *path){
	cout<<" ---> \033[1;31m my_rmdir \033[0m - \""<<path<<"\" \n";
	//Borrar directorio real
	char real_path[ strlen(files_tree->base_path) + strlen(path) + 2 ];
	joint_path( files_tree->base_path, path, real_path );
	int res = rmdir(path);
	if (res == -1){
		return -errno;
	}
	//Borrar directorio virtual
	files_tree->remove(path);
	return 0;
}

//Intenta escribir en el archivo de path.
//Agrega size bytes de buf, desde offset en el archivo objetivo.
//Debe retornar el numero real de bytes escritos.
//Retornar un numero != size puede ser considerado un error por el S.O.
static int my_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *flags){
	cout<<"\033[1;32m";
	cout<<" ---> my_write - \""<<path<<"\" ("<<size<<" bytes desde "<<offset<<")\n";
	//Si es un archivo comprimido hay que tratarlo de modo especial
	FileNode *node = files_tree->find(path);
	if( node == NULL || node->isDirectory() ){
		return -ENOENT;
	}
	if( node->decoder != NULL ){
		cout<<" ---> my_read - Archivo comprimido, escritura no implementada\n";
		
		cout<<"\033[0m";
		return 0;
	}
	cout<<" ---> my_write - Archivo normal, usando lector (fd "<<( (flags==NULL)?0:(flags->fh) )<<")\n";
	//Version C
	int fd = -1;
	bool close_fd = false;
	if(flags == NULL || flags->fh == 0){
		//abrir el archivo y guardar el fd, se cierra el en release
		char real_path[ strlen(files_tree->base_path) + strlen(path) + 2 ];
		joint_path( files_tree->base_path, path, real_path );
		fd = open(real_path, O_WRONLY);
		close_fd = true;
		if (fd == -1){
			cout<<"\033[0m";
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
	//Actualizar archivo virtual
	cout<<" ---> my_write - Actualizando nodo de archivo (offset: "<<sizeof(offset)<<" bytes)\n";
	//Version con lstat
	//node->updateNode(real_path);
	//Version con datos locales
	//Solo actualizo si la suma es MAYOR que el tamaño actual
	if( res > 0 && (unsigned long long)(offset + res) > node->getSize() ){
		node->setSize(offset + res);
	}
	cout<<"\033[0m";
	return res;
}

//Flush is called on each close() of a file descriptor.
//The flush() method may be called more than once for each open().
//Esto implica que el metodo se llama varias veces en un mismo proceso de escritura.
//Por ejemplo, se llama despues de crear un archivo (antes de write) y despues de write (antes de release)
static int my_flush(const char *path, struct fuse_file_info *flags){
	cout<<" ---> \033[1;31m my_flush \033[0m - \""<<path<<"\"\n";
	
	return 0;
}

//Release is called when there are no more references to an open file
//For every open() call there will be exactly one release()
//Solo el ultimo release implica que no hay mas lectura/escritura
static int my_release(const char *path, struct fuse_file_info *flags){
	cout<<" ---> \033[1;31m my_release \033[0m - \""<<path<<"\"\n";
	//Omito los 0, 1 y 2 (los estandar)
	if(flags != NULL && flags->fh > 2){
		cout<<" ---> \033[1;31m my_release \033[0m - Cerrando fd "<<flags->fh<<"\n";
		close(flags->fh);
		flags->fh = 0;
	}
	
	return 0;
}

//Veo llamadas a este metodo con un simple "> touch /tmp/cebib/test.txt"
static int my_releasedir(const char *path, struct fuse_file_info *flags){
	cout<<" ---> \033[1;31m my_releasedir \033[0m - \""<<path<<"\"\n";
	
	return 0;
}

static int my_utimens(const char *path, const struct timespec ts[2]){
	cout<<" ---> \033[1;31m my_utimens \033[0m - Inicio (\""<<path<<"\")\n";
	
//	int res;
//	
//	//Usar path real
//	
//	/* don't use utime/utimes since they follow symlinks */
//	res = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
//	if (res == -1)
//		return -errno;

	return 0;
}

static int my_truncate(const char *path, off_t size){
	cout<<" ---> \033[1;31m my_truncate \033[0m - Inicio (\""<<path<<"\", offset "<<size<<")\n";
	//Verificar si el archivo existe
	FileNode *node = files_tree->find(path);
	if( node == NULL || node->isDirectory() ){
		return -ENOENT;
	}
	//truncate del archivo real
	char real_path[ strlen(files_tree->base_path) + strlen(path) + 2 ];
	joint_path( files_tree->base_path, path, real_path );
	int res = truncate(real_path, size);
	if (res == -1){
		return -errno;
	}
	//actualizar archivo virtual (aqui el truncate ocurrio correctamente)
	node->setSize(size);
	return 0;
}

int main(int argc, char *argv[]) {

	cout<<"Inicio - Preparando variables estaticas\n";
	
	//Enlace de funciones (a la C++) en el struct fuse_operations definido al inicio
	my_oper.getattr = my_getattr;
	my_oper.readdir = my_readdir;
	my_oper.open = my_open;
	my_oper.read = my_read;
	//Funciones adicionales
	my_oper.mknod = my_mknod;
	my_oper.mkdir = my_mkdir;
//	my_oper.create = my_create;
	my_oper.unlink = my_unlink;
	my_oper.rmdir = my_rmdir;
	my_oper.write = my_write;
	my_oper.flush = my_flush;
	my_oper.release = my_release;
	my_oper.releasedir = my_releasedir;
	my_oper.utimens = my_utimens;
	my_oper.truncate = my_truncate;
	
	
	//Inicializar variables estaticas
	referencia = new ReferenceIndexBasic();
	referencia->load(archivo_referencia);
	
	//Defino el texto de FileNode (para que cada nodo cree su decoder)
	FileNode::reference_text = referencia->getText();
	
	
	files_tree = new FilesTree();
	files_tree->loadDirectory("test/");
	files_tree->print();
	
	cout<<"Inicio - Iniciando fuse_main\n";
	
	return fuse_main(argc, argv, &my_oper, NULL);
}









