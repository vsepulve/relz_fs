#include "BitsUtils.h"

BitsUtils::BitsUtils(){
	//Valores por defecto para Golomb
	GOLOMB_BASE = 64;
	GOLOMB_BITS_BASE = 6;
	GOLOMB_MASK_BASE = 0x3f;
}

BitsUtils::~BitsUtils(){}

void BitsUtils::setGolombBase(unsigned int potencia_base){
	if(potencia_base < 1 || potencia_base > 32){
		cerr<<"setGolombBase - Base no soportada (base "<<GOLOMB_BASE<<" conservada)\n";
		return;
	}
	else{
		cout<<"Ajustando golomb para base 2 ^ "<<potencia_base<<" ("<<(1 << potencia_base)<<")\n";
		GOLOMB_BASE = (1 << potencia_base);
		GOLOMB_BITS_BASE = potencia_base;
		GOLOMB_MASK_BASE = (1 << potencia_base) - 1;
	}
}

unsigned int BitsUtils::n_bits(unsigned int num){
	unsigned int ret = 1;
	while( num >>= 1 ){
		++ret;
	}
	return ret;
}

//Escribe los "largo_num" bits de "num" en la "salida", desde su posicion "pos_salida" (en bits)
//Notar que luego de cada escritura, el llamador debe sumar los "largo_num" bits escritos a su "pos_salida" local
// - "salida" es el puntero a la salida para escribir
// - "pos_salida" es la posicion en bits en la salida para escribir
// - "largo_num" es el largo de la escritura en bits
// - "num" es el numero que se desea escribir
void BitsUtils::bitput(unsigned int *salida, unsigned int pos_salida, unsigned int largo_num, unsigned int num){
//	cout<<" -> bitput - "<<largo_num<<" bits de "<<num<<" en posicion "<<pos_salida<<"\n";
	//salida = salida + pos_salida/32, moverse al int donde empezar la escritura
	salida += pos_salida >> BITS_WORD;
	//con esto pos_salida marca el bit dentro del int donde empieza la escritura
//	pos_salida &= (1<<BITS_WORD) - 1;
	pos_salida &= MASK_WORD;
	if (largo_num == WORD_SIZE) {
		*salida |= (*salida & ((1<<pos_salida) - 1)) | (num << pos_salida);
		if (!pos_salida){
			return;
		}
		salida++;
		*salida = (*salida & ~((1<<pos_salida) - 1)) | (num >> (WORD_SIZE - pos_salida));
	}
	else {
		if (pos_salida + largo_num <= WORD_SIZE) {
			*salida = (*salida & ~( ((1<<largo_num) - 1) << pos_salida) ) | (num << pos_salida);
			return;
		}
		*salida = (*salida & ((1<<pos_salida) - 1)) | (num << pos_salida);
		salida++;
		largo_num -= WORD_SIZE - pos_salida;
		*salida = (*salida & ~((1<<largo_num)-1)) | (num >> (WORD_SIZE - pos_salida));
	}
}

//Retorna el numero de "largo_num" bits desde "pos" de la "entrada"
unsigned int BitsUtils::bitget(unsigned int *entrada, unsigned int pos, unsigned int largo_num){
	unsigned int i = (pos >> 5);
	unsigned int j = pos & 0x1f;
	unsigned int answ;
	if( j + largo_num <= WORD_SIZE ){
		answ = (entrada[i] << (WORD_SIZE-j-largo_num)) >> (WORD_SIZE - largo_num);
	}
	else {
		answ = entrada[i] >> j;
		answ = answ | ( (entrada[i+1] << (WORD_SIZE-j-largo_num)) >> (WORD_SIZE-largo_num) );
	}
	return answ;
}

unsigned int BitsUtils::bits_golomb(unsigned int num){
	return ( 1 + (num >> GOLOMB_BITS_BASE) + GOLOMB_BITS_BASE );
}

