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
		
		if( (buff_pos > 0) && (buff[0] == ';' || buff[0] == '>') ){
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
	for( unsigned int i = 0; i < ( (pos_text.size()<10)?pos_text.size():10 ); ++i ){
		cout << "MetadataFasta::filterMetadata - Metadata[" << i << "]: (" << pos_text[i] << ", " << pos_storage[i] << ", " << length_line[i] << ")\n";
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
	
	

















