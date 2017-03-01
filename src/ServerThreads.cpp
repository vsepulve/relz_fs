#include "ServerThreads.h"

// Por ahora dejo esto como variable GLOBAL
// Obviamente esto deberia ser de algun objeto de configuracion
const char *base_path_server = "/cebib_yeast_real";

unsigned int real_path_size(const char *base_path, const char *path){
	// base + / + user(11) + / + path + 0
	return strlen(base_path) + strlen(path) + 14;
}
void create_real_path(char *buff, const char *base_path, unsigned int user, const char *path){
	if( buff == NULL || base_path == NULL || path == NULL ){
		return;
	}
	if( base_path[ strlen(base_path) - 1 ] == '/' ){
//		sprintf(buff, "%s%d/", base_path, user);
		sprintf(buff, "%s", base_path);
	}
	else{
//		sprintf(buff, "%s/%d/", base_path, user);
		sprintf(buff, "%s/", base_path);
	}
	if( path[ 0 ] == '/' ){
		strcpy(buff + strlen(buff), path + 1);
	}
	else{
		strcpy(buff + strlen(buff), path);
	}
}

void thread_stat(int sock_cliente, unsigned int user_id){
	
	ClientReception conexion;
	conexion.setSocket(sock_cliente);
	
//	logger(user_id)<<"Server::thread_stat - Inicio (user_id: "<<user_id<<")\n";
	logger()<<"Server::thread_stat - Inicio (user_id: "<<user_id<<")\n";
	
	// Recibir largo
	// Recibir string
	// Realizar stat
	// Enviar res
	// Serializar y enviar stat_buff
	
	// Notar que en esta version leo por separado el largo y los bytes de path (en lugar de readString)
	// Esto es porque no tengo un buffer pedido y prefiero reducir su tamaño al necesario
	// Tambien notar que en cualquier error (largo o chars) redefino size = 0 (string valido pero vacio)
	
	unsigned int size = 0;
	bool error = false;
	if( ! conexion.readUInt(size) ){
		logger()<<"Server::thread_stat - Error al recibir size.\n";
		size = 0;
		error = true;
	}
	char path[ size + 1 ];
	if( ! conexion.readData(path, size) ){
		logger()<<"Server::thread_stat - Error al recibir path.\n";
		size = 0;
		error = true;
	}
	path[size] = 0;
	logger()<<"Server::thread_stat - path: \""<<path<<"\" ("<<size<<", error: "<<error<<")\n";
	
	int res = -1;
	int new_errno = 0;
	struct stat stbuf;
	
	if( ! error ){
		char real_path[ real_path_size( base_path_server, path ) ];
		create_real_path(real_path, base_path_server, user_id, path);
		logger()<<"Server::thread_stat - real_path: \""<<real_path<<"\"\n";
		res = lstat(real_path, &stbuf);
		new_errno = errno;
	}
	
	// Notar que envio la respuesta incluso en caso de error (pues el cliente la espera en cualquier caso)
	logger()<<"Server::thread_stat - Enviando Respuesta (res: "<<res<<", new_errno: "<<new_errno<<")\n";
	conexion.writeInt(res);
	conexion.writeData((char*)&stbuf, sizeof(struct stat));
	conexion.writeInt(new_errno);
	
	int marca = 0;
	conexion.writeInt(marca);
	logger()<<"Server::thread_stat - Terminando (marca: "<<marca<<")\n";
	
}//fin thread_stat

void thread_access(int sock_cliente, unsigned int user_id){
	
	ClientReception conexion;
	conexion.setSocket(sock_cliente);
	
//	logger(user_id)<<"Server::thread_access - Inicio (user_id: "<<user_id<<")\n";
	logger()<<"Server::thread_access - Inicio (user_id: "<<user_id<<")\n";
	
	// Recibir largo
	// Recibir string
	// Realizar access
	// Enviar res
	
	unsigned int size = 0;
	bool error = false;
	if( ! conexion.readUInt(size) ){
		logger()<<"Server::thread_access - Error al recibir size.\n";
		size = 0;
		error = true;
	}
	char path[ size + 1 ];
	if( ! conexion.readData(path, size) ){
		logger()<<"Server::thread_access - Error al recibir path.\n";
		size = 0;
		error = true;
	}
	path[size] = 0;
	logger()<<"Server::thread_access - path: \""<<path<<"\" ("<<size<<", error: "<<error<<")\n";
	
	int mask = 0;
	if( ! conexion.readInt(mask) ){
		logger()<<"Server::thread_access - Error al recibir mask.\n";
		mask = 0;
	}
	
	int res = -1;
	int new_errno = 0;
	
	if( ! error ){
		char real_path[ real_path_size( base_path_server, path ) ];
		create_real_path(real_path, base_path_server, user_id, path);
		logger()<<"Server::thread_access - real_path: \""<<real_path<<"\"\n";
		res = access(real_path, mask);
		new_errno = errno;
	}
	
	// Notar que envio la respuesta incluso en caso de error (pues el cliente la espera en cualquier caso)
	logger()<<"Server::thread_access - Enviando Respuesta (res: "<<res<<")\n";
	conexion.writeInt(res);
	conexion.writeInt(new_errno);
	
	int marca = 0;
	conexion.writeInt(marca);
	logger()<<"Server::thread_access - Terminando (marca: "<<marca<<")\n";
	
}//fin thread_access

