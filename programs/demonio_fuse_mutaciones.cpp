
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
#include <map>

#include "NanoTimer.h"
#include "ConcurrentLogger.h"
#include "MutationsFile.h"

using namespace std;

// Iniciar: 
// > ./bin/demonio_fuse_mutaciones -d /mount_dir
// Terminar: 
// > fusermount -u /mount_dir

// Configuracion por defecto (sobreescrita con la lectura exitosa de un config)
// ruta del directorio real
static const char *base_path = "./";
// Tamaño preferido para operaciones IO (por ejemplo, write)
// Para usarlo apropiadamente, agrego "-o big_writes" al demonio
static unsigned int io_block_size = 131072;
static const char *base_text_file = "./texto_base.txt";
// Fin Configuracion

// Estructura para enlazar las funciones
static struct fuse_operations my_oper;

// Creo que en esta version basta con un map<path, MutationsFile> y un mutex para ese mapa

// Contenedor de informacion de un archivo
// Diseñada para ser asociada a un mapa de <path, FileData>
// status (podrian haber otros):
//   0 => normal (esta marca no necesita ser explicita)
//   1 => compresion agendada
//   2 => compresion terminada
// file_mutex y mut_file son NULL mientras no sean necesarios
class FileData{
private:
	unsigned char status;
	mutex *file_mutex;
	MutationsFile *mut_file;
public:
	FileData( unsigned char _status = 0, mutex *_file_mutex = NULL, MutationsFile *_mut_file = NULL ){
		status = _status;
		file_mutex = _file_mutex;
		mut_file = _mut_file;
		cout<<"FileData - Constructor (status: "<<(unsigned int)status<<", file_mutex?: "<<(file_mutex!=NULL)<<", mut_file?: "<<(mut_file!=NULL)<<")\n";
	}
	~FileData(){
		cout<<"FileData - Destructor (status: "<<(unsigned int)status<<", file_mutex?: "<<(file_mutex!=NULL)<<", mut_file?: "<<(mut_file!=NULL)<<")\n";
		if(file_mutex != NULL){
			delete file_mutex;
			file_mutex = NULL;
		}
		if(mut_file != NULL){
			delete mut_file;
			mut_file = NULL;
		}
	}
	unsigned char getStatus(){
		return status;
	}
	mutex *getMutex(){
		return file_mutex;
	}
	MutationsFile *getMutationsFile(){
		return mut_file;
	}
	void setStatus(unsigned char _status){
		status = _status;
	}
	void setMutex(mutex *_file_mutex){
		file_mutex = _file_mutex;
	}
	void setMutationsFile(MutationsFile *_mut_file){
		mut_file = _mut_file;
	}
};

mutex global_mutex;
static map<string, FileData> files_map;
static char *base_text = NULL;
static unsigned int base_text_size = 0;

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

bool is_mut(const char *path){
	if(path == NULL){
		return false;
	}
	unsigned int len = strlen(path);
	if( (len > 4) && (strcmp( path + len - 4, ".mut" ) == 0) ){
		return true;
	}
	return false;
}

/*
// Retorna el status asociado a path
static unsigned char get_status(const char *path){
	if(path == NULL){
		return 0;
	}
	lock_guard<mutex> lock(global_mutex);
	unsigned char status = files_map[path].getStatus();
	return status;
}

// Cambia el status asociado a path
static void set_status(const char *path, unsigned char status){
	if(path == NULL){
		return;
	}
	lock_guard<mutex> lock(global_mutex);
	files_map[path].setStatus(status);
}

// Retorna el mutex asociado a path
// Si no existe, lo crea, lo agrega al mapa y lo retorna
static mutex *get_mutex(const char *path){
	if(path == NULL){
		return NULL;
	}
	mutex *file_mutex = NULL;
	lock_guard<mutex> lock(global_mutex);
	file_mutex = files_map[path].getMutex();
	if( file_mutex == NULL ){
		file_mutex = new mutex();
		files_map[path].setMutex(file_mutex);
	}
	return file_mutex;
}
*/