//Escribe el numero "num" en la posicion "pos_escritura" (en bits) de la "salida"
//Retorna el numero de bits usados (para ajustar pos_escritura)
unsigned int BitsUtils::write_golomb(unsigned int *salida, unsigned int pos_escritura, unsigned int num){
	
	unsigned int pos_salida = pos_escritura;
	//unsigned int q = num / GOLOMB_BASE;
	//unsigned int resto = num % GOLOMB_BASE;
	unsigned int q = (num >> GOLOMB_BITS_BASE);
	unsigned int resto = (num & GOLOMB_MASK_BASE);
	
//	cout<<"write_golomb - numero "<<num<<", q: "<<q<<", resto: "<<resto<<" (base "<<GOLOMB_BASE<<", "<<GOLOMB_BITS_BASE<<" bits para el resto)\n";
	
	//Creo que no se puede con una mascara, hay que escribir los 1's en un tipo de ciclo
	while( q > 31){
		//escribir 32 1's
		bitput(salida, pos_salida, 32, 0xffffffff);
		pos_salida += 32;
		q -= 32;
//		cout<<"write_golomb - pos_salida: "<<pos_salida<<"\n";
	}
	
	//Notar que esta mascara debe ser construida de derecha a izquiera
	unsigned int mascara_q = ((1<<q) - 1) ;
	//escribir la mascara (se escriben q+1 bits en total)
	bitput(salida, pos_salida, q+1, mascara_q);
	pos_salida += (q+1);
//	cout<<"write_golomb - pos_salida: "<<pos_salida<<"\n";
	
	//escribir el resto
	bitput(salida, pos_salida, GOLOMB_BITS_BASE, resto);
	pos_salida += GOLOMB_BITS_BASE;
//	cout<<"write_golomb - pos_salida: "<<pos_salida<<"\n";
	
	return (pos_salida - pos_escritura);
}


//Lee el proximo numero a partir de la posicion "pos_lectura" de la "entrada"
//Guarda el numero leido en "num" y retorna el numero de bits leidos
unsigned int BitsUtils::read_golomb(unsigned int *entrada, unsigned int pos_lectura, unsigned int &num){
	
//	cout<<"read_golomb - desde pos: "<<pos_lectura<<"\n";
		
	unsigned int pos_salida = pos_lectura;
	unsigned int q = 0;
	unsigned int resto = 0;
	
	//leer los 1's de q
	//Para numeros grandes esta cantidad podria ser considerable
	//Leer cada 1 individualmente puede ser mucho trabajo (32 llamadas por cada int de 1's)
	//Tratare de construir una mascara para leer los primeros (el final izquierdo del int actual)
	//Luego un ciclo para leer ints entreos (runs de 32 1's)
	//Luego (cuando un entero rompa ese ciclo) leer el resto uno a uno
	
	unsigned int *entrada_local = ( entrada + (pos_salida >> 5) );
	
	//si el resto del entero actual es de 1's, entonces tiene sentido hacer procesos especiales
	//Si no, entonces basta con tomar el primer 0 del entero actual
	//Notar que ese segundo caso es equivalente a pasar las dos verificaciones previas
	unsigned int mask = (0xffffffff << (pos_salida & 0x1f));
	if( (*entrada_local & mask) == mask ){
//		cout<<"read_golomb - resto de entero de 1s\n";
		//1's en el entero actual
		q = 32 - (pos_salida & 0x1f);
		pos_salida += q;
		++entrada_local;
//		cout<<"read_golomb - q1: "<<q<<"\n";
		while( (*entrada_local == 0xffffffff) ){
			q += 32;
			pos_salida += 32;
			++entrada_local;
		}
//		cout<<"read_golomb - q2: "<<q<<"\n";
		//Prepara mascara para revisar el entero actual desde el principio
		mask = 0x1;
	}
	else{
		//Prepara mascara para revisar el resto del entero actual
		mask = 0x1 << (pos_salida & 0x1f);
	}
	//Ahora TIENE que haber un 0 en el entero actual
	//Notar que esto SE PUEDE hacer en 5 pasos con una busqueda binaria de cada mitad
	//Lo dejo asi por simpleza (pues en este caso se aplica a entero completo o parcial)
	for(; mask != 0 ; mask <<= 1){
//		cout<<"Probando mascara "<<mask<<" ("<<((*entrada_local & mask) != mask)<<")\n";
		++pos_salida;
		if( (*entrada_local & mask) != mask ){
			break;
		}
		++q;
	}
//	cout<<"read_golomb - q3: "<<q<<"\n";
	
	
//	cout<<"read_golomb - leyendo resto\n";
	resto = bitget(entrada, pos_salida, GOLOMB_BITS_BASE);
	pos_salida += GOLOMB_BITS_BASE;
//	cout<<"read_golomb - r: "<<resto<<"\n";
	
	//num = resto + (q * GOLOMB_BASE);
	num = resto | (q << GOLOMB_BITS_BASE);
	
//	cout<<"read_golomb - fin (num: "<<num<<", bits: "<<(pos_salida - pos_lectura)<<")\n";
	return (pos_salida - pos_lectura);
}

