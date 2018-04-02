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

#include "ReferenceIndexBasic.h"
#include "NanoTimer.h"

using namespace std;

int main(int argc, char* argv[]){
	
	if(argc != 5){
		cout<<"\nModo de Uso: reducir_ref referencia_serializada salida_texto min_length start_position\n";
		return 0;
	}
	const char *referencia_serializada = argv[1];
	const char *salida_texto = argv[2];
	unsigned int min_length = atoi(argv[3]);
	unsigned int start_pos = atoi(argv[4]);
	
	cout<<"Inicio (min_length: "<<min_length<<", desde \""<<referencia_serializada<<"\" en salida \""<<salida_texto<<"\")\n";
	
//	ReferenceIndexBasic referencia("abracadabracadabrabran", 1);
	ReferenceIndexBasic referencia;
	referencia.load(referencia_serializada);
	
	unsigned int largo = referencia.getLength();
	const char *texto_ref = referencia.getText();
	
	bool *arr_valid = new bool[largo + 1];
	memset( (char*)arr_valid, 0, largo + 1 );
	
	//preparacion de busqueda inicial
	cout<<"Preparando busqueda inicial\n";
	for(unsigned int i = 0; i < start_pos; ++i){
		arr_valid[i] = true;
	}
	
	//busquedas
	cout<<"Iniciando Busquedas\n";
	unsigned int pos, len;
	unsigned int cur_pos = start_pos;
	vector< pair<unsigned int, unsigned int> > factores;
	
	//mensaje de progreso
	unsigned int pro_total = 10000;
	unsigned int pro_parte = largo / pro_total;
	unsigned int pro_cur = pro_parte;
	NanoTimer timer;
	
	while(cur_pos < largo){
		if(cur_pos > pro_cur){
//			cout<<"> "<<(100*cur_pos/largo)<<" \%\n";
			cout<<"> pos "<<cur_pos<<" / "<<largo<<" ("<<round(timer.getMilisec()/(1000*60))<<" min)\n";
			pro_cur += pro_parte;
		}
		pos = 0;
		len = 0;
		referencia.find( texto_ref + cur_pos, largo - cur_pos, pos, len, min_length, arr_valid );
		if( len > 0 ){
			cout<<"factor: "<<pos<<", "<<len<<"\n";
			//Notar que el for no es necesario, pero lo dejo por claridad
			for(unsigned int i = 0; i < len; ++i){
				arr_valid[cur_pos + i] = false;
			}
			//Notar que NO nos importa pos del factor, sino de la pregunta (cur_pos)
			factores.push_back(pair<unsigned int, unsigned int>(cur_pos, len));
			cur_pos += len;
		}
		else{
			arr_valid[cur_pos] = true;
			++cur_pos;
		}
	}
	
	//revisar factores al escribir el resultado
	cout<<"Factores Resulantes: "<<factores.size()<<"\n";
	for(unsigned int i = 0; i < factores.size(); ++i){
		cout<<"<"<<factores[i].first<<", "<<factores[i].second<<">\n";
	}
	
	cur_pos = 0;
	unsigned int cur_factor = 0;
//	cout<<"Texto Ini \""<<texto_ref<<"\"\n";
//	cout<<"Texto Fin \"";
	
	fstream escritor(salida_texto, fstream::trunc | fstream::out);
	unsigned int line_length = 100;
	//+2 para un '\n' y un potencial 0 final
	char *line = new char[line_length + 2];
	unsigned int line_pos = 0;
	
	while( cur_pos < largo ){
		if( cur_factor < factores.size() && factores[cur_factor].first == cur_pos ){
			//factor encontrado, omitirlo
//			cout<<"[]";
			cur_pos += factores[cur_factor].second;
			++cur_factor;
		}
		else{
			//Agregar char a un buffer, escribir el buffer si ya esta lleno
//			cout<< texto_ref[cur_pos] <<"";
			line[line_pos++] = texto_ref[cur_pos];
			if( line_pos == line_length ){
				line[line_pos] = '\n';
				escritor.write(line, line_pos + 1);
				line_pos = 0;
			}
			++cur_pos;
		}
	}
//	cout<<"\"\n";
	//Escritura final del buffer restante
	if( line_pos > 0 ){
		line[line_pos] = '\n';
		escritor.write(line, line_pos + 1);
	}
	escritor.close();
	delete [] line;
	
	cout<<"Borrando\n";
	delete [] arr_valid;
	
	cout<<"Fin\n";
	
}

















