#ifndef _CLIENT_H
#define _CLIENT_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/time.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "config.h"
#include<sys/param.h>
#include<sys/stat.h>
//#include "miner.h"
//#include "config.h"
//#include "compat.h"
//#include <jansson.h>
//#include <curl/curl.h>
//////////////////////////////////////////////////////
////////////数据结构变量////////////////////////////////
//#define CG_RECVSIZE 65500             //读gminer-api的buf
#define SERVER_IP     "192.168.1.185" //连接服务器的地址
#define SERVER_PORT   2000            //服务器的端口
#define CONNECT_MAX 5
#define BUFFER_SIZE 4096
#define PKG_NUM  6
#define COMMAND_SUM 5

extern int sockfd;
extern SSL *ssl;
extern SSL_CTX *ctx;
extern int sendbytes;
extern int recvbytes; //与服务器之间建立socket
extern char buf[BUFFER_SIZE];//与服务器之间的buf
extern char serv_v[3];       //接收服务器开始两字节
extern char serv_cmd[3];     //服务器命仿
extern char body_len[2];     //服务器包长度
extern char auth_id[70];     //保存认证时服务器和本地生成的cUNIQID

extern int pthr_rev_id;

extern char *conf_directory;  //配置文件目录
extern char *luci_conf_directory; //web网页配置文件目录
extern char *luci_cgminer_dir;
extern char *s4_cgc_dir;
extern char *s4_conf_dir;


#ifdef S3_IP
char *luci_conf_directory= "/etc/config/monitor-webset"; //要读取的luci配置文件目录
char *luci_cgminer_dir="/etc/config/cgminer";
#endif
#ifdef S4_IP
char *s4_conf_dir="/config/minermonitor.conf";
char *s4_cgc_dir = "/config/cgminer.conf";
char command_code[COMMAND_SUM][4]={"10","20","30","40","50"};//after switch
char server_command_code[COMMAND_SUM][4]={"01","02","03","04","05"};//source command code from server
char server_command[COMMAND_SUM][10]={"AUTH","devs","summary","stats","pools"};
char dataformat_conf_dir[COMMAND_SUM][80]={"","/config/dataformatconfig/dataformat_devs.conf","/config/dataformatconfig/dataformat_summary.conf","/config/dataformatconfig/dataformat_stats.conf","/config/dataformatconfig/dataformat_pools.conf"};

#endif
#ifdef PC_IP
//char *s4_conf_dir="minermonitor.conf";
//char *s4_cgc_dir = "cgminer.conf";
//char *conf_directory= "monitor.conf"; //要读取的配置文件目录
char *luci_conf_directory= "/home/sonny/Desktop/s3_ssl/S3_ssl/bin/Debug/monitor-webset"; //要读取的luci配置文件目录
char *luci_cgminer_dir="cgminer";
#endif
//char *conf_directory= "/etc/config/monitor.conf"; //要读取的配置文件目录
//char *s4_conf_dir="/config/minermonitor.conf";
//char *s4_cgc_dir = "/config/cgminer.conf";

//For PC Test
//char *s4_conf_dir="minermonitor.conf";
//char *s4_cgc_dir = "cgminer.conf";
//char *conf_directory= "monitor.conf"; //要读取的配置文件目录
//char *luci_conf_directory= "monitor-webset"; //要读取的luci配置文件目录
//char *luci_cgminer_dir="cgminer";


#endif
















