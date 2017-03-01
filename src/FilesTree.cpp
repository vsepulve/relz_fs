
#include "FilesTree.h"

//
// ----- FileNode -----
//

char *FileNode::reference_text = NULL;

FileNode::FileNode(){
	first_child = NULL;
	brother = NULL;
	lector = NULL;
	decoder = NULL;
	directory = false;
	size = 0;
}

FileNode::FileNode(const string &_name, bool _directory, unsigned long long _size, DecoderBlocks *_decoder) :
	directory( _directory ),
	size( _size ),
	name( _name )
{
	cout<<"FileNode::FileNode - \""<<name<<"\"\n";
	first_child = NULL;
	brother = NULL;
	lector = NULL;
	decoder = _decoder;
}

FileNode::~FileNode(){
	cout<<"FileNode::~FileNode - Borrando \""<<name<<"\"\n";
	FileNode *child = first_child;
	FileNode *tmp = NULL;
	while(child != NULL){
		tmp = child->brother;
		delete child;
		child = tmp;
	}
	first_child = NULL;
	if(lector != NULL){
		lector->close();
		delete lector;
		lector = NULL;
	}
	if(decoder != NULL){
		delete decoder;
		decoder = NULL;
	}
}

//Retorna el ultimo hijo (para agregar y cosas similares)
//Para esto itera por todos los hijos, podria guardar un puntero al ultimo
FileNode *FileNode::lastChild() const{
	FileNode *child = first_child;
	while(child != NULL && child->brother != NULL){
		child = child->brother;
	}
	return child;
}

//Retorna el hijo con el texto buscado (o NULL si no lo encuentra)
//Esta version es lineal, y compara el texto de cada hijo
//Si se agregaran alfabeticamente, podria hacerse una busqueda binaria
FileNode *FileNode::find(const string &_name) const{
	FileNode *tmp = first_child;
	while( tmp != NULL && tmp->name.compare(_name) != 0 ){
		tmp = tmp->brother;
	}
	return tmp;
}

//Retorna el nodo recien creado
//Siempre agrega el nuevo hijo al final
//Esto podria modificarse para que los agregara en posicion alfabetica
//Al agregar un hijo, convierto al nodo actual en directorio (ojo con esto)
FileNode *FileNode::add(const string &_name, bool _directory, unsigned long long _size, DecoderBlocks *_decoder){
	FileNode *new_node = NULL;
	if(first_child == NULL){
		first_child = new FileNode(_name, _directory, _size, _decoder);
		new_node = first_child;
	}
	else{
		FileNode *last = lastChild();
		last->brother = new FileNode(_name, _directory, _size, _decoder);
		new_node = last->brother;
	}
	directory = true;
	return new_node;
}

//La logica es similar a un find
//Si los hijos se agregan alfabeticamente, esto se puede binariamente
void FileNode::removeChild(const string &_name){
	if(first_child == NULL){
		return;
	}
	FileNode *tmp = first_child;
	//Si es el primer hijo hay que reasignar first_child 
	if( tmp->name.compare(_name) == 0 ){
		first_child = tmp->brother;
		tmp->brother = NULL;
		delete tmp;
	}
	else{
		while( tmp->brother != NULL && tmp->brother->name.compare(_name) != 0 ){
			tmp = tmp->brother;
		}
		if(tmp->brother != NULL){
			FileNode *delete_node = tmp->brother;
			tmp->brother = tmp->brother->brother;
			delete_node->brother = NULL;
			delete delete_node;
		}
	}
}

void FileNode::print(unsigned int level, bool recursive) const{
	for(unsigned int i = 0; i < level; ++i){
		cout<<"-\t";
	}
	if(name.size() == 0){
		cout<<"[NULL] ("<<size<<")\n";
	}
	else if( isDirectory() ){
		cout<<"\033[1;38;5;4m"<<name<<"\033[0m ("<<size<<")\n";
	}
	else{
		cout<<""<<name<<" ("<<size<<")\n";
	}
	if(recursive){
		FileNode *child = first_child;
		while(child != NULL){
			child->print(level+1);
			child = child->brother;
		}
	}
}

