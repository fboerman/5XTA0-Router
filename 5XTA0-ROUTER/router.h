/*
Copyright Frank Boerman 2014
template made by George Exarchakos
*/

#ifndef ROUTER_H
#define ROUTER_H

#include <string>
#include <list>
#include <limits>
#include <vector>
#include <fstream>
#include <algorithm>

static const int MAXNEIGHBOURS = 20;


struct NEIGHBOR {
	int name;
	int cost;
	
	/** Prints in std::cout the NEIGHBOR. The output format is
		a <neighbor name>:<distance cost> tuple. */
	void print() {
		std::cout << name << ":" << cost << std::endl;
	}

};

struct LSP {
	int creator, from, ttl;
	NEIGHBOR neighbors[MAXNEIGHBOURS];
	//std::list<NEIGHBOR> neighbors = std::list<NEIGHBOR>();

	//constructor
	LSP()
	{
		//init the neighbors with -1 entries
		for (int i = 0; i < MAXNEIGHBOURS; i++)
		{
			neighbors[i].name = -1;
			neighbors[i].cost = -1;
		}
	}

	/** Prints in std::cout the LSP. The output format is one line per attribute and
	the neighbors as a list of <neighbor name>:<distance cost> tuples. */
	void print() {
		std::cout << "Creator: " << creator << std::endl;
		std::cout << "From: " << from << std::endl;
		std::cout << "TTL: " << ttl << std::endl;
		std::cout << "Neighbours: " << std::endl;

		for (int i = 0; i < MAXNEIGHBOURS; i++)
		{
			if (neighbors[i].name == -1)
			{
				break;
			}
			neighbors[i].print();
		}
	}

	void AddNeighbour(NEIGHBOR n) //ads a neighbor to the array, with neighbor struct as input argument
	{
		for (int i = 0; i < MAXNEIGHBOURS; i++)
		{
			if (neighbors[i].name == -1)
			{
				neighbors[i].name = n.name;
				neighbors[i].cost = n.cost;
				break;
			}
		}
	}

	void AddNeighbour(int name, int cost) //adds neighbor to the array, with name and cost as input arguments
	{
		NEIGHBOR n;
		n.name = name;
		n.cost = cost;
		
		AddNeighbour(n);
	}

	LSP* Clone() //clones itself into a new object
	{
		LSP* clone = new LSP;

		clone->creator = this->creator;
		clone->from = this->from;
		clone->ttl = this->ttl;
		for (int i = 0; i < MAXNEIGHBOURS; i++)
		{
			if (neighbors[i].name == -1)
			{
				break;
			}

			clone->neighbors[i] = neighbors[i];
		}

		return clone;
	}
};

unsigned int split(const std::string &txt, std::vector<std::string> &strs, char ch)//splits string on character
{
	unsigned int pos = txt.find(ch);
	unsigned int initialPos = 0;
	strs.clear();

	// Decompose statement
	while (pos != std::string::npos) {
		strs.push_back(txt.substr(initialPos, pos - initialPos));
		initialPos = pos + 1;

		pos = txt.find(ch, initialPos);
	}

	// Add the last one
	strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos));

	return strs.size();
}

struct ROUTE { //entry from the routint table
	int dest, dist, next;

	void print()
	{
		std::cout << dest << ":" << dist << ":" << next << std::endl;
	}
};

class ROUTER {
public:

	//constructor
	ROUTER(int ID)
	{
		_ID = ID;
		_neighbours = std::list<NEIGHBOR>();
		ROUTE* r = new ROUTE;
		r->dest = r->next = ID;
		r->dist = 0;
		_table->push_back(r);
	}

	//deconstructor
	~ROUTER(void)
	{
		delete &_neighbours;
	}


