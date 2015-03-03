#include "stdafx.h"
/*##########################################################################
File name:main.cpp
Desc: test bed/main for the simulated annealing algorithm implementation
Author:girish.deshpande@utdallas.edu
Date:10-24-2012

##########################################################################*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <time.h>
#include "simAnneal.h"


double start_time;
double parse_time;
double exec_time_sa;
double end_time;
using namespace std;
string resFile;

int main(int argc, char * argv[])
{
	ofstream RFILE;

if(argc < 2)
{
    cout<<"================================================"<<endl;
    cout<<"USAGE: $>./a.out filename.are filename.net resultfile.txt"<<endl;
    cout<<"================================================"<<endl;
}
resFile.assign(argv[3]);

double start_time = clock();

	//create area LL
	fileParseArealist(argv[1]);
	//random cut
	generateFirstCut();
	//create netlist LL
	fileParseNetlist(argv[2]);
	//update netlist with partition info
	updateNetListLinks();
	//update area LL with nets
	updateAreaListWithNets();
	
	calculateCutset();
	//displayLinkedList('N');
	//displayLinkedList('A');
	//start SA 
	anneal();
	//--clean up everything
	deleteLinkedList('A');
	deleteLinkedList('N');

double end_time = clock();
double elapsed_time = (end_time - start_time) / CLOCKS_PER_SEC;
cout<<"==========================================="<<endl;
cout<<"TIME OF EXECUTION=["<<elapsed_time<<"] cycles OR ["<<((float)elapsed_time)/CLOCKS_PER_SEC<<"] seconds"<<endl;
cout<<"==========================================="<<endl;

//record execution time -- VERIFY THIS!
RFILE.open(resFile.data(),ofstream::app);
	if(!RFILE.is_open())
	{
	    cout<<"ERROR : could not open a RESULT file"<<endl;

	}
	else
	{
	    RFILE<<"======================================================="<<endl;
	    RFILE<<"TIME OF EXECUTION=["<<elapsed_time<<"] cycles OR ["<<((float)elapsed_time)/CLOCKS_PER_SEC<<"] seconds"<<endl;
	}
RFILE.close();
cout<<"------------End of program----------------"<<endl;

}


