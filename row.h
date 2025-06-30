#include<stdlib.h>
#define MAX 20
#define code 4
#define loc 12
//data of each row 
struct row{
    int ID;//student's ID
    char name[MAX];//student's name
    char branch[code];//student's branch in acronym
    char city[loc];//studnet's city
    //marks of the student in different courses
    int MTH;
    int PHY;
    int CHM;
    int TA;
    int LIF;
}Row;
