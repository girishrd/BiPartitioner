#include "stdafx.h"
/*###########################################################
File name:simAnneal.cpp
Desc: Methods for file parsing and creating required data structures.
	  Also includes the SA implementation
Author:girish.deshpande@utdallas.edu
Date:Oct-2012
############################################################*/
#include "simAnneal.h"
#include<iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <math.h>
#include <limits>
#include <cstdlib>
//#define DEBUG

using namespace std;

//globals -- verify init temp before running
nodeNetlist * startNodePtrNet;
nodeNetlist * endNodePtrNet;
nodeAreaList * startNodePtrArea;
nodeAreaList * endNodePtrArea;
unsigned long int NO_OF_NODES;
unsigned long TOTAL_AREA=0.0;
unsigned int NET_NUMBER=0;
int ALWAYS_ASSIGN;
int CUTSET;
long RANGE_LOWER,RANGE_UPPER,LEFT_AREA,RIGHT_AREA,LOCAL_LEFT_AREA,LOCAL_RIGHT_AREA;
enum nodeAssignScheme {UNIFORM,LEFT_ONLY,RIGHT_ONLY};
float RATIO_CUT=0.0;
float DELTA=0.0;
unsigned long NODE_NUM;
int MOVEVALID=1;

//==========================

//===ANNEALING PARAMETERS====
//=============================
float INIT_TEMP=10;
float GAMMA =0.1;
float FINAL_TEMP=1.0;

//===============================

//--METHODS
/*=================================================================
Function to parse the netlist file
//=================================================================*/
void fileParseNetlist(char fname[])
{

string  FILEDATA;
ifstream NETLISTFILE;
initNetListLinkedList();
NETLISTFILE.open(fname);
if(!NETLISTFILE.is_open())
    {
        cout<<"ERROR:The file "<<fname<<"does not exist or invalid path specified!"<<endl;
        exit(1);
    }
for(int i=0;i<LINES_TO_BE_SKIPPED;i++)
    getline(NETLISTFILE,FILEDATA);

FILEDATA.clear();
while(NETLISTFILE.good())
{
	getline(NETLISTFILE,FILEDATA);
	if(NETLISTFILE.eof())
	break;
	addNewNetListNode(FILEDATA);
}
NETLISTFILE.close();
#ifdef DEBUG
cout<<"LOG:startnodeptr->nodeName="<<startNodePtrNet->nodeName<<endl;
cout<<"LOGendnodeptr->nodeName="<<endNodePtrNet->nodeName<<endl;
#endif

}
/*=================================================================
Function to parse the *.are file and create the AREA list
//=================================================================*/

void fileParseArealist(char fname[])
{
string  AREADATA;
ifstream AREAFILE;
initAreaLinkedList();
AREAFILE.open(fname);
if(!AREAFILE.is_open())
    {
        cout<<"ERROR:"<<fname<<"does not exist or invalid path specified!"<<endl;
        exit(1);
    }
AREADATA.clear();

while(AREAFILE.good())
{
     getline(AREAFILE,AREADATA);
	 //cout<<"AREADATA.data()="<<AREADATA.data()<<endl;
     addNewAreaNode(AREADATA);
     if(AREAFILE.eof())
     break;
}
AREAFILE.close();
}


