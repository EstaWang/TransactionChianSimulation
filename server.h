#ifndef _SERVER_H_
#define _SERVER_H_

#include "transaction.h"
#include "hop.h"
#include "global.h"
#include "transacconvert.h"

pthread_mutex_t mutex;
pthread_cond_t cond;

int server_no;          //the code of server
int server_counter[3];
int server_done[3];
int server_number;      //the number of server

std::list<Transaction*> t_list;
std::list<int> t_client_sockfd_list;

void* coordinator_thread_piecewise(void* arg);
void* coordinator_thread(void* arg);

void* hop_listen_thread(void* arg);
void* hop_exe_thread(void* arg);

void* transaction_listen_thread(void* arg);     //the parameter : listen port
void* transaction_exe_thread(void* arg);        //the parameter : null

#endif // _SERVER_H_
