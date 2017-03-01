#include "RemoteFunctions.h"

RemoteFunctions::RemoteFunctions(){
	host = NULL;
	port = 0;
	user_id = 0;
}

RemoteFunctions::RemoteFunctions(const char *_host, unsigned int _port, unsigned int _user_id){
	setParameters(host, port, user_id);
}

RemoteFunctions::~RemoteFunctions(){
	host = NULL;
	port = 0;
	user_id = 0;
}

void RemoteFunctions::setParameters(const char *_host, unsigned int _port, unsigned int _user_id){
	host = _host;
	port = _port;
	user_id = _user_id;
}

// En este metodo lo crucial es stbuf y res
// Creo que eso se puede serializar directamente en un char* (4 bytes para res y sizeof(struct stat) )
// Despues habria que hacer un memcpy de la serializacion directamente a *stbuf (siempre que sea != NULL)
// Siendo struct, CREO que deberia ser seguro
// LLamada remota con la interface de lstat, usada por getattr
int RemoteFunctions::stat(const char *path, struct stat *stbuf){
	// Objeto que internamente guarda un socket y lo cierra en delete
	// Antes de cerrar la conexion (en su destructor) envia una señal de cerrado al server
	// Incluye un metodo "good()" que indica conexion exitosa
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::lstat - Error de conexion.\n";
		return -1;
	}
	if( ! con.sendRequest( REMOTE_STAT ) ){
		cout<<" ---> RemoteFunctions::lstat - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::lstat - Error al enviar path.\n";
		return -1;
	}
	int res = 0;
	if( ! con.readInt(res) ){
		cout<<" ---> RemoteFunctions::lstat - Error al generar stat.\n";
		return -1;
	}
	cout<<" ---> RemoteFunctions::lstat - res: "<<res<<", leyendo stbuf\n";
	// Notar que si stbuf es NULL, omito la lectura del stat y continuo
	if( (stbuf != NULL) && (! con.readData((char*)stbuf, sizeof(struct stat))) ){
		cout<<" ---> RemoteFunctions::lstat - Error al recibir stat.\n";
		return -1;
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		cout<<" ---> RemoteFunctions::lstat - new_errno: "<<new_errno<<"\n";
		errno = new_errno;
	}
	cout<<" ---> RemoteFunctions::lstat - errno: "<<errno<<"\n";
	return res;
}

// En esta llamada lo imortante es res
// Se puede preguntar (con path y mask) y recibir el entero
int RemoteFunctions::access(const char *path, int mask){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::lstat - Error de conexion.\n";
		return -1;
	}
	if( ! con.sendRequest( REMOTE_ACCESS ) ){
		cout<<" ---> RemoteFunctions::lstat - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::lstat - Error al enviar path.\n";
		return -1;
	}
	if( ! con.writeInt(mask) ){
		cout<<" ---> RemoteFunctions::lstat - Error al enviar mask.\n";
		return -1;
	}
	int res = 0;
	if( ! con.readInt(res) ){
		cout<<" ---> RemoteFunctions::lstat - Error al generar stat.\n";
		return -1;
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		errno = new_errno;
	}
	return res;
}

char *RemoteFunctions::readdir(const char *path){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::readdir - Error de conexion.\n";
		return NULL;
	}
	if( ! con.sendRequest( REMOTE_READDIR ) ){
		cout<<" ---> RemoteFunctions::readdir - Error al enviar request.\n";
		return NULL;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::readdir - Error al enviar path.\n";
		return NULL;
	}
	unsigned int n_bytes = 0;
	if( (! con.readUInt(n_bytes)) ){
		cout<<" ---> RemoteFunctions::readdir - Error al recibir n_bytes.\n";
		return NULL;
	}
	cout<<" ---> RemoteFunctions::readdir - n_bytes: "<<n_bytes<<"\n";
	char *res = new char[ n_bytes + 1 ];
	if( ! con.readData(res, n_bytes) ){
		cout<<" ---> RemoteFunctions::readdir - Error al recibir datos.\n";
		delete [] res;
		return NULL;
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		errno = new_errno;
	}
	//En caso de dir not found, solo escribo 4 bytes
	if( n_bytes < 2 * sizeof(int) ){
		cout<<" ---> RemoteFunctions::readdir - Codigo de dir no encontrado, retornando NULL\n";
		//Numero de error tentativo, hay que revisarlo para asegurar que funcione.
		delete [] res;
		return NULL;
	}
	cout<<" ---> RemoteFunctions::readdir - Retornando, todo ok\n";
	return res;
}

