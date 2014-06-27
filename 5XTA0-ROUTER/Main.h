//Router assignment for course 5XTA0
//copyright Frank Boerman 0802910
//main header file

#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <iostream>
#include <sstream>
#include "router.h"
#include <vector>
#include <algorithm>
#include <fstream>


void Debug(void);
unsigned int split(const std::string &txt, std::vector<std::string> &strs, char ch);
void Forward(LSP* package);
void ReceiveLSP(void);
LSP* GetLSP(void);
NEIGHBOR* GetNeighbour(int id);
template<typename T> T GetInfo(std::string message);
int main(int argc, char *argv[]);
void Test(void);

#endif