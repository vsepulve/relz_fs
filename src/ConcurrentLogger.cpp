#include "ConcurrentLogger.h"

map<unsigned int, std::mutex*> ConcurrentLogger::mutex_map;
std::mutex ConcurrentLogger::global_mutex;

ConcurrentLogger::ConcurrentLogger(unsigned int _user_id){
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
	escritor << 1900 + ltm->tm_year << "/" << 
		((1 + ltm->tm_mon < 10)?"0":"") << 1 + ltm->tm_mon << "/" << 
		((1 + ltm->tm_mday < 10)?"0":"") << 1 + ltm->tm_mday << " " << 
		((1 + ltm->tm_hour < 10)?"0":"") << 1 + ltm->tm_hour << ":" << 
		((1 + ltm->tm_min < 10)?"0":"") << 1 + ltm->tm_min << ":" << 
		((1 + ltm->tm_sec < 10)?"0":"") << 1 + ltm->tm_sec << "\t";
}

ConcurrentLogger::~ConcurrentLogger(){
	escritor.close();
	mutex_map[user_id]->unlock();
}

//template<typename T>
//ConcurrentLogger& operator << (const T& object){
//	if( !(escritor.is_open()) ){
//		cerr<<"Fallo al abrir Log\n";
//	}
//	escritor<<object;
//	if(user_id == 0){
//		cout<<object;
//	}
//	return *this;
//}

void ConcurrentLogger::addUserLock(unsigned int user_id){
	lock_guard<mutex> lock(global_mutex);
	mutex_map.emplace(user_id, new std::mutex());
}

//Notar que este metodo solo debe usarse si se tiene la certeza de que nadie mas esta usando el user_id
//Esto podria controlarse con un contador de uso, pero lo dejo simple por el momento
//En la practica, es viable NO usar este metodo
void ConcurrentLogger::removeUserLock(unsigned int user_id){
	lock_guard<mutex> lock(global_mutex);
	map<unsigned int, mutex*>::iterator it = mutex_map.find(user_id);
	if(it != mutex_map.end()){
		if(it->second != NULL){
			delete it->second;
		}
		mutex_map.erase(it);
	}
}