unsigned int BitsUtils::BitsUtils::write_gamma(unsigned int *salida, unsigned int pos_escritura, unsigned int num){
	
	unsigned int n = n_bits(num) - 1;
	unsigned int pos_salida = pos_escritura;
	
//	cout<<"write_gamma - numero "<<num<<", n: "<<n<<" (pos_salida: "<<pos_salida<<")\n";
	
	//Notar que n NO PUEDE ser mayor que 32
	
//	while( n > 31){
//		//escribir 32 1's
//		bitput(salida, pos_salida, 32, 0x00000000);
//		pos_salida += 32;
//		n -= 32;
//		cout<<"write_gamma - pos_salida: "<<pos_salida<<"\n";
//	}
	
	bitput(salida, pos_salida, n, 0x0);
	pos_salida += n;
	
	bitput(salida, pos_salida, 1, 0x1);
	++pos_salida;
	
	//escribir el resto
	bitput(salida, pos_salida, n, (num & ((1<<n)-1)) );
	pos_salida += n;
//	cout<<"write_gamma - pos_salida: "<<pos_salida<<"\n";
	
	return (pos_salida - pos_escritura);
}

unsigned int BitsUtils::read_gamma(unsigned int *entrada, unsigned int pos_lectura, unsigned int &num){
	
	unsigned int pos_salida = pos_lectura;
	unsigned int n = 0;
	unsigned int resto = 0;
	
	//Leer los 0's hasta el primer 1
	unsigned int *entrada_local = ( entrada + (pos_salida >> BITS_WORD) );
	
	//Notar que n puede ser, a lo mas, 32
//	cout<<"read_gamma - leyendo n\n";
	unsigned int mask_bit = (1 << (pos_salida & 0x1f) );
	for(n = 0; n < 32; ++n){
//		cout<<"Probando mascara "<<mask_bit<<" ("<<((*entrada_local & mask_bit) == mask_bit)<<")\n";
		++pos_salida;
		if( (*entrada_local & mask_bit) == mask_bit ){
			break;
		}
		mask_bit <<= 1;
		if(mask_bit == 0){
			mask_bit = 1;
			++entrada_local;
		}
	}
//	cout<<"read_gamma - n: "<<n<<"\n";
	
	if(n == 32){
		num = 0;
	}
	else if(n == 0){
		num = 1;
	}
	else{
//		cout<<"read_gamma - leyendo resto\n";
		resto = bitget(entrada, pos_salida, n);
		pos_salida += n;
//		cout<<"read_gamma - r: "<<resto<<"\n";
		num = resto | (1 << n);
	}
	
//	cout<<"read_gamma - fin (num: "<<num<<", bits: "<<(pos_salida - pos_lectura)<<")\n";
	return (pos_salida - pos_lectura);
	
}