	/** This member function takes an LSP packet and updates the routing table,
	if necessary. It returns true if at least one change to the the routing
	table was made. It returns false if no updates happened. */
	bool ROUTER::process(LSP packet) {
		//The Forward Search algorithm:
		//put itself on the confirmed list
		//initialize tentative as empty
		//first save the LSP packet
		//after it is checked that the list is initialized (has a member of the router itself)
		if (_receivedlsp.size() == 0)
		{
			//if not than initialize it
			InitLSPList();
		}
		//add the package
		//create a new copy of the package to avoid memory problems
		Add_LSP(packet.Clone());
	

		std::list<ROUTE*>*  confirmed = new std::list<ROUTE*>, tentative = std::list<ROUTE*>();
		ROUTE* NEXT = new ROUTE; NEXT->dest = NEXT->next = _ID; NEXT->dist = 0; confirmed->push_back(NEXT);

		//iterate through the tentative list and execute the forward search algorithm
		//first take the router itself as the to be examined node

		do
		{
			//check if there is a LSP for the current selected node
			LSP* node_packet = Select_LSP(NEXT->dest);
			if (node_packet != nullptr)//if no lsp is received the algorithm wil simply continue selecting the next one
			{
				//iterate through its neighbours
				for (int i = 0; i < MAXNEIGHBOURS; i++)
				{
					//check if the end of the neighbor array is reached before 20
					if (node_packet->neighbors[i].name == -1)
					{
						//no more neighbors so stop the loop
						break;
					}

					ROUTE* tent_item = FindRoute(node_packet->neighbors[i].name, &tentative); //search the item on tentative list
					ROUTE* current_node_route = FindRoute(node_packet->from, _table); //get the route to the current node
					//calculate the cost to get to current selected neighbour
					//cost = distance to current node plus distance from current node to the selected neighbour of that node
					int cost = current_node_route->dist + node_packet->neighbors[i].cost;
					if (FindRoute(node_packet->neighbors[i].name, confirmed) == nullptr && tent_item == nullptr)//not on one of the lists
					{
						//create the route object
						//and fill it
						//with the info
						ROUTE* ro = new ROUTE;
						ro->dest = node_packet->neighbors[i].name;
						ro->dist = cost;
						//if the next is the router itself (aka the first iteration)
						//than the next item should be the neighbour
						//else it should be the current selected item
						if (NEXT->dest == _ID)
						{
							ro->next = ro->dest;
						}
						else
						{
							ro->next = current_node_route->next;
						}
						//push it on the tentative list
						tentative.push_back(ro);
					}
					else if (tent_item != nullptr)//if its on tentative
					{
						//if the cost of this route is less then is currently noted on the tentative list
						if (cost < tent_item->dist)
						{
							//update the route to this route
							tent_item->dest = node_packet->neighbors[i].name;
							tent_item->dist = cost;
							//if the next is the router itself (aka the first iteration)
							//than the next item should be the neighbour
							//else it should be the current selected item
							if (NEXT->dest == _ID)
							{
								tent_item->next = tent_item->dest;
							}
							else
							{
								tent_item->next = current_node_route->next;
							}
						}

					}
				}
			}

			//search for the lowest cost in the tentative list, to move that to the confirmed list
			//reset the NEXT
			NEXT = new ROUTE;
			NEXT->dist = std::numeric_limits<int>::max();
			for (std::list<ROUTE*>::iterator it = tentative.begin(), end = tentative.end(); it != end; ++it)//iterate throught the tentative list
			{
				if ((*it)->dist < NEXT->dist) //if the distance is shorter, select this one
				{
					//select it
					NEXT = (*it);
				}
			}
			//because of my implementation the tentative list can be already empty, if so than the NEXT will be empty because nothing is selected in the previous for loop
			//if this is the case do not move this emtpy entry to confirmed list
			if (tentative.size() != 0)
			{
				//move this selected to the confirmed list
				tentative.remove(NEXT);
				confirmed->push_back(NEXT);
			}
		} while (tentative.size() != 0 || Select_LSP(NEXT->dest) != nullptr);//while the tentative list is not empty, continue with the lowest cost selected from tentative

		//forward search algorithm is done
		//the comfirmed list is the new routing table
		//crossreference confirmed list with table to see if any changes were made
		bool changes = false;
		for (std::list<ROUTE*>::iterator it = confirmed->begin(), end = confirmed->end(); it != end; ++it)
		{
			//try to find it in the old routing table
			ROUTE* old = FindRoute((*it)->dest, _table);
			if (old == nullptr)//if its not on the old routing list than its new so a change has been made
			{
				changes = true;
				break;
			}
			else
			{
				if (old->dist != (*it)->dist || old->next != (*it)->next)
				{
					changes = true;
					break;
				}
			}

		}

		//delete the current table and assign the confirmed list to it
		_table->clear();
		delete _table;
		_table = confirmed;

		return changes;
	}