int RemoteFunctions::mknod(const char *path, mode_t mode, dev_t dev){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::mknod - Error de conexion.\n";
		return -1;
	}
	if( ! con.sendRequest( REMOTE_MKNOD ) ){
		cout<<" ---> RemoteFunctions::mknod - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::mknod - Error al enviar path.\n";
		return -1;
	}
	// n_bytes es el numero de bytes de mode + dev
	// Si este numero es diferente a lo esperado por el server, este fallara intencionalmente la llamada
	unsigned char n_bytes = sizeof(mode_t) + sizeof(dev_t);
	if( ! con.writeByte(n_bytes) ){
		cout<<" ---> RemoteFunctions::mknod - Error al enviar n_bytes.\n";
		return -1;
	}
	if( ! con.writeData( (char*)&mode, sizeof(mode_t) ) ){
		cout<<" ---> RemoteFunctions::mknod - Error al enviar mode.\n";
		return -1;
	}
	if( ! con.writeData( (char*)&dev, sizeof(dev_t) ) ){
		cout<<" ---> RemoteFunctions::mknod - Error al enviar dev.\n";
		return -1;
	}
	int res = 0;
	if( ! con.readInt(res) ){
		cout<<" ---> RemoteFunctions::mknod - Error al realizar mknod.\n";
		return -1;
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		errno = new_errno;
	}
	return res;
}

int RemoteFunctions::mkdir(const char *path, mode_t mode){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::mkdir - Error de conexion.\n";
		return -1;
	}
	if( ! con.sendRequest( REMOTE_MKDIR ) ){
		cout<<" ---> RemoteFunctions::mkdir - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::mkdir - Error al enviar path.\n";
		return -1;
	}
	unsigned char n_bytes = sizeof(mode_t);
	if( ! con.writeByte(n_bytes) ){
		cout<<" ---> RemoteFunctions::mkdir - Error al enviar n_bytes.\n";
		return -1;
	}
	if( ! con.writeData( (char*)&mode, sizeof(mode_t) ) ){
		cout<<" ---> RemoteFunctions::mkdir - Error al enviar mode.\n";
		return -1;
	}
	int res = 0;
	if( ! con.readInt(res) ){
		cout<<" ---> RemoteFunctions::mkdir - Error al realizar mkdir.\n";
		return -1;
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		errno = new_errno;
	}
	return res;
}

int RemoteFunctions::unlink(const char *path){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::unlink - Error de conexion.\n";
		return -1;
	}
	if( ! con.sendRequest( REMOTE_UNLINK ) ){
		cout<<" ---> RemoteFunctions::unlink - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::unlink - Error al enviar path.\n";
		return -1;
	}
	int res = 0;
	if( ! con.readInt(res) ){
		cout<<" ---> RemoteFunctions::unlink - Error al realizar unlink.\n";
		return -1;
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		errno = new_errno;
	}
	return res;
}

int RemoteFunctions::rmdir(const char *path){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::rmdir - Error de conexion.\n";
		return -1;
	}
	if( ! con.sendRequest( REMOTE_RMDIR ) ){
		cout<<" ---> RemoteFunctions::rmdir - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::rmdir - Error al enviar path.\n";
		return -1;
	}
	int res = 0;
	if( ! con.readInt(res) ){
		cout<<" ---> RemoteFunctions::rmdir - Error al realizar rmdir.\n";
		return -1;
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		errno = new_errno;
	}
	return res;
}