/*=================================================================
Function to display lists
//=================================================================*/
void displayLinkedList(char ch)
{
nodeAreaList * tempA;
nodeNetlist  * tempN;

if (ch=='N')
{
    if(startNodePtrNet==NULL)
    {
      cout<<"No Netlist list exists.Exiting..."<<endl;
      exit(1);
    }
    else
    {
        tempN=startNodePtrNet;
        cout<<"=================================="<<endl;
        cout<<"The current(Netlist)linked list is:"<<endl;
        cout<<"=================================="<<endl;
        while(tempN)
        {
			cout<<"Node=["<<tempN->nodeName.data()<<"] of TYPE=["<<tempN->nodeType<<"] belongs to net:["<<tempN->netNumber<<"] "<<endl;
            //cout<<"A="<<tempN->nodeArea<<"   "<<"T="<<tempN->nodeType<<endl;
			tempN=tempN->nextNode;
        }
        cout<<"TOTAL NO OF NETS="<<NET_NUMBER<<endl;
        cout<<"TOTAL AREA="<<TOTAL_AREA<<endl;
        cout<<"=================================="<<endl;
    }

}
else if(ch=='A')
{
    if(startNodePtrArea==NULL)
    {
      cout<<"No Area list exists.Exiting..."<<endl;
      exit(1);
    }
    else
    {
        tempA=startNodePtrArea;
        cout<<"=================================="<<endl;
        cout<<"The current (Area) linked list is:"<<endl;
        cout<<"=================================="<<endl;
        while(tempA)
        {
            cout<<"Node=["<<tempA->nodeName.data()<<"] AREA=["<<tempA->nodeArea<<"] TYPE=["<<tempA->nodeType<<"]"<<endl;
            cout<<"belongs to the nets:"<<endl;
           
			for (unsigned int i=0; i < tempA->listOfNets.size(); i++)
                cout<<tempA->listOfNets[i] <<"-->";
                cout<<endl;
			
            tempA=tempA->nextNode;
        }
        cout<<"=================================="<<endl;
       // cout<<"Last Node=["<<tempA->nodeName.data()<<"] AREA=["<<tempA->nodeArea<<"] TYPE=["<<tempA->nodeType<<"]"<<endl;
    }

}
}
/*=================================================================
Function to clean up after...
//=================================================================*/
void deleteLinkedList(char ch)
{
if(ch=='N')
{
    nodeNetlist * temp = startNodePtrNet;
    while(temp != NULL)
        {
            temp = temp->nextNode;
            delete startNodePtrNet;
            startNodePtrNet = temp;
        }
        cout<<"NOTE: Cleaned up the NETLIST linked list"<<endl;
}
else if(ch=='A')
{
    nodeAreaList * temp = startNodePtrArea;
    while(temp != NULL)
        {
            temp = temp->nextNode;
            delete startNodePtrArea;
            startNodePtrArea = temp;
        }
    cout<<"NOTE: Cleaned up the AREALIST list"<<endl;

}

}
/*=================================================================
Function to create the list for the netlist data
//=================================================================*/
void initNetListLinkedList()
{
nodeNetlist * startNode;

startNode = new nodeNetlist;
if(!startNode)
{
    cout<<"Memory allocation failure! exiting..."<<endl;
    exit(1);
}

startNode->nodeName="startNetlist";
//startNode->nodeArea=0.0;
startNode->nodeType='R';
startNode->netNumber=0;
startNode->nextNode=NULL;
startNodePtrNet=startNode;
endNodePtrNet=startNode;
//#ifdef DEBUG
cout<<"Initalized linked list(NETS)..."<<endl;
//#endif
}

/*=================================================================
Function to add a new node in the list ( append)
//=================================================================*/
void addNewNetListNode(const std::string& FILEDATA)
{
//node *temp;
nodeNetlist * newNode ;
string DATA=FILEDATA;
string name;
size_t aPos,sPos,pPos,spacePos;

 sPos=DATA.find_first_of("s");
 if(sPos!=string::npos)
 {

     aPos=FILEDATA.find_first_of("a");
     if(aPos!=string::npos) //of the type aX sX
     {
        spacePos=DATA.find_first_of(" ");
        if(spacePos!=string::npos)
        {
            name=DATA.substr(aPos,spacePos);
            NET_NUMBER++;

        }
     }
     else //of the type pX sX
     {
        // cout<<"found p1 s1"<<endl;
        pPos=DATA.find_first_of("p");
        spacePos=DATA.find_first_of(" ");
        if(pPos!=string::npos)
        {
            if(spacePos!=string::npos)
            {
              name=DATA.substr(pPos,spacePos);
              NET_NUMBER++;
            }
        }

     }
 }
 //of the type aX 1
 aPos=FILEDATA.find_first_of("a");
 spacePos=FILEDATA.find_first_of(" ");
 if(aPos!=string::npos)
 {
    if(spacePos!=string::npos)
    {
               name=DATA.substr(aPos,spacePos);

    }
 }
 else //of teh type px 1
 {
     pPos=DATA.find_first_of("p");
      if(pPos!=string::npos)
      {
           spacePos=FILEDATA.find_first_of(" ");
           name=DATA.substr(pPos,spacePos);
      }
 }
#ifdef DEBUG
cout<<"LOG:The node name is"<<name.data()<<endl;
cout<<"LOG:The curr NET NO="<<NET_NUMBER<<endl;
#endif
if(startNodePtrNet!=NULL)
    {
       // temp = endNodePtrNet;

        newNode= new nodeNetlist;
        if(!newNode)
        {
            cout<<"Memory allocation failure! exiting..."<<endl;
            exit(1);
        }
        newNode->nodeName=name.data();
       // newNode->nodeArea=0.0;
        newNode->nodeType='U';
        newNode->netNumber=NET_NUMBER;
        newNode->nextNode=NULL;
        endNodePtrNet->nextNode=newNode;
        endNodePtrNet=newNode;
        #ifdef DEBUG
         cout<<"LOG:endnodeptr->nodeName="<<endNodePtrNet->nodeName<<endl;
         cout<<"added new node:Name="<<endNodePtrNet->nodeName.data()<<"Type="<<endNodePtrNet->nodeType<<endl;
		 #endif
    }
}

