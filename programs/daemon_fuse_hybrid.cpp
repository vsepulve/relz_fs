
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

#include "ReferenceIndex.h"
#include "ReferenceIndexBasic.h"

#include "Compressor.h"
#include "CompressorSingleBuffer.h"
#include "TextFilter.h"
#include "TextFilterFull.h"

#include "DecoderBlocks.h"
#include "DecoderBlocksRelz.h"

#include "CoderBlocks.h"
#include "CoderBlocksRelz.h"

#include "Configuration.h"

using namespace std;

// Prueba de FUSE

// Se usa como (el directorio de montaje debe estar creado, el -d es para debug):
// > ./bin/demonio_fuse_direct /cebib -d
// Para permitir acceso a root
// > ./bin/demonio_fuse_hibrido /cebib/ -d -o allow_root
// Para permitir acceso a todos
// > ./bin/demonio_fuse_hibrido /cebib/ -d -o allow_other

// Recordar cerrarlo con (o usarlo si el programa se cae para desmontar el directorio):
// > fusermount -u /cebib

static Configuration config;

// Estructura para enlazar las funciones
static struct fuse_operations my_oper;
// Referencia explicita
//static ReferenceIndexBasic *referencia = NULL;

//Esta es una tabla que convierte cualquier byte en un char valido
//La uso en la generacion de nombres temporales aleatorios
static char chars_table[256];

// Contenedor de informacion de un archivo
// Diseñada para ser asociada a un mapa de <path, FileData>
// status (podrian haber otros):
//   0 => normal (esta marca no necesita ser explicita)
//   1 => compresion agendada
//   2 => compresion terminada
// file_mutex y compressor son NULL mientras no sean necesarios
class FileData{
private:
	unsigned char status;
	mutex *file_mutex;
	Compressor *compressor;
public:
	FileData( unsigned char _status = 0, mutex *_file_mutex = NULL, Compressor *_compressor = NULL ){
		status = _status;
		file_mutex = _file_mutex;
		compressor = _compressor;
//		cout<<"FileData - Constructor (status: "<<(unsigned int)status<<", file_mutex?: "<<(file_mutex!=NULL)<<", compressor?: "<<(compressor!=NULL)<<")\n";
	}
	~FileData(){
//		cout<<"FileData - Destructor (status: "<<(unsigned int)status<<", file_mutex?: "<<(file_mutex!=NULL)<<", compressor?: "<<(compressor!=NULL)<<")\n";
		if(file_mutex != NULL){
			delete file_mutex;
			file_mutex = NULL;
		}
		if(compressor != NULL){
			delete compressor;
			compressor = NULL;
		}
	}
	unsigned char getStatus(){
		return status;
	}
	mutex *getMutex(){
		return file_mutex;
	}
	Compressor *getCompressor(){
		return compressor;
	}
	void setStatus(unsigned char _status){
		status = _status;
	}
	void setMutex(mutex *_file_mutex){
		file_mutex = _file_mutex;
	}
	void setCompressor(Compressor *_compressor){
		compressor = _compressor;
	}
};

mutex global_mutex;
static map<string, FileData> files_map;

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

//Llenado de tabla de chars validos para nombres temporales
//Asume que chars_table tiene 256 bytes
static void prepare_tmp_table(char *chars_table){
	char chars_validos[256];
	unsigned int n_chars = 0;
	for(unsigned int i = (unsigned int)'a'; i <= (unsigned int)'z'; ++i){
		chars_validos[n_chars++] = (char)i;
	}
	for(unsigned int i = (unsigned int)'A'; i <= (unsigned int)'Z'; ++i){
		chars_validos[n_chars++] = (char)i;
	}
	for(unsigned int i = (unsigned int)'0'; i <= (unsigned int)'9'; ++i){
		chars_validos[n_chars++] = (char)i;
	}
	//llenado de chars_table
	unsigned int cur_char = 0;
	for(unsigned int i = 0; i < 256; ++i){
		chars_table[i] = chars_validos[cur_char++];
//		cout<<"prepare_tmp_table - tabla["<<i<<"]: "<<chars_table[i]<<"\n";
		if(cur_char == n_chars){
			cur_char = 0;
		}
	}
}

