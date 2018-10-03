#include "MetadataFasta.h"

MetadataFasta::MetadataFasta(){
	metadata_text = NULL;
	metadata_length = 0;
}

MetadataFasta::~MetadataFasta(){
	if( metadata_text != NULL ){
		delete [] metadata_text;
		metadata_text = NULL;
	}
	metadata_length = 0;
}

	
unsigned long long MetadataFasta::filterMetadata(char *text, unsigned long long text_length){
	
	cout << "MetadataFasta::filterMetadata - Start (text_length: " << text_length << ")\n";
	
	// Por ahora preparo dos buffers internos de todo el texto para separar y despues copiar de vuelta el texto filtrado
	char *buff_text = new char[text_length + 1];
	buff_text[0] = 0;
	char *buff_metadata = new char[text_length + 1];
	buff_metadata[0] = 0;
	
	unsigned int buff_size = 64*1024;
	char buff[buff_size];
	
	unsigned long long read_pos = 0;
	unsigned long long text_pos = 0;
	unsigned long long metadata_pos = 0;
	
//	vector<unsigned int> pos_text;
//	vector<unsigned int> pos_storage;
//	vector<unsigned int> length_line
	
	while( read_pos < text_length ){
		
		cout << "MetadataFasta::filterMetadata - Reading from " << read_pos << " / " << text_length << "\n";
		
		// getline
		unsigned long long line_start = read_pos;
		unsigned int buff_pos = 0;
		for( ; read_pos < text_length; ++read_pos ){
			buff[buff_pos++] = text[read_pos];
			if( text[read_pos] == '\n' ){
				++read_pos;
				break;
			}
		}
		buff[buff_pos] = 0;
//		cout << "MetadataFasta::filterMetadata - Line \"" << buff << "\"\n";
		
		if( (buff_pos > 0) && (buff[0] == ';' || buff[0] == '>' || buff[0] == '#') ){
			cout << "MetadataFasta::filterMetadata - Adding to metadata\n";
			if( buff[buff_pos-1] == '\n' ){
				--buff_pos;
				buff[buff_pos] = 0;
			}
			
			pos_text.push_back(line_start);
			pos_storage.push_back(metadata_pos);
			length_line.push_back(buff_pos);
			
			memcpy(buff_metadata + metadata_pos, buff, buff_pos);
			metadata_pos += buff_pos;
			buff_metadata[metadata_pos] = 0;
		}
		else{
			cout << "MetadataFasta::filterMetadata - Adding to text\n";
			memcpy(buff_text + text_pos, buff, buff_pos);
			text_pos += buff_pos;
			buff_text[text_pos] = 0;
		}
		
	}
	
	cout << "MetadataFasta::filterMetadata - Resulting metadata (" << metadata_pos << "): \"" << buff_metadata << "\"\n";
//	cout << "MetadataFasta::filterMetadata - Resulting text (" << text_pos << "): \"" << buff_text << "\"\n";
	
	cout << "MetadataFasta::filterMetadata - Stored lines: " << pos_text.size() << "\n";
	for( unsigned int i = 0; i < ( (pos_text.size()<10)?pos_text.size():10 ); ++i ){
		cout << "MetadataFasta::filterMetadata - Metadata[" << i << "]: (" << pos_text[i] << ", " << pos_storage[i] << ", " << length_line[i] << ")\n";
	}
	
	// Devolver texto filtrado y borrar buffer
	memcpy( text, buff_text, text_pos );
	text[text_pos] = 0;
	delete [] buff_text;
	
	// Almacenar texto de metadata y borrar buffer
	metadata_length = metadata_pos;
	metadata_text = new char[metadata_length + 1];
	memcpy( metadata_text, buff_metadata, metadata_length );
	metadata_text[metadata_length] = 0;
	delete [] buff_metadata;
	
	return text_pos;
}
	