unsigned int BitsUtils::write_varbyte(unsigned char *buff, unsigned long long num){
	
	unsigned char *salida = buff;
	
	//Si usa menos de 8, 15, 22, o 29 bits (porque se permiten 7 de cada byte)
	if(num < 0x80){
		//1 byte
		*salida = (num & 0x7f);
		++salida;
	}
	else if(num < 0x4000){
		//2 byte
		*salida = ((num >> 7) & 0x7f) | 0x80;
		++salida;
		*salida = (num & 0x7f);
		++salida;
	}
	else if(num < 0x200000){
		//3 byte
		*salida = ((num >> 14) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 7) & 0x7f) | 0x80;
		++salida;
		*salida = (num & 0x7f);
		++salida;
	}
	else if(num < 0x10000000){
		//4 byte
		*salida = ((num >> 21) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 14) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 7) & 0x7f) | 0x80;
		++salida;
		*salida = (num & 0x7f);
		++salida;
	}
	else if(num < 0x800000000ULL){
		//5 byte
		*salida = ((num >> 28) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 21) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 14) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 7) & 0x7f) | 0x80;
		++salida;
		*salida = (num & 0x7f);
		++salida;
	}
	else if(num < 0x40000000000ULL){
		//6 byte
		*salida = ((num >> 35) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 28) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 21) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 14) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 7) & 0x7f) | 0x80;
		++salida;
		*salida = (num & 0x7f);
		++salida;
	}
	else if(num < 0x2000000000000ULL){
		//7 byte
		*salida = ((num >> 42) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 35) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 28) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 21) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 14) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 7) & 0x7f) | 0x80;
		++salida;
		*salida = (num & 0x7f);
		++salida;
	}
	else if(num < 0x100000000000000ULL){
		//8 byte
		*salida = ((num >> 49) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 42) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 35) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 28) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 21) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 14) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 7) & 0x7f) | 0x80;
		++salida;
		*salida = (num & 0x7f);
		++salida;
	}
	else{
		//9 byte
		*salida = ((num >> 56) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 49) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 42) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 35) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 28) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 21) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 14) & 0x7f) | 0x80;
		++salida;
		*salida = ((num >> 7) & 0x7f) | 0x80;
		++salida;
		*salida = (num & 0x7f);
		++salida;
	}
	
	return (salida - buff);
}

//Lee el primer numero en varbyte (y lo almacena en num) retornando el numero de bytes leidos
//Notar que el llamador debe mover el buffer entre lecturas
unsigned int BitsUtils::read_varbyte(unsigned char *buff, unsigned long long &num){
	
	//Notar que en esta version se usan 9 bytes max, quizas pueda usarse eso como ventaja
	//Ademas, eso puede usarse para mejorar la seguridad (maximo 9 ciclos)
	
	num = 0;
	unsigned char *entrada = buff;
	while( (*entrada & 0x80) ){
		num |= (*entrada & 0x7f);
		num <<= 7;
		++entrada;
	}
	num |= *entrada;
	++entrada;
	
	return (entrada - buff);
}

//Lee el primer numero en varbyte (y lo almacena en num) retornando el numero de bytes leidos
//Notar que el llamador debe mover el buffer entre lecturas
//Esta version trunca el numero leido a 32 bits (pero retorna en numero real de bytes leidos)
unsigned int BitsUtils::read_varbyte(unsigned char *buff, unsigned int &num){
	
	//Notar que en esta version se usan 9 bytes max, quizas pueda usarse eso como ventaja
	//Ademas, eso puede usarse para mejorar la seguridad (maximo 9 ciclos)
	
	unsigned long long llnum = 0;
	unsigned char *entrada = buff;
	while( (*entrada & 0x80) ){
		llnum |= (*entrada & 0x7f);
		llnum <<= 7;
		++entrada;
	}
	llnum |= *entrada;
	++entrada;
	
	num = (unsigned int)llnum;
	
	return (entrada - buff);
}

unsigned int BitsUtils::size_varbyte(unsigned long long num){
	if(num < 0x80){
		return 1;
	}
	else if(num < 0x4000){
		return 2;
	}
	else if(num < 0x200000){
		return 3;
	}
	else if(num < 0x10000000){
		return 4;
	}
	else if(num < 0x800000000ULL){
		return 5;
	}
	else if(num < 0x40000000000ULL){
		return 6;
	}
	else if(num < 0x2000000000000ULL){
		return 7;
	}
	else if(num < 0x100000000000000ULL){
		return 8;
	}
	else{
		return 9;
	}
}