//Retorna el numero de bytes para almacenar un tmp_path basado en real_path
//La forma actual es ".XXXXXXXXreal_path" donde X es un char seudo aleatorio (8 de ellos)
//Ntar que para generar X uso una tabla que convierte cualquier byte en un char valido
static unsigned int tmp_path_bytes(const char *real_path){
	if(real_path == NULL){
		return 11;
	}
//	return strlen(real_path) + 6;
//	return strlen(real_path) + 10;
	return strlen(real_path) + 11;
}

//Asume que to_file tiene al menos strlen(real_path) + 6 chars (".", ".tmp" y "\0")
static void tmp_path(const char *real_path, char *to_file){
	unsigned int r1 = rand();
	unsigned int r2 = rand();
	unsigned char *cr1 = (unsigned char*)&r1;
	unsigned char *cr2 = (unsigned char*)&r2;
	if(real_path == NULL || strlen(real_path) < 1){
	
//		sprintf(to_file, ".tmp");

		to_file[0] = '.';
		to_file[1] = chars_table[ cr1[0] ];
		to_file[2] = chars_table[ cr1[1] ];
		to_file[3] = chars_table[ cr1[2] ];
		to_file[4] = chars_table[ cr1[3] ];
		to_file[5] = chars_table[ cr2[0] ];
		to_file[6] = chars_table[ cr2[1] ];
		to_file[7] = chars_table[ cr2[2] ];
		to_file[8] = chars_table[ cr2[3] ];
		to_file[9] = 0;
	
		return;
	}
	unsigned int pos = 0;
	unsigned int len = strlen(real_path);
	for(pos = len - 1; pos > 0; --pos){
		if(real_path[pos] == '/'){
			++pos;
			break;
		}
	}
	
	memcpy(to_file, real_path, pos);
	to_file[pos] = '.';
	to_file[pos + 1] = chars_table[ cr1[0] ];
	to_file[pos + 2] = chars_table[ cr1[1] ];
	to_file[pos + 3] = chars_table[ cr1[2] ];
	to_file[pos + 4] = chars_table[ cr1[3] ];
	to_file[pos + 5] = chars_table[ cr2[0] ];
	to_file[pos + 6] = chars_table[ cr2[1] ];
	to_file[pos + 7] = chars_table[ cr2[2] ];
	to_file[pos + 8] = chars_table[ cr2[3] ];
	to_file[pos + 9] = '.';
	memcpy(to_file + pos + 10, real_path + pos, len - pos);
	to_file[len + 10] = 0;
	
	cout<<"tmp_path - \""<<to_file<<"\"\n";
}

bool is_relz(const char *path){
	if(path == NULL){
		return false;
	}
	unsigned int len = strlen(path);
	if(len > 5 && strcmp( path + len - 5, ".relz" ) == 0){
		return true;
	}
	return false;
}

// (! is_write) => is_read
bool is_write(struct fuse_file_info *flags){
	if( flags->flags & O_WRONLY || flags->flags & O_RDWR){
		return true;
	}
	else{
		return false;
	}
}

// (! is_append) => is_trunc
bool is_append(struct fuse_file_info *flags){
	if( flags->flags & O_APPEND || flags->flags & O_RDWR){
		return true;
	}
	else{
		return false;
	}
}

// Retorna el status asociado a path
static unsigned char get_status(const char *path){
	if(path == NULL){
		return 0;
	}
	lock_guard<mutex> lock(global_mutex);
	cout<<" ---> get_status - \""<<path<<"\", iniciando emplace\n";
	files_map.emplace( pair<string, FileData>(path, {}) );
	unsigned char status = files_map[path].getStatus();
	return status;
}

// Cambia el status asociado a path
static void set_status(const char *path, unsigned char status){
	if(path == NULL){
		return;
	}
	lock_guard<mutex> lock(global_mutex);
	cout<<" ---> set_status - \""<<path<<"\", "<<(unsigned int)status<<", iniciando emplace\n";
	files_map.emplace( pair<string, FileData>(path, {}) );
	cout<<" ---> set_status - setStatus\n";
	files_map[path].setStatus(status);
	cout<<" ---> set_status - Fin\n";
}

// Retorna el mutex asociado a path
// Si no existe, lo crea, lo agrega al mapa y lo retorna
static mutex *get_mutex(const char *path){
	if(path == NULL){
		return NULL;
	}
	mutex *file_mutex = NULL;
	lock_guard<mutex> lock(global_mutex);
	cout<<" ---> set_status - \""<<path<<"\", iniciando emplace\n";
	files_map.emplace( pair<string, FileData>(path, {}) );
	file_mutex = files_map[path].getMutex();
	if( file_mutex == NULL ){
		cout<<" ---> set_status - Creando mutex\n";
		file_mutex = new mutex();
		files_map[path].setMutex(file_mutex);
	}
	cout<<" ---> set_status - Fin\n";
	return file_mutex;
}

