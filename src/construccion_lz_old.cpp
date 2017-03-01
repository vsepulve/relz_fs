#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <iostream>

#include <map>

#include "ReferenceIndexBasic.h"
#include "NanoTimer.h"

using namespace std;

static const unsigned int word = 32;
static const unsigned int bits_word = 5;
static const unsigned int mask_word = 0x1f;


//Escribe los "largo_num" bits de "num" en la "salida", desde su posicion "pos_salida"
//Notar que luego de cada escritura, el llamador debe sumar los "largo_num" bits escritos a su "pos_salida" local
// - "salida" es el puntero a la salida para escribir
// - "pos_salida" es la posicion en bits en la salida para escribir
// - "largo_num" es el largo de la escritura en bits
// - "num" es el numero que se desea escribir
void bitput(unsigned int *salida, unsigned int pos_salida, unsigned int largo_num, unsigned int num){
//	cout<<" -> bitput - "<<largo_num<<" bits de "<<num<<" en posicion "<<pos_salida<<"\n";
	//salida = salida + pos_salida/32, moverse al int donde empezar la escritura
	salida += pos_salida >> bits_word;
	//con esto pos_salida marca el bit dentro del int donde empieza la escritura
//	pos_salida &= (1<<bits_word) - 1;
	pos_salida &= mask_word;
	if (largo_num == word) {
		*salida |= (*salida & ((1<<pos_salida) - 1)) | (num << pos_salida);
		if (!pos_salida){
			return;
		}
		salida++;
		*salida = (*salida & ~((1<<pos_salida) - 1)) | (num >> (word - pos_salida));
	}
	else {
		if (pos_salida + largo_num <= word) {
			*salida = (*salida & ~( ((1<<largo_num) - 1) << pos_salida) ) | (num << pos_salida);
			return;
		}
		*salida = (*salida & ((1<<pos_salida) - 1)) | (num << pos_salida);
		salida++;
		largo_num -= word - pos_salida;
		*salida = (*salida & ~((1<<largo_num)-1)) | (num >> (word - pos_salida));
	}
}

//Escribe el numero "num" en la posicion "pos_escritura" (en bits) de la "salida"
//Retorna el numero de bits usados (para ajustar pos_escritura)
unsigned int write_golomb(unsigned int *salida, unsigned int pos_escritura, unsigned int num){
	
	unsigned int base = 64;
	unsigned int bits_base = 6;
	unsigned int mask_base = 0x3f;
	
	unsigned int pos_salida = pos_escritura;
//	unsigned int q = num / base;
//	unsigned int resto = num % base;
	unsigned int q = (num >> bits_base);
	unsigned int resto = (num & mask_base);
	
//	cout<<"write_golomb - numero "<<num<<", q: "<<q<<", resto: "<<resto<<" (base "<<base<<", "<<bits_base<<" bits para el resto)\n";
	
	//Creo que no se puede con una mascara, hay que escribir los 1's en un tipo de ciclo
	while( q > 31){
		//escribir 32 1's
		bitput(salida, pos_salida, 32, 0xffffffff);
		pos_salida += 32;
		q -= 32;
//		cout<<"write_golomb - pos_salida: "<<pos_salida<<"\n";
	}
	
	//Notar que esta mascara debe ser construida de derecha a izquiera
	//Esto es porque la lectura en cada entro se realiza en ese orden
	//por lo que el 0 debe estar al extremo izquierdo
//	unsigned int mascara_q = (((1<<q) - 1) << 1) & 0xfffffffe;
	unsigned int mascara_q = ((1<<q) - 1) ;
	//escribir la mascara (notar que se escriben q+1 bits en total)
	bitput(salida, pos_salida, q+1, mascara_q);
	pos_salida += (q+1);
//	cout<<"write_golomb - pos_salida: "<<pos_salida<<"\n";
	
	//escribir el resto
	bitput(salida, pos_salida, bits_base, resto);
	pos_salida += bits_base;
//	cout<<"write_golomb - pos_salida: "<<pos_salida<<"\n";
	
	return (pos_salida - pos_escritura);
}

unsigned int floor_bits(unsigned int n){
	unsigned int ret = 0;
	while( n >>= 1 ){
		++ret;
	}
	return ret;
}

unsigned int bits_real(unsigned int n){
	unsigned int ret = 1;
	while( n >>= 1 ){
		++ret;
	}
	return ret;
}

