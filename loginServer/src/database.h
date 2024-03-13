#include <sqlite3.h>
#ifndef DATABASE_H
#define DATABASE_H

int open_database(const char *dbfile);
int close_database();
int insertDisplay(void *para,int ncolumn, char ** columnvalue,char *columnname[]);
int queryDisplay(void *para,int ncolumn, char ** columnvalue,char *columnname[]);
int dataControl(char * packData,int);
/*int insertQueryModel(const char * buf,int size);
int queryModel(const char * buf,int size);
int insertModel(const char * buf,int size);*/
int insertQueryModel(char * buf,int size);
int queryModel(char * buf,int size);
int insertModel(char * buf,int size);

#endif