void thread_readdir(int sock_cliente, unsigned int user_id){
	
	ClientReception conexion;
	conexion.setSocket(sock_cliente);
	
//	logger(user_id)<<"Server::thread_readdir - Inicio (user_id: "<<user_id<<")\n";
	logger()<<"Server::thread_readdir - Inicio (user_id: "<<user_id<<")\n";
	
	// Recibir largo
	// Recibir string
	
	// Contar dirs (y calcular n_bytes que depende de los nombres)
	// Enviar n_bytes y n_files
	
	// Volver a iterar por los archivos
	// Tomar datos para enviar y calcular n_bytes (de este envio)
	// Enviar n_bytes y datos necesarios del archivo
	
	// Notar que en esta version leo por separado el largo y los bytes de path (en lugar de readString)
	// Esto es porque no tengo un buffer pedido y prefiero reducir su tamaño al necesario
	// Tambien notar que en cualquier error (largo o chars) redefino size = 0 (string valido pero vacio)
	
	unsigned int size = 0;
	bool error = false;
	if( ! conexion.readUInt(size) ){
		logger()<<"Server::thread_readdir - Error al recibir size.\n";
		size = 0;
		error = true;
	}
	char path[ size + 1 ];
	if( ! conexion.readData(path, size) ){
		logger()<<"Server::thread_readdir - Error al recibir path.\n";
		size = 0;
		error = true;
	}
	path[size] = 0;
	logger()<<"Server::thread_readdir - path: \""<<path<<"\" ("<<size<<", error: "<<error<<")\n";
	
	char real_path[ real_path_size( base_path_server, path ) ];
	create_real_path(real_path, base_path_server, user_id, path);
	logger()<<"Server::thread_readdir - real_path: \""<<real_path<<"\"\n";
	
//	unsigned int struct_size = sizeof(struct dirent);
	unsigned int n_bytes = 0;
	
	DIR * directory = opendir(real_path);
	int new_errno = errno;
	if ( ! error && directory != NULL){
		
		// Preparo datos para iterar por los archivos
		
		// Burocracia para construir el struct con memoria pedida
		int name_max = pathconf(real_path, _PC_NAME_MAX);
		// Si el limite no esta definido, adivino
		if (name_max == -1){
			name_max = 255;
		}
		int len = offsetof(struct dirent, d_name) + name_max + 1;
		dirent *child_dir = (struct dirent *)malloc( len );
		dirent *test_dir = NULL;
		// Por lo menos enviare n_files
		n_bytes = sizeof(int);
		unsigned int n_files = 0;
		long long st_ino = 0;
		int st_mode = 0;
		
		while(true){
			readdir_r( directory, child_dir, &test_dir );
			if(test_dir == NULL){
				break;
			}
			++n_files;
			// bytes para st_ino y st_mode
			n_bytes += sizeof(long long) + sizeof(int);
			// bytes para largo del string nombre
			n_bytes += sizeof(int);
			// bytes para el string nombre
			n_bytes += strlen(child_dir->d_name);
		}
		closedir(directory);
		
		logger()<<"Server::thread_readdir - n_files: "<<n_files<<", n_bytes: "<<n_bytes<<"\n";
		
		// Envio metadatos para luego iterar
		// - n_bytes (en bytes igual a todo lo que sigue)
		// - n_files (numero de archivos)
		// - (luego) datos de cada archivo
		conexion.writeInt(n_bytes);
		conexion.writeInt(n_files);
		
		// Reabro y vuelvo a iterar para enviar los archivos del dir
		directory = opendir(real_path);
		unsigned int cur_dir = 0;
		for( ; directory != NULL && cur_dir < n_files; ++cur_dir){
			readdir_r( directory, child_dir, &test_dir );
			if(test_dir == NULL){
				break;
			}
			// Enviare d_ino en un unsigned long long y d_type en unsigned int (listo para stat)
			st_ino = (long long)(child_dir->d_ino);
			st_mode = (int)(child_dir->d_type);
			st_mode <<= 12;
			
			logger()<<"Server::thread_readdir - archivo["<<cur_dir<<"]: st_ino: "<<st_mode<<", st_ino: "<<st_mode<<", nombre: \""<<child_dir->d_name<<"\" (largo: "<<strlen(child_dir->d_name)<<")\n";
			
			conexion.writeLong( st_ino );
			conexion.writeInt( st_mode );
			
			n_bytes = strlen(child_dir->d_name);
			conexion.writeInt(n_bytes);
			conexion.writeData( child_dir->d_name, n_bytes );
			
		}
		// Envio la cola de bytes en caso de salida temprana del ciclo
		st_ino = 0;
		st_mode = 0;
		for( ; cur_dir < n_files; ++cur_dir){
			logger()<<"Server::thread_readdir - archivo["<<cur_dir<<"]: vacio (ADVERTENCIA)\n";
			conexion.writeLong( st_ino );
			conexion.writeInt( st_mode );
			// Notar que esto funciona incluso en caso de error
			// Esto es porque se enviaria un string menor leido independientemente en el receptor
			// Entonces el buffer sera suficiente, y la lectura continuara adecuadamente
			n_bytes = 0;
			conexion.writeInt(n_bytes);
			// No es encesario escribir nada mas
		}
		
		free(child_dir);
	}
	else{
		// Codio de error
		n_bytes = 0;
		conexion.writeInt(n_bytes);
	}
	//En cualquier caso, mando errno
	conexion.writeInt(new_errno);
	
	int marca = 0;
	conexion.writeInt(marca);
	logger()<<"Server::thread_readdir - Terminando (marca: "<<marca<<")\n";
	
}//fin thread_readdir

