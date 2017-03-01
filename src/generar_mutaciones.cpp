
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

using namespace std;

bool valid_char(char c){
	if( c == 'A' || c == 'T' || c == 'C' || c == 'G' ){
		return true;
	}
	else{
		return false;
	}
}

int main(int argc, char *argv[]){
	
	if(argc != 4){
		cout<<"\nModo de Uso: generar_mutaciones entrada salida prob\n";
		cout<<"Agrega mutaciones aleatorias con probabilidad \"prob\" a los datos de \"entrada\".\n";
		cout<<"\n";
		return 0;
	}
	
	const char *archivo_entrada = argv[1];
	const char *archivo_salida = argv[2];
	float prob = atof(argv[3]);
	
	if( prob <= 0 || prob > 1.0 ){
		cout<<"Probabilidad "<<prob<<" inválida, debe estar en rango (0, 1].\n";
		return 0;
	}
	
	cout<<"Inicio (de \""<<archivo_entrada<<"\", en \""<<archivo_salida<<"\", con probabilidad "<<prob<<")\n";
	
	fstream lector(archivo_entrada, fstream::in);
	fstream escritor(archivo_salida, fstream::out | fstream::trunc);
	
	if( !lector.good() || !escritor.good() ){
		cerr<<"Problemas al abrir archivos\n";
		return 1;
	}
	
	lector.seekg(0, lector.end);
	unsigned long long text_size = lector.tellg();
	lector.seekg(0, lector.beg);
	
	char *text = new char[text_size + 1];
	unsigned int block_size = 1024*1024;
	
	cout<<"Cargando "<<text_size<<" chars\n";
	
	unsigned long long total_leido = 0;
	unsigned int lectura = 0;
	while( total_leido < text_size ){
		lector.read( text + total_leido, block_size );
		lectura = lector.gcount();
		if( lectura == 0 ){
			break;
		}
		total_leido += lectura;
	}
	lector.close();
	text[total_leido] = 0;
	text_size = total_leido;
	
	cout<<"Chars cargados: "<<text_size<<"\n";
	
	char mutacion[0xff];
	for(unsigned int i = 0; i < 0xff; ++i){
		mutacion[i] = i;
	}
	mutacion['A'] = 'T';
	mutacion['T'] = 'A';
	mutacion['C'] = 'G';
	mutacion['G'] = 'C';
	
	unsigned int mod_dist = (unsigned int)( (2.0 / prob) - 1.0 );
	if( mod_dist < 1 ){
		mod_dist = 1;
	}
	cout<<"mod_dist: "<<mod_dist<<"\n";
	
	unsigned int total_mutado = 0;
	// la proxima posicion a mutar (notar que puede ser ll)
	unsigned long long next = 0;
	char c = 0;
	
	while( true ){
		
		// Version 2: Calculando distancia rand
		next += 1 + (rand() % mod_dist);
		while( next < text_size && !valid_char( c = text[next]) ){
			++next;
		}
		if( next < text_size ){
//			cout<<"mutacion["<<total_mutado<<"]: char "<<c<<" en pos "<<next<<"\n";
			text[next] = mutacion[(unsigned char)c];
			++total_mutado;
		}
		else{
			break;
		}
		
	}
	
	unsigned long long total_escrito = 0;
	while( escritor.good() && (total_escrito < text_size) ){
		if( total_escrito +  block_size > text_size ){
			block_size = text_size - total_escrito;
		}
		escritor.write( text + total_escrito, block_size );
		// Aqui estoy asumiendo que la escritura siempre es exitosa
		total_escrito += block_size;
	}
	escritor.close();
	
	cout<<"Proceso terminado (leido: "<<text_size<<", mutado: "<<total_mutado<<" / "<<((long double)total_mutado/text_size)<<")\n";
	
	delete [] text;
	
	cout<<"Fin\n";
	
	return 0;
}













