#ifndef _HOP_H_
#define _HOP_H_

#include <string>

enum table_access_type {READ, INSERT, UPDATE};

class Hop
{
public:
	Hop()
	{

	}

    /*
    constructor
    db : the name of database
    t  : the name of tables
    sql: the sql statement
    access_type : the access type is "write" or "read"
    */
    Hop(std::string server_name, std::string db, std::string t, std::string sql,
     bool access_type, table_access_type type)
	{
	    server = server_name;
		database = db;
		table = t;
		sql_statement = sql;
		write_type = access_type;
		table_access = type;
	}

	Hop(std::string server_name, std::string db, std::string t, std::string sql,
     bool access_type, table_access_type type, int seq, int tran_num)
	{
	    server = server_name;
		database = db;
		table = t;
		sql_statement = sql;
		write_type = access_type;
		table_access = type;
		seq_num_in_transaction = seq;
		transaction_num = tran_num;
	}

    std::string get_server();           /*get the string of server*/
	std::string get_database();         /*get the string of database*/
	std::string get_table();            /*get the string of table*/
	std::string get_sql_statement();    /*get the sql statement*/

	bool is_write();                    /*get the access type: true is write; false is read*/
	table_access_type get_table_access_type();

	int get_sequence_num();
	int get_transaction_num();

	int get_counter();
	void set_counter(int c);

	bool is_commute();
	void set_commute(bool cu);

private:
    std::string server;                 /*the code of server*/
	std::string database;               /*the code of database*/
	std::string table;                  /*the code of table*/
	std::string sql_statement;          /*the sql statement*/

	table_access_type table_access;     /*the type of table access (READ, INSERT, UPDATE)*/
	bool write_type;                    /*write or read?*/

	int seq_num_in_transaction;         /*the sequence number of the hop in the transaction*/
	int transaction_num;                /*the transaction number*/

    int counter;                        /*used for original ordering*/

    bool commute;
};

#endif // _HOP_H_