void thread_mknod(int sock_cliente, unsigned int user_id){
	
	ClientReception conexion;
	conexion.setSocket(sock_cliente);
	
//	logger(user_id)<<"Server::thread_mknod - Inicio (user_id: "<<user_id<<")\n";
	logger()<<"Server::thread_mknod - Inicio (user_id: "<<user_id<<")\n";
	
	unsigned int size = 0;
	bool error = false;
	if( ! conexion.readUInt(size) ){
		logger()<<"Server::thread_mknod - Error al recibir size.\n";
		size = 0;
		error = true;
	}
	char path[ size + 1 ];
	if( ! conexion.readData(path, size) ){
		logger()<<"Server::thread_mknod - Error al recibir path.\n";
		size = 0;
		error = true;
	}
	path[size] = 0;
	logger()<<"Server::thread_mknod - path: \""<<path<<"\" ("<<size<<", error: "<<error<<")\n";
	unsigned char n_bytes = 0;
	mode_t mode;
	dev_t dev;
	if( ! error && ! conexion.readByte(n_bytes) ){
		logger()<<"Server::thread_mknod - Error al recibir n_bytes.\n";
		error = true;
	}
	if( n_bytes != (sizeof(mode_t) + sizeof(dev_t)) ){
		logger()<<"Server::thread_mknod - n_bytes incorrecto.\n";
		error = true;
	}
	if(! error && ! conexion.readData((char*)&mode, sizeof(mode_t)) ){
		logger()<<"Server::thread_mknod - Error al recibir mode.\n";
		error = true;	
	}
	if(! error && ! conexion.readData((char*)&dev, sizeof(dev_t)) ){
		logger()<<"Server::thread_mknod - Error al recibir dev.\n";
		error = true;	
	}
	
	int res = -1;
	int new_errno = errno;
	
	if( ! error ){
		char real_path[ real_path_size( base_path_server, path ) ];
		create_real_path(real_path, base_path_server, user_id, path);
		logger()<<"Server::thread_mknod - real_path: \""<<real_path<<"\"\n";
		
		if( S_ISREG(mode) ){
			logger()<<"Server::thread_mknod - S_ISREG \n";
			res = open(real_path, O_CREAT | O_EXCL | O_WRONLY, mode);
			new_errno = errno;
			if(res >= 0){
				logger()<<"Server::thread_mknod - Open Ok\n";
				res = close(res);
			}
		}
		else if( S_ISFIFO(mode) ){
			logger()<<"Server::thread_mknod - S_ISFIFO \n";
			res = mkfifo(real_path, mode);
			new_errno = errno;
		}
		else{
			logger()<<"Server::thread_mknod - ELSE \n";
			res = mknod(real_path, mode, dev);
			new_errno = errno;
		}
		
	}
	
	// Notar que envio la respuesta incluso en caso de error (pues el cliente la espera en cualquier caso)
	logger()<<"Server::thread_mknod - Enviando Respuesta (res: "<<res<<", new_errno: "<<new_errno<<")\n";
	conexion.writeInt(res);
	conexion.writeInt(new_errno);
	
	int marca = 0;
	conexion.writeInt(marca);
	logger()<<"Server::thread_mknod - Terminando (marca: "<<marca<<")\n";
	
}//fin thread_mknod

