#include "server.h"

#include <iostream>

#include <mysql/mysql.h>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "transacconvert.h"

void* coordinator_thread_piecewise(void* arg)
{
    t_chain_param* param = (t_chain_param*)arg;

    int client_sockfd = param->client_sockfd;
    Transaction* t = param->t;

    unsigned int server_port;
    std::string hop_string;

    int ret;
    std::string send_string;
    std::string recv_string;
    int sockfd;
    sockaddr_in serv_addr;
    char buf[128];
    memset(buf, 0, sizeof(char)*128);

    //the first hop
    Hop& h = t->hops.front();
    MYSQL* con = mysql_init(NULL);
    if (con == NULL)
    {
        std::cout<<"failed to init mysql connection.\n";
        exit(1);
    }
    if (mysql_real_connect(con, "localhost", "root", "rtio", "csdiDB", 0, NULL, 0) == NULL)
    {
        std::cout<<"failed to connect mysql server.\n";
        exit(1);
    }
    if (mysql_query(con, h.get_sql_statement().c_str()));
    {
        std::cout<<"failed to execute the first hop.\n";
        exit(1);
    }
    t->hops.pop_front();
    send_string.assign("COMPLETE");
    ret = send(client_sockfd, send_string.c_str(), send_string.size(), 0);
    if (ret == -1)
    {
        std::cout<<"send complete error.\n";
        exit(1);
    }
    close(client_sockfd);

    // the remaining hops
    while (t->hops.empty() != 0)
    {
        h = t->hops.front();

        //make sure the corresponding server of a hop and set the counter to make original ordering
        if (h.get_server() == "server_01")
        {
            server_port = SERV_1_HOP_PORT;
            server_counter[0]++;
            h.set_counter(server_counter[0]);
        }
        else if (h.get_server() == "server_02")
        {
            server_port = SERV_2_HOP_PORT;
            server_counter[1]++;
            h.set_counter(server_counter[1]);
        }
        else if (h.get_server() == "server_03")
        {
            server_port = SERV_3_HOP_PORT;
            server_counter[2]++;
            h.set_counter(server_counter[2]);
        }
        else
        {
            std::cout<<"Unknown server code!\n";
            pthread_exit(NULL);
        }

        //connect the corresponding server and send the hops
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1)
        {
            std::cout<<"socket error\n";
            exit(1);
        }
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(server_port);
        serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        bzero(&(serv_addr.sin_zero), 8);
        ret = connect(sockfd, (sockaddr*)&serv_addr, sizeof(sockaddr));
        if (ret == -1)
        {
            std::cout<<"connect error.\n";
            exit(1);
        }
        hop_string.clear();
        hop_string = TransacConvert::hop_to_string(&h);
        ret = send(sockfd, hop_string.c_str(), hop_string.size(), 0);
        if (ret == -1)
        {
            std::cout<<"send error.\n";
            exit(1);
        }
        ret = recv(sockfd, buf, 128, 0);
        recv_string.clear();
        recv_string.assign(buf);
        if (recv_string.compare("COMPLETE") != 0)
        {
            std::cout<<"remote hop failed\n";
            exit(1);
        }
        close(sockfd);

        t->hops.pop_front();
    }

    //I should delete some resources here
    delete param;

    pthread_exit(NULL);
}

void* hop_exe_thread(void* arg)
{
    hop_exe_param* hp = (hop_exe_param*)arg;
    int remote_server_no;
    if (hp->h->get_server() == "server_01")
    {
        remote_server_no = 0;
    }
    else if (hp->h->get_server() == "server_02")
    {
        remote_server_no = 1;
    }
    else if (hp->h->get_server() == "server_03")
    {
        remote_server_no = 2;
    }

    while (true)
    {
        if (server_done[remote_server_no] == hp->h->get_counter()-1)
        {
            MYSQL* con = mysql_init(NULL);
            if (con == NULL)
            {
                std::cout<<"failed to init mysql connection.\n";
                exit(1);
            }
            if (mysql_real_connect(con, "localhost", "root", "rtio", "csdiDB", 0, NULL, 0) == NULL)
            {
                std::cout<<"failed to connect mysql server.\n";
                exit(1);
            }
            if (mysql_query(con, hp->h->get_sql_statement().c_str()));
            {
                std::cout<<"failed to execute the first hop.\n";
                exit(1);
            }
            server_done[remote_server_no]++;

            std::string str = "COMPLETE";
            send(hp->clientfd, str.c_str(), str.size(), 0);
            close(hp->clientfd);
            delete hp;

            break;
        }
    }
    pthread_exit(NULL);
}

