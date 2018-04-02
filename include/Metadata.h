#ifndef _METADATA_H
#define _METADATA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <vector>
#include <map>

#include "BitsUtils.h"
#include "BytesReader.h"

using namespace std;

enum{
	META_SAVE_UNC = 0,
	META_SAVE_VBYTE = 1
};

class Metadata{

public:
	
protected: 
	
	//Metadatos para lowcase (posiciones absolutas)
	vector< pair<unsigned long long, unsigned long long> > *lowcase_runs;
	
	//Metadatos para new lines (posiciones absolutas)
	vector<unsigned long long> *nl_pos;
	//Skip list (1 nivel) de nl_pos (muestreo de 1/step_size)
	vector<unsigned long long> *nl_skip;
//	const static unsigned int step_size = 128;
//	const static unsigned int step_size_bits = 7;
	const static unsigned int step_size = 1024;
	const static unsigned int step_size_bits = 10;
	
	//Datos para norm + ex
	//En la primera version, uso estos datos solo para el save
	//En el load, reconstruyo nl_pos
//	vector< pair<unsigned int, unsigned int> > *nl_ex;
//	vector<unsigned int> *nl_skip_ex;
//	unsigned int nl_norm;
//	unsigned int nl_total;
	
	//Marca del modo de guardado (uncompress / varbyte)
	unsigned char save_mode;
	
	//Metodos internos de guardado y carga de metadatos
	void saveLowcaseUncompress(fstream *escritor);
	void loadLowcaseUncompress(fstream *lector);
	void saveLowcaseVarByte(fstream *escritor);
	void loadLowcaseVarByte(fstream *lector);
	unsigned int sizeLowcase();
	void loadLowcaseUncompress(BytesReader *lector);
	void loadLowcaseVarByte(BytesReader *lector);
	
	//Como los anteriores, pero para NewLines
	void saveNewLinesUncompress(fstream *escritor);
	void loadNewLinesUncompress(fstream *lector);
	void saveNewLinesVarByte(fstream *escritor);
	void loadNewLinesVarByte(fstream *lector);
	//Versiones norm + ex
	void saveNewLinesVarByteEx(fstream *escritor);
	void loadNewLinesVarByteEx(fstream *lector);
	unsigned int sizeNewLines();
	void loadNewLinesUncompress(BytesReader *lector);
	void loadNewLinesVarByte(BytesReader *lector);
	//Faltan las versiones Ex para BytesReader
	
	//Construccion de la skip list
	void prepareNewLinesSkip();
	
	void deleteData();
	
	// Esto es solo para hacer mas legibles las pruebas de 32 bits
	static const unsigned long long max_int = 0xffffffff;
	
public: 
	
	Metadata();
	
	//Notar que Metadata toma posesion de los vectores y los borrara en su destructor
	Metadata(unsigned char _save_mode, vector< pair<unsigned long long, unsigned long long> > *_lowcase_runs = NULL, vector<unsigned long long> *_nl_pos = NULL);
	
	//Notar que Metadata toma posesion de los vectores y los borrara en su destructor
//	Metadata(unsigned char _save_mode, vector< pair<unsigned long long, unsigned long long> > *_lowcase_runs, unsigned int _nl_total, unsigned int _nl_norm, vector< pair<unsigned long long, unsigned long long> > *_nl_ex);
	
	virtual ~Metadata();
	
	//Usa un byte para definir lo guardado. Con ese byte, tiene certeza de datos nulos.
	// 0: Sin Datos
	// 1: lowcase_runs descomprimido, sin nl_pos
	// 2: nl_pos descomprimido, sin lowcase_runs
	// 3: lowcase_runs y nl_pos descomprimido
	// 4: lowcase_runs en vbyte, sin nl_pos
	// 5: nl_pos en vbyte, sin lowcase_runs
	// 6: lowcase_runs y nl_pos en vbyte
	virtual void save(fstream *escritor);
	
	virtual void load(fstream *lector);
	
	virtual void load(BytesReader *lector);
	
	// Retorna el numero total de bytes que seran usados al almacenar los metadatos
	// Considera los datos disponibles, el modo actual, y el byte de marca
	virtual unsigned int size();
	
	// Ajusta lowercase en los length caracteres de buff, partiendo desde la posicion absoluta real ini del texto
	virtual void adjustCase(char *buff, unsigned long long ini, unsigned int length);
	
	// Retorna el total de NL (para calcular el tamaño real de un archivo)
	// Dejo esto en 32 bits por ahora (esto limita el numero maximo de caracteres newline)
	virtual unsigned int totalNewLines();
	
	// Retorna el numero de NL en posiciones menores (exclusivo) a "pos"
	// Utiliza nl_skip si esta definido y NO usa start (nl_med = nl_der - nl_izq)
	virtual unsigned int countNewLines(unsigned long long pos);
	
	// Ajusta el texto de buff agregando newlines (usando otro buffer para copiar el texto)
	// Asume que la pos de inicio ya esta ajustada para ser absoluta (ini = rel_pos + nl_izq) al menos por ahora
	// Puede recibir un buffer con memoria para reescribir el contenido de buff
	// Asume que tanto el buff como el copy_buff (de ser != NULL) tiene (length + nl_med + 1) bytes
	// Si no se le entrega un copy_buff, pide uno localmente
	virtual void adjustNewLines(char *buff, unsigned long long ini, unsigned int length, unsigned int nl_izq, unsigned int nl_med, char *copy_buff = NULL);
	
	// Analiza y filtra el texto del buffer para una escritura (desde la posicion absoluta pos_ini del texto actual)
	// Lo limpia de case y newlines, y guarda esos datos ajustando los metadatos (agregando o reemplazando valores)
	// Deja el texto filtrado en out_buff y los valores ajustados en adjusted_length y adjusted_pos_ini
	virtual void filterNewText(const char *in_buff, unsigned int length, unsigned long long pos_ini, char *out_buff, unsigned int &adjusted_length, unsigned long long &adjusted_pos_ini);
//	virtual void filterNewText(const char *in_buff, unsigned int length, unsigned int pos_ini, char *out_buff, unsigned int &adjusted_length, unsigned int &adjusted_pos_ini);
	
};







#endif //_METADATA_H