void thread_mkdir(int sock_cliente, unsigned int user_id){
	
	ClientReception conexion;
	conexion.setSocket(sock_cliente);
	
//	logger(user_id)<<"Server::thread_mkdir - Inicio (user_id: "<<user_id<<")\n";
	logger()<<"Server::thread_mkdir - Inicio (user_id: "<<user_id<<")\n";
	
	unsigned int size = 0;
	bool error = false;
	if( ! conexion.readUInt(size) ){
		logger()<<"Server::thread_mkdir - Error al recibir size.\n";
		size = 0;
		error = true;
	}
	char path[ size + 1 ];
	if( ! conexion.readData(path, size) ){
		logger()<<"Server::thread_mkdir - Error al recibir path.\n";
		size = 0;
		error = true;
	}
	path[size] = 0;
	logger()<<"Server::thread_mkdir - path: \""<<path<<"\" ("<<size<<", error: "<<error<<")\n";
	
	unsigned char n_bytes = 0;
	mode_t mode;
	if( ! error && ! conexion.readByte(n_bytes) ){
		logger()<<"Server::thread_mkdir - Error al recibir n_bytes.\n";
		error = true;
	}
	if( n_bytes != sizeof(mode_t) ){
		logger()<<"Server::thread_mkdir - n_bytes incorrecto.\n";
		error = true;
	}
	if(! error && ! conexion.readData((char*)&mode, n_bytes) ){
		logger()<<"Server::thread_mkdir - Error al recibir mode.\n";
		error = true;	
	}
	
	int res = -1;
	int new_errno = errno;
	
	if( ! error ){
		char real_path[ real_path_size( base_path_server, path ) ];
		create_real_path(real_path, base_path_server, user_id, path);
		logger()<<"Server::thread_mkdir - real_path: \""<<real_path<<"\"\n";

		res = mkdir(real_path, mode);
		new_errno = errno;
		
		// Aqui quizas haya que eliminar el compresor o algo adicional
		
	}
	
	// Notar que envio la respuesta incluso en caso de error (pues el cliente la espera en cualquier caso)
	logger()<<"Server::thread_mkdir - Enviando Respuesta (res: "<<res<<", new_errno: "<<new_errno<<")\n";
	conexion.writeInt(res);
	conexion.writeInt(new_errno);
	
	int marca = 0;
	conexion.writeInt(marca);
	logger()<<"Server::thread_mkdir - Terminando (marca: "<<marca<<")\n";
	
}//fin thread_unlink

void thread_unlink(int sock_cliente, unsigned int user_id){
	
	ClientReception conexion;
	conexion.setSocket(sock_cliente);
	
//	logger(user_id)<<"Server::thread_unlink - Inicio (user_id: "<<user_id<<")\n";
	logger()<<"Server::thread_unlink - Inicio (user_id: "<<user_id<<")\n";
	
	unsigned int size = 0;
	bool error = false;
	if( ! conexion.readUInt(size) ){
		logger()<<"Server::thread_unlink - Error al recibir size.\n";
		size = 0;
		error = true;
	}
	char path[ size + 1 ];
	if( ! conexion.readData(path, size) ){
		logger()<<"Server::thread_unlink - Error al recibir path.\n";
		size = 0;
		error = true;
	}
	path[size] = 0;
	logger()<<"Server::thread_unlink - path: \""<<path<<"\" ("<<size<<", error: "<<error<<")\n";
	
	int res = -1;
	int new_errno = errno;
	
	if( ! error ){
		char real_path[ real_path_size( base_path_server, path ) ];
		create_real_path(real_path, base_path_server, user_id, path);
		logger()<<"Server::thread_unlink - real_path: \""<<real_path<<"\"\n";

		res = unlink(real_path);
		new_errno = errno;
		
		// Aqui quizas haya que eliminar el compresor o algo adicional
		
	}
	
	// Notar que envio la respuesta incluso en caso de error (pues el cliente la espera en cualquier caso)
	logger()<<"Server::thread_unlink - Enviando Respuesta (res: "<<res<<", new_errno: "<<new_errno<<")\n";
	conexion.writeInt(res);
	conexion.writeInt(new_errno);
	
	int marca = 0;
	conexion.writeInt(marca);
	logger()<<"Server::thread_unlink - Terminando (marca: "<<marca<<")\n";
	
}//fin thread_unlink

