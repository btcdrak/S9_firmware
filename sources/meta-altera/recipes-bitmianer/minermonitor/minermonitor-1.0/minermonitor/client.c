#include "read_conf.h"
#include "openssl.h"
#include "client.h"
#include "cgminer_api.h"
#include "gen_middata.h"
#include "send_pkg.h"
#include "debug.h"
#include "rev_queue.h"
#include "write_conf.h"
#include "config.h"
#include <pthread.h>
#include <semaphore.h>

#include "cgminer_api.h"

#include "dataexchange.h"
#define MAXQSIZE 9       // queue size MAXQSIZE-1
#define NO_DATA_TIME 26 //How many seconds to detect any data 26*30S=780S about 15min
#define WIAT_RES_TIMEOUT 60  //60s//10min
char *ip_toconnect="192.168.1.187";
PKG_Format package;
PKG_Format deal_package;
int SW=0;
int NO_data=1; //long time  NO data flag
int connect_count;
int wait_res;
char data_time;
sem_t q1_sem;
sem_t q2_sem;
int pthr_rev_id = 0;
//////////////////////////////////////////////////////////////////
////////////////////connect-server////////////////////////////////
void ShowCerts(SSL * ssl)
{
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL)
    {
        DEBUG_printf("A digital certificate information:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        DEBUG_printf("certificate: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        DEBUG_printf("Issued by the: %s\n", line);
        free(line);
        X509_free(cert);
    }
    else
        DEBUG_printf("No certificate information！\n");
}
int encrypt_connect_server()
{
    /* SSL 库初始化*/
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(TLSv1_client_method());
    if (ctx == NULL)
    {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    /* 基于 ctx 产生一个新的 SSL */
    ssl = SSL_new(ctx);
    //将ssl与 TCP SOCKET连接
    SSL_set_fd(ssl, sockfd);
    /* 建立 SSL 连接 */
    if (SSL_connect(ssl) == -1 || strcmp("(NONE)",SSL_get_cipher(ssl)) == 0)
    {
        ERR_print_errors_fp(stderr);
        return -1;
    }
    DEBUG_printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
    ShowCerts(ssl);
    return 1;
}

int sock_connect_server()
{
    if(1==SW)
    {
        DEBUG_printf("Web  Switch is OFF!\n");
        exit(1);
    }
    struct hostent *host;
    struct sockaddr_in serv_addr;
    int temp;
    short int port;

    #ifdef S3_IP
    char ip_str[60]={0};
    strcpy( ip_str,read_luci_conf("ip_server"));
    char *str=strchr(ip_str,':');
    *(str++)='\0';
    port=atoi(str);
    if ((host = gethostbyname(ip_str)) == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }
    #endif
    #ifdef S4_IP
    char ip_str[60];
    read_s4_switch(s4_conf_dir,"ip_server",ip_str);
    char *str=strchr(ip_str,':');
    *(str++)='\0';
    port=atoi(str);
    if ((host = gethostbyname(ip_str)) == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }
    #endif
    #ifdef PC_IP
    port=2000;
    if ((host = gethostbyname(ip_toconnect)) == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }
    #endif
    //socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }
    //set sockaddr_in strut
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(serv_addr.sin_zero), 8);
    //to connect server
    if((temp=connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr)))== -1)
    {
        //turnoff_s4(s4_conf_dir);
        //perror("connect failed");
        DEBUG_printf("connect failed\n");
        r_connect = 0;
        return temp;
    }
    r_connect = 1;
    DEBUG_printf("server connected\n");
    return temp;
}


void send_rev_err()
{
    DEBUG_printf("send or recv: Broken pipe\n");
    //exit(0);
}
void getkill()
{
    exit(0);
}
/////////////////////////////////////////////////////////////////
///////////////////set a TIMER//////////////////////////////////
void set_time(int time1,int time2)
{
    struct itimerval itv;
    itv.it_interval.tv_sec=time1;//first value
    itv.it_interval.tv_usec=0;
    itv.it_value.tv_sec=time2;
    itv.it_value.tv_usec=0;
    setitimer(ITIMER_REAL,&itv,NULL);
}