void MetadataFasta::save(fstream *writer){

	cout << "MetadataFasta::save - Start\n";
	
	unsigned int n_lines = pos_text.size();
	writer->write((char*)&n_lines, sizeof(int));
	
	for( unsigned int i = 0; i < n_lines; ++i ){
		unsigned long long n = pos_text[i];
		writer->write((char*)&n, sizeof(long long));
	}
	
	for( unsigned int i = 0; i < n_lines; ++i ){
		unsigned long long n = pos_storage[i];
		writer->write((char*)&n, sizeof(long long));
	}
	
	for( unsigned int i = 0; i < n_lines; ++i ){
		unsigned int n = length_line[i];
		writer->write((char*)&n, sizeof(int));
	}
	
	cout << "MetadataFasta::save - Saving " << metadata_length << " bytes of text\n";
	writer->write((char*)&metadata_length, sizeof(long long));
	writer->write(metadata_text, metadata_length);
	
	cout << "MetadataFasta::save - End\n";
	
}

void MetadataFasta::load(fstream *reader){
	cout << "MetadataFasta::load - Start\n";
	
	unsigned int n_lines = 0;
	reader->read((char*)&n_lines, sizeof(int));
	cout << "MetadataFasta::load - n_lines: " << n_lines << "\n";
	
//	cout << "MetadataFasta::load - Loading pos_text\n";
	for( unsigned int i = 0; i < n_lines; ++i ){
		unsigned long long n = 0;
		reader->read((char*)&n, sizeof(long long));
		pos_text.push_back(n);
	}
	
//	cout << "MetadataFasta::load - Loading pos_storage\n";
	for( unsigned int i = 0; i < n_lines; ++i ){
		unsigned long long n = 0;
		reader->read((char*)&n, sizeof(long long));
		pos_storage.push_back(n);
	}
	
//	cout << "MetadataFasta::load - Loading length_line\n";
	for( unsigned int i = 0; i < n_lines; ++i ){
		unsigned int n = 0;
		reader->read((char*)&n, sizeof(int));
		length_line.push_back(n);
	}
	
//	cout << "MetadataFasta::load - Preparing Metadata text\n";
	metadata_length = 0;
	reader->read((char*)&metadata_length, sizeof(long long));
//	cout << "MetadataFasta::load - metadata_length: " << metadata_length << "\n";
	
	if(metadata_text != NULL){
		delete [] metadata_text;
	}
	metadata_text = new char[metadata_length + 1];
	
//	cout << "MetadataFasta::load - Loading Metadata text\n";
	reader->read(metadata_text, metadata_length);
	metadata_text[metadata_length] = 0;
	
	cout << "MetadataFasta::load - Resulting metadata (" << metadata_length << "): \"" << metadata_text << "\"\n";
//	cout << "MetadataFasta::filterMetadata - Resulting text (" << text_pos << "): \"" << buff_text << "\"\n";
	
	cout << "MetadataFasta::load - Stored lines: " << pos_text.size() << "\n";
	for( unsigned int i = 0; i < ( (pos_text.size()<100)?pos_text.size():100 ); ++i ){
		cout << "MetadataFasta::filterMetadata - Metadata[" << i << "]: (" << pos_text[i] << ", " << pos_storage[i] << ", " << length_line[i] << ")\n";
	}
	
//	NanoTimer timer;
//	for(unsigned int i = 0; i < 10000; ++i){
//		unsigned long long n1 = countText(i);
//	}
//	cout << "MetadataFasta::load - Time countText: " << timer.getMilisec() << "\n";
//	timer.reset();
//	for(unsigned int i = 0; i < 10000; ++i){
//		unsigned long long n1 = countTextBin(i);
//	}
//	cout << "MetadataFasta::load - Time countTextBin: " << timer.getMilisec() << "\n";
//	
	cout << "MetadataFasta::load - Testing countTextBin\n";
	for(unsigned int i = 0; i < 100000; ++i){
		unsigned long long n1 = countText(i);
		unsigned long long n2 = countTextBin(i);
		if( n1 != n2 ){
			cout << "MetadataFasta::load - Error (" << n2 << " != " << n1 << ")\n";
			exit(0);
		}
	}
	
	
	
	cout << "MetadataFasta::load - End\n";
	
}
	