unsigned int calcular_bits(unsigned int largo_prefijo, unsigned int opcion){
	if(opcion == 0){
		//Elias-gamma
		return 1 + 2 * floor_bits(largo_prefijo);
	}
	else if(opcion == 1){
		//Elias-delta
		//Desactivado por ahora (al ser numeros mas bien pequeños, no deberia servir)
	}
	else if(opcion == 2){
		//Golomb 64
		unsigned int b = 64;
		unsigned int bits_b = 6;
		unsigned int q = (largo_prefijo/b) + 1;
		return q + bits_b;
	}
	else if(opcion == 3){
		//Golomb 32
		unsigned int b = 32;
		unsigned int bits_b = 5;
		unsigned int q = (largo_prefijo/b) + 1;
		return q + bits_b;
	}
	else if(opcion == 4){
		//Golomb 16
		unsigned int b = 16;
		unsigned int bits_b = 4;
		unsigned int q = (largo_prefijo/b) + 1;
		return q + bits_b;
	}
	else if(opcion == 5){
		//vByte
		unsigned int bits = bits_real(largo_prefijo);
		if(bits <= 7){
			return 8;
		}
		else if(bits <= 14){
			return 16;
		}
		else if(bits <= 21){
			return 24;
		}
		else if(bits <= 28){
			return 32;
		}
		else{
			return 40;
		}
	}
	
	return 0;
}