bool FileNode::isDirectory() const{
	return directory;
}

unsigned long long FileNode::getSize() const{
	return size;
}
	
void FileNode::setSize(unsigned long long _size){
	size = _size;
}

void FileNode::loadDirectory(const char *path){
	
	if( path == NULL || strlen(path) == 0){
		cerr<<"FileNode::loadDirectory - Error, Path invalido\n";
		return;
	}
	
	//+2 por '/' y el 0 final
	char real_path[ strlen(path) + name.size() + 2];
	if( path[strlen(path) - 1] != '/' && name[0] != '/'){
		sprintf(real_path, "%s/%s", path, name.c_str());
	}
	else if( path[strlen(path) - 1] == '/' && name[0] == '/'){
		sprintf(real_path, "%s%s", path, name.c_str() + 1);
	}
	else{
		sprintf(real_path, "%s%s", path, name.c_str());
	}
	
	cout<<"loadDirectory - real_path: \""<<real_path<<"\"\n";
	
	struct stat file_info;
	bool _directory = false;
	unsigned long long _size = 0;
	DecoderBlocks *_decoder = NULL;
	DIR *directory = NULL;
	dirent *child_dir = NULL;
	dirent *test_dir = NULL;
	//burocracia para construir el struct con memoria pedida
	int name_max = pathconf(real_path, _PC_NAME_MAX);
	//Si el limite no esta definido, adivino
	if (name_max == -1){
		name_max = 255;
	}
	int len = offsetof(struct dirent, d_name) + name_max + 1;
	child_dir = (struct dirent*)malloc(len);
	//Esta llamada parece segura (pues tiene closedir que deberia borrar la memoria local)
	directory = opendir(real_path);
	if(directory != NULL){
		while(true){
			//readdir_r es thread-safe (notar que recibe dos punteros para dejar el resultado)
			// - El primero es un struct con memoria pedida para guardar el resultado
			// - El segundo es espacio para un puntero al resultado (el mismo struc del segundo parametro, o NULL)
			// - Ese segundo puntero es usado para la prueba de END OF STREAM con NULL
			readdir_r( directory, child_dir, &test_dir );
			if(test_dir == NULL){
				break;
			}
			cout<<" -> \""<<child_dir->d_name<<"\" (dir? "<<( (child_dir->d_type == DT_DIR)?"Si":"No" )<<")\n";
			if( strlen(child_dir->d_name) == 0 
				|| strcmp(child_dir->d_name, ".") == 0
				|| strcmp(child_dir->d_name, "..") == 0 ){
				continue;
			}
			string child_name;
			if( child_dir->d_name[0] != '/' ){
				child_name += "/";
			}
			child_name += child_dir->d_name;
			string child_path(real_path);
			if(real_path[strlen(real_path) - 1] != '/' && child_dir->d_name[0] != '/'){
				child_path += "/";
				child_path += child_dir->d_name;
			}
			else if(real_path[strlen(real_path) - 1] == '/' && child_dir->d_name[0] == '/'){
				child_path += (child_dir->d_name + 1);
			}
			else{
				child_path += child_dir->d_name;
			}
			
			//Voy a agregar el stat del nuevo nodo aqui (le pasare los datos en el constructor)
			lstat(child_path.c_str(), &file_info);
			_directory = S_ISDIR(file_info.st_mode);
			_size = (long long)(file_info.st_size);
			_decoder = NULL;
			//Tambien agrego aqui la revision de tipo comprimido
			//Esto provocara la construccion de un descompresor de inmediato (para TODOS los relz)
			//Despues hay que pensar como solo crear estos cuando sea necesarios
			//Sin embargo, para hacer esto seria necesaria la referencia... puntero estatico de la clase !
			if( FileNode::reference_text != NULL 
				&& child_path.length() > 5
				&& strcmp( child_path.c_str() + child_path.length() - 5, ".relz" ) == 0 ){
				cout<<" -> Archivo relz\n";
//				_decoder = new DecoderBlocks(child_path.c_str(), FileNode::reference_text);
				_decoder = new DecoderBlocksRelz(child_path.c_str(), FileNode::reference_text);
				_size = _decoder->getTextSize();
			}
			
			cout<<" -> \""<<child_path<<"\" (_directory: "<<_directory<<", _size: "<<_size<<")\n";
			add( child_name, _directory, _size, _decoder);
			
		}
		closedir( directory );
	}
	//free del struct pedido con malloc
	free(child_dir);

	//Iterar por cada hijo del nodo, y hacer llamada recursiva (solo para directorios, claro)
	FileNode *child = first_child;
	while(child != NULL){
		if( child->isDirectory() ){
			child->loadDirectory(real_path);
		}
		child = child->brother;
	}
	
}