unsigned int MetadataFasta::size(){
	
	unsigned int n_bytes = 0;
	
	// n_lines
	n_bytes += sizeof(int);
	
	// pos_text
	n_bytes += pos_text.size() * sizeof(long long);
	
	// pos_storage
	n_bytes += pos_text.size() * sizeof(long long);
	
	// length_line
	n_bytes += pos_text.size() * sizeof(int);
	
	// metadata_length
	n_bytes += sizeof(long long);
	
	// metadata_text
	n_bytes += metadata_length;
	
	return n_bytes;
}
	
	
void MetadataFasta::adjustText(char *out_buff, unsigned long long pos_ini, unsigned int copied_chars, char *adjust_buffer){
	
	cout << "MetadataFasta::adjustText - Start (pos_ini: " << pos_ini << ", copied_chars: " << copied_chars << ")\n";
	
	cout << "MetadataFasta::adjustText - Original text: \"" << out_buff << "\"\n";
	
	
	unsigned int write_pos = 0;
	unsigned int read_pos = 0;
	bool first = true;
	unsigned int special_copy = 0;
	for( unsigned int i = 0; i < pos_text.size(); ++i ){
		// Obviamente el primer paso se saca de una busqueda binaria simple
		// Aqui estoy perdiendo la cola de potenciales metadatos previos, hay que considerar el largo tambien
		
		if( pos_text[i] < pos_ini ){
			if( pos_text[i] + length_line[i] >= pos_ini ){
				// Caso especial, escribir la cola de una linea de metadata cortada
				cout << "MetadataFasta::adjustText - Special case, adding truncated metadata\n";
				special_copy = pos_text[i] + length_line[i] - pos_ini;
				unsigned long long pos = pos_storage[i] + length_line[i] - special_copy;
				cout << "MetadataFasta::adjustText - Adding " << special_copy << " chars from metadata (+1 newline) from pos " << pos << " (" << pos_storage[i] << " + " << length_line[i] << " - " << special_copy << ")\n";
				memcpy(adjust_buffer + write_pos, metadata_text + pos, special_copy);
				write_pos += special_copy;
				adjust_buffer[write_pos++] = '\n';
				++special_copy;
			}
			continue;
		}
		if( pos_text[i] >= pos_ini + copied_chars ){
			break;
		}
		
		// Copia de chars del texto previo al metadata
		unsigned int copy_len = pos_text[i];
		cout << "MetadataFasta::adjustText - Initial copy_len: " << pos_text[i] << "\n";
		if( first ){
			if( pos_ini > copy_len ){
				copy_len = 0;
			}
			else{
				copy_len -= pos_ini;
			}
			if( special_copy > copy_len ){
				copy_len = 0;
			}
			else{
				copy_len -= special_copy;
			}
			cout << "MetadataFasta::adjustText - Mod copy_len: " << pos_text[i] << " - " << pos_ini << " - " << special_copy << "\n";
		}

//		unsigned int copy_len = pos_text[i];
//		cout << "MetadataFasta::adjustText - Initial copy_len: " << copy_len << "\n";
		if( ! first ){
			// Descuento la linea anterior
			copy_len -= (pos_text[i-1] + length_line[i-1] + 1);
			cout << "MetadataFasta::adjustText - Adjusted copy_len: " << copy_len << " (ajuste: - " << pos_text[i-1] << " - " << length_line[i-1] << " - 1)\n";
		}
		
		first = false;
		
		cout << "MetadataFasta::adjustText - Adding " << copy_len << " chars from original text\n";
		memcpy(adjust_buffer + write_pos, out_buff + read_pos, copy_len);
		write_pos += copy_len;
		read_pos += copy_len;
		cout << "MetadataFasta::adjustText - Adding " << length_line[i] << " chars from metadata (+1 newline)\n";
		memcpy(adjust_buffer + write_pos, metadata_text + pos_storage[i], length_line[i]);
		write_pos += length_line[i];
		adjust_buffer[write_pos++] = '\n';
		adjust_buffer[write_pos] = 0;
		// Condicion de salida si ya escribio lo suficiente?
		// Quizas recibir el largo esperado del texto de salida
	}
	
//	cout << "MetadataFasta::adjustText - adjust_buffer: \"" << adjust_buffer << "\" (write_pos: " << write_pos << ", strlen: " << strlen(adjust_buffer) << ")\n";
	
	// Agergar la cola del texto
	if( write_pos < copied_chars ){
		cout << "MetadataFasta::adjustText - Adding " << (copied_chars - write_pos) << " chars from original text to finish\n";
		memcpy(adjust_buffer + write_pos, out_buff + read_pos, copied_chars - write_pos);
		write_pos = copied_chars;
		adjust_buffer[write_pos] = 0;
	}
	
//	cout << "MetadataFasta::adjustText - adjust_buffer (final): \"" << adjust_buffer << "\"\n";
	
	// Si habiamos escrito mas, basta con desecharlo
	write_pos = copied_chars;
	
	// Devolver texto a la salida
	memcpy(out_buff, adjust_buffer, write_pos);
	out_buff[write_pos] = 0;
	
	cout << "MetadataFasta::adjustText - Resulting text: \"" << out_buff << "\"\n";
	
	
	cout << "MetadataFasta::adjustText - End\n";

}