/*=================================================================
Function to add a new area node to the area list
//=================================================================*/
void addNewAreaNode(const std::string& AREADATA)
{

unsigned long area=0;
string strtempName;
string strtempArea;
string DATA=AREADATA;
//node *temp;
nodeAreaList * newNode ;
size_t posSpace,posAlpha;
//extract area of node
posSpace=DATA.find_first_of(" ");
if(posSpace!=string::npos)
{
    strtempArea=DATA.substr(posSpace+1);
    area= atoi(strtempArea.c_str());
}

//extract node name
posAlpha=DATA.find_first_of("ap");
if(posAlpha!=string::npos)
{
    strtempName=DATA.substr(posAlpha,posSpace);
  
}
if(startNodePtrArea!=NULL)
    {
        //temp = endNodePtrArea;
        newNode= new nodeAreaList;
        if(!newNode)
        {
            cout<<"ERROR:Out of memory"<<endl;
            exit(1);
        }
        newNode->nodeName=strtempName.data();
        newNode->nodeArea=area;
        newNode->nodeType="U";
        newNode->nextNode=NULL;
        endNodePtrArea->nextNode=newNode;
        endNodePtrArea=newNode;
        NO_OF_NODES++;
        #ifdef DEBUG
        cout<<"Node # :"<<NO_OF_NODES<<"last node added="<<endNodePtrArea->nodeName<<endl;
        #endif
    }
}
/*=================================================================
Function to create a new area list
//=================================================================*/
void initAreaLinkedList()
{
nodeAreaList * startNode;
startNode = new nodeAreaList;
if(!startNode)
{
    cout<<"ERROR:Out of memory"<<endl;
    exit(1);
}
startNode->nodeName="startAreaList";
startNode->nodeArea=0.0;
startNode->nodeType="U";
startNode->nextNode=NULL;
startNodePtrArea=startNode;
endNodePtrArea=startNode;
//#ifdef DEBUG
cout<<"Initalized linked list (AREALIST)..."<<endl;
}
/*========================================================================
Function to generate a random cut
========================================================================*/
void generateFirstCut()
{
    if(!startNodePtrArea)
    {
        cout<<"ERROR: The area linked list does not exist!"<<endl;
        exit(1);
    }
    else
    {   nodeAreaList * temp = startNodePtrArea;
        while(temp->nextNode)
        {
                TOTAL_AREA+=temp->nodeArea;
                #ifdef DEBUG
                cout<<"LOG:current circuit area="<<TOTAL_AREA<<endl;
                #endif
                temp=temp->nextNode;
        }
        #ifdef DEBUG
        cout<<"==============================================="<<endl;
        cout<<"LOG: Total area="<<TOTAL_AREA<<endl;
        #endif
        RANGE_LOWER=(TOTAL_AREA/2)-(AREA_TOLERANCE*(TOTAL_AREA/2));
        RANGE_UPPER=(TOTAL_AREA/2)+(AREA_TOLERANCE*(TOTAL_AREA/2));
        #ifdef DEBUG
        cout<<"The range for the area distribution is:=["<<RANGE_LOWER<<"] and ["<<RANGE_UPPER<<"]"<<endl;
        cout<<"==============================================="<<endl;
        #endif
    }
    nodeAreaList * temp=startNodePtrArea;
    long nodeCount=1.0;
    ALWAYS_ASSIGN=UNIFORM;
    while(temp!=NULL)
    {
        if((nodeCount%2==0) && (ALWAYS_ASSIGN==UNIFORM))
        {
            {
                temp->nodeType.assign("L");
                LEFT_AREA+=temp->nodeArea;
                nodeCount++;
                temp=temp->nextNode;
                if((RANGE_LOWER<=LEFT_AREA) && (LEFT_AREA<=RANGE_UPPER))
                {
                    ALWAYS_ASSIGN=RIGHT_ONLY;
                    #ifdef DEBUG
                    cout<<"==========================================================="<<endl;
                    cout<<"LOG:The left area exceeded lower bound=["<<RANGE_LOWER<<"]<=["<<LEFT_AREA<<"]<=["<<RANGE_UPPER<<"]"<<endl;
                    cout<<"LOG:Assigned"<<"["<<(nodeCount/2)<<"]"<<"nodes to the LEFT"<<endl;
                    cout<<"LOG:Will assign"<<"["<<NO_OF_NODES-(nodeCount/2)<<"]"<<"nodes to the RIGHT"<<endl;
                    cout<<"==========================================================="<<endl;
                    #endif
                }

            }
        }
        else if((nodeCount%2!=0) &&(ALWAYS_ASSIGN==UNIFORM))
        {
            if(ALWAYS_ASSIGN==UNIFORM)
            {
                temp->nodeType.assign("R");
                RIGHT_AREA+=temp->nodeArea;
                nodeCount++;
                temp=temp->nextNode;
                if((RANGE_LOWER<=LEFT_AREA) && (LEFT_AREA<=RANGE_UPPER))
                {
                    ALWAYS_ASSIGN=RIGHT_ONLY;
                    #ifdef DEBUG
                    cout<<"====================================================================================="<<endl;
                    cout<<"LOG:The right area exceeded lower bound=["<<RANGE_LOWER<<"]<=["<<RIGHT_AREA<<"]<=["<<RANGE_UPPER<<"]"<<endl;
                    cout<<"LOG:Assigned ["<<(nodeCount/2)<<"] nodes to the RIGHT"<<endl;
                    cout<<"LOG:Will assign"<<"["<<NO_OF_NODES-(nodeCount/2)<<"]"<<"nodes to the LEFT"<<endl;
                    cout<<"====================================================================================="<<endl;
                    #endif
                }
            }
        }

        if(ALWAYS_ASSIGN==LEFT_ONLY)
        {
            temp->nodeType.assign("L");
            LEFT_AREA+=temp->nodeArea;
            nodeCount++;
            temp=temp->nextNode;

        }
        else if(ALWAYS_ASSIGN==RIGHT_ONLY)
        {
            temp->nodeType.assign("R");
            RIGHT_AREA+=temp->nodeArea;
            nodeCount++;
            temp=temp->nextNode;
         }

    }//while ends
}
/*=========================================================================================
function to update the Netlist LL with the partition type ( L,R)
==========================================================================================*/
void updateNetListLinks()
{
unsigned long  instance=0.0;
    if((startNodePtrArea==NULL) || (startNodePtrNet==NULL))
    {
        cout<<"ERROR:The AREA/NET linked list does not exist!"<<endl;
        exit(1);

    }
    else //update the netlist with L/R
    {
        #ifdef DEBUG
        cout<<"------------------------------------"<<endl;
        cout<<"Updating NETLIST list...please wait"<<endl;
        cout<<"------------------------------------"<<endl;
        #endif
        nodeAreaList *tempA=startNodePtrArea;
        //tempA=startNodePtrArea->nextNode;
        nodeNetlist *tempN=startNodePtrNet;
		while(tempA)
        {
            tempN=startNodePtrNet;
            instance=0;
            while(tempN)
            {
                if(strcmp(tempA->nodeName.data(),tempN->nodeName.data())==0)
                {

                    tempN->nodeType.clear();
                    tempN->nodeType.assign(tempA->nodeType.data());
                    instance ++;
                }

                tempN=tempN->nextNode;
            }//whiel tempN is not null
            tempA=tempA->nextNode;
        }

    }
#ifdef DEBUG
cout<<"Updating NETLIST list ...complete!"<<endl;
#endif
}






