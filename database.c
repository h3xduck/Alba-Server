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

struct resultStringArray getLastRow(){
    printf("****************++++DATABASE++++********************\n");

    MYSQL_RES *res;
    MYSQL_ROW row;
    struct resultStringArray result;
    result.contentArray = malloc(3*sizeof(char*));
    result.lengthArray = malloc(3*sizeof(int));
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
            int element_length = strlen(row[ii]);
            printf("Row %i has length of %i \n",ii, element_length);
            result.contentArray[ii] = malloc(element_length*sizeof(char));
            result.lengthArray[ii] = element_length;
            strcpy(result.contentArray[ii], row[ii]); 
            printf("Row %i received as \"%s\"\n", ii, result.contentArray[ii]);
        }
        
    }
    printf("Parsing results: Completed\n");
		 
	mysql_free_result(res);
    mysql_close(con);
    return result;
}