void FileNode::updateNode(const char *real_path){
	cout<<"FileNode::updateNode - Inicio ("<<real_path<<")\n";
	if(real_path == NULL || strlen(real_path) < 1){
		return;
	}
	struct stat file_info;
	lstat(real_path, &file_info);
	directory = S_ISDIR(file_info.st_mode);
	size = (long long)(file_info.st_size);
	if( decoder != NULL ){
		delete decoder;
		decoder = NULL;
	}
	if( FileNode::reference_text != NULL 
		&& strlen(real_path) > 5
		&& strcmp( real_path + strlen(real_path) - 5, ".relz" ) == 0 ){
		cout<<" -> Archivo relz\n";
//		decoder = new DecoderBlocks(real_path, FileNode::reference_text);
		decoder = new DecoderBlocksRelz(real_path, FileNode::reference_text);
		size = decoder->getTextSize();
	}
}


//
// ----- FilesTree -----
//

FilesTree::FilesTree(){
	root = NULL;
	base_path = NULL;
}

FilesTree::~FilesTree(){
	if(root != NULL){
		delete root;
		root = NULL;
	}
	if(base_path != NULL){
		delete [] base_path;
		base_path = NULL;
	}
}

void FilesTree::add(const char *path){
	
	if(path == NULL || strlen(path) == 0 || path[0] != '/'){
		cerr<<"FilesTree::add - Error, path invalido\n";
		return;
	}
	
	cout<<"FilesTree::add - Inicio (\""<<path<<"\")\n";
	
	if(root == NULL){
		cout<<"FilesTree::add - Creando raiz\n";
		root = new FileNode("/", true);
	}
	FileNode *cur_node = root;
	FileNode *child = NULL;
	
	unsigned int largo = strlen(path);
	unsigned int ini = 0;
	//Notar que aqui tengo seguridad de que el string es no nulo, y que empieza por '/'
	//Los margenes dejan el '/' inicial de cada token (justo como se necesitan)
	//Cada token agregado al interior del for, ES UN DIRECTORIO (pues termina en '/')
	//Si el path solo tiene directorios, TODOS son tomados al interior del for si termina en '/'
	//Si el ultimo token NO termina en '/', no es considerado directorio
	for(unsigned int i = 0; i < largo; ++i){
		if( path[i] == '/' ){
			if( i - ini > 1){
				string s(path + ini, i - ini);
				cout<<"FilesTree::add - Buscando hijo \""<<s<<"\"\n";
				child = cur_node->find(s);
				if(child == NULL){
					child = cur_node->add(s, true);
				}
				cur_node = child;
			}//if... largo valido
			ini = i;
		}//if... token
	}//for... cada char
	//archivo final
	if( largo - ini > 1){
		string s(path + ini, largo - ini);
		cout<<"FilesTree::add - Buscando hijo \""<<s<<"\"\n";
		child = cur_node->find(s);
		if(child == NULL){
			child = cur_node->add(s, false);
		}
	}//if... token final de largo valido
	
	cout<<"FilesTree::add - Fin\n";
}

