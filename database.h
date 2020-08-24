#ifndef DB_H_  
#define DB_H_
#include <mysql/mysql.h>

MYSQL connectDB(); //Establishes a connection with the DB.
char** getAllRows(); //Returns array of strings, containing all rows.
char** getLastRow(); //Returns last row of the DB.

 

#endif