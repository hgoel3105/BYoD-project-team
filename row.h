#ifndef ROW_H
#define ROW_H

#define name_size 20
#define branch_size 4
#define loc_size 12
//data of each row 
typedef struct{
    int ID;//student's ID
    char name[name_size];//student's name
    char branch[branch_size];//student's branch in acronym
    char city[loc_size];//studnet's city
    //marks of the student in different courses
    int MTH;
    int PHY;
    int CHM;
    int TA;
    int LIF;
}Row;

#endif