void thread_rmdir(int sock_cliente, unsigned int user_id){
	
	ClientReception conexion;
	conexion.setSocket(sock_cliente);
	
//	logger(user_id)<<"Server::thread_rmdir - Inicio (user_id: "<<user_id<<")\n";
	logger()<<"Server::thread_rmdir - Inicio (user_id: "<<user_id<<")\n";
	
	unsigned int size = 0;
	bool error = false;
	if( ! conexion.readUInt(size) ){
		logger()<<"Server::thread_rmdir - Error al recibir size.\n";
		size = 0;
		error = true;
	}
	char path[ size + 1 ];
	if( ! conexion.readData(path, size) ){
		logger()<<"Server::thread_rmdir - Error al recibir path.\n";
		size = 0;
		error = true;
	}
	path[size] = 0;
	logger()<<"Server::thread_rmdir - path: \""<<path<<"\" ("<<size<<", error: "<<error<<")\n";
	
	int res = -1;
	int new_errno = errno;
	
	if( ! error ){
		char real_path[ real_path_size( base_path_server, path ) ];
		create_real_path(real_path, base_path_server, user_id, path);
		logger()<<"Server::thread_rmdir - real_path: \""<<real_path<<"\"\n";

		res = rmdir(real_path);
		new_errno = errno;
	}
	
	// Notar que envio la respuesta incluso en caso de error (pues el cliente la espera en cualquier caso)
	logger()<<"Server::thread_rmdir - Enviando Respuesta (res: "<<res<<", new_errno: "<<new_errno<<")\n";
	conexion.writeInt(res);
	conexion.writeInt(new_errno);
	
	int marca = 0;
	conexion.writeInt(marca);
	logger()<<"Server::thread_rmdir - Terminando (marca: "<<marca<<")\n";
	
}//fin thread_rmdir

void thread_rename(int sock_cliente, unsigned int user_id){
	
	ClientReception conexion;
	conexion.setSocket(sock_cliente);
	
//	logger(user_id)<<"Server::thread_rename - Inicio (user_id: "<<user_id<<")\n";
	logger()<<"Server::thread_rename - Inicio (user_id: "<<user_id<<")\n";
	
	unsigned int size = 0;
	bool error = false;
	if( ! conexion.readUInt(size) ){
		logger()<<"Server::thread_rename - Error al recibir size.\n";
		size = 0;
		error = true;
	}
	char from[ size + 1 ];
	if( ! conexion.readData(from, size) ){
		logger()<<"Server::thread_rename - Error al recibir from.\n";
		size = 0;
		error = true;
	}
	from[size] = 0;
	logger()<<"Server::thread_rename - from: \""<<from<<"\" ("<<size<<", error: "<<error<<")\n";
	
	size = 0;
	if( ! conexion.readUInt(size) ){
		logger()<<"Server::thread_rename - Error al recibir size.\n";
		size = 0;
		error = true;
	}
	char to[ size + 1 ];
	if( ! conexion.readData(to, size) ){
		logger()<<"Server::thread_rename - Error al recibir to.\n";
		size = 0;
		error = true;
	}
	to[size] = 0;
	logger()<<"Server::thread_rename - to: \""<<to<<"\" ("<<size<<", error: "<<error<<")\n";
	
	int res = -1;
	int new_errno = errno;
	
	if( ! error ){
		char real_from[ real_path_size( base_path_server, from ) ];
		create_real_path(real_from, base_path_server, user_id, from);
		char real_to[ real_path_size( base_path_server, to ) ];
		create_real_path(real_to, base_path_server, user_id, to);
		logger()<<"Server::thread_rename - real_from: \""<<real_from<<"\", real_to: \""<<real_to<<"\"\n";
		
		// Por ahora realizo el rename directo
		// Aqui HAY QUE CONTROLAR la conversion de archivos comprimidos
		
		res = rename(real_from, real_to);
		new_errno = errno;
	}
	
	// Notar que envio la respuesta incluso en caso de error (pues el cliente la espera en cualquier caso)
	logger()<<"Server::thread_rename - Enviando Respuesta (res: "<<res<<", new_errno: "<<new_errno<<")\n";
	conexion.writeInt(res);
	conexion.writeInt(new_errno);
	
	int marca = 0;
	conexion.writeInt(marca);
	logger()<<"Server::thread_rename - Terminando (marca: "<<marca<<")\n";
	
}//fin thread_rename

