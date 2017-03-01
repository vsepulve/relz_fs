#ifndef _BITS_UTILS_H
#define _BITS_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>

using namespace std;

class BitsUtils {

private: 
	
	//Parametros de bit[get/put] para 32 bits (fijos y estaticos) 
	static const unsigned int WORD_SIZE = 32;
	static const unsigned int BITS_WORD = 5;
	static const unsigned int MASK_WORD = 0x1f;
	
	//Parametros para golomb (dependen de cada instancia)
	unsigned int GOLOMB_BASE;
	unsigned int GOLOMB_BITS_BASE;
	unsigned int GOLOMB_MASK_BASE;
	
public: 
	
	BitsUtils();
	
	~BitsUtils();
	
	//Cambia la base de golomb a 2 ^ potencia_base
	//potencia_base debe ser entre [2, 32]
	void setGolombBase(unsigned int potencia_base);
	
	//Numero de bits (real) para escribir un numero
	unsigned int n_bits(unsigned int num);

	//Escribe los "largo_num" bits de "num" en la "salida", desde su posicion "pos_salida" (en bits)
	//Notar que luego de cada escritura, el llamador debe sumar los "largo_num" bits escritos a su "pos_salida" local
	// - "salida" es el puntero a la salida para escribir
	// - "pos_salida" es la posicion en bits en la salida para escribir
	// - "largo_num" es el largo de la escritura en bits
	// - "num" es el numero que se desea escribir
	void bitput(unsigned int *salida, unsigned int pos_salida, unsigned int largo_num, unsigned int num);

	//Retorna el numero de "largo_num" bits desde "pos" de la "entrada"
	unsigned int bitget(unsigned int *entrada, unsigned int pos, unsigned int largo_num);
	
	//Retorna el numero de bits a ser usados para una escritura
	unsigned int bits_golomb(unsigned int num);

	//Escribe el numero "num" en la posicion "pos_escritura" (en bits) de la "salida"
	//Retorna el numero de bits usados (para ajustar pos_escritura)
	unsigned int write_golomb(unsigned int *salida, unsigned int pos_escritura, unsigned int num);

	//Lee el proximo numero a partir de la posicion "pos_lectura" de la "entrada"
	//Guarda el numero leido en "num" y retorna el numero de bits leidos
	unsigned int read_golomb(unsigned int *entrada, unsigned int pos_lectura, unsigned int &num);

	unsigned int read_gamma(unsigned int *entrada, unsigned int pos_lectura, unsigned int &num);

	unsigned int write_gamma(unsigned int *salida, unsigned int pos_escritura, unsigned int num);
	
	//Escribe el numero en varbyte al inicio del buffer y retorna el numero de bytes usados
	//Notar que el llamador debe mover el buffer entre escrituras
	unsigned int write_varbyte(unsigned char *buff, unsigned long long num);
	
	//Lee el primer numero en varbyte (y lo almacena en num) retornando el numero de bytes leidos
	//Notar que el llamador debe mover el buffer entre lecturas
	unsigned int read_varbyte(unsigned char *buff, unsigned long long &num);
	
	//Lee el primer numero en varbyte (y lo almacena en num) retornando el numero de bytes leidos
	//Notar que el llamador debe mover el buffer entre lecturas
	//Esta version trunca el numero leido a 32 bits (pero retorna en numero real de bytes leidos)
	unsigned int read_varbyte(unsigned char *buff, unsigned int &num);
	
	//Retorna el numero exacto de bytes que serian usados al escribir este numero con write_varbyte
	//Este metodo se puede usar para preparar un buffer del tamaño adecuado
	unsigned int size_varbyte(unsigned long long num);
};








#endif //_BITS_UTILS_H

