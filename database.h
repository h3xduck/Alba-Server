#ifndef DB_H_  
#define DB_H_
#include <mysql/mysql.h>

struct resultStringArray{
    int* lengthArray;
    char** contentArray;
};

MYSQL connectDB(); //Establishes a connection with the DB.
char** getAllRows(); //Returns array of strings, containing all rows.
struct resultStringArray getLastRow(); //Returns last row of the DB.



#endif