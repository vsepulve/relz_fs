
#include <thread>
#include <mutex>

#include <iostream>
#include <sstream>
#include <fstream>

//conexxion
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

//mapas
#include <map>

//para crear directorios
#include <sys/stat.h>
#include <dirent.h>
#include "dirList.h"

//#include "Logger.h"
#include "CheckUser.h"
#include "ConcurrentLogger.h"

#include "ReferenceIndex.h"
#include "ReferenceIndexBasic.h"
#include "CoderBlocks.h"
#include "DecoderBlocks.h"
#include "RecoderBlocks.h"
#include "Communication.h"

// NUEVOS TIPOS de request_type (POV cliente)
// 0 : se guarda para caso de error o request vacio (quizas pueda usarse para algo tipo ping)
// 1 : envio directo de archivo (sin procesamiento)
// 2 : pedir archivo (sin procesamiento)
// 3 : listar archivos (es decir, pedir lista remota de archivos)
// 4 : envio de archivo para compresion en el server (envia descomprimido y el server comprime)
// 5 : envio de archivo con doble compresion (comprime localmente, envia y el server recomprime)
// 100 : kill, orden de cerrar el server.

using namespace std;

//Constantes (estas podrian ser constantes verdaras, o variables estaticas de algun objeto de configuracion)
unsigned int THREADS_PROCESO = 16;
unsigned int BLOCK_SIZE = 1000000;

//manejador de clientes

//recive directamente un archivo y lo guarda sin procesar nada
void handler_receive(int sock_cliente, unsigned int user_id) {
	//creando logger y limpiando arreglos de entrada
	
	logger(user_id)<<"------------------------------\n";
	logger(user_id)<<"Server::handler_receive - Inicio.\n";
	
	char respuesta = 0;
	
	char prefix[20];
	sprintf(prefix, "datos/%d", user_id);
	
	if( ! receiveFileWrite(sock_cliente, prefix, NULL, BLOCK_SIZE, NULL) ){
		logger(user_id)<<"Server::handler_receive - Error recibiendo archivo.\n";
		write(sock_cliente, &respuesta, 1);
		close(sock_cliente);
		return;
	}
	
	logger(user_id)<<"Server::handler_receive - Archivo recibido y guardado, enviando respuesta.\n";
	
	respuesta = 1;
	if( write(sock_cliente, &respuesta, 1) < 0){
		logger(user_id)<<"Server::handler_receive - Error al enviar respuesta.\n";
		close(sock_cliente);
	}
	
	close(sock_cliente);
	logger(user_id)<<"Server::handler_receive - Fin.\n";
	
}//fin handler_receive

//recibe datos comprimidos
//los guarda, descomprime en un tmp, recomprime y envia el resultado
void handler_recompress(int sock_cliente, unsigned int user_id, ReferenceIndex *reference, unsigned int n_threads, unsigned int block_size) {
	//creando logger y limpiando arreglos de entrada
	
	logger(user_id)<<"------------------------------\n";
	logger(user_id)<<"Server::handler_recompress - Inicio.\n";
	
	char respuesta = 0;
	
	char prefix[20];
	sprintf(prefix, "datos/%d", user_id);
	
	char received_file[512];
	
	
	
	/*
	
	if( ! receiveFileWrite(sock_cliente, prefix, NULL, BLOCK_SIZE, received_file) ){
		logger(user_id)<<"Server::handler_recompress - Error recibiendo archivo.\n";
		write(sock_cliente, &respuesta, 1);
		close(sock_cliente);
		return;
	}
	
	//Archivos temporales para guardar el precomprimido y descomprimir
	char received_tmp[512];
	sprintf(received_tmp, "%s.tmp", received_file);
	
	char received_tmp_dec[512];
	sprintf(received_tmp_dec, "%s.tmp.dec", received_file);
	
	//Mover el archivo recibido a precomprimido
	if( rename(received_file, received_tmp) != 0 ){
		logger(user_id)<<"Server::handler_recompress - Error renombrando archivo.\n";
		write(sock_cliente, &respuesta, 1);
		close(sock_cliente);
		return;
	}
	
	//Archivo tmp terminado, descomprimir
	logger(user_id)<<"Server::handler_recompress - Descomprimiendo en \""<<received_tmp_dec<<"\".\n";
	DecoderBlocks descompresor(received_tmp, reference->getText());
	descompresor.decodeFullText(received_tmp_dec);
	
	//Archivo descomprimido, coprimir archivo final
	logger(user_id)<<"Server::handler_recompress - Recomprimiendo en \""<<received_file<<"\".\n";
	CoderBlocks compresor(reference);
	compresor.compress(received_tmp_dec, received_file, n_threads, block_size);
	
	//Borrar archivos temporales
	logger(user_id)<<"Server::handler_recompress - Borrando archivos temporales.\n";
	remove(received_tmp_dec);
	remove(received_tmp);
	
	*/
	
	char *bytes = NULL;
	unsigned int total_bytes = 0;
	if( (bytes = receiveFile(sock_cliente, received_file, total_bytes) ) == NULL ){
		logger(user_id)<<"Server::handler_recompress - Error recibiendo archivo.\n";
		write(sock_cliente, &respuesta, 1);
		close(sock_cliente);
		return;
	}
	
	char final_file[512];
	sprintf(final_file, "%s/%s", prefix, received_file);
	checkDirectory(final_file);
	
	RecoderBlocks recoder(reference);
	recoder.compress(bytes, total_bytes, final_file, n_threads);
	
	
	logger(user_id)<<"Server::handler_recompress - Archivo recomprimido, enviando respuesta.\n";
	
	respuesta = 1;
	if( write(sock_cliente, &respuesta, 1) < 0){
		logger(user_id)<<"Server::handler_recompress - Error al enviar respuesta.\n";
		close(sock_cliente);
	}
	
	close(sock_cliente);
	logger(user_id)<<"Server::handler_recompress - Fin.\n";
	return;
	
}//fin handler_recompress