/*========================================================================
Function to calculate the cutset frm the NETLIST list
================================================================*/



void calculateCutset()
{
nodeNetlist *tempN;
string previous;
int change;
CUTSET=0;
#ifdef DEBUG
cout<<"Calculating cutset...please wait"<<endl;
#endif
  tempN=startNodePtrNet;
for(unsigned int net=0;net<NET_NUMBER;net++)
{
   
    tempN=startNodePtrNet;
    while(tempN->netNumber!=net)  
    {
        tempN=tempN->nextNode; 
    }

    if(strcmp(tempN->nodeType.data(),"L")==0) 
    {
         previous.assign("L");
    }
    else if(strcmp(tempN->nodeType.data(),"R")==0)
    {
         previous.assign("R");
    }
	else //this is the node of type 'U' 
	{

	}
    change =0;//check for the first 'change'
    while((tempN->netNumber==net)&&(change==0))
    {
      tempN=tempN->nextNode; 

      if(!tempN)
        break;
	  // check if the node actually belongs to the ith net
      if((tempN!=NULL)&&(tempN->netNumber==net)) 
      {

           if(strcmp(tempN->nodeType.data(),previous.data())!=0) 
           {
              change=1;
              CUTSET++;
            }
            else
            {
             //   cout<<"same as previous"<<endl;
            }
      }

    }
}
#ifdef DEBUG
cout<<"================================"<<endl;
cout<<"Final CUTSET="<<CUTSET<<endl;
cout<<"================================"<<endl;
#endif
}

