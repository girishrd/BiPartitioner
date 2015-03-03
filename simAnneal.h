#include <string>
#include <vector>
//---------------------------------
using namespace std;
//---------------------------------
//------------input file parameters
#define LINES_TO_BE_SKIPPED 5

//=======================================
//--------- ANNEALING PARAMETERS------
//=======================================
#define AREA_TOLERANCE 0.20

//------------cooling schedule 
#define GAMMA_V_1 20
#define GAMMA_V_2 10
#define GAMMA_V_3 5
#define GAMMA_V_4 0.1

#define T_RANGE_1 1000
#define T_RANGE_2 100
#define T_RANGE_3 10
#define T_RANGE_4 5
//=======================================

//---methods
void fileParseNetlist(char * fname);
void fileParseArealist(char * fname);
void initNetListLinkedList();
void initAreaLinkedList();
void displayLinkedList(char ch);
void addNewNetListNode(const std::string& s);
void addNewAreaNode(const std::string& s);
void deleteLinkedList(char ch);
void generateFirstCut();
void updateNetListLinks();
void updateAreaListWithNets();
void updateNetListAfterMove();
void calculateCutset();
void updateAreaList();
void shift_random_node();
void calculateRatiocut();
void random_num(unsigned long int);
void anneal();

extern string resFile;

//=====================================
//---------DATA STRUCTURES-------
//======================================
struct nodeAreaList //a7{
    string nodeName;
    long nodeArea;
    string nodeType;
    nodeAreaList * nextNode;
   vector<unsigned int> listOfNets;// 5, 8 , 12

};

struct nodeNetlist{
    string nodeName; //a0
    //long nodeArea;
    string nodeType;
    unsigned int netNumber; 
    nodeNetlist * nextNode;

};

//--------------EOF



