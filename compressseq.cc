#include <iostream>
#include <cstdlib>
#include <fstream>

using namespace std;

int WINDOWSIZE = 30;

class element { 
	public:
		element* prev;
		char kar; 
		element* next;
};//element

class list{
	public:
		element* entrance;
		element* exit;
};//list

void print(element* entrance){
	while (entrance != NULL){
		cout << entrance->kar << " ";
		entrance = entrance->next;
	}//while
	cout << endl;
}//print


void append(char character, element* & entrance, element* & exit){
	element* added = new element;
	added->prev = NULL;
	added->kar = character;
	added->next = NULL;
	if (entrance == NULL && exit == NULL){
		entrance = added;
		exit = added;
	}//if
	else{
		added->prev = exit;
		exit->next = added;
		exit = added;
	}//else
}//append

void deletefirst(element* & entrance, element* & exit){
	if (entrance != NULL){
		element* hulp = entrance;
		entrance = entrance->next;
		if (entrance == NULL)
			exit = NULL;
		delete hulp;
	}//if
}//deletefirst

element* findkar(char kar, int & index, element* entrance, element* exit){
	index = 0;
	element* hulp = entrance;
	while(hulp != NULL && hulp->kar != kar){
		hulp = hulp->next;
		index++;
	}//while
	return hulp;
}//findkar


void compress(){
	ifstream invoer;
	char kar;
	int windowlength = 0;
	element* entrance = NULL;
	element* exit = NULL;
	element* pointindex;
	element* iterator;
	int index = 0;
	bool substringfound = false;
	int len = 0;
	bool found;

	string file;
	cout << endl << "Voer een tekstfile in (bijv 'iets.txt'):" << endl;
	//cin >> file;
	file = "dblp.xml.50MB";
	invoer.open(file.c_str());
	//cout << endl << "Voer de grootte van de window in: " << endl;
	//cin >> WINDOWSIZE;
	WINDOWSIZE = 512;
	cout << endl << "Gecodeerde tekst: " << endl << endl;

	kar = invoer.get();

	while ( ! invoer.eof ()) {
		
		if(substringfound == false){
			pointindex = findkar(kar, index, entrance, exit);
			if (pointindex != NULL){
				substringfound = true;
				iterator = pointindex->next;
				len = 1;
			}//if
			else{
			  cout << "(0,0," << kar << ")" << endl;
			}
		}//if
		else{ 
			if(iterator != NULL && iterator->kar == kar){
				iterator = iterator->next;
				len++;
			}//if
			else{

				//search for possible longer substrings
				int newindex = index+1;
				element* zoek;
				element* zoekstart = exit;
				found = false;
				for(int i = 1; i < len; i++)
					zoekstart = zoekstart->prev;
				zoek = zoekstart;
				while(iterator != NULL && !found && iterator != zoekstart){
					if(iterator->kar == zoek->kar)
						zoek = zoek->next;
					else zoek = zoekstart;
					if(zoek == NULL){
						if (iterator->next->kar == kar){ 
							found = true; 
							iterator = iterator->next;
						}//if
						else{
							iterator = iterator->next; 
							newindex++; 
							zoek = zoekstart;
						}//else
					}//if
					else{
						newindex++;
						iterator = iterator->next;
					}//else
				}//while

				if(found){
					index = newindex;
					iterator = iterator->next;
					len++;
					substringfound = true;
				}//if
				else{
					cout << "(" << windowlength - index - len << "," << len << "," << kar << ")" << endl;
					substringfound = false;
					index = 0;
					len = 0;
				}//else
			}//else
		}//else
			
		
		//update the list add new kar and possibly remove oldest kar
		
		append(kar, entrance, exit);
		windowlength++;
		if (windowlength > WINDOWSIZE){
			deletefirst(entrance, exit);
			windowlength--;
			if(substringfound) index--;
		}//if
		//print(entrance);

		kar = invoer.get();
	}//while
	cout << endl;
}//compress

void decompress(){
	ifstream invoer;
	char kar = -1;
	int windowlength = 0;
	int offset = 0, length = 0, ascii = 0;
	element* entrance = NULL;
	element* exit = NULL;

	string file;
	cout << endl << "Voer een tekstfile in (bijv 'iets.txt'):" << endl;
	cin >> file;
	invoer.open(file.c_str());
	cout << endl << "Voer de grootte van de window in: " << endl;
	cin >> WINDOWSIZE;
	cout << endl << "Gedecodeerde tekst: " << endl << endl;

	kar = invoer.get();
	while ( ! invoer.eof () && kar != '\n') {

		//getting the input values of a triple
		kar = invoer.get();
		while (kar != ','){
			offset = offset * 10 + kar - '0';
			kar = invoer.get();
		}//while
		kar = invoer.get();
		while (kar != ','){
			length = length * 10 + kar - '0';
			kar = invoer.get();
		}//while
		kar = invoer.get();
		while (kar != ')' && kar != '-'){
			ascii = ascii * 10 + kar - '0';
			kar = invoer.get();
		}//while

		//processing the input values
		if(offset != 0){
			element* hulp = exit;
			for (int i = 1; i < offset; i++){ 
				if (hulp != NULL) hulp = hulp->prev; 
			}//for
			for (int i = 0; i < length; i++){
				if(hulp == NULL) {
					cout << endl << "Error: Windowsize too small! Aborting..." << endl;
					return;
				}//if
				append(hulp->kar, entrance, exit);
				windowlength++;
				cout << hulp->kar;
				hulp = hulp->next;
				if (windowlength > WINDOWSIZE){
					deletefirst(entrance, exit);
					windowlength--;
				}//if
			}//for	
		}//if
		cout << (char)ascii; 
		append((char)ascii, entrance, exit);
		windowlength++;
		if (windowlength > WINDOWSIZE){
			deletefirst(entrance, exit);
			windowlength--;
		}//if

		//resetting input values for next triple
		while (kar != '(' && kar != '\n') kar = invoer.get();
		length = 0; offset = 0; ascii = 0;
	}//while
	cout << endl << endl;
}//decompress

int main(){
  //char keuze;
	// cout << endl << "(C)oderen of (D)ecoderen" << endl;
	// cin >> keuze;	
	// if (keuze == 'C' || keuze == 'c') compress();
	// else if (keuze == 'D' || keuze == 'd') decompress();
	compress();
	return 1;
}//main
