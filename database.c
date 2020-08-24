#include "database.h"
#include <mysql/mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

MYSQL *con;

MYSQL connectDB(){
    con = mysql_init(NULL);
    char *server = "localhost";
	char *user = "Vernom";
	char *password = "vernomPassword";
	char *database = "VERNOM";

    if (con == NULL) {
        fprintf(stderr, "%s\n", mysql_error(con));
        exit(1);
    }

    if (mysql_real_connect(con, server, user, password,
                           database, 0, NULL, 0) == NULL) {
        fprintf(stderr, "%s\n", mysql_error(con));
        mysql_close(con);
        exit(1);
    }

    return *con;
}

char** getLastRow(){
    MYSQL_RES *res;
    MYSQL_ROW row;
    char **resultArray = malloc(3*sizeof(char*));
    if (mysql_query(con, "SELECT * FROM logs")) {
        fprintf(stderr, "%s\n", mysql_error(con));
        mysql_close(con);
        exit(1);
    }

    res = mysql_use_result(con);
	printf("DATE\tCONTENT\tUSER\n");
	while ((row = mysql_fetch_row(res)) != NULL){
        printf("%s\t%s\t%s \n", row[0],row[1],row[2]);
        for(int ii=0; ii<3; ii++){
            resultArray[ii] = malloc(50*sizeof(char));
            strcpy(resultArray[ii], row[ii]); 
        }
        
    }
		 
	mysql_free_result(res);
    mysql_close(con);
    return resultArray;
}