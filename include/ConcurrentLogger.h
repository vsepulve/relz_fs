#ifndef _CONCURRENT_LOGGER_H
#define _CONCURRENT_LOGGER_H

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#include <mutex>
#include <map>

using namespace std;

//Defino "logger" como un alias de ConcurrentLogger
//Se usa de modo:
// - logger(userd_id)<<m1<<m2<<...<<mN;
// - Usa el user_id como llave de un mapa de mutex para control de concurrencia
// - Escribe en logs/log_N.log (N es el user_id, o "global" si user_id == 0)
// - Si user_id == 0, TAMBIEN escribe en cout (pero omite el tiempo por linea)
// - Todo user_id usado (incluso 0) DEBE SER AGREGADO manualmente via logger::addUserLock(user_id)

class ConcurrentLogger;
typedef ConcurrentLogger logger;

enum{
	color_black = 30,
	color_red = 31,
	color_green = 32,
	color_yellow = 33,
	color_blue = 34,
	color_magenta = 35,
	color_cyan = 36,
	color_white = 37
};

class CoutColor{
private:
	unsigned int color;
public:
	CoutColor(unsigned int _color){
		color = _color;
		init();
	}
	~CoutColor(){
		reset();
	}
	void reset(){
		cout<<"\033[0m";
	}
	void init(){
		cout<<"\033[1;"<<color<<"m";
	}
};

class ConcurrentLogger{
private:

	//variables estaticas de la clase
	static constexpr const char *file_base = "logs/log_";
	static map<unsigned int, mutex*> mutex_map;
	static mutex global_mutex;
	
	//Variables de la instancia
	ofstream escritor;
	unsigned int user_id;
	
public:
	ConcurrentLogger(unsigned int _user_id = 0);
	
	~ConcurrentLogger();
	
	template<typename T>
	ConcurrentLogger& operator << (const T& object){
		if( !(escritor.is_open()) ){
			cerr<<"Fallo al abrir Log\n";
		}
		else{
			escritor<<object;
			if(user_id == 0){
				cout<<object;
			}
		}
		return *this;
	}
	
	//Esta version es para parchar el problema de char[] en template
	//De otro modo (usando solo el anterior) habria que castear a (char*) en cada llamada
	ConcurrentLogger& operator << (const char *object){
		if( !(escritor.is_open()) ){
			cerr<<"Fallo al abrir Log\n";
		}
		else{
			escritor<<object;
			if(user_id == 0){
				cout<<object;
			}
		}
		return *this;
	}
	
	static void addUserLock(unsigned int user_id);
	
	//Notar que este metodo solo debe usarse si se tiene la certeza de que nadie mas esta usando el user_id
	//Esto podria controlarse con un contador de uso, pero lo dejo simple por el momento
	//En la practica, es viable NO usar este metodo
	static void removeUserLock(unsigned int user_id);
	
};


#endif //_CONCURRENT_LOGGER_H