int RemoteFunctions::rename(const char *from, const char *to){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::rename - Error de conexion.\n";
		return -1;
	}
	if( ! con.sendRequest( REMOTE_RENAME ) ){
		cout<<" ---> RemoteFunctions::rename - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(from) ){
		cout<<" ---> RemoteFunctions::rename - Error al enviar string from.\n";
		return -1;
	}
	if( ! con.writeString(to) ){
		cout<<" ---> RemoteFunctions::rename - Error al enviar string to.\n";
		return -1;
	}
	int res = 0;
	if( ! con.readInt(res) ){
		cout<<" ---> RemoteFunctions::rename - Error al realizar rmdir.\n";
		return -1;
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		errno = new_errno;
	}
	return res;
}

int RemoteFunctions::chmod(const char *path, mode_t mode){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::chmod - Error de conexion.\n";
		return -1;
	}
	if( ! con.sendRequest( REMOTE_CHMOD ) ){
		cout<<" ---> RemoteFunctions::chmod - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::chmod - Error al enviar path.\n";
		return -1;
	}
	// n_bytes es el numero de bytes de mode
	// Si este numero es diferente a lo esperado por el server, este fallara intencionalmente la llamada
	unsigned char n_bytes = sizeof(mode_t);
	if( ! con.writeByte(n_bytes) ){
		cout<<" ---> RemoteFunctions::chmod - Error al enviar n_bytes.\n";
		return -1;
	}
	if( ! con.writeData( (char*)&mode, sizeof(mode_t) ) ){
		cout<<" ---> RemoteFunctions::chmod - Error al enviar mode.\n";
		return -1;
	}
	int res = 0;
	if( ! con.readInt(res) ){
		cout<<" ---> RemoteFunctions::chmod - Error al realizar chmod.\n";
		return -1;
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		errno = new_errno;
	}
	return res;
}

int RemoteFunctions::chown(const char *path, uid_t uid, gid_t gid){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::chown - Error de conexion.\n";
		return -1;
	}
	if( ! con.sendRequest( REMOTE_CHOWN ) ){
		cout<<" ---> RemoteFunctions::chown - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::chown - Error al enviar path.\n";
		return -1;
	}
	// n_bytes es el numero de bytes de mode + dev
	// Si este numero es diferente a lo esperado por el server, este fallara intencionalmente la llamada
	unsigned char n_bytes = sizeof(uid_t) + sizeof(gid_t);
	if( ! con.writeByte(n_bytes) ){
		cout<<" ---> RemoteFunctions::chown - Error al enviar n_bytes.\n";
		return -1;
	}
	if( ! con.writeData( (char*)&uid, sizeof(uid_t) ) ){
		cout<<" ---> RemoteFunctions::chown - Error al enviar uid.\n";
		return -1;
	}
	if( ! con.writeData( (char*)&gid, sizeof(gid_t) ) ){
		cout<<" ---> RemoteFunctions::chown - Error al enviar gid.\n";
		return -1;
	}
	int res = 0;
	if( ! con.readInt(res) ){
		cout<<" ---> RemoteFunctions::chown - Error al realizar chown.\n";
		return -1;
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		errno = new_errno;
	}
	return res;
}

int RemoteFunctions::truncate(const char *path, off_t size){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::truncate - Error de conexion.\n";
		return -1;
	}
	if( ! con.sendRequest( REMOTE_TRUNCATE ) ){
		cout<<" ---> RemoteFunctions::truncate - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::truncate - Error al enviar path.\n";
		return -1;
	}
	if( ! con.writeULong(size) ){
		cout<<" ---> RemoteFunctions::truncate - Error al enviar size.\n";
		return -1;
	}
	int res = 0;
	if( ! con.readInt(res) ){
		cout<<" ---> RemoteFunctions::truncate - Error al realizar truncate.\n";
		return -1;
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		errno = new_errno;
	}
	return res;
}