// Retorna el MutationsFile asociado a path (Puede retornar NULL si hay algun problema)
// Si no existe, lo crea, lo agrega al mapa y lo retorna
// Para crear un nuevo MutationsFile, usa real_path o base_path (el primero != NULL)
static MutationsFile *get_mutations(const char *path, const char *real_path, const char *base_path = NULL){
	if(path == NULL){
		return NULL;
	}
	MutationsFile *mut_file = NULL;
	cout<<"get_mutations - Inicio\n";
	lock_guard<mutex> lock(global_mutex);
	mut_file = files_map[path].getMutationsFile();
	if( mut_file == NULL ){
		cout<<"get_mutations - MutationsFile NULL, creando\n";
		if( real_path != NULL ){
			cout<<"get_mutations - usando real_path\n";
			mut_file = new MutationsFile(real_path, base_text, base_text_size);
		}
		else if( base_path != NULL ){
			cout<<"get_mutations - usando base_path\n";
			char real_path[ strlen(base_path) + strlen(path) + 2 ];
			joint_path( base_path, path, real_path );
			mut_file = new MutationsFile(real_path, base_text, base_text_size);
		}
		else{
			cerr<<"get_mutations - Rutas insuficientes para crear MutationsFile\n";
			mut_file = NULL;
		}
		cout<<"get_mutations - Agregando MutationsFile\n";
		files_map[path].setMutationsFile(mut_file);
	}
	cout<<"get_mutations - Fin\n";
	return mut_file;
}

/*
// Borra el FileData asociado al path
static void delete_file_data(const char *path){
	if(path == NULL){
		return;
	}
	lock_guard<mutex> lock(global_mutex);
	files_map.erase(path);
}
*/