unsigned long long MetadataFasta::countText(unsigned long long pos){

//	cout << "MetadataFasta::countText - Start (pos: " << pos << ")\n";
	
	unsigned long long res = 0;
	for( unsigned int i = 0; i < pos_text.size(); ++i ){
		if( pos_text[i] >= pos ){
			break;
		}
		unsigned long long ini = pos_text[i];
		// agreggo el newline
		unsigned int len = length_line[i] + 1;
		
		// pos: 10, ini: 7, len: 6 => len deseado 3
		if( ini + len > pos ){
			len = pos - ini;
		}
		res += len;
	}
	
//	cout << "MetadataFasta::countText - End (res: " << res << ")\n";
	
	return res;
}

unsigned long long MetadataFasta::countTextBin(unsigned long long pos){

//	cout << "MetadataFasta::countTextBin - Start (pos: " << pos << ")\n";
	
	if( pos >= (pos_text.back() + length_line.back() + length_line.size()) ){
//		cout << "MetadataFasta::countTextBin - Greater than collection\n";
		return (pos_storage.back() + length_line.back() + length_line.size());
	}
	
	// Version Binaria
	unsigned int l = 0;
	unsigned int h = pos_text.size() - 1;
	unsigned int m;
	while(l < h){
		m = l + ((h-l)>>1);
		if( pos_text[m] < pos ){
			l = m+1;
		}
		else{
			h = m;
		}
	}
//	cout<<"MetadataFasta::countTextBin - BB finished (h: " << h << " por " << pos_text[h] << " / " << pos << ")\n";
	if( (h > 0) && (pos_text[h] + 1) > pos ){
//		cout<<"MetadataFasta::countTextBin - Adjusting\n";
		--h;
	}
	
//	cout<<"MetadataFasta::countTextBin - h: " << h << ", pos_text: " << pos_text[h] << ", pos_storage: " << pos_storage[h] << ", length_line: " << length_line[h] << "\n";
	
	unsigned long long res = pos_storage[h] + h;
	
//	cout<<"MetadataFasta::countTextBin - " << pos << " < " << (pos_text[h] + length_line[h] + h + 1) << "? por h: " << h << "\n";
	if( pos < (pos_text[h] + length_line[h] + 1) ){
//		cout<<"MetadataFasta::countTextBin - Adding " << (pos - pos_text[h]) << "\n";
		res +=  (pos - pos_text[h]);
	}
	else{
		res += length_line[h] + 1;
	}
	
//	cout << "MetadataFasta::countTextBin - End (res: " << res << ")\n";
	
	return res;
}