int RemoteFunctions::statfs(const char *path, struct statvfs *stbuf){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::statfs - Error de conexion.\n";
		return -1;
	}
	if( ! con.sendRequest( REMOTE_STATFS ) ){
		cout<<" ---> RemoteFunctions::statfs - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::statfs - Error al enviar path.\n";
		return -1;
	}
	// Solo envio stbuf si es valido
	unsigned char n_bytes = 0;
	if( stbuf == NULL ){
		if( ! con.writeByte(n_bytes) ){
			cout<<" ---> RemoteFunctions::statfs - Error al enviar n_bytes.\n";
			return -1;
		}
	}
	else{
		n_bytes = sizeof(struct statvfs);
		if( ! con.writeByte(n_bytes) ){
			cout<<" ---> RemoteFunctions::statfs - Error al enviar n_bytes.\n";
			return -1;
		}
		if( ! con.writeData( (char*)stbuf, n_bytes ) ){
			cout<<" ---> RemoteFunctions::statfs - Error al enviar stbuf.\n";
			return -1;
		}
	}
	int res = 0;
	if( ! con.readInt(res) ){
		cout<<" ---> RemoteFunctions::statfs - Error al realizar statfs.\n";
		return -1;
	}
	// Solo si envie stbuf espero recibirlo de vuelta (si envie 0, no espero recibir nada aqui)
	// Ademas si lo envian DEBE ser del tamaño correcto
	if( stbuf != NULL ){
		if( ! con.readData( (char*)stbuf, n_bytes ) ){
			cout<<" ---> RemoteFunctions::statfs - Error al recibir stbuf.\n";
			return -1;
		}
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		errno = new_errno;
	}
	return res;
}

int RemoteFunctions::fallocate(const char *path, int mode, off_t offset, off_t length, int flags){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::fallocate - Error de conexion.\n";
		return -1;
	}
	if( ! con.sendRequest( REMOTE_FALLOCATE ) ){
		cout<<" ---> RemoteFunctions::fallocate - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::fallocate - Error al enviar path.\n";
		return -1;
	}
	if( ! con.writeInt(mode) ){
		cout<<" ---> RemoteFunctions::fallocate - Error al enviar mode.\n";
		return -1;
	}
	if( ! con.writeULong(offset) ){
		cout<<" ---> RemoteFunctions::fallocate - Error al enviar offset.\n";
		return -1;
	}
	if( ! con.writeULong(length) ){
		cout<<" ---> RemoteFunctions::fallocate - Error al enviar length.\n";
		return -1;
	}
	if( ! con.writeInt(flags) ){
		cout<<" ---> RemoteFunctions::fallocate - Error al enviar flags.\n";
		return -1;
	}
	int res = 0;
	if( ! con.readInt(res) ){
		cout<<" ---> RemoteFunctions::fallocate - Error al realizar fallocate.\n";
		return -1;
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		errno = new_errno;
	}
	return res;
}

int RemoteFunctions::open(const char *path, int flags){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::open - Error de conexion.\n";
		return -1;
	}
	if( ! con.sendRequest( REMOTE_OPEN ) ){
		cout<<" ---> RemoteFunctions::open - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::open - Error al enviar path.\n";
		return -1;
	}
	if( ! con.writeInt(flags) ){
		cout<<" ---> RemoteFunctions::open - Error al enviar flags.\n";
		return -1;
	}
	int res = 0;
	if( ! con.readInt(res) ){
		cout<<" ---> RemoteFunctions::open - Error al realizar open.\n";
		return -1;
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		errno = new_errno;
	}
	return res;
}

int RemoteFunctions::release(const char *path, int flags){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::release - Error de conexion.\n";
		return -1;
	}
	if( ! con.sendRequest( REMOTE_RELEASE ) ){
		cout<<" ---> RemoteFunctions::release - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::release - Error al enviar path.\n";
		return -1;
	}
	if( ! con.writeInt(flags) ){
		cout<<" ---> RemoteFunctions::release - Error al enviar flags.\n";
		return -1;
	}
	int res = 0;
	if( ! con.readInt(res) ){
		cout<<" ---> RemoteFunctions::release - Error al realizar release.\n";
		return -1;
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		errno = new_errno;
	}
	return res;
}