//Borra el nodo final del path (INCLUYENDO HIJOS, no hace distincion de directorio)
//Este metodo usara casi todo el codigo de FilesTree::find
//Quzias se pueda reusar de algun modo (que retorne al padre)
void FilesTree::remove(const char *path){
	
	if(path == NULL || strlen(path) == 0 || path[0] != '/'){
		cerr<<"FilesTree::remove - Error, path invalido\n";
		return;
	}
	
	cout<<"FilesTree::remove - Inicio (\""<<path<<"\")\n";
	
	if(root == NULL){
		return;
	}
	
	//Borrar root (path == "/")
	if( strlen(path) == 1 ){
		delete root;
		root = NULL;
		return;
	}
	
	FileNode *cur_node = root;
	FileNode *child = NULL;
	
	unsigned int largo = strlen(path);
	unsigned int ini = 0;
	for(unsigned int i = 0; i < largo; ++i){
		if( path[i] == '/' ){
			if( i - ini > 1){
				string s(path + ini, i - ini);
				cout<<"FilesTree::remove - Buscando hijo \""<<s<<"\"\n";
				if(i == largo - 1){
					//Ultimo nodo (el que debe ser borrado)
					cout<<"FilesTree::remove - Borrando\n";
					cur_node->removeChild(s);
				}//if... nodo a ser borrado
				else{
					//Nodo intermedio, buscar del modo normal
					child = cur_node->find(s);
					if(child == NULL){
						return;
					}
					cur_node = child;
				}//else... nodo normal
			}//if... largo valido
			ini = i;
		}//if... token
	}//for... cada char
	//Si queda un nodo, este TIENE que ser el ultimo (el que debe ser borrado)
	if( largo - ini > 1){
		string s(path + ini, largo - ini);
		cout<<"FilesTree::remove - Buscando hijo \""<<s<<"\" (Borrando)\n";
		cur_node->removeChild(s);
	}//if... token final de largo valido
	
}

//Retorna el node asociado al path, o NULL si no lo encuentra
//Hay un caso especial para la raiz (por el momento es estricto, quizas deba mejorarse)
FileNode *FilesTree::find(const char *path) const{
	
	if(path == NULL || strlen(path) == 0 || path[0] != '/'){
		cerr<<"FilesTree::find - Error, path invalido\n";
		return NULL;
	}
	
	cout<<"FilesTree::find - Inicio (\""<<path<<"\")\n";
	
	if(root == NULL){
		return NULL;
	}
	
	if( strlen(path) == 1 ){
		return root;
	}
	
	FileNode *cur_node = root;
	FileNode *child = NULL;
	
	unsigned int largo = strlen(path);
	unsigned int ini = 0;
	for(unsigned int i = 0; i < largo; ++i){
		if( path[i] == '/' ){
			if( i - ini > 1){
				string s(path + ini, i - ini);
				cout<<"\""<<s<<"\"\n";
				child = cur_node->find(s);
				if(child == NULL){
					return NULL;
				}
				cur_node = child;
			}//if... largo valido
			ini = i;
		}//if... token
	}//for... cada char
	if( largo - ini > 1){
		string s(path + ini, largo - ini);
		cout<<"\""<<s<<"\"\n";
		child = cur_node->find(s);
		if(child == NULL){
			return NULL;
		}
	}//if... token final de largo valido
	
	return child;
}

void FilesTree::print() const{
	cout<<"FilesTree::print - inicio\n";
	if(base_path == NULL){
		cout<<"base_path: [NULL]\n";
	}
	else{
		cout<<"base_path: \""<<base_path<<"\"\n";
	}
	if(root == NULL){
		cout<<"[NULL]\n";
	}
	else{
		root->print(0, true);
	}
	cout<<"FilesTree::print - fin\n";
}

void FilesTree::loadDirectory(const char *path){

	if(path == NULL || strlen(path) == 0){
		cerr<<"FilesTree::loadDirectory - Error, Path invalido\n";
		return;
	}
	
	cout<<"FilesTree::loadDirectory - Inicio\n";
	
	if(root != NULL){
		delete root;
	}
	if(base_path != NULL){
		delete [] base_path;
	}
	
	cout<<"FilesTree::loadDirectory - Preparando Variables\n";
	
	root = new FileNode("/", true);
	base_path = new char[strlen(path) + 1];
	strcpy(base_path, path);
	
	cout<<"FilesTree::loadDirectory - Iniciando Recursion\n";
	
	root->loadDirectory(base_path);
	
	cout<<"FilesTree::loadDirectory - Fin\n";
	
}






