// Retorna el compressor asociado a path (Puede retornar NULL si hay algun problema)
// Si no existe, lo crea, lo agrega al mapa y lo retorna
// Para crear un nuevo compressor, usa real_path o base_path (el primero != NULL)
static Compressor *get_compressor(const char *path, const char *real_path, const char *base_path = NULL){
	if(path == NULL){
		return NULL;
	}
	Compressor *compressor = NULL;
//	cout<<"get_compressor - Inicio\n";
	lock_guard<mutex> lock(global_mutex);
	files_map.emplace( pair<string, FileData>(path, {}) );
	compressor = files_map[path].getCompressor();
	if( compressor == NULL ){
//		cout<<"get_compressor - Compressor NULL, creando\n";
		ReferenceIndexBasic *referencia = config.getReference(path);
		if( real_path != NULL ){
//			cout<<"get_compressor - usando real_path\n";
			compressor = new CompressorSingleBuffer(
				real_path, 
				new CoderBlocksRelz(referencia),
				new DecoderBlocksRelz(referencia->getText()),
				new TextFilterFull()
				);
		}
		else if( base_path != NULL ){
//			cout<<"get_compressor - usando base_path\n";
			char real_path[ strlen(base_path) + strlen(path) + 2 ];
			joint_path( base_path, path, real_path );
			compressor = new CompressorSingleBuffer(
				real_path,
				new CoderBlocksRelz(referencia),
				new DecoderBlocksRelz(referencia->getText()),
				new TextFilterFull()
				);
		}
		else{
			cerr<<"get_compressor - Rutas insuficientes para crear compressor\n";
			compressor = NULL;
		}
//		cout<<"get_compressor - Agregando compressor\n";
		files_map[path].setCompressor(compressor);
	}
//	cout<<"get_compressor - Fin\n";
	return compressor;
}

// Borra el FileData asociado al path
static void delete_file_data(const char *path){
	cout<<" --> delete_file_data - \""<<path<<"\"\n";
	if(path == NULL){
		return;
	}
	lock_guard<mutex> lock(global_mutex);
	files_map.erase(path);
}

static unsigned int compression_threads(const char *path){
	if( path == NULL || strlen(path) < 1 ){
		return 1;
	}
	fstream lector(path, fstream::in);
	if( (! lector.is_open()) || (! lector.good()) ){
		return 1;
	}
	lector.seekg(0, lector.end);
	unsigned int length = lector.tellg();
	lector.close();
	unsigned int n_threads = length / (config.compress_block_size << 4);
	if( n_threads < 1 ){
		return 1;
	}
	if( n_threads > config.compress_max_threads ){
		return config.compress_max_threads;
	}
	return n_threads;
}

static int my_getattr(const char *path, struct stat *stbuf){
	cout<<" ---> my_getattr - \""<<path<<"\" \n";
	char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
	joint_path( config.base_path, path, real_path );
	int res = lstat(real_path, stbuf);
	//Notar que reemplazo este valor con uno de la configuracion
	//Es el tamaño preferido para operaciones IO (por ejemplo, write)
	//Para usarlo apropiadamente, agrego "-o big_writes" al demonio
	stbuf->st_blksize = config.io_block_size;
	cout<<" ---> my_getattr - st_blksize: "<<stbuf->st_blksize<<" (st_blocks "<<stbuf->st_blocks<<")\n";
	if (res == -1){
		return -errno;
	}
	//Si es comprimido, ajustar atributos
	if( is_relz(path) ){
		CoutColor color(color_green);
		cout<<" ---> my_getattr - Archivo comprimido\n";
		//Revisar status
//		unsigned char status = status_map[path];
		unsigned char status = get_status(path);
		if( status == 1 ){
			//compresion agendada, dejar datos reales
			cout<<" ---> my_getattr - Status Agendado\n";
		}
		else{
			//Asumo que en cualquier otro caso, la compresion debe estar terminada (y lo refuerzo reseteando el status)
			//Si el status es realmente desconocido, podria realizarse una prueba del archivo para determinar
			set_status(path, 2);
			
			cout<<" ---> my_getattr - Status Comprimido\n";
			Compressor *compressor = get_compressor(path, real_path);
			if(compressor != NULL){
				cout<<" ---> my_getattr - Usando Compressor ("<<compressor->getTextSize()<<" chars)\n";
				stbuf->st_size = compressor->getTextSize();
			}
		}
	}
	return 0;
}

