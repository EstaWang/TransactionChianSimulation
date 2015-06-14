#include "transacconvert.h"

#include <sstream>
#include <iostream>
#include <list>

std::string int2str(int num)
{
    std::stringstream convert_stream;
    convert_stream<<num;
    return convert_stream.str();
}

int str2int(std::string str)
{
    int result;
    std::stringstream convert_stream(str);
    convert_stream>>result;
    return result;
}

std::string TransacConvert::transaction_to_string(Transaction* t)
{
    std::string result = "<";

    result.append(int2str(t->get_transaction_number()));       //the No of transaction
    result.append(",");
    result.append(int2str(t->hops.size()));                 //the size of transaction, e.g., the number of hops
    result.append(",");

    for (std::list<Hop>::iterator iter=t->hops.begin(); iter != t->hops.end(); iter++)
    {
        result.append("H");
        result.append(",");
        result.append(iter->get_server());
        result.append(",");
        result.append(iter->get_database());
        result.append(",");
        result.append(iter->get_table());
        result.append("|");
        result.append(iter->get_sql_statement());
        result.append("|");
        if (iter->is_write())
        {
            result.append("1");
        }else{
            result.append("0");
        }
        result.append(",");
        switch (iter->get_table_access_type())
        {
        case READ:
            result.append("0");
            break;
        case INSERT:
            result.append("1");
            break;
        case UPDATE:
            result.append("2");
            break;
        default:
            result.append("unknown");
            break;
        }
        result.append(",");
        result.append("P");
        result.append(",");
    }

    result.append(">");

    return result;
}

Transaction* TransacConvert::string_to_transaction(std::string t_str)
{
    int hops_num;

    std::string server;                 /*the code of server*/
	std::string database;               /*the code of database*/
	std::string table;                  /*the code of table*/
	std::string sql_statement;          /*the sql statement*/

	table_access_type table_access;     /*the type of table access (READ, INSERT, UPDATE)*/
	bool write_type;                    /*write or read?*/

	int transaction_num;                /*the transaction number*/

	int step_point = 0;
	std::string str_temp;

	if (t_str[0] != '<' || t_str[t_str.size()-1] != '>')
    {
        return NULL;
    }

    for (step_point++; t_str[step_point] != ','; step_point++)
    {
        str_temp.append(1, t_str[step_point]);
    }
    transaction_num = str2int(str_temp);
    str_temp.clear();

    for (step_point++; t_str[step_point] != ','; step_point++)
    {
        str_temp.append(1, t_str[step_point]);
    }
    hops_num = str2int(str_temp);
    str_temp.clear();

    Transaction* T_chain = new Transaction(transaction_num);

    for (int i=0; i<hops_num; i++)
    {
        step_point++;
        if (t_str[step_point] != 'H')
        {
            return NULL;
        }
        step_point++;
        if (t_str[step_point] != ',')
        {
            return NULL;
        }
        //server
        for (step_point++; t_str[step_point] != ','; step_point++)
        {
            str_temp.append(1, t_str[step_point]);
        }
        server.assign(str_temp);
        str_temp.clear();
        //database
        for (step_point++; t_str[step_point] != ','; step_point++)
        {
            str_temp.append(1, t_str[step_point]);
        }
        database.assign(str_temp);
        str_temp.clear();
        //table
        for (step_point++; t_str[step_point] != '|'; step_point++)
        {
            str_temp.append(1, t_str[step_point]);
        }
        table.assign(str_temp);
        str_temp.clear();
        //sql statement
        for (step_point++; t_str[step_point] !='|'; step_point++)
        {
            str_temp.append(1, t_str[step_point]);
        }
        sql_statement.assign(str_temp);
        str_temp.clear();
        //write or read
        step_point++;
        if (t_str[step_point] == '1')
        {
            write_type = true;
        }else{
            write_type = false;
        }
        //type of sql statement to table
        step_point += 2;
        switch (t_str[step_point])
        {
        case '0':
            table_access = READ;
            break;
        case '1':
            table_access = INSERT;
            break;
        case '2':
            table_access = UPDATE;
            break;
        default:
            return NULL;
            break;
        }
        step_point++;
        if (t_str[step_point] != ',')
        {
            return NULL;
        }
        step_point++;
        if (t_str[step_point] != 'P')
        {
            return NULL;
        }
        step_point++;

        Hop* NewHop = new Hop(server, database, table, sql_statement, write_type, table_access, i, transaction_num);
        T_chain->hops.push_back(*NewHop);

        server.clear();
        database.clear();
        table.clear();
        sql_statement.clear();
    }

    return T_chain;
}

/*
*H,server_01,csdiDB,follow|insert into follow (follower_id, followee_id) values (001, 006);|,1,1,counter,P
*/

std::string TransacConvert::hop_to_string(Hop* h)
{
    std::string result;
    result.append("H");
    result.append(",");
    result.append(h->get_server());
    result.append(",");
    result.append(h->get_database());
    result.append(",");
    result.append(h->get_table());
    result.append("|");
    result.append(h->get_sql_statement());
    result.append("|");
    if (h->is_write())
    {
        result.append("1");
    }else{
        result.append("0");
    }
    result.append(",");
    switch (h->get_table_access_type())
    {
    case READ:
        result.append("0");
        break;
    case INSERT:
        result.append("1");
        break;
    case UPDATE:
        result.append("2");
        break;
    default:
        break;
    }
    result.append(",");
    result.append(int2str(h->get_counter()));
    result.append(",");
    result.append("P");

    return result;
}

Hop* TransacConvert::string_to_hop(std::string h_str)
{
    std::string server;                 /*the code of server*/
	std::string database;               /*the code of database*/
	std::string table;                  /*the code of table*/
	std::string sql_statement;          /*the sql statement*/

	table_access_type table_access;     /*the type of table access (READ, INSERT, UPDATE)*/
	bool write_type;                    /*write or read?*/

	int counter;

	int step_point;
	std::string str_temp;

	if (h_str[0] != 'H' || h_str[h_str.size()-1] != 'P')
    {
        return NULL;
    }
    for (step_point = 2; h_str[step_point]!=','; step_point++)
    {
        server.append(1, h_str[step_point]);
    }
    for (step_point++; h_str[step_point] != ','; step_point++)
    {
        database.append(1, h_str[step_point]);
    }
    for (step_point++; h_str[step_point] != '|'; step_point++)
    {
        table.append(1, h_str[step_point]);
    }
    for (step_point++; h_str[step_point] != '|'; step_point++)
    {
        sql_statement.append(1, h_str[step_point]);
    }
    step_point++;
    if (h_str[step_point] == '1')
    {
        write_type = true;
    }else{
        write_type = false;
    }
    //type of sql statement to table
    step_point += 2;
    switch (h_str[step_point])
    {
    case '0':
        table_access = READ;
        break;
    case '1':
        table_access = INSERT;
        break;
    case '2':
        table_access = UPDATE;
        break;
    default:
        return NULL;
        break;
    }
    str_temp.clear();
    for (step_point += 2; h_str[step_point] != ','; step_point++)
    {
        str_temp.append(1, h_str[step_point]);
    }
    counter = str2int(str_temp);

    Hop* NewHop = new Hop(server, database, table, sql_statement, write_type, table_access);
    NewHop->set_counter(counter);
    return NewHop;
}
