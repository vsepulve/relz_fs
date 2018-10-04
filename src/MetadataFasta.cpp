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
	
unsigned char* MetadataFasta::CompressWithLzma(const char *text, size_t length, int level, size_t &output_length){
//std::string MetadataFasta::CompressWithLzma(const std::string& in, int level) {
	
	size_t worst_space = length + (length >> 2) + 128;
	unsigned char *tmp_buff = new unsigned char[worst_space];
	cout << "CompressWithLzma - worst_space: " << worst_space << "\n";
	output_length = 0;
	int res = lzma_easy_buffer_encode(
			level, LZMA_CHECK_CRC32, NULL,
//			static_cast<const unsigned char*>(text), 
			(const unsigned char*)text, 
			length,
			tmp_buff, 
			&output_length, 
			worst_space
	);
	if( res != LZMA_OK ){
		cerr << "CompressWithLzma - Error while compressing\n";
		delete [] tmp_buff;
		output_length = 0;
		return NULL;
	}
	cout << "CompressWithLzma - final space: " << output_length << "\n";
	unsigned char *result = new unsigned char[output_length + 1];
	memcpy(result, tmp_buff, output_length);
	result[output_length] = 0;
	delete [] tmp_buff;
	return result;

//  std::string result;
//  result.resize(in.size() + (in.size() >> 2) + 128);
//  cout << "CompressWithLzma - worst out_pos: " << (in.size() + (in.size() >> 2) + 128) << "\n";
//  size_t out_pos = 0;
//  if (LZMA_OK != lzma_easy_buffer_encode(
//      level, LZMA_CHECK_CRC32, NULL,
//      reinterpret_cast<uint8_t*>(const_cast<char*>(in.data())), in.size(),
//      reinterpret_cast<uint8_t*>(&result[0]), &out_pos, result.size()))
//    abort();
//  cout << "CompressWithLzma - out_pos: " << out_pos << "\n";
//  result.resize(out_pos);
//	return result;
}