static int my_access(const char *path, int mask){
	cout<<" ---> my_access - \""<<path<<"\" \n";
	char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
	joint_path( config.base_path, path, real_path );
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
	char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
	joint_path( config.base_path, path, real_path );
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
		
		//filler CON stat (notar el mode)
		//Si el hijo es comprimido, puede ser necesario hacer ajustes
		//Una opcion es cortar el nombre aqui, pero en ese caso hay que marcarlo de algun modo como comprimido
		//Eso es porque el getattr usaria este nombre, no sabra que fue recortado
		//Otra opcion es no agregar extension, sino usar la original y reconocerla como comprimido
		//En este caso habria que agregar una verificacion de contenido
		
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
	char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
	joint_path(config.base_path, path, real_path);
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
	//Verificacion de tipo para setear status y compresor
	if( is_relz(path) ){
		cout<<" ---> my_mknod - Archivo comprimido, preparando status\n";
//		status_map[path] = 1;
		set_status(path, 1);
		//crear compresor? (por ahora se crea solo al final)
	}
	return 0;
}

static int my_mkdir(const char *path, mode_t mode){
	cout<<" ---> my_mkdir - \""<<path<<"\" \n";
	//Crear directorio real
	char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
	joint_path( config.base_path, path, real_path );
	int res = mkdir(real_path, mode);
	if (res == -1){
		return -errno;
	}
	return 0;
}

//Eliminar el archivo de path
static int my_unlink(const char *path){
	CoutColor color(color_red);
	cout<<" ---> my_unlink - \""<<path<<"\" \n";
	//Borrar archivo real
	char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
	joint_path( config.base_path, path, real_path );
	int res = unlink(real_path);
	
	cout<<" ---> my_unlink - Borrando archivo de Mapas\n";
	//Borrar datos en mapas
	delete_file_data(path);
	
	if (res == -1){
		return -errno;
	}
	return 0;
}

//Eliminar el directorio de path
static int my_rmdir(const char *path){
	cout<<" ---> my_rmdir - \""<<path<<"\" \n";
	//Borrar directorio real
	char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
	joint_path( config.base_path, path, real_path );
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
	
//	if( is_relz(from) != is_relz(to) ){
//		cout<<" ---> my_rename - Operacion no permitida para archivos relz\n";
//		return -errno;
//	}
	
	char real_from[ strlen(config.base_path) + strlen(from) + 2 ];
	joint_path( config.base_path, from, real_from );
	char real_to[ strlen(config.base_path) + strlen(to) + 2 ];
	joint_path( config.base_path, to, real_to );
	
	//Caso especial 1: norm -> relz
	//Se comprime el norm en el relz
	//Se borra el norm
	if( ! is_relz(from) && is_relz(to) ){
		Compressor *compressor = get_compressor(to, real_to);
		if( compressor == NULL ){
			return -errno;
		}
		cout<<" ---> my_rename - Comprimiendo en \""<<real_to<<"\"\n";
		if( ! compressor->compress(real_from, compression_threads(real_from), config.compress_block_size) ){
			return -errno;
		}
		cout<<" ---> my_rename - Borrando \""<<real_from<<"\"\n";
		int res = unlink(real_from);
		if (res == -1){
			return -errno;
		}
		//Borrar datos en mapas (de todos los mapas, solo por seguridad)
//		status_map.erase(from);
//		delete_compressor(from);
		delete_file_data(from);
	}
	//Caso especial 2: relz -> norm
	//Se descomprime relz en norm
	//Se borra relz
	else if( is_relz(from) && ! is_relz(to) ){
		Compressor *compressor = get_compressor(from, real_from);
		if( compressor == NULL ){
			return -errno;
		}
		cout<<" ---> my_rename - Descomprimiendo en \""<<real_to<<"\"\n";
		if( ! compressor->decompress(real_to, config.decompress_line_size) ){
			return -errno;
		}
		cout<<" ---> my_rename - Borrando \""<<real_from<<"\"\n";
		int res = unlink(real_from);
		if (res == -1){
			return -errno;
		}
		//Borrar datos en mapas (de todos los mapas, solo por seguridad)
//		status_map.erase(from);
//		delete_compressor(from);
		delete_file_data(from);
	}
	//Caso normal (from y to son del mismo tipo)
	//Rename normal
	else{
		int res = rename(real_from, real_to);
		if (res == -1){
			return -errno;
		}
	}
	return 0;
}