void thread_chmod(int sock_cliente, unsigned int user_id){
	
	ClientReception conexion;
	conexion.setSocket(sock_cliente);
	
//	logger(user_id)<<"Server::thread_chmod - Inicio (user_id: "<<user_id<<")\n";
	logger()<<"Server::thread_chmod - Inicio (user_id: "<<user_id<<")\n";
	
	unsigned int size = 0;
	bool error = false;
	if( ! conexion.readUInt(size) ){
		logger()<<"Server::thread_chmod - Error al recibir size.\n";
		size = 0;
		error = true;
	}
	char path[ size + 1 ];
	if( ! conexion.readData(path, size) ){
		logger()<<"Server::thread_chmod - Error al recibir path.\n";
		size = 0;
		error = true;
	}
	path[size] = 0;
	logger()<<"Server::thread_chmod - path: \""<<path<<"\" ("<<size<<", error: "<<error<<")\n";
	
	unsigned char n_bytes = 0;
	mode_t mode;
	if( ! error && ! conexion.readByte(n_bytes) ){
		logger()<<"Server::thread_chmod - Error al recibir n_bytes.\n";
		error = true;
	}
	if( n_bytes != sizeof(mode_t) ){
		logger()<<"Server::thread_chmod - n_bytes incorrecto.\n";
		error = true;
	}
	if(! error && ! conexion.readData((char*)&mode, n_bytes) ){
		logger()<<"Server::thread_chmod - Error al recibir mode.\n";
		error = true;	
	}
	
	int res = -1;
	int new_errno = errno;
	
	if( ! error ){
		char real_path[ real_path_size( base_path_server, path ) ];
		create_real_path(real_path, base_path_server, user_id, path);
		logger()<<"Server::thread_chmod - real_path: \""<<real_path<<"\"\n";

		res = chmod(real_path, mode);
		new_errno = errno;
	}
	
	// Notar que envio la respuesta incluso en caso de error (pues el cliente la espera en cualquier caso)
	logger()<<"Server::thread_chmod - Enviando Respuesta (res: "<<res<<", new_errno: "<<new_errno<<")\n";
	conexion.writeInt(res);
	conexion.writeInt(new_errno);
	
	int marca = 0;
	conexion.writeInt(marca);
	logger()<<"Server::thread_chmod - Terminando (marca: "<<marca<<")\n";
	
}//fin thread_chmod

void thread_chown(int sock_cliente, unsigned int user_id){
	
	ClientReception conexion;
	conexion.setSocket(sock_cliente);
	
//	logger(user_id)<<"Server::thread_chown - Inicio (user_id: "<<user_id<<")\n";
	logger()<<"Server::thread_chown - Inicio (user_id: "<<user_id<<")\n";
	
	unsigned int size = 0;
	bool error = false;
	if( ! conexion.readUInt(size) ){
		logger()<<"Server::thread_chown - Error al recibir size.\n";
		size = 0;
		error = true;
	}
	char path[ size + 1 ];
	if( ! conexion.readData(path, size) ){
		logger()<<"Server::thread_chown - Error al recibir path.\n";
		size = 0;
		error = true;
	}
	path[size] = 0;
	logger()<<"Server::thread_chown - path: \""<<path<<"\" ("<<size<<", error: "<<error<<")\n";
	
	unsigned char n_bytes = 0;
	uid_t uid;
	gid_t gid;
	if( ! error && ! conexion.readByte(n_bytes) ){
		logger()<<"Server::thread_chown - Error al recibir n_bytes.\n";
		error = true;
	}
	if( n_bytes != (sizeof(uid_t) + sizeof(gid_t)) ){
		logger()<<"Server::thread_chown - n_bytes incorrecto.\n";
		error = true;
	}
	if(! error && ! conexion.readData((char*)&uid, sizeof(uid_t)) ){
		logger()<<"Server::thread_chown - Error al recibir uid.\n";
		error = true;	
	}
	if(! error && ! conexion.readData((char*)&gid, sizeof(gid_t)) ){
		logger()<<"Server::thread_chown - Error al recibir gid.\n";
		error = true;	
	}
	
	int res = -1;
	int new_errno = errno;
	
	if( ! error ){
		char real_path[ real_path_size( base_path_server, path ) ];
		create_real_path(real_path, base_path_server, user_id, path);
		logger()<<"Server::thread_chown - real_path: \""<<real_path<<"\"\n";

		res = lchown(real_path, uid, gid);
		new_errno = errno;
	}
	
	// Notar que envio la respuesta incluso en caso de error (pues el cliente la espera en cualquier caso)
	logger()<<"Server::thread_chown - Enviando Respuesta (res: "<<res<<", new_errno: "<<new_errno<<")\n";
	conexion.writeInt(res);
	conexion.writeInt(new_errno);
	
	int marca = 0;
	conexion.writeInt(marca);
	logger()<<"Server::thread_chown - Terminando (marca: "<<marca<<")\n";
	
}//fin thread_chown