//////////////////////////////////////////////////////////////////////
/////////////Timer detection and server link is norma/////////////////
void tcp_state()
{
    DEBUG_printf("tcp_state!\n");
    struct tcp_info info;
    int len=sizeof(info);
    if(1== SW )
    {
        DEBUG_printf("Web  Switch is OFF!\n");
        exit(1);
    }
    getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
    if((info.tcpi_state!=TCP_ESTABLISHED))
    {
        DEBUG_printf("Off Line!!! in tcp_state()\n");
        r_connect=0;
        clear_recv(deal_package);
        memset(auth_id,0,70); //clear auth
        if(sock_connect_server()==-1)
        {
            DEBUG_printf("tcp_state, sock_connect_server failed!\n");
        }
        else if(encrypt_connect_server()==-1)
        {
            receivedata = sockreceivedata;
            senddata = socksenddata;

            close(sockfd);
            sock_connect_server();
            DEBUG_printf("encrypt connect failed.\n");
        }
        else
        {
            receivedata = encryptreceivedata;
            senddata = encryptsenddata;
        }
    }
    else
    {
        DEBUG_printf("Connect to normal!!!\n");
    }
}

int recv_serpkg_struct()
{
    int tmp_time=0;
    int ret=0;
    ret=rev_data_struct();
    if(ret==0)
    {
        DEBUG_printf("OFF LINE !!!!Please Wait!!! in recv_serpkg_struct() in client\n");
        while(1)
        {
            if(r_connect == 0)
            {
                sleep(30);//try to receive data after 30s if it's break
            }
            if(rev_data_struct()>0)
            {
                return 1;
            }
        }
    }
    else if(ret==-1)
    {
        return -1;
    }
    return 1;
}

void signal_check()
{
    set_time(30,30);               //定时30s
    signal(SIGALRM, tcp_state);    //定时检查与server的链接
    signal(SIGPIPE, send_rev_err); // send或recv错误处理
    signal(SIGTERM,getkill);//exit when it's killed
}

void *read_switch(void)
{
    while(1)
    {
        #ifdef S3_WRITE
        if(strcmp("off",read_luci_conf("off_on"))==0)
        {
            SW=1;
        }
        #endif

        #ifdef S4_WRITE
        char key_switch[15];
        if(strcmp("off",read_s4_switch(s4_conf_dir,"api-switch",key_switch))==0)
        {
            SW=1;
        }
        DEBUG_printf("key_switch = %s\n",key_switch);
        #endif
        sleep(15);
    }
}

void *pthr_rev()
{
    while(1)
    {
        if((Q_cmd.rear+1)%MAXQSIZE!=Q_cmd.front && (Q_respond.rear+1)%MAXQSIZE!=Q_respond.front)
        {
            if(-1==recv_serpkg_struct())
            {
                DEBUG_printf("Bad Package REV!\n");
                sleep(1);
                continue;
            }
            if(package.pkg_cmd[0]!='f')
            {
                DEBUG_printf("this is command\n");
                EnQueue(&Q_cmd,package);
                //sem_post(&q1_sem);
            }
            else
            {
                DEBUG_printf("this is respond \n");
                EnQueue(&Q_respond,package);
                //sem_post(&q2_sem);
            }
        }
        else
        {
            DEBUG_printf("the queue is full\n");
            sleep(5);
        }
    }
}
void set_dconfig_flag()
{
    #ifdef S4_IP
    write_s4_respond(s4_conf_dir,"fx");
    #endif
    #ifdef S3_IP
    write_s3_auth("fx");
    #endif
}
void Initsem()
{
    int ret;
    ret= sem_init(&q1_sem, 0, 0);
    if(ret != 0)
    {
        perror("Semaphore initialization failed");
        exit(EXIT_FAILURE);
    }
    ret= sem_init(&q2_sem, 0, 0);
    if(ret != 0)
    {
        perror("Semaphore initialization failed");
        exit(EXIT_FAILURE);
    }
}
void mask_sig()
{
    sigset_t newmask;
    sigemptyset(&newmask);                 // clear
    sigfillset(&newmask);
    sigdelset(&newmask, SIGINT);          //without SIGINT
    sigdelset(&newmask, SIGPIPE);          //without SIGINT
}