	/** This member funtion takes an LSP packet and an empty array of NEGIHBORs (to).
	It returns the LSP that needs to be forwarded. Based on the LSP,
	the empty array (to) of NEIGHBORs is filled with the NEIGHBORs of
	this ROUTER that the packet needs to be forwarded to.
	If packet is NULL, the function should return a LSP generated by this
	ROUTER.*/
	LSP ROUTER::forward(LSP* packet, NEIGHBOR *to) {
		//check if packet for router itself is asked (NULL)
		//if so create a packet for own router
		if (packet == NULL)
		{
			//allocate new object
			//and fill in info
			packet = new LSP();
			packet->creator = packet->from = _ID;
			packet->ttl = 3;

			for (std::list<NEIGHBOR>::iterator it = _neighbours.begin(), end = _neighbours.end(); it != end; ++it)
			{
				packet->AddNeighbour(*it);
			}
		}
		else
		{
			//reduce the ttl
			packet->ttl--;
			//and update the from value to our own name
			packet->from = _ID;
		}
		//check if the ttl is still valid
		if (packet->ttl <= 0)
		{
			//return an empty *to array
			to[0].name = -1;
			to[0].cost = -1;
			//skip the adding below
			return *packet;
		}
		//send it to all our neighbors except from the one who send it to us and the one who created it
		//add these to the to array
		int i = 0;
		for (std::list<NEIGHBOR>::iterator it = _neighbours.begin(), end = _neighbours.end(); it != end; it++)
		{
			if (it->name != packet->from && it->name != packet->creator)
			{
				//add it to the vector
				to[i] = (*it);
				i++;
			}

		}

		//put an -1 entry at the end of the to array to signal the end of it
		to[i].name = -1;
		to[i].cost = -1;

		return *packet; // Modify as needed
	}

	/** Prints in std::cout the list of NEIGHBORs of this ROUTER. The output format is
	a list of <neighbor name>:<distance cost> tuples. */
	void ROUTER::print_neighbors() {
		std::cout << "Neighbour list of router " << _ID << std::endl;

		//check if there are any neighbours
		if (_neighbours.size() > 0)
		{
			for (std::list<NEIGHBOR>::iterator it = _neighbours.begin(), end = _neighbours.end(); it != end; ++it) //iterate through the list and invoke print on all the members
			{
				it->print();
			}
		}
		else
		{
			std::cout << "List is empty!" << std::endl;
		}
	}

	/** Prints in std::cout the routing table of this ROUTER. The output format is
	a list of <destination node name>:<distance cost>:<next node name>
	triplets. */
	void ROUTER::print_routes() {
		std::cout << "Routing table of router " << _ID << std::endl;
		if (_table->size() > 0)//check if table is filled, than iterate through it or report emptiness
		{
			for (std::list<ROUTE*>::iterator it = _table->begin(), end = _table->end(); it != end; ++it)
			{
				(*it)->print();
			}
		}
		else
		{
			std::cout << "Table is empty!" << std::endl;
		}
	}