void thread_truncate(int sock_cliente, unsigned int user_id){
	
	ClientReception conexion;
	conexion.setSocket(sock_cliente);
	
//	logger(user_id)<<"Server::thread_truncate - Inicio (user_id: "<<user_id<<")\n";
	logger()<<"Server::thread_truncate - Inicio (user_id: "<<user_id<<")\n";
	
	unsigned int size = 0;
	bool error = false;
	if( ! conexion.readUInt(size) ){
		logger()<<"Server::thread_truncate - Error al recibir size.\n";
		size = 0;
		error = true;
	}
	char path[ size + 1 ];
	if( ! conexion.readData(path, size) ){
		logger()<<"Server::thread_truncate - Error al recibir path.\n";
		size = 0;
		error = true;
	}
	path[size] = 0;
	logger()<<"Server::thread_truncate - path: \""<<path<<"\" ("<<size<<", error: "<<error<<")\n";
	
	unsigned long long trunc_size = 0;
	if( ! error && ! conexion.readULong(trunc_size) ){
		logger()<<"Server::thread_truncate - Error al recibir trunc_size.\n";
		error = true;
	}
	
	int res = -1;
	int new_errno = errno;
	
	if( ! error ){
		char real_path[ real_path_size( base_path_server, path ) ];
		create_real_path(real_path, base_path_server, user_id, path);
		logger()<<"Server::thread_truncate - real_path: \""<<real_path<<"\"\n";

		res = truncate(real_path, (off_t)trunc_size);
		new_errno = errno;
	}
	
	// Notar que envio la respuesta incluso en caso de error (pues el cliente la espera en cualquier caso)
	logger()<<"Server::thread_truncate - Enviando Respuesta (res: "<<res<<", new_errno: "<<new_errno<<")\n";
	conexion.writeInt(res);
	conexion.writeInt(new_errno);
	
	int marca = 0;
	conexion.writeInt(marca);
	logger()<<"Server::thread_truncate - Terminando (marca: "<<marca<<")\n";
	
}//fin thread_truncate

void thread_statfs(int sock_cliente, unsigned int user_id){
	
	ClientReception conexion;
	conexion.setSocket(sock_cliente);
	
//	logger(user_id)<<"Server::thread_statfs - Inicio (user_id: "<<user_id<<")\n";
	logger()<<"Server::thread_statfs - Inicio (user_id: "<<user_id<<")\n";
	
	unsigned int size = 0;
	bool error = false;
	if( ! conexion.readUInt(size) ){
		logger()<<"Server::thread_statfs - Error al recibir size.\n";
		size = 0;
		error = true;
	}
	char path[ size + 1 ];
	if( ! conexion.readData(path, size) ){
		logger()<<"Server::thread_statfs - Error al recibir path.\n";
		size = 0;
		error = true;
	}
	path[size] = 0;
	logger()<<"Server::thread_statfs - path: \""<<path<<"\" ("<<size<<", error: "<<error<<")\n";
	
	unsigned char n_bytes = 0;
	struct statvfs stbuf;
	if( ! error && ! conexion.readByte(n_bytes) ){
		logger()<<"Server::thread_statfs - Error al recibir n_bytes.\n";
		error = true;
	}
	if( n_bytes != sizeof(struct statvfs) ){
		logger()<<"Server::thread_statfs - n_bytes incorrecto.\n";
		error = true;
	}
	if(! error && ! conexion.readData( (char*)&stbuf, sizeof(struct statvfs) ) ){
		logger()<<"Server::thread_statfs - Error al recibir stbuf.\n";
		error = true;	
	}
	
	int res = -1;
	int new_errno = errno;
	
	if( ! error ){
		char real_path[ real_path_size( base_path_server, path ) ];
		create_real_path(real_path, base_path_server, user_id, path);
		logger()<<"Server::thread_statfs - real_path: \""<<real_path<<"\"\n";

		res = statvfs(real_path, &stbuf);
		new_errno = errno;
	}
	
	// Notar que envio la respuesta incluso en caso de error (pues el cliente la espera en cualquier caso)
	logger()<<"Server::thread_statfs - Enviando Respuesta (res: "<<res<<", new_errno: "<<new_errno<<")\n";
	conexion.writeInt(res);
	// Reenvio stbuf solo si fue recivido validamente
	if( ! error ){
		conexion.writeData( (char*)&stbuf, sizeof(struct statvfs) );
	}
	conexion.writeInt(new_errno);
	
	int marca = 0;
	conexion.writeInt(marca);
	logger()<<"Server::thread_statfs - Terminando (marca: "<<marca<<")\n";
	
}//fin thread_statfs