/*--------------------------------------------------------------------
Function to update AREA list with the nets ( Not used )
-----------------------------------------------------------------------*/
void updateAreaListWithNets()
{
nodeAreaList * nodeA=startNodePtrArea;
nodeNetlist   * nodeN;
nodeA=nodeA->nextNode;
#ifdef DEBUG
cout<<"Updating AREA list ...please wait"<<endl;
#endif

while(nodeA)
{
        nodeN=startNodePtrNet;//start from the beginning of NETLIST

        while(nodeN)
        {
            nodeN=nodeN->nextNode;
            if(!nodeN)
            break;
            if(strcmp(nodeN->nodeName.data(),nodeA->nodeName.data())==0)
            {
                nodeA->listOfNets.push_back(nodeN->netNumber);
            }

        }

nodeA=nodeA->nextNode;
if(!nodeA)
break;
}
#ifdef DEBUG
cout<<"Updating AREA list ...complete!"<<endl;
#endif

}

/*======================================================================================================
//-The simulated annealing algo- call this after the lists have been initialized and random cut generated
=====================================================================================================*/
void anneal()
{
	float prob;
	unsigned long int total_nodes;
	float expp =2.71;
	float costfunc_old ; //= ratioCut(); //remove_test
	float costfunc_new;  //= ratioCut(); //remove_test
	unsigned long node_count=0;
	float E,x,ex1,alpha;
	string node_type;
	ofstream RESULTFILE;

	cout<<"Running simulated annealing.Please wait..."<<endl;


	RESULTFILE.open(resFile.data());
	if(!RESULTFILE.is_open())
	{
	    cout<<"ERROR : could not open a RESULT file"<<endl;
	    exit(1);
	}
	RESULTFILE<<"CUTSET 		    RATIOCUT    		 TEMP"<<endl;
	//calculateCutset(); 
	calculateRatiocut();
 
	//float LRC= (float)LOCAL_LEFT_AREA/(float)(LOCAL_LEFT_AREA+LOCAL_RIGHT_AREA);
	//float RRC= (float)LOCAL_RIGHT_AREA/(float)(LOCAL_LEFT_AREA+LOCAL_RIGHT_AREA);
	//calculateCutset();
	costfunc_old=1.00;
    
	//INIT_TEMP=NO_OF_NODES; //girish:try initalizing INIT_TEMP to some other value esp for larger netlists
	
	//simulated annealing loop
	while(INIT_TEMP > FINAL_TEMP) //temp loop should iterate till the system cools down to FINAL_TEMP
	{

		total_nodes = 20;//NO_OF_NODES; //fix the inner loop at 20 iterations (TEST)

		while(total_nodes) 
		{
			
			//choose some random node and determine if the move satisfies ratio cut constraints
			shift_random_node();

			//if the move satisfies ratio cut constraints, make the move probabilistically
			if(MOVEVALID==1)
			{
				
			    	#ifdef DEBUG
			    	cout<<"=============================="<<endl;
			    	cout<<"STARTED ANNEALING FOR TEMP="<<INIT_TEMP<<endl;
			    	cout<<"================================"<<endl;
			    	#endif
				//calculate RC before making the move locally 
				float LRC= (float)LOCAL_LEFT_AREA/(float)(LOCAL_LEFT_AREA+LOCAL_RIGHT_AREA);
				float RRC= (float)LOCAL_RIGHT_AREA/(float)(LOCAL_LEFT_AREA+LOCAL_RIGHT_AREA);

				calculateCutset();
				costfunc_new= (float) CUTSET /(float) (LRC * RRC);
				//cout<<"costfunc_old=["<<costfunc_old<<"] and costfunc_new=["<<costfunc_new<<endl;
				E = costfunc_new - costfunc_old;
				x = (E)/INIT_TEMP;
				ex1 = expp;
				//determine probability of making the move at this temp
				alpha = pow(ex1,x);
				if (alpha>1)
				    prob=1;
				else
				    prob=alpha;
				random_num(1);
				#ifdef DEBUG
				cout<<"=============================="<<endl;
				cout<<"ALPHA=["<<alpha<<"] and DELTA=["<<DELTA<< "] and PROBABILITY=["<<prob<<"]"<<endl;
				cout<<"================================"<<endl;
				#endif

				//make the move - update area list with latest  L and R
				if(prob >= DELTA)
				{
					//cout<<"==============MAKING MOVE============================="<<endl;

				    //since move is actually made, we update the GLOBAL L&R areas
		    		LEFT_AREA=LOCAL_LEFT_AREA;
				    RIGHT_AREA=LOCAL_RIGHT_AREA;
				    calculateRatiocut();
					costfunc_old=costfunc_new;
				    //update area node about the change
				    //get area of the node, total area, left area and type.
				    nodeAreaList * nodeA=startNodePtrArea;
				    node_count=0;
				    while(node_count< NODE_NUM)
				    {
				        nodeA=nodeA->nextNode;
				        node_count++;
				        //cout<<"node_count="<<node_count<<endl;
				    }
				    if(strcmp(nodeA->nodeType.data(),"L")==0)
				    {

				        nodeA->nodeType.clear();
				        nodeA->nodeType.assign("R");

				    }
				    else if(strcmp(nodeA->nodeType.data(),"R")==0)
				    {
				        nodeA->nodeType.clear();
				        nodeA->nodeType.assign("L");

				    }
				    //update netlist linked list TYPE field using area list
				    //updateNetListLinks();
					updateNetListAfterMove();
				    //calculate latest cut set -after making the move (what about the ratio cut?)
				    calculateCutset();
				    //record results	  
				    RESULTFILE<<CUTSET<<" 			  "<<RATIO_CUT<<" 	 		"<<INIT_TEMP<<endl;

				 }//if prob delta
				
			 }//if movevalid

            MOVEVALID=1;//resetting flag
			total_nodes--;

		}


       //update GAMMA based on the current temperature value
        if((INIT_TEMP / T_RANGE_1) > 1.0) //100000
        {
            GAMMA = GAMMA_V_1;
        }
        else if((INIT_TEMP / T_RANGE_2) > 1.0)
        {
                GAMMA = GAMMA_V_2;
        }
        else if((INIT_TEMP / T_RANGE_3) > 1.0)
        {
            GAMMA = GAMMA_V_3;
        }
        else if((INIT_TEMP / T_RANGE_4) > 1.0)
        {
            GAMMA = GAMMA_V_4;
        }

		INIT_TEMP= INIT_TEMP-GAMMA; //update the temp


		#ifdef DEBUG
		cout<<"==============================="<<endl;
		cout<<"NEW INITIAL TEMP="<<INIT_TEMP<<endl;
		cout<<"GAMMA IS="<<GAMMA<<endl;
		cout<<"==============================="<<endl;
		#endif
	}//outer while

	RESULTFILE.close();
}//fn ends