void sig_handle(int signum)
{
    DEBUG_printf("receive signal :%d\n",signum);
    //SIGUSR1:pause;SIGUSR2:start;
    if(signum == SIGUSR1)
    {
        set_time(6000000,6000000);
        //todo:suspand pthr_rev

        //todo:disconnect with server,clear the two queue
        r_connect = 0;
        close(sockfd);
        ClearQueue(&Q_cmd);//clear the comdmand queue.
        ClearQueue(&Q_respond);
    }
    else if(signum == SIGUSR2)
    {
        //todo:clear the two queue,connect with server
        ClearQueue(&Q_cmd);//clear the comdmand queue.
        ClearQueue(&Q_respond);
        set_time(30,30);
        tcp_state();
        //todo:restart pthr_rev

    }
}
void init_daemon()
{
    int pid;
    int i;
    pid=fork();
    if(pid<0)
        exit(1);  //创建错误，退出
    else if(pid>0) //父进程退出
        exit(0);

    setsid(); //使子进程成为组长
    pid=fork();
    if(pid>0)
        exit(0); //再次退出，使进程不是组长，这样进程就不会打开控制终端
    else if(pid<0)
        exit(1);

    //关闭进程打开的文件句柄
    for(i=0;i<NOFILE;i++)
        close(i);
    chdir("/root/test");  //改变目录
    umask(0);//重设文件创建的掩码
    return;
}

int main(int argc, char *argv[])
{
    init_daemon();
    int ret = 0;
    int  auth_flag=0;
    if(argc>1)
    {
        sleep(40);
    }
    mask_sig();
    //Initsem();
    InitQueue(&Q_cmd,&Q_respond);
    set_dconfig_flag();           //set the default auth flag in file

    pthread_t thread_id;
    ret = pthread_create(&thread_id, NULL, (void*)read_switch, NULL);//read web  config switch
    if (ret)
    {
        DEBUG_printf("Create pthread error!\n");
        return 1;
    }
    sleep(1);
    if(1==SW)
    {
        DEBUG_printf("Web  Switch is OFF!\n");
        exit(1);
    }
    //刚开始连接失败时，60s试图连一次，直到连上为至
    while(sock_connect_server() == -1)
    {
        //DEBUG_printf("sockconnect failed.\n");
        sleep(30);
    }
    if(encrypt_connect_server()==-1)
    {
        receivedata = sockreceivedata;
        senddata = socksenddata;
        DEBUG_printf("encrypt connect failed.\n");
        close(sockfd);
        sock_connect_server();
    }
    else
    {
        receivedata = encryptreceivedata;
        senddata = encryptsenddata;
    }
    signal_check(30);
    signal(SIGUSR1,sig_handle);
    signal(SIGUSR2,sig_handle);
    pthread_t thread_rev;
    pthr_rev_id = pthread_create(&thread_rev, NULL, (void*)pthr_rev, NULL);//create a receiving threads
    if (pthr_rev_id)
    {
        DEBUG_printf("Create pthread error!\n");
        return 1;
    }
    sleep(2);
    while(1)
    {
        DEBUG_printf("next command!\n");
        if(1==SW)
        {
            DEBUG_printf("Web  Switch is OFF!\n");
            exit(1);
        }
        //sem_wait(&q1_sem);
        if(DeQueue(&Q_cmd,&deal_package))
        {
            DEBUG_printf("get a command!\n");
            if(strcmp(deal_package.pkg_cmd,"01")==0)
            {
                auth_flag=1;
            }
            QueueTraverse(Q_cmd,visit);
            if(!deal_cmd(deal_package))
                continue;
            DEBUG_printf("after deal_cmd()\n");
            while(1)
            {
                if(0==NUL_Queue(&Q_respond))
                {
                    //printf("I am wait queue\n");
                    sleep(1);
                    wait_res++;
                    if(wait_res % WIAT_RES_TIMEOUT == 0)
                    {
                        DEBUG_printf("Wait RES TIMEOUT\n");
                        break;
                        //exit(1);
                    }
                }
                else
                {
                    wait_res=0;
                    break;
                }
            }
            //sem_wait(&q2_sem);
            if(DeQueue(&Q_respond,&deal_package))
            {
                #ifdef S4_IP
                if(auth_flag==1)
                {
                    auth_flag=0;
                    write_s4_respond(s4_conf_dir,deal_package.pkg_cmd);
                }
                #endif
                #ifdef S3_IP
                if(auth_flag==1)
                {
                    auth_flag=0;
                    write_s3_auth(deal_package.pkg_cmd);
                }
                #endif
                QueueTraverse(Q_respond,visit);
                rev_respond();
                clear_recv(deal_package);
            }
            else
            {
                DEBUG_printf("respond queue  empty\n");
            }
            NO_data=0;//data recived
        }
        else
        {
            DEBUG_printf("queue empty1\n");
        }
        sleep(1);
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);
    return 1;
}





