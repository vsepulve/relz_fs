#include "dirList.h"

	dirList::dirList(){
		files = new vector<pair<string,int> >;
		directories = new vector<string>;
	}
	dirList::dirList(char* serial){
		// cout<<"constructor serial inicio \n";
		// cout<<"serial de ingreso = "<<serial<<"\n";
		files = new vector<pair<string,int> >;
		directories = new vector<string>;
		//rellenar las estructuras
		stringstream * ss = new stringstream();
		(*ss)<<serial;
		//parseamos el stream
		// cout<<"sacando datos de (*ss)\n";
		unsigned int nd;
		string separator;
		(*ss)>>nd;
		(*ss)>>separator;
		// cout<<"nd = "<<nd<<"\n";
		for(unsigned int i = 0 ; i< nd ; i++){
			string dir_name;
			(*ss)>>dir_name;
			(*ss)>>separator;
			// cout<<"dir name = "<<dir_name<<"\n";
			directories->push_back(dir_name);
		}
		unsigned int nf;
		(*ss)>>nf;
		(*ss)>>separator;
		// cout<<"nf = "<<nf<<"\n";
		for(unsigned int i = 0 ; i< nf ; i++){
			string f_name;
			unsigned int f_size;
			(*ss)>>f_name;
			(*ss)>>separator;
			(*ss)>>f_size;
			(*ss)>>separator;
			// cout<<"file name = "<<f_name<<", "<<"file size = "<<f_size<<" Bytes\n";
			files->push_back(pair<string,int>(f_name,f_size));
		}

	}
	dirList::~dirList(){
		files->clear();
		directories->clear();
		delete files;
		delete directories;

	}
	void dirList::add_file(pair<string,int> file){
		files->push_back(file);
	}
	void dirList::add_dir(string dir){
		directories->push_back(dir);
	}

	void dirList::print(ostream *out){
		// (*out)<<"Files ["<<files->size()<<"], Directories["<<directories->size()<<"]\n";
		// (*out)<<"numero de dirs = "<<directories->size()<<"\n";
		char * s_size = new char[16];
		for(unsigned int i = 0 ;i < directories->size() ; i++){
			(*out)<<"<DIR>\t + "<<(((*directories)[i]))<<"\n";
		}
		for(unsigned int i = 0 ;i < files->size() ; i++){
			unsigned int size = (unsigned int)(((*files)[i]).second);
			if(size < 1025){
				sprintf(s_size,"%d Bytes", size);
			}
			else if (size < (1024*1024)){
				sprintf(s_size,"%d KB", (size/1024));
			}
			else if (size < (1024*1024*1024)){
				sprintf(s_size,"%d MB", (size/(1024*1024)));

			}
			else{
				sprintf(s_size,"%d GB", (size/(1024*1024*1024)));
			}



			(*out)<<"<FILE>\t - "<<"["<<s_size<<"] \t > "<<(((*files)[i]).first)<<"\n";
		}
		delete[] s_size;
	}

	void dirList::serialize(char * ret){
		//escribir numero de directorios
		stringstream *ss = new stringstream();
		unsigned int number_of_d = directories->size();
		(*ss)<<number_of_d<<" , ";
		for(unsigned int i = 0 ;i < number_of_d ; i++){
			(*ss)<<(((*directories)[i]))<<" , ";
			// cout<<"\t\t Dir = "<<(((*directories)[i]))<<", length = ["<<(((*directories)[i])).size()<<"]\n";
		}
		unsigned int number_of_f = files->size();
		(*ss)<<number_of_f<<" , ";
		for(unsigned int i = 0 ;i < number_of_f ; i++){
			(*ss)<<(((*files)[i]).first)<<" , ";
			(*ss)<<(((*files)[i]).second)<<" , ";
			// cout<<"\t\t File = "<<(((*files)[i]).first)<<", length = ["<<(((*files)[i]).first).size()<<"],  size = ["<<(((*files)[i]).second)<<"]\n";
		}

		// int len = strlen((*ss).str().c_str());
		sprintf(ret,"%s",(*ss).str().c_str());
		// cout<<"creando el char *, ret = "<<ret<<"\n";
		// cout<<"largo de ss = "<<len<<"\n";
		(*ss).clear();
		delete ss;
	}

	int dirList::size(){
		//escribir numero de directorios
		stringstream *ss = new stringstream();
		unsigned int number_of_d = directories->size();
		(*ss)<<number_of_d<<" , ";
		for(unsigned int i = 0 ;i < number_of_d ; i++){
			(*ss)<<(((*directories)[i]))<<" , ";
			// cout<<"\t\t Dir = "<<(((*directories)[i]))<<", length = ["<<(((*directories)[i])).size()<<"]\n";
		}
		unsigned int number_of_f = files->size();
		(*ss)<<number_of_d<<" , ";
		for(unsigned int i = 0 ;i < number_of_f ; i++){
			(*ss)<<(((*files)[i]).first)<<" , ";
			(*ss)<<(((*files)[i]).second)<<" , ";
			// cout<<"\t\t File = "<<(((*files)[i]).first)<<", length = ["<<(((*files)[i]).first).size()<<"],  size = ["<<(((*files)[i]).second)<<"]\n";
		}

		int len = strlen((*ss).str().c_str());
		// cout<<"largo de ss = "<<len<<"\n";
		(*ss).clear();
		delete ss;
		return len;
	}