/*========================================================
 function to calculate ratio cut
========================================================*/
void calculateRatiocut()
{
	
	RATIO_CUT= (float)LEFT_AREA/(float)(LEFT_AREA+RIGHT_AREA);
#ifdef DEBUG
	cout<<endl<<"RATIO CUT="<<RATIO_CUT<<endl;
#endif

}
/*========================================================
 Decide whether the randomly chosen  node (move) is valid
========================================================*/
void shift_random_node()
{
    nodeAreaList * nodeA;
    unsigned int node_count=0;
	string node_type;
	long node_area;//LOCAL_LEFT_AREA,LOCAL_RIGHT_AREA;

    LOCAL_LEFT_AREA=LEFT_AREA;
    LOCAL_RIGHT_AREA=RIGHT_AREA;

    random_num(NO_OF_NODES);
	//get area of the node, total area, left area and type.
    nodeA=startNodePtrArea;
    node_count=0;
    while(node_count< NODE_NUM)
    {
        nodeA=nodeA->nextNode;
        node_count++;
    }
    
    node_area=nodeA->nodeArea;
    node_type.assign(nodeA->nodeType.data());
	if(strcmp(node_type.data(),"L")==0)
	{
		//moving node to right partition... therefore decrement the left area by the current area
		LOCAL_LEFT_AREA = LOCAL_LEFT_AREA - node_area;
		LOCAL_RIGHT_AREA= LOCAL_RIGHT_AREA + node_area;

	}
	else if(strcmp(node_type.data(),"R")==0)
	{
        //moving node  to left partition... therefore increment the left node area by the current area
		LOCAL_LEFT_AREA = LOCAL_LEFT_AREA + node_area;
		LOCAL_RIGHT_AREA= LOCAL_RIGHT_AREA - node_area;

	}
	//check if the ratio cut is ok (within bounds) if yes, declare the move VALID!
	if((RANGE_LOWER<=LOCAL_LEFT_AREA) && (LOCAL_LEFT_AREA<=RANGE_UPPER))
	{
		float rc= (float)LOCAL_LEFT_AREA/(float)(LOCAL_LEFT_AREA+LOCAL_RIGHT_AREA);
		if((0.4 <= rc) && (rc <= 0.6))
		{
			MOVEVALID=1;
					
		}
	}
	else 
	{	    
	    MOVEVALID=0;
	}
}
/*========================================================
 function to generate random number (used for node selection and DELTA)
========================================================*/
void random_num(unsigned long int max)
{
   	if (max>1)
    	{
        	NODE_NUM=rand() % max +1;
	}
	else
	{
	    	float x;
        	x=((double) rand() / (RAND_MAX+1)) ;
        	DELTA= (x);
	}
}