int main(int argc, char* argv[]){

	if(argc != 4){
		cout<<"\nModo de Uso: construccion_lz entrada_referencia entrada_comprimir nombre_salida\n";
		cout<<"Crea archivos \"nombre_salida.pos.bin\", \"nombre_salida.fac.bin\" y \"nombre_salida.bit.bin\"\n";
		cout<<"Tambien creara un archivo temporal que borra al final \"nombre_salida.tmp\"\n";
		cout<<"\n";
		return 0;
	}
	
	const char *entrada_referencia = argv[1];
	const char *entrada_comprimir = argv[2];
	const char *nombre_salida = argv[3];
	
	cout<<"Inicio (referencia "<<entrada_referencia<<", procesar "<<entrada_comprimir<<", salida "<<nombre_salida<<")\n";
	
	cout<<"Cargando string\n";
	NanoTimer timer;
	
	unsigned int max_linea = 1024;
	char *linea = new char[max_linea];
	unsigned int pos = 0;
	unsigned int contador = 0;
	
	fstream lector(entrada_referencia, fstream::in);
	lector.seekg (0, lector.end);
	unsigned int largo_text = lector.tellg();
	lector.seekg (0, lector.beg);
	
	char *text = new char[largo_text+1];
	
	while(true){
		lector.getline(linea, max_linea);
		if(!lector.good() || strlen(linea) < 1){
			break;
		}
		
		if(++contador % 1000000 == 0){
			cout<<"Linea "<<contador<<" ("<<strlen(linea)<<" chars)\n";
		}
		
		//En esta prueba voy a FILTRAR lo que no sea A, T, C o G
		for(unsigned int i = 0; i < strlen(linea); ++i){
			if(linea[i] == 'A' || linea[i] == 'T' || linea[i] == 'C' || linea[i] == 'G'){
				text[pos] = linea[i];
				++pos;
			}
		}
		
	}
	lector.close();
	text[pos] = 0;
	cout<<"Caracteres cargados: "<<pos<<" en "<<timer.getMilisec()<<" ms\n";
	
	cout<<"Construyendo Referencia\n";
	timer.reset();
	ReferenceIndexBasic *referencia = new ReferenceIndexBasic(text);
	double milisec = timer.getMilisec();
	cout<<"construido en "<<milisec<<" ms\n";
	
	delete [] text;
	
	cout<<"Cargando Texto a comprimir\n";
	
	timer.reset();
	lector.open(entrada_comprimir, fstream::in);
	lector.seekg (0, lector.end);
	largo_text = lector.tellg();
	lector.seekg (0, lector.beg);
	
	text = new char[largo_text+1];
	pos = 0;
	while(true){
		lector.getline(linea, max_linea);
		if(!lector.good() || strlen(linea) < 1){
			break;
		}
		
		if(++contador % 1000000 == 0){
			cout<<"Linea "<<contador<<" ("<<strlen(linea)<<" chars)\n";
		}
		
		//En esta prueba voy a FILTRAR lo que no sea A, T, C o G
		for(unsigned int i = 0; i < strlen(linea); ++i){
			if(linea[i] == 'A' || linea[i] == 'T' || linea[i] == 'C' || linea[i] == 'G'){
				text[pos] = linea[i];
				++pos;
			}
		}
			
	}
	text[pos] = 0;
	unsigned int largo_real = pos;
	cout<<"Caracteres cargados: "<<pos<<" en "<<timer.getMilisec()<<" ms\n";
	
	cout<<"Comprimiendo Texto\n";
	timer.reset();
	
	pos = 0;
	
	unsigned int pos_prefijo, largo_prefijo;
	
	map<unsigned int, unsigned int> histograma;
	unsigned int size_bucket = 10000;
	
	unsigned int max_pos = 0;
	unsigned int max_largo = 0;
	unsigned int n_factores = 0;
	
	//La idea es guardar en un archivo independiente las posiciones (despues se vuelcan a otro archivo)
	//En el archivo primario tiene los factores (sin posicion) y lso textos del corte
	//Quizas se pueda guardar a parte el arreglo de bits por factores (en un vector? o en un buff de un cierto tamaño fijo)
	
	//archivo de bit de marca
	char *archivo_salida_bit = new char[1024];
	sprintf(archivo_salida_bit, "%s.bit.bin", nombre_salida);
	fstream salida_bit(archivo_salida_bit, fstream::trunc | fstream::binary | fstream::out);
	unsigned int int_bits = 0;
	unsigned int pos_int_bits = 0;
	unsigned int max_pos_int_bits = 32;
	
	//archivo principal de factores
	char *archivo_salida_fac = new char[1024];
	sprintf(archivo_salida_fac, "%s.fac.bin", nombre_salida);
	fstream salida_fac(archivo_salida_fac, fstream::trunc | fstream::binary | fstream::out);
	unsigned int *buff_fac = new unsigned int[1024 + 10];
	unsigned int pos_int_fac = 0;
	unsigned int max_pos_int_fac = 32 * 1024;
	
	
	
	contador = 0;
	while(pos < largo_real){
		
		referencia->find(&(text[pos]), largo_real-pos, pos_prefijo, largo_prefijo);
		
//		cout<<"("<<pos_prefijo<<", "<<largo_prefijo<<")\n";
		
		if(pos_prefijo > max_pos){
			max_pos = pos_prefijo;
		}
		
		
		if(largo_prefijo > 16){
			//factor normal
			
			//Guardar largo
			
			
			
		}
		else{
			//factor corto, copia de texto (marcar)
			int_bits |= (1 << pos_int_bits);
			//escribir en el buffer de factores
			//Se escribe el largo en 4 bits
			//Luego se escriben los caracteres en 2bpb
			
			//En cierto caso hay que guardar el buffer en el archivo
			//Sin embargo, para ello TODOS LOS ENTEROS escritos deben estar completos
			//Solo debe haber padding de 0s al FINAL DEL ARCHIVO (ultima escritura)
			//Ademas, cuando se escribe el buffer debe moverse los bits del final (no escritos)
			//al principio y ajustar la posicion en el buffer
			
			//una forma de hacerlo es cuando pos > max
			//este modo, siempre pueden copiarse max y mover desde el entero siguiente
			
			bitput(buff_fac, pos_buff_fac, 4, largo_prefijo);
			pos_buff_fac += 4;
			for(unsigned int i = 0; i < largo_prefijo; ++i){
				switch( text[pos + i] ){
					case 'A' : 
						bitput(buff_fac, pos_buff_fac, 2, 0);
						pos_buff_fac += 2;
						break;
					case 'C' : 
						bitput(buff_fac, pos_buff_fac, 2, 1);
						pos_buff_fac += 2;
						break;
					case 'G' : 
						bitput(buff_fac, pos_buff_fac, 2, 2);
						pos_buff_fac += 2;
						break;
					case 'T' : 
						bitput(buff_fac, pos_buff_fac, 2, 3);
						pos_buff_fac += 2;
						break;
				}
			}//for... cada letra del factor
			
		}//else... factor corto
		
		//Escribir el buffer de factores
		if(pos_buff_fac > max_pos_buff_fac){
			//escribir los int de max_pos_buff_fac (exactos)
			//max_pos_int_fac / 32 ints
			salida_fac.write((char*)(buff_fac), (max_pos_int_fac >> 5) * sizeof(int));
			
			//mover el final del buffer al principio
			//Creo que esto podria ser una cantidad fija mas clara, por ahora dejo 10
			for(unsigned int i = 0; i < 10; ++i){
				buff_fac[i] = buff_fac[i + (max_pos_int_fac >> 5)];
			}
			
			//ajustar la posicion
			pos_buff_fac -= max_pos_int_fac;
			
		}
		
		//Escribir el buffer de ints
		++pos_int_bits;
		if(pos_int_bits == max_pos_int_bits){
			salida_bit.write((char*)(&int_bits), sizeof(int));
			int_bits = 0;
			pos_int_bits = 0;
		}
		
		pos += largo_prefijo;
		
//		if(++contador > 100){
//			break;
//		}
		
		if(pos_prefijo > max_pos_local){
			max_pos_local = pos_prefijo;
		}
		if(n_factores % 10000 == 0){
			cout<<"Max local: "<<max_pos_local<<" ("<<bits_real(max_pos_local)<<")\n";
			max_pos_local = 0;
		}
		
	}
	cout<<"caracteres guardados: "<<pos<<" / "<<largo_real<<" en "<<timer.getMilisec()<<" ms\n";
	
	salida_bit.close();
	salida_fac.close();
	
	cout<<"-----\n";
	cout<<"Histograma de Largos de Factores (puntual)\n";
	map<unsigned int, unsigned int>::iterator it_histograma;
	for(it_histograma = histograma.begin(); it_histograma != histograma.end(); it_histograma++){
		cout<<""<<it_histograma->first<<"\t"<<it_histograma->second<<"\n";
	}
	cout<<"-----\n";
//	cout<<"Histograma de Posiciones de Factores (buckets de "<<size_bucket<<")\n";
//	for(it_histograma = histograma_pos.begin(); it_histograma != histograma_pos.end(); it_histograma++){
//		cout<<""<<size_bucket*it_histograma->first<<"\t"<<it_histograma->second<<"\n";
//	}
//	cout<<"-----\n";
	
	//Notar que agrego 1 byte para escribir el numero de bits
	n_bits_pos_fijo = 8 + n_factores * bits_real(max_pos);
	n_bits_largo_fijo = 8 + n_factores * bits_real(max_largo);
	//1 bit extra por factor para el caso mejor
	n_bits_total_mejor += n_factores;
	cout<<"\nPesos (en bits y en MB)\n";
	cout<<"\nTotal de Factores: "<<n_factores<<"\n";
	cout<<"-----\n";
	cout<<"pos_fijo: "<<n_bits_pos_fijo<<" "<<(long double)n_bits_pos_fijo/(8*1024*1024)<<"\n";
	cout<<"pos_gamma: "<<n_bits_pos_gamma<<" "<<(long double)n_bits_pos_gamma/(8*1024*1024)<<"\n";
	cout<<"pos_vbyte: "<<n_bits_pos_vb<<" "<<(long double)n_bits_pos_vb/(8*1024*1024)<<"\n";
	cout<<"-----\n";
	cout<<"largo_fijo: "<<n_bits_largo_fijo<<" "<<(long double)n_bits_largo_fijo/(8*1024*1024)<<"\n";
	cout<<"largo_gamma: "<<n_bits_largo_gamma<<" "<<(long double)n_bits_largo_gamma/(8*1024*1024)<<"\n";
	cout<<"largo_g64: "<<n_bits_largo_g64<<" "<<(long double)n_bits_largo_g64/(8*1024*1024)<<"\n";
	cout<<"largo_g32: "<<n_bits_largo_g32<<" "<<(long double)n_bits_largo_g32/(8*1024*1024)<<"\n";
	cout<<"largo_g16: "<<n_bits_largo_g16<<" "<<(long double)n_bits_largo_g16/(8*1024*1024)<<"\n";
	cout<<"largo_vbyte: "<<n_bits_largo_vb<<" "<<(long double)n_bits_largo_vb/(8*1024*1024)<<"\n";
	cout<<"-----\n";
	cout<<"mejor caso: "<<n_bits_total_mejor<<" "<<(long double)n_bits_total_mejor/(8*1024*1024)<<"\n";
	cout<<"-----\n";
	
	
	
	
	
	
	cout<<"Fin\n";
	
}

