void* hop_listen_thread(void* arg)
{
    int port = *((int*)arg);
    int ret;            /*return value*/
    int sockfd;         /*The socket used for listening*/
    int clientsfd;      /*The socket used for data transferring*/
    sockaddr_in host_addr;      /*local IP and port*/
    sockaddr_in client_addr;    /*client's IP and port*/
    unsigned int addrlen;
    char buf[BUF_SIZE];
    int cnt;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        std::cout<<"socket error\n";
        exit(1);
    }

    host_addr.sin_family = AF_INET;             /*TCP/IP protocol*/
    host_addr.sin_port = htons(port);      /*let system randomly choose a unused port*/
    host_addr.sin_addr.s_addr = INADDR_ANY;     /*the IP of the host*/
    bzero(&(host_addr.sin_zero), 8);

    ret = bind(sockfd, (sockaddr*)&(host_addr), sizeof(sockaddr));  /*binding*/
    if (ret == -1)
    {
        std::cout<<"bind error\n";
        exit(1);
    }
    ret = listen(sockfd, BACKLOG);      /*set the socket as listening mode which waits for connection*/
    if (ret == -1)
    {
        std::cout<<"listen error\n";
        exit(1);
    }
    std::cout<<"Waiting for the hop connection\n";

    std::string receive_str;
    std::string send_str;
    while (true)
    {
        memset(buf, 0, sizeof(char)*BUF_SIZE);
        addrlen = sizeof(sockaddr_in);
        clientsfd = accept(sockfd, (sockaddr*)&client_addr, &addrlen);   /*accept the request*/
        std::cout<<"clientsfd: "<<clientsfd<<"\n";
        if (clientsfd == -1)
        {
            std::cout<<"hop　listener accept error\n";
            continue;
        }
        cnt = recv(clientsfd, buf, BUF_SIZE, 0);
        if (cnt == -1)
        {
            std::cout<<"recv error\n";
            exit(1);
        }
        receive_str.assign(buf);
        Hop* h = TransacConvert::string_to_hop(receive_str);
        //create a hop exe thread
        pthread_t id;
        hop_exe_param* hp = new hop_exe_param();
        hp->clientfd = clientsfd;
        hp->h = h;
        ret = pthread_create(&id, NULL, &hop_exe_thread, (void*)hp);
    }
    pthread_exit(NULL);
}