// En esta primera version solo considero datos descomprimidos
// Notar que el offset es solo para el envio, no importa en la recepcion
// Tambien notar que buf es valido para recibir la respuesta
// Este metodo espera un maximo de size chars en esa respuesta
// Por ultimo, size es tanto lo pedido como el tamaño valido de buff
int RemoteFunctions::read(const char *path, char *buf, size_t size, off_t offset, int flags){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::read - Error de conexion.\n";
		return -1;
	}
	if( ! con.sendRequest( REMOTE_READ ) ){
		cout<<" ---> RemoteFunctions::read - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::read - Error al enviar path.\n";
		return -1;
	}
	if( ! con.writeUInt(size) ){
		cout<<" ---> RemoteFunctions::read - Error al enviar size.\n";
		return -1;
	}
	if( ! con.writeULong(offset) ){
		cout<<" ---> RemoteFunctions::read - Error al enviar offset.\n";
		return -1;
	}
	if( ! con.writeInt(flags) ){
		cout<<" ---> RemoteFunctions::read - Error al enviar flags.\n";
		return -1;
	}
	// En este caso recibo res (que indica el largo de la respuesta si es > 0)
	// Luego los datos (solo si res es > 0)
	// Finalmente, errno
	int res = 0;
	if( ! con.readInt(res) ){
		cout<<" ---> RemoteFunctions::read - Error al realizar read.\n";
		return -1;
	}
	if( res > 0 ){
		if( ! con.readData( buf, res ) ){
			cout<<" ---> RemoteFunctions::read - Error al recibir chars.\n";
			return -1;
		}
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		errno = new_errno;
	}
	return res;
}
int RemoteFunctions::write(const char *path, const char *buf, size_t size, off_t offset, int flags){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::write - Error de conexion.\n";
		return -1;
	}
	if( ! con.sendRequest( REMOTE_WRITE ) ){
		cout<<" ---> RemoteFunctions::write - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::write - Error al enviar path.\n";
		return -1;
	}
	if( ! con.writeUInt(size) ){
		cout<<" ---> RemoteFunctions::write - Error al enviar size.\n";
		return -1;
	}
	if( ! con.writeData( buf, size) ){
		cout<<" ---> RemoteFunctions::write - Error al enviar chars.\n";
		return -1;
	}
	if( ! con.writeULong(offset) ){
		cout<<" ---> RemoteFunctions::write - Error al enviar offset.\n";
		return -1;
	}
	if( ! con.writeInt(flags) ){
		cout<<" ---> RemoteFunctions::write - Error al enviar flags.\n";
		return -1;
	}
	int res = 0;
	if( ! con.readInt(res) ){
		cout<<" ---> RemoteFunctions::write - Error al realizar write.\n";
		return -1;
	}
	int new_errno = 0;
	if( con.readInt(new_errno) ){
		errno = new_errno;
	}
	return res;
}

	
// Metodo para copiar un archivo del server a una ruta local
// La idea es que open llame a este metodo y lo abra para read / write local
// Retorna 1 si crea el archivo, 0 si ya existia y estaba actualizado
int RemoteFunctions::receiveFile(const char *path, const char *path_local){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::receiveFile - Error de conexion.\n";
		return -1;
	}
	// Revisar si el archivo local existe
	// Si existe, no hacer nada por ahora
	// La idea es, en ese caso, revisar la version o fecha de modificacion para actualizar
	// Si el archivo no existe, enviar peticion de archivo
	// Tomar la respuesta (largo y luego bytes)
	// Escribir los bytes en el archivo local mientras se leen
	cout<<" ---> RemoteFunctions::receiveFile - Inicio.\n";
	
	FILE *file = fopen(path_local, "r");
	if (file){
		cout<<" ---> RemoteFunctions::receiveFile - Archivo existe, retornando.\n";
		fclose(file);
		return 0;
	}
	cout<<" ---> RemoteFunctions::receiveFile - Archivo no encontrado, pidiendo datos.\n";
	if( ! con.sendRequest( REMOTE_RECEIVE ) ){
		cout<<" ---> RemoteFunctions::receiveFile - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::receiveFile - Error al enviar path.\n";
		return -1;
	}
	unsigned long long size = 0;
	if( ! con.readULong(size) ){
		cout<<" ---> RemoteFunctions::receiveFile - Error al recibir size.\n";
		return -1;
	}
	cout<<" ---> RemoteFunctions::receiveFile - size: "<<size<<".\n";
	// Preparar buffer local para la lectura / escritura
	unsigned int buff_size = 1024;
	char buff[buff_size + 1];
	unsigned long long total = 0;
	
	cout<<" ---> RemoteFunctions::receiveFile - Creando \""<<path_local<<"\" y leyendo bytes.\n";
	fstream escritor(path_local, fstream::trunc | fstream::out);
	while( total < size){
		if( size - total < buff_size ){
			buff_size = size - total;
		}
		if( ! con.readData(buff, buff_size) ){
			cout<<" ---> RemoteFunctions::receiveFile - Error al recibir data.\n";
			break;
		}
		buff[buff_size] = 0;
		cout<<" ---> RemoteFunctions::receiveFile - buff: \""<<buff<<"\".\n";
		if(escritor.good()){
			escritor.write(buff, buff_size);
		}
		// Notar que estoy sumando directamente buff_size
		// Quzias sea mejor preguntar por los bytes escritos
		total += buff_size;
	}
	if(escritor.good()){
		escritor.close();
	}
	else{
		cout<<" ---> RemoteFunctions::receiveFile - Problemas con escritor.\n";
	}
	
	cout<<" ---> RemoteFunctions::receiveFile - Fin.\n";
	return 1;
}