void handler_send(int sock_cliente, unsigned int user_id) {
	
	logger(user_id)<<"------------------------------\n";
	logger(user_id)<<"Server::handler_send - inicio. \n";
	
	//NOMBRE DEL ARCHIVO
	
	logger(user_id)<<"Server::handler_send - Recibiendo nombre de archivo.\n";
	char filename[512];
	if( receiveString(sock_cliente, filename, 512) == 0 ){
		logger(user_id)<<"Server::handler_send - Error en lectura de nombre.\n";
		close(sock_cliente);
		return;
	}
	
	logger(user_id)<<"Server::handler_send - Preparando path de archivo.\n";
	char to_search[512];
	sprintf(to_search, "datos/%d/%s", user_id, filename);
	
	//Enviar el archivo (metadatos y el archivo mismo)
	//El metodo se encarga de verificar existencia y enviar codigo en caso de fallo.
	
	if( ! sendFile(sock_cliente, to_search, filename, NULL, BLOCK_SIZE) ){
		logger(user_id)<<"Server::handler_send - Error al leer/enviar archivo.\n";
		close(sock_cliente);
		return;
	}
	
	logger(user_id)<<"Server::handler_send - Envio exitoso terminado.\n";
	
	close(sock_cliente);
	return;
	
} //fin handler_send