void* transaction_listen_thread(void* arg)
{
    int port = *((int*)arg);
    int ret;            /*return value*/
    int sockfd;         /*The socket used for listening*/
    int clientsfd;      /*The socket used for data transferring*/
    sockaddr_in host_addr;      /*local IP and port*/
    sockaddr_in client_addr;    /*client's IP and port*/
    unsigned int addrlen;
    char buf[BUF_SIZE];
    int cnt;

    memset(buf, 0, sizeof(char)*BUF_SIZE);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        std::cout<<"socket error\n";
        exit(1);
    }

    host_addr.sin_family = AF_INET;             /*TCP/IP protocol*/
    host_addr.sin_port = htons(port);      /*let system randomly choose a unused port*/
    host_addr.sin_addr.s_addr = INADDR_ANY;     /*the IP of the host*/
    bzero(&(host_addr.sin_zero), 8);

    ret = bind(sockfd, (sockaddr*)&(host_addr), sizeof(sockaddr));  /*binding*/
    if (ret == -1)
    {
        std::cout<<"bind error\n";
        exit(1);
    }
    ret = listen(sockfd, BACKLOG);      /*set the socket as listening mode which waits for connection*/
    if (ret == -1)
    {
        std::cout<<"listen error\n";
        exit(1);
    }
    std::cout<<"Waiting for the client connection\n";

    std::string receive_str;
    std::string send_str;

    while (true)
    {
        addrlen = sizeof(sockaddr_in);
        clientsfd = accept(sockfd, (sockaddr*)&client_addr, &addrlen);   /*accept the request*/
        std::cout<<"clientsfd: "<<clientsfd<<"\n";
        if (clientsfd == -1)
        {
            std::cout<<"accept error\n";
            continue;
        }
        std::cout<<"Transaction listening - Client IP:"<<inet_ntoa(client_addr.sin_addr)<<"\n";
        memset(buf, 0, sizeof(char)*BUF_SIZE);
        cnt = recv(clientsfd, buf, BUF_SIZE, 0);
        if (cnt == -1)
        {
            std::cout<<"recv error\n";
            exit(1);
        }
        receive_str.assign(buf);
        if (receive_str == "SendT")
        {
            std::cout<<"ready for receive transaction string\n";
            send_str.assign("ACK");
            cnt = send(clientsfd, send_str.c_str(), 3, 0);
            if (cnt == -1)
            {
                std::cout<<"send error\n";
                exit(1);
            }
            memset(buf, 0, sizeof(char)*BUF_SIZE);
            cnt = recv(clientsfd, buf, BUF_SIZE, 0);
            if (cnt == -1)
            {
                std::cout<<"recv error\n";
                exit(1);
            }
            receive_str.assign(buf);
            //std::cout<<receive_str<<"\n";
            Transaction* t = TransacConvert::string_to_transaction(receive_str);
            //std::cout<<"get transaction string\n";

            pthread_mutex_lock(&mutex);

            t_list.push_front(t);
            t_client_sockfd_list.push_front(clientsfd);

            if (t_list.size() >= 1)
            {
                pthread_cond_signal(&cond);
                std::cout<<"wake up the transaction consumer!\n";
            }

            pthread_mutex_unlock(&mutex);
        }
        else
        {
            std::cout<<"Not SendT\n";
            std::cout<<receive_str<<"\n";
            close(clientsfd);
        }
    }
    pthread_exit(NULL);
}


void* transaction_exe_thread(void* arg)
{
    int client_sockfd;
    Transaction* t = NULL;

    while (true)
    {
        pthread_mutex_lock(&mutex);
        while (t_list.size() < 1)
        {
            pthread_cond_wait(&cond, &mutex);
        }
        client_sockfd = t_client_sockfd_list.back();
        t = t_list.back();
        std::cout<<"start to execute transaction\n";
        pthread_mutex_unlock(&mutex);

        //SC-graph analysis
        /*
        t_chain_param* tp = new t_chain_param();
        tp->client_sockfd = client_sockfd;
        tp->t = t;
        pthread_t id;
        pthread_create(&id, NULL, &coordinator_thread_piecewise, tp);
        */
        //only for testing
        std::cout<<"Transaction execution: "<<TransacConvert::transaction_to_string(t)<<std::endl;
        std::string send_string = "Proceeded";
        send(client_sockfd, send_string.c_str(), send_string.size(), 0);
        close(client_sockfd);
        t_client_sockfd_list.pop_back();
        t_list.pop_back();
        delete t;
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    int t_listen_port = SERV_1_PORT;
    pthread_t t_listen_id;
    pthread_t t_exe_id;

    pthread_create(&t_listen_id, NULL, &transaction_listen_thread, (void*)&t_listen_port);
    pthread_create(&t_exe_id, NULL, &transaction_exe_thread, NULL);

    pthread_join(t_listen_id, NULL);
    pthread_join(t_exe_id, NULL);

    return 0;
}
