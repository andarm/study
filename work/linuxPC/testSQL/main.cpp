/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2016年03月29日 21时24分28秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>

#include <iostream>
using namespace std;
#include "./linux_sqlite/sqlite3.h"
int callback(void*,int,char**,char**);
int main()
{
    sqlite3* db;
    int nResult = sqlite3_open("test.db",&db);
    if (nResult != SQLITE_OK)
    {
        cout<<"打开数据库失败："<<sqlite3_errmsg(db)<<endl;
        return 0;
    }
    else
    {
        cout<<"数据库打开成功"<<endl;
    }

    char* errmsg;

    nResult = sqlite3_exec(db,"create table fuck(id integer primary key autoincrement,name varchar(100))",NULL,NULL,&errmsg);
     if (nResult != SQLITE_OK)
     {
         sqlite3_close(db);
         cout<<errmsg;
         sqlite3_free(errmsg);
        return 0;
    }
    string strSql;
    strSql+="begin;\n";
    for (int i=0;i<100;i++)
    {
        strSql+="insert into fuck values(null,'heh');\n";
    }
    strSql+="commit;";
    //cout<<strSql<<endl;

    nResult = sqlite3_exec(db,strSql.c_str(),NULL,NULL,&errmsg);

    if (nResult != SQLITE_OK)
    {
        sqlite3_close(db);
        cout<<errmsg<<endl;
        sqlite3_free(errmsg);
        return 0;
    }

    strSql = "select * from fuck";
    //nResult = sqlite3_exec(db,strSql.c_str(),callback,NULL,&errmsg);
    char** pResult;
    int nRow;
    int nCol;
    nResult = sqlite3_get_table(db,strSql.c_str(),&pResult,&nRow,&nCol,&errmsg);
      if (nResult != SQLITE_OK)
    {
        sqlite3_close(db);
        cout<<errmsg<<endl;
        sqlite3_free(errmsg);
        return 0;
    }

    string strOut;
    int nIndex = nCol;
    for(int i=0;i<nRow;i++)
    {
        for(int j=0;j<nCol;j++)
        {
            strOut+=pResult[j];
            strOut+=":";
            strOut+=pResult[nIndex];
            strOut+="\n";
            ++nIndex;
        }
    }
    sqlite3_free_table(pResult);
    cout<<strOut<<endl;
    sqlite3_close(db);
    return 0;
}