static int my_chmod(const char *path, mode_t mode){
	cout<<" ---> my_chmod - \""<<path<<"\" \n";
	char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
	joint_path( config.base_path, path, real_path );
	int res = chmod(real_path, mode);
	if (res == -1){
		return -errno;
	}
	return 0;
}

static int my_chown(const char *path, uid_t uid, gid_t gid){
	cout<<" ---> my_chown - \""<<path<<"\" \n";
	char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
	joint_path( config.base_path, path, real_path );
	int res = lchown(real_path, uid, gid);
	if (res == -1){
		return -errno;
	}
	return 0;
}

static int my_truncate(const char *path, off_t size){
	CoutColor color(color_yellow);
	cout<<" ---> my_truncate - Inicio (\""<<path<<"\", offset "<<size<<")\n";

	mutex *file_mutex = get_mutex(path);
	lock_guard<mutex> lock(*file_mutex);
	
	int res = 0;
	if( get_status(path) == 2 ){
		// archivo comprimido, descomprimo size y recomprimo
		
		// Descomprimo el original en partes hacia tmp (size chars)
		// Luego borro el compresor
		// Luego tomo un nuevo compresor para real_path (vacio en ese momento)
		// Luego comprimo tmp en ese compresor y listo
		
		char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
		joint_path( config.base_path, path, real_path );
		char tmp_file[ tmp_path_bytes(real_path) ];
		tmp_path(real_path, tmp_file);
		
		//Muevo el texto a un tmp y lo comprimo en el definitivo
		cout<<" ---> my_truncate - Guardando "<<size<<" chars de \""<<real_path<<"\" en \""<<tmp_file<<"\"\n";
		Compressor *compressor = get_compressor(path, real_path);
		if(compressor == NULL){
			cout<<" ---> my_truncate - Error, No se pudo obtener compresor\n";
			res = -1;
		}
		else if(compressor->getTextSize() == size){
			cout<<" ---> my_truncate - Tamaño exacto, no es necesario hacer nada\n";
		}
		else{
			// Descomprimo (y escribo en tmp) hasta un total de size
			fstream escritor(tmp_file, fstream::trunc | fstream::out);
			if( size > 0 ){
				// Buffer y otros datos para descompresion
				unsigned int n_chars = config.decompress_line_size;
				char *buff = new char[n_chars + 1];
				long long total = 0;
				unsigned int read_size = 0;
				while( total < size ){
					if( total + n_chars > size ){
						n_chars = size - total;
					}
					read_size = compressor->read( (unsigned int)total, n_chars, buff );
					// Si no obtengo datos suficientes, DEBO setear en 0's el buffer
					if( read_size <  n_chars){
						memset(buff + read_size, 0, n_chars - read_size);
					}
					escritor.write(buff, n_chars);
					total += n_chars;
				}
				delete [] buff;
			}
			escritor.close();
			
			// Borrar compresor
			cout<<" ---> my_truncate - Escritura terminada, eliminado datos y borrando archivo\n";
			delete_file_data(path);
			remove(real_path);
			
			// Tomar compresor nuevamente (mismo path)
			cout<<" ---> my_truncate - Recreando compresor\n";
			compressor = get_compressor(path, real_path);
			
			// Comprimir tmp en ese compresor
			if(compressor != NULL){
				cout<<" ---> my_truncate - Comprimiendo\n";
				compressor->compress(tmp_file, compression_threads(tmp_file), config.compress_block_size);
			}
			
			// Terminar seteando status y borrando temp
			cout<<" ---> my_truncate - set_status...\n";
			set_status(path, 2);
			cout<<" ---> my_truncate - remove...\n";
			remove(tmp_file);
			cout<<" ---> my_truncate - fin\n";
		}
		
	}
	else{
		// truncate normal del archivo real
		char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
		joint_path( config.base_path, path, real_path );
		res = truncate(real_path, size);
	}
	if (res == -1){
		return -errno;
	}
	return 0;
}

