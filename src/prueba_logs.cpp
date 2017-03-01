#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <thread>
#include <mutex>

#include <map>
#include <vector>

#include "ConcurrentLogger.h"

using namespace std;

/*
class ConcurrentLogger{
private:

	//variables estaticas de la clase
	static constexpr const char *file_base = "logs/log_";
	static map<unsigned int, std::mutex*> mutex_map;
	static std::mutex global_mutex;
	
	//Variables de la instancia
	ofstream escritor;
	unsigned int user_id;
	
public:
	ConcurrentLogger(unsigned int _user_id = 0){
		user_id = _user_id;
		char file_name[128];
		if(user_id == 0){
			sprintf(file_name, "%sglobal.log", file_base);
		}
		else{
			sprintf(file_name, "%s%d.log", file_base, user_id);
		}
		mutex_map[user_id]->lock();
		escritor.open(file_name, ofstream::app);
		time_t now = time(0);
		tm *ltm = localtime(&now);
		escritor<<1900 + ltm->tm_year <<"/"<<1 + ltm->tm_mon<<"/"<<ltm->tm_mday <<" "<<1 + ltm->tm_hour << ":"<< + ltm->tm_min << ":"<<1 + ltm->tm_sec <<"\t";
//		if(user_id == 0){
//			cout<<1900 + ltm->tm_year <<"/"<<1 + ltm->tm_mon<<"/"<<ltm->tm_mday <<" "<<1 + ltm->tm_hour << ":"<< + ltm->tm_min << ":"<<1 + ltm->tm_sec <<"\t";
//		}
	}
	~ConcurrentLogger(){
		escritor.close();
		mutex_map[user_id]->unlock();
	}
	
	template<typename T>
	ConcurrentLogger& operator << (const T& object){
//		cout<<"ConcurrentLogger - ["<<object<<"]\n";
//		cout<<"("<<object<<")";
		if( !(escritor.is_open()) ){
			cerr<<"Fallo al abrir Log\n";
		}
		escritor<<object;
		if(user_id == 0){
			cout<<object;
		}
		return *this;
	}
	
	static void addUserLock(unsigned int user_id){
		global_mutex.lock();
		mutex_map.emplace(user_id, new std::mutex());
		global_mutex.unlock();
	}
	
	//Notar que este metodo solo debe usarse si se tiene la certeza de que nadie mas esta usando el user_id
	//Esto podria controlarse con un contador de uso, pero lo dejo simple por el momento
	//En la practica, es viable NO usar este metodo
	static void removeUserLock(unsigned int user_id){
		global_mutex.lock();
		mutex_map.erase(user_id);
		global_mutex.unlock();
	}
	
};

map<unsigned int, std::mutex*> ConcurrentLogger::mutex_map;
std::mutex ConcurrentLogger::global_mutex;
*/

void test_thread(unsigned int thread_id, unsigned int user_id){
	
	for(unsigned int i = 0; i < 10000; ++i){
		logger(user_id)<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<" "<<thread_id<<"\n";
	}
	
}

int main(int argc, char* argv[]){

	cout<<"Inicio\n";
	
	unsigned int n_threads = 8;
	unsigned int user_id = 999;
	
	ConcurrentLogger::addUserLock(user_id);
	
//	ConcurrentLogger(0)<<"1 "<<"2 "<<"3 \n";
	vector<std::thread> vector_threads;
	for(unsigned int i = 0; i < n_threads; ++i){
		vector_threads.push_back( std::thread(test_thread, i, user_id) );
	}
	for(unsigned int i = 0; i < n_threads; ++i){
		vector_threads[i].join();
	}
	vector_threads.clear();
	
	ConcurrentLogger::removeUserLock(user_id);
	
	cout<<"Fin\n";
	
}

















