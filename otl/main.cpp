//g++ main.cpp -I$ORACLE_HOME/rdbms/public -L$ORACLE_HOME/lib -lclntsh
#include <iostream>
#include <string>

#define OTL_ORA11G_R2
#define OTL_ORA_UTF8

#include "otlv4.h"

#define CONN_STR "sudoku/huang@orcl"
//#define CONN_STR "sqlplus / as sysdba"

void select(otl_connect& conn)
{
    std::cout << "begin select" << std::endl;

    std::string strSQL = "select * from sudoku.test";
    otl_stream otl_str;
    otl_str.open(1, strSQL.c_str(), conn);

    char username[20], password[20], address[100];
    int age, sex;

    while (!otl_str.eof()) {
        otl_str >> username >> password >> age >> sex >> address;

        std::cout << username << ", " << password << ", " << age << ", " << sex <<  std::endl;
    }
    otl_str.close();

    std::cout << "end select" << std::endl;
    return ;
}

void insert(otl_connect& conn)
{
    std::cout << "begin insert" << std::endl;

    std::string strSQL = "insert into sudoku.test(username, password) values('test1', 'ptest')";
    otl_stream otl_str;
    otl_str.open(1, strSQL.c_str(), conn);
    otl_str.close();

    std::cout << "end insert" << std::endl;
}

void get_column_attr(otl_connect& conn)
{
    std::cout << "begin get_column_attr" << std::endl;

    std::string strSQL = "select * from sudoku.test where 1 = 2";
    otl_nocommit_stream os(1, strSQL.c_str(), conn);

    otl_column_desc * pdesc;
    int dataLen = 0;
    pdesc = os.describe_select(dataLen);

    std::cout << "colume number: " << dataLen << std::endl;

    for (int i = 0; i < dataLen; i++) {
        printf("column: name:%s, dbtype:%d, otl_var_dbtype:%d, dbsize:%d, scale:%d, prec:%d, nullok:%d\n",
                pdesc[i].name,
                pdesc[i].dbtype,
                pdesc[i].otl_var_dbtype,
                pdesc[i].dbsize,
                pdesc[i].scale,
                pdesc[i].prec,
                pdesc[i].nullok);
    }
    os.close();

    std::cout << "end get_column_attr" << std::endl;
}

void select_bind(otl_connect& conn)
{
    std::cout << "begin select" << std::endl;

    std::string strSQL = "select * from sudoku.test where age = :age<int>";
    otl_stream otl_str;
    otl_str.open(100, strSQL.c_str(), conn);
    otl_str << 10;

    char username[20], password[20], address[100];
    int age, sex;

    while (!otl_str.eof()) {
        otl_str >> username >> password >> age >> sex >> address;

        std::cout << username << ", " << password << ", " << age << ", " << sex <<  std::endl;
    }
    otl_str.close();

    std::cout << "end select" << std::endl;
    return ;
}

int main(int argc, char * argv[])
{
    otl_connect::otl_initialize();
    otl_connect db;

    try {
        std::cout << "begin to connect" << std::endl;
        db.rlogon(CONN_STR);
        std::cout << "login in succ" << std::endl;

        //select(db);
        //insert(db);
        //select(db);
        select_bind(db);

        //get_column_attr(db);

    } catch (otl_exception& p) {
        std::cerr << "error" << std::endl;
    }
    db.logoff();
}