static int my_utimens(const char *path, const struct timespec ts[2]){
	cout<<" ---> my_utimens - Inicio (\""<<path<<"\")\n";
//	char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
//	joint_path( config.base_path, path, real_path );
//	int res = utimensat(0, real_path, ts, AT_SYMLINK_NOFOLLOW);
//	cout<<" ---> my_utimens - res: "<<res<<" (de \""<<real_path<<"\")\n";
//	if (res == -1){
//		return -errno;
//	}
	return 0;
}

static int my_open(const char *path, struct fuse_file_info *flags) {
//	CoutColor color(color_red);
	cout<<" ---> my_open - \""<<path<<"\" ("<<( is_write(flags)?"-- W --":"-- R --" )<<")\n";

	mutex *file_mutex = get_mutex(path);
	lock_guard<mutex> lock(*file_mutex);
	
	char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
	joint_path( config.base_path, path, real_path );
	if( is_relz(path) ){
		cout<<" ---> my_open - Archivo Comprimido\n";
		
		//si la lectura es de escritura hay que hacer algo especial aqui
		//Una opcion es retornarlo al estado agendado (=> descomprimirlo y preparalo para escrituras)
		//Si el archivo esta agendado, no hay problema
		//Notar que esto solo importa si el open es de escritura
		
//		unsigned char status = status_map[path];
		unsigned char status = get_status(path);
		if( status == 1 ){
			cout<<" ---> my_open - Compresion Agendada, usando fd\n";
			int fd = -1;
			fd = open(real_path, flags->flags);
			if (fd == -1){
				return -errno;
			}
			flags->fh = fd;
		}//if... status agendado
		else if( status == 2){
			cout<<" ---> my_open - Compresion Terminada\n";
			
		}//else if... status comprimido
		else{
		
			cout<<" ---> my_open - status invalido ("<<(unsigned int)status<<"), dejando datos reales\n";
			int fd = -1;
			fd = open(real_path, flags->flags);
			if (fd == -1){
				return -errno;
			}
			flags->fh = fd;
		}//else... status desconocido
		return 0;
		
	}//if... is_relz
	else{
		int fd = -1;
		fd = open(real_path, flags->flags);
		if (fd == -1){
			return -errno;
		}
		flags->fh = fd;
	}
	
//	cout<<" ---> my_open - Archivo abierto en fd: "<<flags->fh<<"\n";
	return 0;
}