void updateNetListAfterMove()
{
	unsigned int node_count=0;
	//we know that we have moved NODE_NUM node
	//so just update NODE_NUMs nets in NETLIST

	 nodeAreaList *tempA=startNodePtrArea;
	 nodeNetlist *tempN=startNodePtrNet;
        //tempA=startNodePtrArea->nextNode;
     

	 //go to the NODE_NUMth node in AREA list 
	 while(node_count < NODE_NUM )
	 {
		 tempA=tempA->nextNode;
		 node_count++;
	 }
	//cout<<"the node ["<<tempA->nodeName<<"] has been chosen of type ["<<tempA->nodeType.data()<<"]"<<endl;
    //vector<unsigned int>::iterator it;
	 //for each net to which the node belongs
	for( int i=0; i < tempA->listOfNets.size(); i++)
	{
		// cout<<"checking for net#["<<tempA->listOfNets[i]<<"]"<<endl;
	   //search for the (*it) th net node in NETLIST
		tempN=startNodePtrNet;
		 //while the net no is somevalue keep checking NETLIST for all instances of the node NUM_NODE
			 while(tempN->netNumber != tempA->listOfNets[i])
			 {
				 tempN=tempN->nextNode;
			 }

			 if(tempN)
			 {
					 while( (tempN->netNumber==tempA->listOfNets[i]) && (tempN) )
					 {
						if(strcmp(tempA->nodeName.data(),tempN->nodeName.data())==0)
						{

							tempN->nodeType.clear();
							tempN->nodeType.assign(tempA->nodeType.data());
							//cout<<"updated ["<<tempN->nodeName<<"] node of netno=["<<tempN->netNumber<<"] in the NETLIST with type=["<<tempA->nodeType.data()<<"]"<<endl;
							if(tempN!=endNodePtrNet)
							   tempN=tempN->nextNode;
							else
								tempN=startNodePtrNet;
							//resetting net no
							//tempN->netNumber=0;	
							//go to next node
						               
						}
						else
						{
							if(tempN!=endNodePtrNet)
									tempN=tempN->nextNode;
							else
								tempN=startNodePtrNet;
				               
						}
						
					 }//whiel tempN is not null
			 }
		 }//for


}



//---------------------EOF-------------------