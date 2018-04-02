#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <iterator>
#include <sstream>
#include <fstream>
#include <string>

#include <map>
#include <vector>

#include "ReferenceIndex.h"
#include "ReferenceIndexBasic.h"

#include "NanoTimer.h"
#include "BitsUtils.h"

#include "PositionsCoderBlocks.h"
#include "LengthsCoderBlocks.h"
#include "BlockHeaders.h"

#include "CoderBlocks.h"
#include "DecoderBlocks.h"
#include "Communication.h"

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//Files
#include <string>
#include "dirList.h"

// NUEVOS TIPOS de request_type
// 0 : se guarda para caso de error o request vacio (quizas pueda usarse para algo tipo ping)
// 1 : envio directo de archivo (sin procesamiento)
// 2 : pedir archivo (sin procesamiento)
// 3 : listar archivos (es decir, pedir lista remota de archivos)
// 4 : envio de archivo para compresion en el server (envia descomprimido y el server comprime)
// 5 : envio de archivo con doble compresion (comprime localmente, envia y el server recomprime)

using namespace std;

//tamaño del bloque de compresion
unsigned int BLOCK_SIZE = 1000000;

//Tamaño (inicial) del buffer de comunicacion
unsigned int BUFFER_SIZE = 1000000;

int main(int argc, char* argv[]){

	if(argc != 5){
		cout<<"\nModo de Uso: shell_cliente [reference_file] [n_threads] [user_id] [port]\n";
		cout<<"Se inicia cargando una referencia preparada.\n";
		cout<<"Utiliza n_threads threads como maximo.\n";
		return 0;
	}
	
	const char *archivo_referencia = argv[1];
	unsigned int n_threads = atoi(argv[2]);
	unsigned int user_id = atoi(argv[3]);
	unsigned int port = atoi(argv[4]);
	
	NanoTimer timer;
	char *comm_buffer = new char[BUFFER_SIZE];
	
	const char *host_name = "localhost";
	int sockfd;

	cout<<"Iniciando Shell Cliente (referencia: \""<<archivo_referencia<<"\", max_threads: "<<n_threads<<", user_id: "<<user_id<<")\n";
	
	//local-compress (compress, lcompress) [entrada salida]
	
	//remote-compress (rcompress) [entrada salida]
	// - lee el archivo de texto "entrada", filtrando caracteres
	// - comprime el archivo localmente (en una version "salida.tmp")
	// - borra los bytes del texto de "entrada"
	// - vuelve a leer los bytes del archivo comprimido local ("tmp")
	// - envia esos bytes al server en trozos hasta terminar la comunicacion -exitosa-
	// - (opcional) para evaluar la correctitud? pordia usar el md5 de los bytes (calculado por ambos, enviado por el cliente para la copmprobacion)
	// - esperar respuesta del server
	// - el server por mientras, recibe el "tmp", lo descomprime, y lo recomprime en "salida"
	// - el server guarda una copia "salida"
	// - luego envia "salida" al cliente
	// - (opcional) de nuevo puede usarse md5 para comprobar correctitud
	// - (detalles) el cliente envia su id al server antes que nada.
	// - (detalles) notar que la version de la referencia esta EXPLICITAMENTE en el archivo.
	// - (detalles) se envian los largos en bytes de los archivos antes de escribirlos, siempre.
	
	//remote-store (store) [entrada salida]
	// - Simplemente envia el archivo para su almacenamiento
	// - No procesa en ningun momento el archivo
	// - Simplemente lo guarda con el nombre de salida en el server
	
	//store-compress (scompress) [entrada salida]
	// - Similar a rcompress, pero simplemente deja el resultado en el server
	// - Esta version NO retorna el archivo comprimido (solo lo deja en el server)
	
	//get [entrada salida]
	// - Pide el archivo al server (con nombre de entrada) y lo guarda localmente (en salida)
	
	//get-part [entrada pos largo salida]
	
	//list (ls) []
	
	//local-decompress (decompress) [entrada_comprimida]
	
	//update-reference (update)
	
	
	//cargar referencia inicial y otros datos necesarios
//	ReferenceIndex *referencia = new ReferenceIndexBasic();
	ReferenceIndex *referencia = new ReferenceIndexRR();
	referencia->load(archivo_referencia);
	
	CoderBlocks *compresor = new CoderBlocks(referencia);
	
	string term;
	vector<string> comandos;
	unsigned int max_line = 1024;
	char line[max_line];
	
	while(true){
		cout<<"shell > ";
		comandos.clear();
		
		//leer linea
		cin.getline(line, max_line);
		//iterator para tokenizar la linea
		//(notar que la variable temporal para stringstream ES NECESARIA por algun motivo absurdo)
		stringstream toks(line);
		istream_iterator<string> cin_it(toks);
		istream_iterator<string> eos;
		//tomo cada term
		while(cin_it != eos){
			comandos.push_back(*cin_it);
			cin_it++;
		}
		
//		//verificacion de resultados
//		cout<<"(echo, "<<comandos.size()<<" terms) ";
//		for(unsigned int i = 0; i < comandos.size(); ++i){
//			cout<<"\""<<comandos[i]<<"\" ";
//		}
//		cout<<"\n";
		
		//COMANDOS PERMITIDOS
		
		//compress (compresion local)
		//Este comando comprime un archivo_entrada en un archivo_salida
		if( comandos[0].compare("local-compress") == 0
		 || comandos[0].compare("lcompress") == 0 
		 || comandos[0].compare("compress") == 0 ){
			if(comandos.size() == 3){
				string nombre_entrada = comandos[1];
				string nombre_salida = comandos[2];
				cout<<"Coprimiendo \""<<nombre_entrada<<"\" en \""<<nombre_salida<<"\"\n";
				timer.reset();
				compresor->compress(nombre_entrada.c_str(), nombre_salida.c_str(), n_threads, BLOCK_SIZE);
				cout<<"[TIME]\t- Compresion terminada en "<<timer.getMilisec()<<" ms\n";
			}
			else{
				cout<<"Modo de uso:\n";
				cout<<"Shell > compress archivo_entrada archivo_salida\n";
			}
		}//local-compress
		
		if( comandos[0].compare("remote-store") == 0
		 || comandos[0].compare("rstore") == 0 
		 || comandos[0].compare("store") == 0 ){
			if(comandos.size() == 3){
				//Simplemente envia el archivo para almacenamiento remoto
				//No procesa nada
				string nombre_entrada = comandos[1];
				string nombre_salida = comandos[2];
				timer.reset();
				
				// - conectar
				// - enviar peticion 1
				// - enviar archivo
				// - terminar
				
				//conectar
				if( (sockfd = connectServer(host_name, port)) < 0){
					cerr<<"\tERROR al conectar.\n";
					continue;
				}
				
				//enviar request / ID
				if( ! sendId(sockfd, 1, user_id) ){
					cerr<<"\tERROR al enviar ID.\n";
					continue;
				}
				
				//enviar archivo
				if( ! sendFile(sockfd, nombre_entrada.c_str(), nombre_salida.c_str(), comm_buffer, BUFFER_SIZE) ){
					cerr<<"\tERROR al enviar archivo.\n";
					continue;
				}
				
				cout<<"\tEnvio terminado, esperando confirmacion del server...\n";
				
				//Verificacion
				char res = 0;
				if( (read(sockfd, &res, 1) < 0) || (res == 0) ){
					cerr<<"\tERROR al leer respuesta del server.\n";
				}
				close(sockfd);
				cout<<"[TIME]\tEnvio terminado en "<<timer.getMilisec()<<" ms\n";
				
			}//if... parametros correctos
			else{
				cout<<"Modo de uso:\n";
				cout<<"Shell > "<<comandos[0]<<" archivo_entrada archivo_salida\n";
			}
		}//remote-store
		
		if( comandos[0].compare("store-compress") == 0
		 || comandos[0].compare("scompress") == 0 ){
			if(comandos.size() == 3){
				string nombre_entrada = comandos[1];
				string nombre_salida = comandos[2];
				
				//La logica deberia ser algo como:
				// - Compresion local (tmp)
				// - conectar
				// - enviar id de la peticion 5
				// - enviar archivo tmp
				// - desconectar
				
				//Notar que usa EL MISMO request que rcompress
				//La diferencia que es que NO se reconecta para pedir el resultado
				
				timer.reset();
				
				//Compresion local (en tmp)
				char nombre_entrada_tmp[128];
				sprintf(nombre_entrada_tmp, "%s.relz.tmp", nombre_entrada.c_str() );
				cout<<"Coprimiendo \""<<nombre_entrada<<"\" en \""<<nombre_entrada_tmp<<"\"\n";
				compresor->compress(nombre_entrada.c_str(), nombre_entrada_tmp, n_threads, BLOCK_SIZE);
				cout<<"[TIME]\t- Compresion terminada en "<<timer.getMilisec()<<" ms\n";
				
				//conectar
				if( (sockfd = connectServer(host_name, port)) < 0){
					cerr<<"\tERROR al conectar.\n";
					continue;
				}
				
				//enviar request / ID 5
				if( ! sendId(sockfd, 5, user_id) ){
					cerr<<"\tERROR al enviar ID.\n";
					continue;
				}
				
				//enviar archivo
				if( ! sendFile(sockfd, nombre_entrada_tmp, nombre_salida.c_str(), comm_buffer, BUFFER_SIZE) ){
					cerr<<"\tERROR al enviar archivo.\n";
					continue;
				}
				
				cout<<"\tEnvio terminado, esperando recompresion del server...\n";
				
				//Verificacion
				char res = 0;
				if( (read(sockfd, &res, 1) < 0) || (res == 0) ){
					cerr<<"\tERROR al leer respuesta del server.\n";
				}
				
				//borrar tmp
				remove(nombre_entrada_tmp);
				
				//terminar
				close(sockfd);
				cout<<"[TIME]\tCompresion Remota terminada en "<<timer.getMilisec()<<" ms\n";
				
			}//if... parametros correctos
			else{
				cout<<"Modo de uso:\n";
				cout<<"Shell > "<<comandos[0]<<" archivo_entrada archivo_salida\n";
			}
		}//store-compress
		
		if( comandos[0].compare("remote-compress") == 0
		 || comandos[0].compare("rcompress") == 0 ){
			if(comandos.size() == 3){
				string nombre_entrada = comandos[1];
				string nombre_salida = comandos[2];
				
				//La logica deberia ser algo como:
				// - Compresion local (tmp)
				// - conectar
				// - enviar id de la peticion 5
				// - enviar archivo tmp
				// - desconectar
				// - re-conectar
				// - enviar id de la peticion 2
				// - recibir/escribir archivo final
				// - desconectar
				
				timer.reset();
				
				//Compresion local (en tmp)
				char nombre_entrada_tmp[128];
				sprintf(nombre_entrada_tmp, "%s.relz.tmp", nombre_entrada.c_str() );
				cout<<"Coprimiendo \""<<nombre_entrada<<"\" en \""<<nombre_entrada_tmp<<"\"\n";
				NanoTimer timer_local;
				compresor->compress(nombre_entrada.c_str(), nombre_entrada_tmp, n_threads, BLOCK_SIZE);
				cout<<"[TIME]\t- Compresion terminada en "<<timer_local.getMilisec()<<" ms\n";
				
				//conectar
				if( (sockfd = connectServer(host_name, port)) < 0){
					cerr<<"\tERROR al conectar.\n";
					continue;
				}
				
				//enviar request / ID 5
				if( ! sendId(sockfd, 5, user_id) ){
					cerr<<"\tERROR al enviar ID.\n";
					continue;
				}
				
				//enviar archivo
				if( ! sendFile(sockfd, nombre_entrada_tmp, nombre_salida.c_str(), comm_buffer, BUFFER_SIZE) ){
					cerr<<"\tERROR al enviar archivo.\n";
					continue;
				}
				
				cout<<"\tEnvio terminado, esperando recompresion del server...\n";
				
				//Verificacion
				char res = 0;
				if( (read(sockfd, &res, 1) < 0) || (res == 0) ){
					cerr<<"\tERROR al leer respuesta del server.\n";
				}
				close(sockfd);
				
				//borrar tmp
				remove(nombre_entrada_tmp);
				
				//conectar
				if( (sockfd = connectServer(host_name, port)) < 0){
					cerr<<"\tERROR al conectar.\n";
					continue;
				}
				
				//enviar request / ID 2
				if( ! sendId(sockfd, 2, user_id) ){
					cerr<<"\tERROR al enviar ID.\n";
					continue;
				}
				
				cout<<"\tPidiendo archivo \""<<nombre_salida<<"\" al server.\n";
				
				//enviar nombre de archivo deseado
				//Notar que pide el archivo nombre_salida (en el server, para dejarlo localmente con el mismo nombre)
				if( ! sendString(sockfd, nombre_salida.c_str()) ){
					cerr<<"\tERROR al enviar nombre de archivo.\n";
					continue;
				}
				
				//recibir / escribir archivo
				if( ! receiveFileWrite(sockfd, NULL, comm_buffer, BUFFER_SIZE, NULL) ){
					cerr<<"\tERROR al recibir archivo.\n";
					continue;
				}
				
				//terminar
				close(sockfd);
				cout<<"[TIME]\tCompresion Remota y Recepcion terminada en "<<timer.getMilisec()<<" ms\n";
				
			}//if... parametros correctos
			else{
				cout<<"Modo de uso:\n";
				cout<<"Shell > "<<comandos[0]<<" archivo_entrada archivo_salida\n";
			}
		}//remote-compress

		if( comandos[0].compare("remote-get") == 0
		 || comandos[0].compare("rget") == 0 
		 || comandos[0].compare("get") == 0 ){
			if(comandos.size() == 2){
				string nombre_archivo = comandos[1];
				timer.reset();
				
				//conectar
				if( (sockfd = connectServer(host_name, port)) < 0){
					cerr<<"\tERROR al conectar.\n";
					continue;
				}
				
				//enviar request / ID 2
				if( ! sendId(sockfd, 2, user_id) ){
					cerr<<"\tERROR al enviar ID.\n";
					continue;
				}
				
				cout<<"\tPidiendo archivo \""<<nombre_archivo<<"\" al server.\n";
				
				//enviar nombre de archivo deseado
				if( ! sendString(sockfd, nombre_archivo.c_str()) ){
					cerr<<"\tERROR al enviar nombre de archivo.\n";
					continue;
				}
				
				//recibir / escribir archivo
				//De momento SE OMITE el nombre de salida
				//El metodo siguiente tendria que poder renombrarlo en la salida
				if( ! receiveFileWrite(sockfd, NULL, comm_buffer, BUFFER_SIZE, NULL) ){
					cerr<<"\tERROR al recibir archivo.\n";
					continue;
				}
				
				//terminar
				close(sockfd);
				cout<<"[TIME]\tRecepcion terminada en "<<timer.getMilisec()<<" ms\n";
			}
			else{
				cout<<"Modo de uso:\n";
				cout<<"Shell > "<<comandos[0]<<" nombre_archivo\n";
			}
		}//remote-get
		
		if( comandos[0].compare("local-decompress") == 0 
		 || comandos[0].compare("decompress") == 0 ){
			if(comandos.size() == 3){
				string nombre_entrada = comandos[1];
				string nombre_salida = comandos[2];
				timer.reset();
				
				cout<<"Preparando Descompresor (desde \""<<nombre_entrada<<"\")\n";
				DecoderBlocks descompresor(nombre_entrada.c_str(), referencia->getText());
				
				cout<<"Descomprimiendo (en \""<<nombre_salida<<"\")\n";
				descompresor.decodeFullText(nombre_salida.c_str());
	
				cout<<"[TIME]\tDescompresion terminada en "<<timer.getMilisec()<<" ms\n";
			}
			else{
				cout<<"Modo de uso:\n";
				cout<<"Shell > "<<comandos[0]<<" archivo_entrada archivo_salida\n";
			}
			
		}//decompress
		
		if( comandos[0].compare("remote-list") == 0
		 || comandos[0].compare("rlist") == 0 
		 || comandos[0].compare("rls") == 0 ){
			if(comandos.size() == 1 || comandos.size() == 2){
				string dir_name;
				if(comandos.size() == 1){
					dir_name = ".";
				}
				else{
					dir_name = comandos[1];
				}
				
				//conectar
				if( (sockfd = connectServer(host_name, port)) < 0){
					cerr<<"\tERROR al conectar.\n";
					continue;
				}
				
				//enviar request / ID 2
				if( ! sendId(sockfd, 3, user_id) ){
					cerr<<"\tERROR al enviar ID.\n";
					continue;
				}
				
				cout<<"\tPidiendo lista de archivos de \""<<dir_name<<"\" al server.\n";
				
				//enviar directorio
				if( ! sendString(sockfd, dir_name.c_str()) ){
					cerr<<"\tERROR al enviar nombre de archivo.\n";
					continue;
				}
				
				//recibir lista serializada en c-string
				//Notar que esto podria ser parte de un DirectoryList::receive(sockfd)
				unsigned int largo_lista = 0;
				char *lista = NULL;
				if( (lista = receiveString(sockfd, largo_lista)) == NULL ){
					cerr<<"\tERROR al recibir lista de archivos.\n";
					continue;
				}
				
				cout<<"\t- --------------------------------\n";
				if(largo_lista > 0){
					dirList dl(lista);
					dl.print( &cout );
				}
				else{
					cout<<"\tDirectorio \""<<dir_name<<"\" no encontrado.\n";
				}
				cout<<"\t- --------------------------------\n";
				
				delete [] lista;
				close(sockfd);
				
			}
			else{
				cout<<"Modo de uso:\n";
				cout<<"Shell > "<<comandos[0]<<" [ruta_remota]\n";
			}
		}//remote-list
		
		
		
		
		
		
		//help
		if(comandos[0].compare("h") == 0 || comandos[0].compare("help") == 0){
			cout<<"\n";
			cout<<"<Comandos>\n";
			
			cout<<"  local-compress [entrada] [salida]\n";
			cout<<"    - Comprime localmente (alias lcompress).\n";
			
			cout<<"  local-decompress [entrada] [salida]\n";
			cout<<"    - Descomprime localmente (alias decompress).\n";
			
			cout<<"  remote-compress [entrada] [salida]\n";
			cout<<"    - Comprime remotamente y recibe el resultado (alias rcompress).\n";
			
			cout<<"  remote-store [entrada] [salida]\n";
			cout<<"    - Envia entrada para almacenado remoto (alias store).\n";
			
			cout<<"  remote-get [nombre_archivo]\n";
			cout<<"    - Copia localmente el archivo del server. (alias get).\n";
			
			cout<<"  remote-list\n";
			cout<<"    - Lista archivos del directorio remoto (alias rls).\n";
			
			cout<<"  kill\n";
			cout<<"    - Envia señal de cerrado al servidor y cierra el cliente.\n";
			
			cout<<"  quit\n";
			cout<<"    - Cierra el cliente.\n";
			
			cout<<"  help\n";
			cout<<"    - Muestra esta ayuda.\n";
			
			cout<<"\n";
		}//help
		
		//kill (comando de cerrar server y salida)
		//Este comando SOLO existe en el prototipo
		if(comandos[0].compare("kill") == 0){
			
			//conectar
			if( (sockfd = connectServer(host_name, port)) < 0){
				cerr<<"\tERROR al conectar.\n";
				continue;
			}
			
			cout<<"\tEnviando señal de salida al server\n";
				
			//enviar request / ID
			if( ! sendId(sockfd, 100, user_id) ){
				cerr<<"\tERROR al enviar ID.\n";
				continue;
			}
			
			close(sockfd);
			
			cout<<"saliendo...\n";
			break;
		}//kill
		
		//quit / q (comando de salida)
		if(comandos[0].compare("q") == 0 || comandos[0].compare("quit") == 0){
			cout<<"saliendo...\n";
			break;
		}//quit
		
	}//while... true
	
	cout<<"Borrando...\n";
	
	if(referencia != NULL){
		delete referencia;
	}
	
}