void thread_fallocate(int sock_cliente, unsigned int user_id){
	
	ClientReception conexion;
	conexion.setSocket(sock_cliente);
	
//	logger(user_id)<<"Server::thread_fallocate - Inicio (user_id: "<<user_id<<")\n";
	logger()<<"Server::thread_fallocate - Inicio (user_id: "<<user_id<<")\n";
	
	unsigned int size = 0;
	bool error = false;
	if( ! conexion.readUInt(size) ){
		logger()<<"Server::thread_fallocate - Error al recibir size.\n";
		size = 0;
		error = true;
	}
	char path[ size + 1 ];
	if( ! conexion.readData(path, size) ){
		logger()<<"Server::thread_fallocate - Error al recibir path.\n";
		size = 0;
		error = true;
	}
	path[size] = 0;
	logger()<<"Server::thread_fallocate - path: \""<<path<<"\" ("<<size<<", error: "<<error<<")\n";
	
	int mode;
	unsigned long long offset;
	unsigned long long length;
	int flags;
	if( ! error && ! conexion.readInt(mode) ){
		logger()<<"Server::thread_fallocate - Error al recibir mode.\n";
		error = true;
	}
	if( ! error && ! conexion.readULong(offset) ){
		logger()<<"Server::thread_fallocate - Error al recibir offset.\n";
		error = true;
	}
	if( ! error && ! conexion.readULong(length) ){
		logger()<<"Server::thread_fallocate - Error al recibir length.\n";
		error = true;
	}
	if( ! error && ! conexion.readInt(flags) ){
		logger()<<"Server::thread_fallocate - Error al recibir flags.\n";
		error = true;
	}
	
	int res = -1;
	int new_errno = errno;
	
	if( ! error ){
		char real_path[ real_path_size( base_path_server, path ) ];
		create_real_path(real_path, base_path_server, user_id, path);
		logger()<<"Server::thread_fallocate - real_path: \""<<real_path<<"\"\n";
		
		// Notar que en esta version estoy omitiendo mode
		int fd = open(real_path, O_WRONLY);
		if (fd == -1){
			res = -errno;
			new_errno = errno;
		}
		else{
			res = -posix_fallocate(fd, (off_t)offset, (off_t)length);
			new_errno = errno;
			close(fd);
		}
	}
	
	// Notar que envio la respuesta incluso en caso de error (pues el cliente la espera en cualquier caso)
	logger()<<"Server::thread_fallocate - Enviando Respuesta (res: "<<res<<", new_errno: "<<new_errno<<")\n";
	conexion.writeInt(res);
	conexion.writeInt(new_errno);
	
	int marca = 0;
	conexion.writeInt(marca);
	logger()<<"Server::thread_fallocate - Terminando (marca: "<<marca<<")\n";
	
}//fin thread_fallocate

// Notar que el nombre es desde la perspectiva del cliente
// El server esta resolviendo para ENVIAR un archivo al cliente que lo recibira
void thread_receive(int sock_cliente, unsigned int user_id){
	
	ClientReception conexion;
	conexion.setSocket(sock_cliente);
	
//	logger(user_id)<<"Server::thread_receive - Inicio (user_id: "<<user_id<<")\n";
	logger()<<"Server::thread_receive - Inicio (user_id: "<<user_id<<")\n";
	
	unsigned int size = 0;
	bool error = false;
	if( ! conexion.readUInt(size) ){
		logger()<<"Server::thread_receive - Error al recibir size.\n";
		size = 0;
		error = true;
	}
	char path[ size + 1 ];
	if( ! conexion.readData(path, size) ){
		logger()<<"Server::thread_receive - Error al recibir path.\n";
		size = 0;
		error = true;
	}
	path[size] = 0;
	logger()<<"Server::thread_receive - path: \""<<path<<"\" ("<<size<<", error: "<<error<<")\n";
	
	// Abrir el archivo
	// Tomar el largo
	// Enviar el largo
	// Leer y enviar el contenido del archivo (esto requiere un buffer de lectura)
	
	char real_path[ real_path_size( base_path_server, path ) ];
	create_real_path(real_path, base_path_server, user_id, path);
	logger()<<"Server::thread_receive - real_path: \""<<real_path<<"\"\n";
		
	fstream lector(real_path, fstream::in);
	if( (! lector.good()) || (! lector.is_open()) ){
		cerr<<"Compressor::thread_receive - Error abriendo archivo \""<<real_path<<"\"\n";
		error = true;
	}
	
	
	if( ! error ){
		lector.seekg (0, lector.end);
		unsigned long long file_size = lector.tellg();
		lector.seekg (0, lector.beg);
	
		logger()<<"Server::thread_receive - Enviando largo de archivo ("<<file_size<<")\n";
		conexion.writeULong(file_size);
	
		unsigned int buff_size = 1024;
		char buff[buff_size];
		unsigned long long total = 0;
		logger()<<"Server::thread_receive - Leyendo y enviando bytes.\n";
		while(total < file_size){
			if(total - file_size < buff_size){
				buff_size = total - file_size;
			}
			lector.read(buff, buff_size);
			conexion.writeData(buff, buff_size);
			// Notar que estoy sumando directamente buff_size
			// Quzias sea mejor preguntar por los bytes leidos
			total += buff_size;
		}
		lector.close();
	}
	else{
		conexion.writeULong(0);
	}
	
	int marca = 0;
	conexion.writeInt(marca);
	logger()<<"Server::thread_receive - Terminando (marca: "<<marca<<")\n";
	
}
















