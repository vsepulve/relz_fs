#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <random>

#include <map>
#include <vector>

using namespace std;

int main(int argc, char* argv[]){
	
	if(argc != 6){
		cout << "\nUsage: prepare_reference_text input_file output_file filter? segment_length n_segments\n";
		cout << "Simple program to prepare a reference text from a large text\n";
		cout << "The output is uppercased and with only 1 newline symbol at the end\n";
		cout << "If filter is == 1, the program omits lines starting with '>', ';' and '#' from the input file\n";
		cout << "\n";
		return 0;
	}
	
	const char *input_file = argv[1];
	const char *output_file = argv[2];
	bool filter = (atoi(argv[3]) == 1);
	unsigned int segment_length = atoi(argv[4]);
	unsigned int n_segments = atoi(argv[5]);
	
	cout << "Start (input_file \"" << input_file << "\", output_file \"" << output_file << "\", filter? " << filter << ")\n";
	
	fstream reader(input_file, fstream::in);
	if(! reader.good() ){
		cerr<<"Error opening " << input_file << "\n";
		return 0;
	}
	reader.seekg (0, reader.end);
	unsigned long long file_size = reader.tellg();
	reader.seekg (0, reader.beg);
	cout << "Reading " << file_size << " bytes\n";
	
	char *full_text = new char[file_size + 1];
	reader.read(full_text, file_size);
	full_text[file_size] = 0;
	
	char *filtered_text = new char[file_size + 1];
	
	// Buffer para procesar linea
//	unsigned int buff_size = 64*1024;
//	char buff[buff_size];
	
	unsigned long long read_pos = 0;
	unsigned long long write_pos = 0;
	
	while( read_pos < file_size ){
		
		unsigned long long line_start = read_pos;
		for( ; read_pos < file_size; ++read_pos ){
			if( full_text[read_pos] == '\n' ){
				++read_pos;
				break;
			}
		}
		// Linea entre line_start y read_pos (el ultimo char PUEDE ser '\n', o no)
		
		// debug
		string line(full_text + line_start, (read_pos - line_start - 1));
		cout << "Line: \"" << line << "\"\n";
		
		if( filter && (full_text[line_start] == ';' || full_text[line_start] == '>' || full_text[line_start] == '#') ){
			cout << "Omiting Line\n";
		}
		else{
			for(unsigned int i = line_start; i < read_pos; ++i){
				char c = toupper(full_text[i]);
				if( c != '\n' ){
					filtered_text[write_pos++] = c;
				}
			}
		}
		
	}
	reader.close();
	
	filtered_text[write_pos] = 0;
	cout << "Filtered Text: \"" << filtered_text<< "\"\n";
	
	fstream writer(output_file, fstream::out | fstream::trunc );
	
	// Fase Karp Rabin
	// La idea seria iterar por el texto generando los kmers de cierto largo con sus frecuencias y la posicion del primero
	// Depsues, ordenamos por frecuencia y agregamos una cierta cantidad de los mas frecuentes
	
	
	
	
	// Fase de segmentos aleatorios
	if( write_pos <= segment_length ){
		cout << "Error, filtered_text too small (" << write_pos << ", segment_length: " << segment_length << ")\n";
		return 0;
	}
	
//	random_device seed;
//	mt19937 generator(seed());
	mt19937 generator(0);
	uniform_int_distribution<> pos_dist(0, write_pos - segment_length - 1);
	for(unsigned int i = 0; i < n_segments; ++i){
		unsigned int pos = pos_dist(generator);
		writer.write(filtered_text + pos, segment_length);
	}
	
	writer << "\n";
	writer.close();
	
	cout << "End\n";
	
}

