static int my_getattr(const char *path, struct stat *stbuf){
	cout<<" ---> my_getattr - \""<<path<<"\" \n";
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path( base_path, path, real_path );
	int res = lstat(real_path, stbuf);
	//Notar que reemplazo este valor con uno de la configuracion
	//Es el tamaño preferido para operaciones IO (por ejemplo, write)
	//Para usarlo apropiadamente, agrego "-o big_writes" al demonio
	stbuf->st_blksize = io_block_size;
	if (res == -1){
		return -errno;
	}
	if( is_mut(path) ){
		CoutColor color(color_green);
		cout<<" ---> my_getattr - Archivo con Mutaciones\n";
		//Revisar status
		MutationsFile *mut_file = get_mutations(path, real_path);
		if(mut_file != NULL){
			cout<<" ---> my_getattr - Usando MutationsFile ("<<mut_file->size()<<" chars)\n";
			stbuf->st_size = mut_file->size();
		}
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
	cout<<" ---> my_readdir - Revisando directorio (NULL? "<<(directory == NULL)<<")\n";
	if (directory == NULL){
		free(child_dir);
		return -ENOENT;
	}
	while(true){
		cout<<" ---> my_readdir - Tomando archivo\n";
		readdir_r( directory, child_dir, &test_dir );
		if(test_dir == NULL){
			break;
		}
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = child_dir->d_ino;
		st.st_mode = child_dir->d_type << 12;
		cout<<" ---> my_readdir - filler( \""<<child_dir->d_name<<"\" )\n";
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
	CoutColor color(color_yellow);
	cout<<" ---> my_mknod - \""<<path<<"\" (mode: "<<mode<<")\n";
	if( is_mut(path) ){
		cout<<" ---> my_mknod - Operacion no permitida en MutationFile\n";
		return -1;
	}
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path(base_path, path, real_path);
	//Creacion del archivo real
	//En teoria basta con un mknod, pero esto es mas portable y seguro
	int res = -1;
	if( S_ISREG(mode) ){
		cout<<" ---> my_mknod - S_ISREG \n";
		res = open(real_path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if(res >= 0){
			cout<<" ---> my_mknod - Open Ok\n";
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
		cout<<" ---> my_mknod - Error al crear archivo real\n";
		return -errno;
	}
	return 0;
}

static int my_mkdir(const char *path, mode_t mode){
	cout<<" ---> my_mkdir - \""<<path<<"\" \n";
	//Crear directorio real
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path( base_path, path, real_path );
	int res = mkdir(real_path, mode);
	if (res == -1){
		return -errno;
	}
	return 0;
}

//Eliminar el archivo de path
static int my_unlink(const char *path){
	cout<<" ---> my_unlink - \""<<path<<"\" \n";
	if( is_mut(path) ){
		cout<<" ---> my_unlink - Operacion no permitida en MutationFile\n";
		return -1;
	}
	
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

//Este metodo afecta las llaves de los mapas
//Ademas, en este momento is_relz depende de la ruta (por la extension)
//Por seguridad, no permitire esta operacion en relz de momento
static int my_rename(const char *from, const char *to){
	cout<<" ---> my_rename - \""<<from<<"\" -> \""<<to<<"\" \n";
	
	if( is_mut(from) || is_mut(to) ){
		cout<<" ---> my_rename - Operacion no permitida en MutationFile\n";
		return -1;
	}
	
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
	cout<<" ---> my_truncate - Inicio (\""<<path<<"\", offset "<<size<<")\n";
	
	if( is_mut(path) ){
		cout<<" ---> my_truncate - Operacion no permitida en MutationFile\n";
		return -1;
	}
	
	// truncate normal del archivo real
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
	cout<<" ---> my_open - \""<<path<<"\"\n";
	
	char real_path[ strlen(base_path) + strlen(path) + 2 ];
	joint_path( base_path, path, real_path );
	
	int fd = -1;
	fd = open(real_path, flags->flags);
	if (fd == -1){
		return -errno;
	}
	flags->fh = fd;
	
	return 0;
}

static int my_read_fd(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *flags){
	int fd = -1;
	bool close_fd = false;
	if(flags == NULL || flags->fh == 0){
		//abrir el archivo localmente
		char real_path[ strlen(base_path) + strlen(path) + 2 ];
		joint_path( base_path, path, real_path );
		fd = open( real_path, flags->flags );
		close_fd = true;
		if (fd == -1){
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
	return res;
}

static int my_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *flags) {
	cout<<" ---> my_read - Inicio (\""<<path<<"\", "<<size<<" bytes desde "<<offset<<")\n";
	
	int res = 0;
	if( is_mut(path) ){
		cout<<" ---> my_read - Usando MutationsFile\n";
		MutationsFile *mut_file = get_mutations(path, NULL, base_path);
		if(mut_file != NULL){
			res = (int)(mut_file->read(buf, size, offset));
		}
		else{
			cout<<" ---> my_read - Advertencia, MutationsFile NULL\n";
		}
	}//if... relz
	else{
		res = my_read_fd(path, buf, size, offset, flags);
	}//else... normal
	
	cout<<" ---> my_read - Fin (res: "<<res<<")\n";
	return res;
}

static int my_write_fd(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *flags){
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
	return res;
}

//Intenta escribir en el archivo de path.
//Agrega size bytes de buf, desde offset en el archivo objetivo.
//Debe retornar el numero real de bytes escritos.
//Retornar un numero != size puede ser considerado un error por el S.O.
static int my_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *flags){
	
	cout<<" ---> my_write - Inicio (\""<<path<<"\", "<<size<<" bytes desde "<<offset<<", fd: "<<( (flags==NULL)?0:(flags->fh) )<<")\n";
	
	int res = 0;
	if( is_mut(path) ){
		cout<<" ---> my_write - Operacion no permitida en MutationFile\n";
	}
	else{
		res = my_write_fd(path, buf, size, offset, flags);
	}
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
	
	return 0;
}

//Release is called when there are no more references to an open file
//For every open() call there will be exactly one release()
//Solo el ultimo release implica que no hay mas lectura/escritura
static int my_release(const char *path, struct fuse_file_info *flags){
	cout<<" ---> my_release - \""<<path<<"\"\n";
	//Omito los 0, 1 y 2 (los estandar)
	if(flags != NULL && flags->fh > 2){
		cout<<" ---> my_release - Cerrando fd "<<flags->fh<<"\n";
		close(flags->fh);
		flags->fh = 0;
	}
	return 0;
}

//Veo llamadas a este metodo con un simple "> touch /tmp/cebib/test.txt"
static int my_releasedir(const char *path, struct fuse_file_info *flags){
	cout<<" ---> my_releasedir - \""<<path<<"\"\n";
	
	return 0;
}

static int my_fallocate(const char *path, int mode, off_t offset, off_t length, struct fuse_file_info *flags) {
	(void) flags;
	cout<<" ---> my_fallocate - \""<<path<<"\"\n";
	
	if( is_mut(path) ){
		cout<<" ---> my_fallocate - Operacion no permitida en MutationFile\n";
		return -1;
	}
	
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

static bool load_config( const char *path ){
	
	ifstream lector(path, ifstream::in);
	if( ! lector.is_open() ){
		cout<<"load_config - Problemas al abrir archivo \""<<path<<"\"\n";
		return false;
	}
	
	unsigned int buff_size = 1024;
	char *buff = new char[buff_size];
	memset(buff, 0, buff_size);
	
	string line, name, mark, value;
	
	while( lector.good() ){
		lector.getline(buff, buff_size);
		unsigned int lectura = lector.gcount();
		unsigned int pos = 0;
		while( (pos < lectura) && buff[pos] == ' ' ){
			++pos;
		}
		if( (pos+1 >= lectura) || (buff[pos] == '#') ){
			continue;
		}
		//Linea valida de largo > 0
//		cout<<"Procesando \""<<buff<<"\" (pos: "<<pos<<" / "<<lectura<<")\n";
		string line(buff);
		stringstream toks(line);
		
		name = "";
		mark = "";
		value = "";
		
		toks>>name;
		toks>>mark;
		toks>>value;
		
		if( (mark.length() != 1) || (mark.compare("=") != 0) || (value.length() < 1) ){
			continue;
		}
		
		cout<<"Seteando \""<<name<<"\"\n";
		
		if( name.compare("base_path") == 0 ){
			toks>>value;
			cout<<"Valor \""<<value<<"\"\n";
			char *text = new char[ 1 + value.length() ];
			strcpy(text, value.c_str());
			base_path = text;
		}
		if( name.compare("base_text_file") == 0 ){
			toks>>value;
			cout<<"Valor \""<<value<<"\"\n";
			char *text = new char[ 1 + value.length() ];
			strcpy(text, value.c_str());
			base_text_file = text;
		}
		else if( name.compare("io_block_size") == 0 ){
			toks>>value;
			cout<<"Valor \""<<value<<"\"\n";
			io_block_size = atoi( value.c_str() );
		}
		else{
			cout<<"Omitiendo valor de \""<<name<<"\"\n";
		}
	
	}
	delete [] buff;
	
	return true;
}

int main(int argc, char *argv[]) {
	
	cout<<"Inicio - Preparando variables estaticas\n";
	
	if( load_config("demonio_mutaciones.config") 
		|| load_config("../demonio_mutaciones.config")
		){
		cout<<"Configuracion Cargada\n";
	}
	else{
		cout<<"Configuracion Inicial\n";
	}
	
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
	
	//Cargar Texto Base
	//static const char *base_text_file = "./texto_base.txt";
	//static char *base_text = NULL;
	//static unsigned int base_text_size = 0;
	
	fstream lector(base_text_file, fstream::in);
	if( ! lector.good() ){
		cerr<<"Error leyendo texto base de \""<<base_text_file<<"\"\n";
		return 0;
	}
	
	lector.seekg(0, lector.end);
	unsigned int expected_size = lector.tellg();
	lector.seekg(0, lector.beg);
	
	cout<<"Cargando "<<expected_size<<" chars\n";
	base_text = new char[expected_size + 1];
	unsigned int block_size = 1024*1024;
	unsigned int total_leido = 0;
	unsigned int lectura = 0;
	while( total_leido < expected_size ){
		lector.read( base_text + total_leido, block_size );
		lectura = lector.gcount();
		if( lectura == 0 ){
			break;
		}
		total_leido += lectura;
	}
	lector.close();
	base_text[total_leido] = 0;
	base_text_size = total_leido;
	cout<<"Texto base de "<<base_text_size<<" chars leido\n";
	
	cout<<"Inicio - Iniciando fuse_main\n";
	
	return fuse_main(argc, argv, &my_oper, NULL);
}