void handler_list(int _sock_cliente, unsigned int user_id) {
//creando logger y limpiando arreglos de entrada
	
	logger(user_id)<<"------------------------------\n";
	logger(user_id)<<"Server::handler_list - inicio. \n";
	
	cout<<"Inicio handler_list para user "<<user_id<<"\n";
	//recibiendo variables
	
	logger(user_id)<<"Server::handler_list - Leyendo largo de DIR. \n";
	
	cout<<"\tLeyendo largo de DIR.\n";
	
	int largo_dir = 0;
	unsigned int n_recv = 0;
	n_recv = read(_sock_cliente, &largo_dir, sizeof(largo_dir));
	if(n_recv < 1){
		logger(user_id)<<"Server::handler_list - Error en lectura de largo del archivo, Terminando. \n";
		
		cout<<"\tError en lectura de largo del archivo.\n";
		
		close(_sock_cliente);
		exit(-1);
	}
	cout<<"\tLeyendo Nombre de DIR.\n";
	char * dir_name = new char[largo_dir+1];
	n_recv = read(_sock_cliente, dir_name, largo_dir);
	if(n_recv<1){
		logger(user_id)<<"Server::handler_list - Error en lectura del nombre del archivo, Terminando. \n";
		cout<<"\tError en lectura del nombre del archivo.\n";
		close(_sock_cliente);
		exit(-1);
	}
	dir_name[largo_dir]='\0';
	
	logger(user_id)<<"Server::handler_list - Largo de DIR = ["<<largo_dir<<"] DIR = ["<<dir_name<<"] \n";
	
	cout<<"\tNombre Recibido = ["<<dir_name<<"], de largo del DIR = ["<<largo_dir<<"]\n";

	logger(user_id)<<"Server::handler_list - Construyendo directorio local.\n";
		
	cout<<"\tConstruyendo directorio local.\n";

	//creando directorio objetivo
	//datos/usr/$_dir_name, tamaños "./" = 2 , "datos/" = 6, "usr/" = 11 (max usr = 4500000000), $_dir_name = largo_dir;
	int real_dir_length = 2 + 6 + 11 + largo_dir+1;
	char * local_curr_dir = new char[real_dir_length]; //+1 porsia
	sprintf(local_curr_dir,"./datos/%d/%s", user_id, dir_name);
	dirList * dl = new dirList();

	//añadiendo seguridad para que no coloquen directorios ..
	for(int u = 0; u<real_dir_length-4;u++){
		if(local_curr_dir[u] == '/' && local_curr_dir[u+1] == '.' && local_curr_dir[u+2] == '.' && local_curr_dir[u+3] == '/'){
			local_curr_dir[u+1]= '/';
			local_curr_dir[u+2]= '/';		
			u+=2;
		}
	}
	if(local_curr_dir[strlen(local_curr_dir)-3] == '/' && local_curr_dir[strlen(local_curr_dir)-2] == '.' && local_curr_dir[strlen(local_curr_dir-1)] == '.'){
		local_curr_dir[strlen(local_curr_dir)-2]='/';
		local_curr_dir[strlen(local_curr_dir)-1]='/';
	}

	logger(user_id)<<"Server::handler_list - Listando archivos ["<<local_curr_dir<<"] y construyendo msg para envio.\n";
	
	cout<<"\tListando archivos ["<<local_curr_dir<<"] y construyendo msg para envio.\n";
	//listando los archivos actuales
	DIR *dir = NULL;
	dirent *pdir = NULL;
	// dir = opendir(local_curr_dir);	 // open current directory
	dir = opendir(local_curr_dir);
	//######## colcar la logica de leer el directorio en el objeto dirList
	if(dir == NULL){
		//no se pudo abrir el directorio
		logger(user_id)<<"Server::handler_recv - No se pudo abrir el directorio ["<<local_curr_dir<<"]\n";
		
		cerr<<"\tError No se pudo abrir el directorio ["<<local_curr_dir<<"]\n";
		
		unsigned int send_error = 0;
		if( !write(_sock_cliente, &send_error, sizeof(send_error) ) ){
			logger(user_id)<<"Server::handler_recv - Error en el envio de mensaje de error al cliente\n";
			cerr<<"\tError en envio de mensaje de error a ["<<user_id<<"]\n";
		}
		else{
			logger(user_id)<<"Server::handler_recv - Exito en el envio de mensaje de error al cliente\n";
			cerr<<"\tExito en envio de mensaje de error a["<<user_id<<"]\n";		
		}
		logger(user_id)<<"Server::handler_list - Fin.\n";
		delete dl;
		cout<<"Finalizando handler_list para user "<<user_id<<"\n";
		close(_sock_cliente);
	}
	else{
		while ((pdir = readdir(dir))) {
			struct stat st;
			char * full_local_path = new char[real_dir_length + strlen(pdir->d_name) + 1];
			sprintf(full_local_path,"%s/%s",local_curr_dir,pdir->d_name);
			lstat(full_local_path, &st);
			if(S_ISDIR(st.st_mode)){
				dl->add_dir(pdir->d_name);
			}
			else{
				dl->add_file(pair<string,int>(pdir->d_name,st.st_size));
			}
			delete[] full_local_path;
		}
		closedir(dir);


		char * msg;
		unsigned int largo_msg = dl->size();
		msg = new char[largo_msg+2];
		// char * msg;
		dl->serialize(msg);
		// cout<<"msg serializado = "<<msg<<"\n";

		cout<<"\tEnviando largo de msg ["<<largo_msg<<"]\n";
		logger(user_id)<<"Server::handler_list - Enviando largo de msg  ["<<largo_msg<<"]\n";
		if( !write(_sock_cliente, &largo_msg, sizeof(largo_msg) ) ){
			logger(user_id)<<"Server::handler_recv - Error en envio de largo de msg\n";
			cerr<<"\tError en envio de largo de msg a ["<<user_id<<"]\n";
		}
		else{
			logger(user_id)<<"Server::handler_recv - Exito en envio de largo de msg\n";
			cerr<<"\tExito en envio de largo de msg a["<<user_id<<"]\n";		
		}

		logger(user_id)<<"Server::handler_list - Enviando  msg de largo ["<<largo_msg<<"]\n";
		cout<<"\tEnviando  msg ["<<largo_msg<<"]\n";



		if(!write(_sock_cliente,msg, largo_msg+1) ){
			logger(user_id)<<"Server::handler_recv - Error en envio msg\n";
			cerr<<"\tError en envio msg a ["<<user_id<<"]\n";
		}
		else{
			logger(user_id)<<"Server::handler_recv - Exito en envio de msg\n";
			cerr<<"\tExito en envio de msg a ["<<user_id<<"]\n";		
		}

	 //	// cout<<"creando estructura de msg\n";
		// dirList * dl2 = new dirList(msg);
  //   	dl2->print(&cout);
  //   	delete dl2;
  //   	// cout<<"fin de imprimir estructura\n";

		delete [] msg;
		delete dl;
		close(_sock_cliente);
		
		logger(user_id)<<"Server::handler_list - Fin.\n";
		cout<<"Finalizando handler_list para user "<<user_id<<"\n";
	}
//fin manejador
}









