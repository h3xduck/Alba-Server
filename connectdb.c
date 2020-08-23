#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    MYSQL *con = mysql_init(NULL);
    MYSQL_RES *res;
    MYSQL_ROW row;
    char *server = "localhost";
	char *user = "Vernom"; /*usuario para consultar la base de datos */
	char *password = "vernomPassword"; /* contraseña para el usuario en cuestion */
	char *database = "VERNOM"; /*nombre de la base de datos a consultar */

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

    if (mysql_query(con, "SELECT * FROM logs")) {
        fprintf(stderr, "%s\n", mysql_error(con));
        mysql_close(con);
        exit(1);
    }

    res = mysql_use_result(con);
	printf("DATE\tCONTENT\tUSER\n");
	while ((row = mysql_fetch_row(res)) != NULL){
        printf("%s\t%s\t%s \n", row[0],row[1],row[2]);
    }
		 
	mysql_free_result(res);

    mysql_close(con);
    exit(0);
}