//std::string MetadataFasta::DecompressWithLzma(const std::string& in) {
char * MetadataFasta::DecompressWithLzma(const unsigned char *input, size_t length, size_t &output_length) {
	static const size_t kMemLimit = 1 << 30;  // 1 GB.
	lzma_stream strm = LZMA_STREAM_INIT;
	std::string result;
	result.resize(8192);
	size_t result_used = 0;
	lzma_ret ret;
	ret = lzma_stream_decoder(&strm, kMemLimit, LZMA_CONCATENATED);
	if (ret != LZMA_OK){
		abort();
	}
	size_t avail0 = result.size();
//	strm.next_in = reinterpret_cast<const uint8_t*>(in.data());
	strm.next_in = input;
//	strm.avail_in = in.size();
	strm.avail_in = length;
	strm.next_out = reinterpret_cast<uint8_t*>(&result[0]);
	strm.avail_out = avail0;
	while (true) {
		ret = lzma_code(&strm, strm.avail_in == 0 ? LZMA_FINISH : LZMA_RUN);
		if (ret == LZMA_STREAM_END) {
			result_used += avail0 - strm.avail_out;
			// Guaranteed by lzma_stream_decoder().
			if (0 != strm.avail_in)  {
				abort();
			}
			result.resize(result_used);
			lzma_end(&strm);
//			return result;
			output_length = result.size();
			char *res = new char[output_length + 1];
			memcpy(res, &result[0], output_length);
			res[output_length] = 0;
			return res;
		}
		if (ret != LZMA_OK){
			abort();
		}
		if (strm.avail_out == 0) {
			result_used += avail0 - strm.avail_out;
			result.resize(result.size() << 1);
			strm.next_out = reinterpret_cast<uint8_t*>(&result[0] + result_used);
			strm.avail_out = avail0 = result.size() - result_used;
		}
	}
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
	
//	cout << "MetadataFasta::save - Saving " << metadata_length << " bytes of text\n";
//	writer->write((char*)&metadata_length, sizeof(long long));
//	writer->write(metadata_text, metadata_length);
	
	char compression_mark = 0;
	size_t compressed_size = 0;
	unsigned char *compressed = CompressWithLzma(metadata_text, (size_t)metadata_length, 6, compressed_size);
	if( compressed == NULL || (compressed_size > (size_t)metadata_length) ){
		cout << "MetadataFasta::save - Saving metadata text UNCOMPRESSED\n";
		writer->write(&compression_mark, 1);
		writer->write((char*)&metadata_length, sizeof(long long));
		writer->write(metadata_text, metadata_length);
	}
	else{
		cout << "MetadataFasta::save - Saving metadata text LZMA\n";
		compression_mark = 1;
		writer->write(&compression_mark, 1);
		writer->write((char*)&compressed_size, sizeof(size_t));
		writer->write((char*)compressed, compressed_size);
	}
	if( compressed != NULL ){
		delete [] compressed;
	}
	
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
	char compression_mark = 0;
	metadata_length = 0;
	if(metadata_text != NULL){
		delete [] metadata_text;
	}
	reader->read(&compression_mark, 1);
	if( compression_mark == 1 ){
		cout << "MetadataFasta::load - Reading text LZMA\n";
		size_t compressed_size = 0;
		reader->read((char*)&compressed_size, sizeof(size_t));
		cout << "MetadataFasta::load - compressed_size: " << compressed_size << "\n";
		unsigned char *compressed = new unsigned char[compressed_size + 1];
		reader->read((char*)compressed, compressed_size);
		compressed[compressed_size] = 0;
		size_t uncompressed_size = 0;
		cout << "MetadataFasta::load - Decompressing...\n";
		metadata_text = DecompressWithLzma(compressed, compressed_size, uncompressed_size);
		metadata_length = uncompressed_size;
		cout << "MetadataFasta::load - Deleting compressed buffer\n";
		delete [] compressed;
	}
	else{
		cout << "MetadataFasta::load - Reading text UNCOMPRESSED\n";
		reader->read((char*)&metadata_length, sizeof(long long));
		cout << "MetadataFasta::load - metadata_length: " << metadata_length << "\n";
		metadata_text = new char[metadata_length + 1];
		reader->read(metadata_text, metadata_length);
		metadata_text[metadata_length] = 0;
	}
	
	
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
	
//	cout << "MetadataFasta::load - Testing LZMA\n";
//	string test_text = "alabardalalalbalalalalalaala";
//	test_text += "bardalaalabardalalalbalalalalalaalabardalaa";
//	test_text += "labardalalalbalalalalalaalabardalaalabardal";
//	test_text += "alalbalalalalalaalabardalaalabardalalalbala";
//	test_text += "lalalalaalabardalaalabardalalalbalalalalala";
//	test_text += "alabardala";
//	cout << "text: " << test_text << " (" << test_text.length() << ")\n";
//	size_t compressed_size = 0;
//	unsigned char *compressed = CompressWithLzma(test_text.data(), test_text.length(), 6, compressed_size);
//	cout << "compressed: (" << compressed_size << ")\n";
//	size_t uncompressed_size = 0;
//	char *uncompressed = DecompressWithLzma(compressed, compressed_size, uncompressed_size);
//	cout << "uncompressed: " << uncompressed << " (" << uncompressed_size << ")\n";
	
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
	
	// compression_mark
	n_bytes += 1;
	
	size_t compressed_size = 0;
	unsigned char *compressed = CompressWithLzma(metadata_text, (size_t)metadata_length, 6, compressed_size);
	if( compressed == NULL || (compressed_size > (size_t)metadata_length) ){
		cout << "MetadataFasta::size - UNCOMPRESSED\n";
		
		// metadata_length
		n_bytes += sizeof(long long);
		
		// metadata_text
		n_bytes += metadata_length;
	}
	else{
		cout << "MetadataFasta::size - LZMA\n";
		
		// compressed_size
		n_bytes += sizeof(size_t);
		
		// compressed
		n_bytes += compressed_size;
	}
	if( compressed != NULL ){
		delete [] compressed;
	}
	
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















