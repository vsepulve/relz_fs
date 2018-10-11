#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <random>
#include <algorithm>

#include <map>
#include <vector>

#include "NanoTimer.h"

using namespace std;

unsigned long long karpRabin(const char *filtered_text, unsigned int segment_length, unsigned int *pow_table, unsigned int kr_mod){
	unsigned long long hash = 0;
	for(unsigned int i = 0, k = segment_length-1; i < segment_length; i++, k--) {
		hash = hash + ((unsigned long long)(filtered_text[i]) * pow_table[k]) % kr_mod;
		hash = hash % kr_mod;
	}
	return hash;
}

int main(int argc, char* argv[]){
	
	if(argc != 8){
		cout << "\nUsage: prepare_reference_text input_file output_file filter? segment_length segments_size kr_length kr_size\n";
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
	unsigned int segments_size = atoi(argv[5]);
	unsigned int kr_length = atoi(argv[6]);
	unsigned int kr_size = atoi(argv[7]);
	
	unsigned int n_segments = segments_size / segment_length;
	unsigned int n_kr = kr_size / kr_length;
	
	cout << "Start (input_file \"" << input_file << "\", output_file \"" << output_file << "\", filter? " << filter << ", segmenst: " << n_segments << " x " << segment_length << ", krs: " << n_kr << " x " << kr_length << ")\n";
	
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
	NanoTimer timer;
	
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
//		cout << "Line: \"" << line << "\"\n";
		
		if( filter && (full_text[line_start] == ';' || full_text[line_start] == '>' || full_text[line_start] == '#') ){
//			cout << "Omiting Line\n";
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
//	cout << "Filtered Text: \"" << filtered_text<< "\"\n";
	cout << "Text Filtered in " << timer.getMilisec() << " ms (" << write_pos << " / " << file_size << " chars)\n";
	
	if( write_pos <= segment_length ){
		cout << "Error, filtered_text too small (" << write_pos << ", segment_length: " << segment_length << ")\n";
		return 0;
	}
	
	fstream writer(output_file, fstream::out | fstream::trunc );
	
	// Fase Karp Rabin
	// La idea seria iterar por el texto generando los kmers de cierto largo con sus frecuencias y la posicion del primero
	// Depsues, ordenamos por frecuencia y agregamos una cierta cantidad de los mas frecuentes
	
	if( n_kr > 0){
		// Mapa para frecs con hash -> pair(frec, pos)
		map<unsigned long long, pair<unsigned int, unsigned int> > frecs;
	
		unsigned int voc_bits = 8;
		unsigned int kr_mod = 15485863;
		unsigned int table_size = 10000000;
		cout << "Preparing KarpRabin (voc_bits: " << voc_bits << ", kr_mod: " << kr_mod << "), PowTable of size " << 10000000 << "\n";
		unsigned int *pow_table = new unsigned int[table_size];
		pow_table[0] = 1;
		for(unsigned int i = 1; i < table_size; ++i){
			pow_table[i] = (pow_table[i-1] * (1<<voc_bits)) % kr_mod;
		}
		// Preparacion del primer hash
		unsigned long long hash = 0;
//		unsigned long long test_hash = 0;
		for(unsigned int i = 0, k = kr_length-1; i < kr_length; i++, k--) {
			hash = hash + ((unsigned long long)(filtered_text[i]) * pow_table[k]) % kr_mod;
			hash = hash % kr_mod;
		}
		string line(filtered_text, kr_length);
//		cout << "hash (" << line << "): " << hash << " (" << karpRabin(line.data(), line.length(), pow_table, kr_mod) << ")\n";
		frecs[hash] = pair<unsigned int, unsigned int>(1, 0);
	
		timer.reset();
		for(unsigned int i = 0; i < write_pos-kr_length; ++i){
			// substract char i
	//		cout << "Removing char " << i << "\n";
			unsigned long long kr1 = ((unsigned long long)(filtered_text[i]) * pow_table[0]) % kr_mod;
	//		cout << "hash (" << filtered_text[i] << "): " << kr1 << "\n";
			hash = (hash + kr_mod - ((kr1 * pow_table[kr_length-1]) % kr_mod)) % kr_mod;
		
			string line(filtered_text + i + 1, kr_length - 1);
	//		cout << "hash (" << line << "): " << hash << " (" << karpRabin(line.data(), line.length(), pow_table, kr_mod) << ")\n";
		
			// add char i + kr_length
	//		cout << "Adding char " << (i + kr_length) << "\n";
			unsigned long long kr2 = ((unsigned long long)(filtered_text[i + kr_length]) * pow_table[0]) % kr_mod;
			hash = (kr2 + (hash * pow_table[1]) % kr_mod) % kr_mod;
		
			// Debug Test
//			line = string(filtered_text + i + 1, kr_length);
//			test_hash = karpRabin(line.data(), line.length(), pow_table, kr_mod);
	//		cout << "hash (" << line << "): " << hash << " (" << test_hash << ")\n";
//			if( hash != test_hash ){
//				cout << "Error en karp rabin\n";
//				return 0;
//			}
			
//			auto it = frecs.find(hash);
			map<unsigned long long, pair<unsigned int, unsigned int>>::iterator it = frecs.find(hash);
			if( it == frecs.end() ){
				frecs[hash] = pair<unsigned int, unsigned int>(1, i+1);
			}
			else{
//				(frecs[hash].first)++;
				(it->second.first)++;
			}
		}
		delete [] pow_table;
		cout << "KarpRabin finished in " << timer. getMilisec() << " ms\n";
	
		vector< pair<unsigned int, unsigned int> > arr_frecs;
		for( auto it : frecs ){
			arr_frecs.push_back(it.second);
		}
		sort(arr_frecs.begin(), arr_frecs.end());
		
		cout << "Extracting " << n_kr << " KR segments from " << arr_frecs.size() << "\n";
		if( n_kr > arr_frecs.size() ){
			n_kr = arr_frecs.size();
		}
		
		for(unsigned int i = 0; i < n_kr; ++i){
			pair<unsigned int, unsigned int> par = arr_frecs[ arr_frecs.size() - 1 - i ];
			string line(filtered_text + par.second, kr_length);
			cout << "frec[" << line << "]: " << par.first << "\n";
			if( par.first <= 1 ){
				cout << "Min frec, stoping\n";
				break;
			}
			writer.write(filtered_text + par.second, kr_length);
		}
	
	} // KarpRabin
	
	
	// Fase de segmentos aleatorios
	
//	random_device seed;
//	mt19937 generator(seed());
	mt19937 generator(0);
	uniform_int_distribution<unsigned long long> pos_dist(0ull, write_pos - segment_length - 1ull);
	for(unsigned int i = 0; i < n_segments; ++i){
		unsigned long long pos = pos_dist(generator);
		writer.write(filtered_text + pos, segment_length);
	}
	
	writer << "\n";
	writer.close();
	
	cout << "End\n";
	
}

















