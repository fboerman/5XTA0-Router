//Router assignment for course 5XTA0
//copyright Frank Boerman 0802910
//main cpp file

#include "Main.h"

using namespace std;

//globals
int ID, Nneighbors;	
ROUTER* myrouter;

template<typename T> T GetInfo(string message)//gets integer or string from input stream and checks for errors
{
	T result;
	while (1)//ask loop
	{
		//ask the message and take from inputstream
		cout << message << " ";
		cin >> result;
		//check the inputstream
		//if failed, reset the stream, else report back
		if (!cin || cin.fail())
		{
			cout << "Invalid input, please try again." << endl;
			cin.clear();
			cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		}
		else
		{
			return result;
		}
	}
}

NEIGHBOR* GetNeighbour(int id)//asks for a neighbour of given id from the user, returns a neighbor struct with user given details
{
	stringstream ss; //stream to easily concate integers and strings
	NEIGHBOR* Neigh;

	int N, D;
	cout << "Please specify the index(N) and distance/cost(D) of the neighbouring node(s)" << endl;
	//first id
	//create the message
	ss << "N" << id << ":";
	N = GetInfo<int>(ss.str());
	//reset stream and create new message
	ss.str("");
	ss.clear();
	//than distance
	ss << "D" << id << ":";
	D = GetInfo<int>(ss.str());
	//reset the stream for new use
	ss.str("");
	ss.clear();

	//create the new neighbour and return it
	Neigh = new NEIGHBOR;
	Neigh->name = N;
	Neigh->cost = D;

	return Neigh;
}

LSP* GetLSP() //asks for a LSP packet from user, returns LSP struct pointer
{
	LSP* package = new LSP;
	//get the info from user
	cout << "Provide the LSP:" << endl;
	package->creator = GetInfo<int>("Creator:");
	package->from = GetInfo<int>("From:");
	package->ttl = GetInfo<int>("TTL:");
	int n = GetInfo<int>("No. of neighbours in LSP:");
	//ask for all the neighbours
	for (int i = 0; i < n; i++)
	{
		package->AddNeighbour(*GetNeighbour(i+1));
	}

	return package;
}

void Forward(LSP* package)//generate and print the to be forwarded LSP package and destinations
{
	
	//list<NEIGHBOR*>* destination = new list<NEIGHBOR*>;//destination of the forwarded lsp
	NEIGHBOR to[MAXNEIGHBOURS+1];//+1 for the terminating signal

	//process  the forward request
	LSP pack = myrouter->forward(package, to);
	//check if the package needs to be forwarded at all
	if (to[0].name == -1)
	{
		cout << "No forwarding of package" << endl;
		return;
	}

	cout << "The LSP to be forwarded is:" << endl;
	pack.print();
	cout << "Going to:" << endl;
	//for (int i = 0; i < to->size(); i++)
	//{
	//	(*to)[i].print();
	//}
	int i = 0;
	while (1) //because the length of the array cant be predeterined, we run through it and break at the -1 -1 element
	{
		if (to[i].name != -1)
		{
			to[i].print();
			i++;
		}
		else
		{
			break;
		}
	}   
}

void ReceiveLSP()//receives and processes lsp package from user
{
	LSP* package = GetLSP();
	//add the lsp to the router database
	//myrouter->Add_LSP(package);
	if (myrouter->process(*package)) //process the package and check if routing table was updated, report this to user
	{
		cout << "Routing table was updated." << endl;
	}
	else
	{
		cout << "Routing table was not updated." << endl;
	}
	Forward(package);
}

void Test()
{
	ifstream* file = new ifstream;

	string fname = GetInfo<string>("Please specify the filename for scenario:");
	//fileformat:
	//per line 1 node declaration:
	//<node id>;<neighour id>:<distance>;<neighour id>:<distance>;...
	//read the file

	file->open(fname);
	if (file->fail()) //open the stream
	{
		cout << "Error reading file " << fname << endl;
		return;
	}
	myrouter->Test(file);
	file->close();
}

int main(int argc, char *argv[])//main function
{

	//intialization with input validation
	ID = GetInfo<int>("Router ID:");
	Nneighbors = GetInfo<int>("No. of neighbours:");

	cout << "Neighbors:" << endl;

	//create the router object
	myrouter = new ROUTER(ID);

	//ask for the number of neighbors specified
	for (int i = 0; i < Nneighbors; i++)
	{
		//ask for a new neighbor and add it to the router
		if (!myrouter->addNeighbor(*GetNeighbour(i + 1)))
		{
			cout << "Too many neighbours specified!" << endl;
			break;
		}
	}

	int choice;
	while (1) //the main loop for the menu
	{
		//print the menu
		cout << "Choose one of the commands:" << endl;
		cout << "\t1. Receive an LSP" << endl;
		cout << "\t2. Generate own LSP" << endl;
		cout << "\t3. Print neighbor list" << endl;
		cout << "\t4. Print routing table" << endl;
		cout << "\t5. TestBench" << endl;
		cout << "\t6. Exit program" << endl;
		//get the option
		choice = GetInfo<int>("\tInput your choice:");

		//check if choice is valid
		if (choice > 6 || choice < 1)
		{
			cout << "Invalid option, try again." << endl;
			continue;
		}

		switch (choice)
		{
		case 1://receive lsp (users gives one)
			ReceiveLSP();
			break;

		case 2://generate  lsp (router makes one from its table)
			Forward(NULL);
			break;

		case 3://print the current neighbours
			myrouter->print_neighbors();
			break;

		case 4://print the routing table
			myrouter->print_routes();
			break;

		case 5://debug option
			Test();
			break;

		case 6://exit the program
			return 1;
			break;
		}
	}
}