// Metodo para enviar un archivo al server desde una ruta local
// La idea es que release llame a este metodo para dejar el archivo en el server
int RemoteFunctions::sendFile(const char *path, const char *path_local){
	ServerConnection con(host, port, user_id);
	if( ! con.good() ){
		cout<<" ---> RemoteFunctions::sendFile - Error de conexion.\n";
		return -1;
	}
	
	// Abrir el archivo local para asegurar que existe y tomar el largo
	// Enviar el path y el largo
	// Leer y enviar los bytes
	
	cout<<" ---> RemoteFunctions::sendFile - Inicio (path_local: \""<<path_local<<"\").\n";
	
	// Envio el path igual, si hay problemas con el archivo local, queda creado con size 0 en el server
	if( ! con.sendRequest( REMOTE_SEND ) ){
		cout<<" ---> RemoteFunctions::sendFile - Error al enviar request.\n";
		return -1;
	}
	if( ! con.writeString(path) ){
		cout<<" ---> RemoteFunctions::sendFile - Error al enviar path.\n";
		return -1;
	}
	
	fstream lector(path_local, fstream::in);
	unsigned long long file_size = 0;
	if( ! lector.good() ){
		cout<<" ---> RemoteFunctions::sendFile - Problemas abriendo archivo.\n";
		if( ! con.writeULong(file_size) ){
			cout<<" ---> RemoteFunctions::sendFile - Error al enviar file_size.\n";
			return -1;
		}
	}
	else{
		lector.seekg (0, lector.end);
		file_size = lector.tellg();
		lector.seekg (0, lector.beg);
		if( ! con.writeULong(file_size) ){
			cout<<" ---> RemoteFunctions::sendFile - Error al enviar file_size ("<<file_size<<").\n";
			return -1;
		}
		unsigned int buff_size = 1024;
		char buff[buff_size];
		unsigned long long total = 0;
		cout<<" ---> RemoteFunctions::sendFile - Leyendo y enviando "<<file_size<<" bytes.\n";
		
		while(total < file_size){
			if(total - file_size < buff_size){
				buff_size = total - file_size;
			}
			lector.read(buff, buff_size);
			if( ! con.writeData(buff, buff_size) ){
				cout<<" ---> RemoteFunctions::sendFile - Error al enviar bytes, cancelando.\n";
				break;
			}
			// Notar que estoy sumando directamente buff_size
			// Quzias sea mejor preguntar por los bytes leidos
			total += buff_size;
		}
		lector.close();
	}
	
	cout<<" ---> RemoteFunctions::sendFile - Fin.\n";
	return 0;
}
