int main(int argc, char* argv[]){
	
	if(argc != 3){
		cerr<<"uso: server [port] [reference_file]\n";
		exit(-1);
	}
	
	//leyendo variables
	int port = atoi(argv[1]);
	const char *reference_file = argv[2];
	
	//Para usar el logger, primero debo agregar al user 0 (de sistema)
	//Esto deberia hacerse internamente en la clase y de modo estatico
	ConcurrentLogger::addUserLock(0);
	
	logger()<<"------------------------------\n";
	logger()<<"Server::main - Inicializando\n";
	
	CheckUser* users = new CheckUser("db/Users");
	
	logger()<<"Server::main - Cargando referencia desde \""<<reference_file<<"\"\n";
	
	ReferenceIndex *reference = new ReferenceIndexBasic();
	reference->load(reference_file);
	
	//intentando enlazar puerto
	logger()<<"Server::main - Enlazando puerto "<<port<<"\n";
	int sock_servidor, sock_cliente;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	logger()<<"Server::main - Creando Socket\n";
	sock_servidor = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_servidor < 0){
		logger()<<"Server::main - Error al crear socket.\n";
		return 0;
	}
	memset((char*)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	logger()<<"Server::main - Enlazando.\n";
	if (bind(sock_servidor, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		logger()<<"Server::main - Error en bind.\n";
		return 0;
	}

	logger()<<"Server::main - Iniciando listen.\n";
	listen(sock_servidor, 100);
	clilen = sizeof(cli_addr);
	bool procesar = true;
	while(procesar){
		//conversacion con cliente
		logger()<<"Server::main - Esperando clientes.\n";
		
		sock_cliente = accept(sock_servidor, (struct sockaddr *)&cli_addr, &clilen);
		
		logger()<<"Server::main - Cliente aceptado (Sock "<<sock_cliente<<").\n";
		
		//Leer el id / request y lo verifico
		RequestID request;
		if( ! receiveId(sock_cliente, request) ){
			logger()<<"Server::main - Error al leer ID.\n";
			continue;
		}
		
		if( ! users->valid(request.user_id, request.md5) ){
			logger()<<"Server::main - User invalido ("<<request.user_id<<", "<<request.md5<<").\n";
			continue;
		}
		
		logger()<<"Server::main - user "<<request.user_id<<", request "<<(unsigned int)request.type<<"\n";
		ConcurrentLogger::addUserLock(request.user_id);
		switch(request.type){
			case 0:
				logger()<<"Server::main - Request vacio, ignorando.\n";
				break;
			case 1:
				logger()<<"Server::main - Creando handler_receive para user "<<request.user_id<<" en sock "<<sock_cliente<<".\n";
				thread(handler_receive, sock_cliente, request.user_id).detach();
				break;
			case 2:
				logger()<<"Server::main - Creando handler_send para user "<<request.user_id<<" en sock "<<sock_cliente<<".\n";
				thread(handler_send, sock_cliente, request.user_id).detach();
				break;
			case 3:
				logger()<<"Server::main - Creando handler_list para user "<<request.user_id<<" en sock "<<sock_cliente<<".\n";
				thread(handler_list, sock_cliente, request.user_id).detach();
				break;
			case 4:
				logger()<<"Server::main - Creando handler_compress para user "<<request.user_id<<" en sock "<<sock_cliente<<".\n";
//				thread(handler_compress, sock_cliente, request.user_id, reference, THREADS_PROCESO, BLOCK_SIZE).detach();
				break;
			case 5:
				logger()<<"Server::main - Creando handler_recompress para user "<<request.user_id<<" en sock "<<sock_cliente<<".\n";
				thread(handler_recompress, sock_cliente, request.user_id, reference, THREADS_PROCESO, BLOCK_SIZE).detach();
				break;
			case 100:
				logger()<<"Server::main - Cerrando Server.\n";
				close(sock_cliente);
				close(sock_servidor);
				procesar = false;
				break;
			default:
				logger()<<"Server::main - Handler NO definido para usuario "<<request.user_id<<", ignorando.\n";
				break;
		}
		
	}//while... true
	
	//Fin conversacion con cliente
	logger()<<"Server::main - Finzalizando.\n";
	
	delete users;
	delete reference;
	
}