	/*reads from file with format:
	every line is an lsp
	<id>;neighbor_id:neighbor_distance;neighbor_id:neighbor_distance;etc
	*/
	void ROUTER::Test(std::ifstream* file)//reads testscenario from file and executes it
	{
		std::string line;

		std::cout << "Parsing file ..." << std::endl;
		while (getline(*file, line))//read line per line
		{
			//split the line at ;
			std::vector<std::string> v;
			split(line, v, ';');
			//create and fill in the lsp
			LSP* package = new LSP;
			package->creator = package->from = atoi(v[0].c_str());
			package->ttl = 3;
			//iterate through the neighbours and add them
			for (int i = 1; i < v.size(); i++)//start at index 1 to skip the id
			{
				std::vector<std::string> l;
				split(v[i], l, ':');
				package->AddNeighbour(atoi(l[0].c_str()), atoi(l[1].c_str()));//convert string to integer and give it to the addneighbor routine
			}
			//process and forward package
			//by forwarding the package the info is printing so user can check
			//Add_LSP(package);
			process(*package);

			//copy from main.cpp function Forward
			NEIGHBOR to[MAXNEIGHBOURS + 1];//+1 for the terminating signal

			//process the package
			std::cout << "The LSP to be forwarded is:" << std::endl;
			forward(package, to).print();
			std::cout << "Going to:" << std::endl;
			//print the to array
			int i = 0;
			while (1)
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

		std::cout << "Done parsing and processing file!" << std::endl;
		print_routes(); //print the routing table to show the results
	}

	/** This member function takes an NEIGHBOR n and adds it to the
	neighbor list of this ROUTER. If the NEGIHBOR n already exists
	or the neighbor list is full, the function return false. Otherwise,
	it returns true. */
	bool ROUTER::addNeighbor(NEIGHBOR n) {
		//check the size and if exists already
		if (_neighbours.size() == MAXNEIGHBOURS || FindNeighbourID(n.name))
		{
			return false;
		}
		//if its ok, put the neighbour at the end of the list
		_neighbours.push_back(n);
		//and update the routing table with this neighbour
		//create a corresponding ROUTE object and push it on the list
		ROUTE* r = new ROUTE;
		r->dest = r->next = n.name;
		r->dist = n.cost;
		_table->push_back(r);
		return true;
	}

private:

	//help functions for the public functions

	//initializes the lsp list with a lsp of the router itself with info of its neighbours
	void ROUTER::InitLSPList()
	{
		LSP* packet = new LSP;
		packet->creator = packet->from = _ID;
		packet->ttl = 0;

		for (std::list<NEIGHBOR>::iterator it = _neighbours.begin(), end = _neighbours.end(); it != end; ++it)
		{
			packet->AddNeighbour(*it);
		}

		_receivedlsp.push_back(packet);
	}


	//check if given id is already in neighbor list
	//returns true if found, false otherwise
	bool ROUTER::FindNeighbourID(int ID)
	{
		//loop through the neighbours list using iterator and check the id
		for (std::list<NEIGHBOR>::iterator it = _neighbours.begin(), end = _neighbours.end(); it != end; ++it)
		{
			if (it->name == ID)
			{
				return true;
			}
		}

		//loop ended so id not found
		return false;
	}


	void ROUTER::Add_LSP(LSP* packet) //adds give LSP to received list, replaces old one if already received with given id.
	{
		//search for existing ones, destroys and replaces lsp if existing one found, otherwist just push it on the list
		std::list<LSP*>::iterator it = _receivedlsp.begin(), end = _receivedlsp.end();
		while (it != end)
		{
			if ((*it)->creator == packet->creator) //existing one found, delete it
			{
				std::list<LSP*>::iterator obsolete = it;
				it++;
				_receivedlsp.erase(obsolete);
			}
			else
			{
				it++;
			}
		}
		//possible existing one is deleted now, so now add the new one
		_receivedlsp.push_back(packet);
	}

	LSP* ROUTER::Select_LSP(int ID) //searches and returns (if found, otherwise nullptr) the lsp packet from received lsp list with given creator id
	{
		for (std::list<LSP*>::iterator it = _receivedlsp.begin(), end = _receivedlsp.end(); it != end; ++it)//iterate through list
		{
			if ((*it)->creator == ID)//check if the id has been found
			{
				return (*it);//if so return the pointer to object
			}
		}

		return nullptr;
	}

	ROUTE* ROUTER::FindRoute(int ID, std::list<ROUTE*>* list) //find the route with to given id from an routingtable, returns nullptr if not found
	{
		for (std::list<ROUTE*>::iterator it = list->begin(), end = list->end(); it != end; ++it)//iterate through list
		{
			if ((*it)->dest == ID)//check if the id has been found
			{
				return (*it);//if so return the pointer to object
			}
		}
		//if not found than return a nullpointer
		return nullptr;
	}

//private values
	int _ID; //its own id
	std::list<NEIGHBOR> _neighbours;
	std::list<ROUTE*>* _table =  new std::list<ROUTE*>;
	std::list<LSP*> _receivedlsp = std::list<LSP*>();
};

#endif