static int my_read_fd(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *flags){
	int fd = -1;
	bool close_fd = false;
	if(flags == NULL || flags->fh == 0){
		//abrir el archivo localmente
		char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
		joint_path( config.base_path, path, real_path );
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
	
	mutex *file_mutex = get_mutex(path);
	lock_guard<mutex> lock(*file_mutex);

	cout<<" ---> my_read - lock tomado\n";
	
//	CoutColor color(color_green);
	int res = 0;
	if( is_relz(path) ){
		cout<<" ---> my_read - Archivo Comprimido\n";
//		unsigned char status = status_map[path];
		unsigned char status = get_status(path);
		if( status == 1 ){
			cout<<" ---> my_read - Compresion Agendada, usar texto\n";
			res = my_read_fd(path, buf, size, offset, flags);
		}//status agendado
		else if( status == 2){
			cout<<" ---> my_read - Compresion Terminada, usando compressor\n";
			Compressor *compressor = get_compressor(path, NULL, config.base_path);
			if(compressor != NULL){
				cout<<" ---> my_read - compressor->read...\n";
				res = (int)(compressor->read(offset, size, buf));
			}
			else{
				cout<<" ---> my_read - Advertencia, compressor NULL\n";
			}
		}//status terminado
		else{
			cout<<" ---> my_read - status invalido ("<<(unsigned int)status<<"), dejando datos reales\n";
			res = my_read_fd(path, buf, size, offset, flags);
		}//status desconocido
	}//if... relz
	else{
		res = my_read_fd(path, buf, size, offset, flags);
	}//else... normal
//	cout<<" ---> my_read - Fin (res: "<<res<<")\n";
	return res;
}

static int my_write_fd(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *flags){
	//Version C
	int fd = -1;
	bool close_fd = false;
	if(flags == NULL || flags->fh == 0){
		//abrir el archivo localmente
		char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
		joint_path( config.base_path, path, real_path );
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
	
	mutex *file_mutex = get_mutex(path);
	lock_guard<mutex> lock(*file_mutex);

	cout<<" ---> my_write - lock tomado\n";

//	CoutColor color(color_yellow);
	int res = 0;
	if( is_relz(path) ){
//		cout<<" ---> my_write - Relz\n";
//		unsigned char status = status_map[path];
		unsigned char status = get_status(path);
		if(status == 1){
//			cout<<" ---> my_write - Compresion agendada, usando fd\n";
			res = my_write_fd(path, buf, size, offset, flags);
		}
		else if(status == 2){
			cout<<" ---> my_write - Compresion Terminada, llamando write\n";
			// Esto si el open no lo descomprime, claro
			Compressor *compressor = get_compressor(path, NULL, config.base_path);
			if(compressor != NULL){
				char tmp_file[ tmp_path_bytes(compressor->getMasterFile()) ];
				tmp_path(compressor->getMasterFile(), tmp_file);
				res = (int)(compressor->write(buf, size, offset, tmp_file));
			}
			else{
				cout<<" ---> my_write - Advertencia, compressor NULL\n";
			}
		}
		else{
//			cout<<" ---> my_write - status invalido ("<<(unsigned int)status<<"), dejando datos reales\n";
			res = my_write_fd(path, buf, size, offset, flags);
		}
	}
	else{
//		cout<<" ---> my_write - fd\n";
		res = my_write_fd(path, buf, size, offset, flags);
	}
	cout<<" ---> my_write - Fin (res: "<<res<<")\n";
	return res;
}

static int my_statfs(const char *path, struct statvfs *stbuf){
	cout<<" ---> my_statfs - \""<<path<<"\"\n";
	char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
	joint_path( config.base_path, path, real_path );
	int res = statvfs(real_path, stbuf);
	cout<<" ---> my_statfs - f_bsize: "<<stbuf->f_bsize<<", f_frsize: "<<stbuf->f_frsize<<"\n";
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
	CoutColor color(color_red);
	cout<<" ---> my_release - \""<<path<<"\"\n";
	//Omito los 0, 1 y 2 (los estandar)
	if(flags != NULL && flags->fh > 2){
		cout<<" ---> my_release - Cerrando fd "<<flags->fh<<" ("<<( is_write(flags)?"-- W --":"-- R --" )<<")\n";
		close(flags->fh);
		flags->fh = 0;
	}
	
	mutex *file_mutex = get_mutex(path);
	lock_guard<mutex> lock(*file_mutex);
	
	//tratamiento final de archivo comprimido
	//Solo lo considero en caso de escritura (en lectura no hago nada adicional)
	if( is_write(flags) && is_relz(path) ){
		cout<<" ---> my_release - Terminando escritura de archivo comprimido\n";
//		unsigned char status = status_map[path];
		unsigned char status = get_status(path);
		if(status == 1){
			cout<<" ---> my_release - Fin de compresion agendada, comprimiendo...\n";
			//comprimir
			char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
			joint_path( config.base_path, path, real_path );
			char tmp_file[ tmp_path_bytes(real_path) ];
			tmp_path(real_path, tmp_file);
			
			//Muevo el texto a un tmp y lo comprimo en el definitivo
			cout<<" ---> my_release - Renombrando texto (\""<<real_path<<"\" -> \""<<tmp_file<<"\")\n";
			rename(real_path, tmp_file);
			Compressor *compressor = get_compressor(path, real_path);
			if(compressor != NULL){
				compressor->compress(tmp_file, compression_threads(tmp_file), config.compress_block_size);
			}
			remove(tmp_file);
			
			//actualizar status
			set_status(path, 2);
			
		}
		else if(status == 2){
			cout<<" ---> my_release - Compresion terminada\n";
		}
		else{
			cout<<" ---> my_release - status invalido ("<<(unsigned int)status<<")\n";
		}
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
	char real_path[ strlen(config.base_path) + strlen(path) + 2 ];
	joint_path( config.base_path, path, real_path );
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
	
	if( argc < 2 ){
		cout << "Usage: daemon_fuse_hybrid mount_directory [fuse_args] config_file\n";
		cout << "Example: ./bin/daemon_fuse_hybrid ./test -d -o big_writes daemon_fuse.json\n";
		return 0;
	}
	
	string config_file = argv[argc - 1];
	config.loadConfiguration(config_file);
	
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
	
	//Inicializar variables estaticas
//	referencia = new ReferenceIndexBasic();
//	referencia->load( config.getReference(NULL) );
	
	prepare_tmp_table(chars_table);
	
	cout << "Starting fuse_main...\n";
	
	return fuse_main(argc - 1, argv, &my_oper, NULL);
}









