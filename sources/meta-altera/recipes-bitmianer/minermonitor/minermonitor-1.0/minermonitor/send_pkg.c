#include "send_pkg.h"
#include "debug.h"
#include "config.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "dataexchange.h"
#include "rev_queue.h"
char *ip_pc="192.168.1.113";

char buf[BUFFER_SIZE];//与服务器之间的buf
char serv_v[3];       //接收服务器开始两字节
char serv_cmd[3];     //服务器命�?
char body_len[2];     //服务器包长度
char auth_id[70];     //保存认证时服务器和本地生成的cUNIQID
int sockfd;
SSL_CTX *ctx;
SSL *ssl;
int sendbytes;
int recvbytes; //�?
PKG_Format package;
PKG_Format deal_package;
GETS4_CONFIG get_cgminer_conf;
GETS3_CONFIG gets3_cgminer_conf;
POOLS_D_COUNT changed_pools;
char rev_ID[40];

///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////
////////////清空上次，接收到的数�?/////////////
void clear_recv(PKG_Format pkg_struct)
{
    memset(serv_v,0,3);
    memset(serv_cmd,0,3);
    memset(body_len,0,2);
    memset(buf,0,BUFFER_SIZE);

    memset(pkg_struct.pkg_head,0,3);
    memset(pkg_struct.pkg_cmd,0,3);
    memset(pkg_struct.pkg_body,0,BUFFER_SIZE);
    memset(rev_ID,0,40);
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
///////////接收服务器数据，并对头解枿并判斿/////////////

int  rev_data_struct()
{
    char buff[100];
    memset(buff,0,100);
    DEBUG_printf("r_connect = %d.\n",r_connect);
    if((recvbytes = receivedata(ssl,sockfd,&package.pkg_head,2)) == -1)         //开头两字节
    {
        DEBUG_printf("recv data failed1.\n");
        r_connect = 0;//the failure of receiving data prove the break of connection,mark it at here.
        clear_recv(deal_package);
        ClearQueue(&Q_cmd);//clear the comdmand queue when failed to receive data.
        ClearQueue(&Q_respond);//clear the comdmand queue when failed to receive data.
        return 0;
        //perror("recv");
        //exit(1);
    }
    DEBUG_printf("recv package.pkg_head:%s.\n",package.pkg_head);
    package.pkg_head[recvbytes]='\0';
    if(recvbytes==0)
    {
        DEBUG_printf("the first two byte not 00   it is 0\n");
        return 0;
    }
    else if(strcmp(package.pkg_head,"00")!=0)
    return -1;  //bad package
    if((recvbytes = receivedata(ssl,sockfd,&package.pkg_cmd,2)) == -1) //命令两字�?
    {
        r_connect = 0;
        return 0;
        //perror("recv");
        //exit(1);
    }
    if(recvbytes==0)
        return 0;
    else if(recvbytes!=2)
        return -1; //bad package
    package.pkg_cmd[recvbytes]='\0';
    DEBUG_printf("rev from server cmd = \n%s\n",package.pkg_cmd);
    if ((recvbytes = receivedata(ssl,sockfd,&package.pkg_body_len,2)) == -1)  //接收的body长度两字�?
    {
        r_connect = 0;
        return 0;
        //perror("recv");
        //exit(1);
    }
    if(recvbytes==0)
        return 0;
    else if(recvbytes!=2)
        return -1; //bad package
    int sum;
    //DEBUG_printf("pkg_body_len[0]=%c\n",package.pkg_body_len[0]);
    //DEBUG_printf("pkg_body_len[1]=%c\n",package.pkg_body_len[1]);
    sum=package.pkg_body_len[0]*256+package.pkg_body_len[1];                              // 计算body长度
    DEBUG_printf("rev from server sum=\n%d\n",sum);
    memset(package.pkg_body,0,BUFFER_SIZE);
    if(sum==0)
    {
        package.pkg_body[0]='\0';
    }
    else
    {
        if ((recvbytes = receivedata(ssl,sockfd,&package.pkg_body,sum)) == -1)  //按长度接收body
        {
            r_connect = 0;
            return 0;
            //perror("recv");
            //exit(1);
        }
        package.pkg_body[recvbytes]='\0';
        DEBUG_printf("rev_buf=\n%s\n",package.pkg_body);
        if(recvbytes==0)
        {
            r_connect = 0;
            return 0;
        }
        else
        {
            if(recvbytes!=sum)                                 //判断接收的body是�??满�?
            {
                DEBUG_printf("PAKGE ERROR!!!!!\n");
                return -1;
                //exit(1);
            }
        }
    }
    return 1;
}//*/


/////////////////////////////////////////////////
//////////////rev_respond///////////////////////
//////////接收服务器，第二次返回的数据////////////////
void rev_respond()
{
    char *res;
    //if(0==rev_data())
    res=deal_package.pkg_cmd;
    //判断server的回复信号，
    if(strcmp(res,"f1")==0)
    {
        DEBUG_printf("Server respond  F1\n\n");
        exit(0);
    }
    else if(strcmp(res,"f2")==0)
    {
        DEBUG_printf("Server respond  F2\n\n");
        exit(0);
    }
    else if(strcmp(res,"f3")==0)
    {
        DEBUG_printf("Server respond  F3\n\n");
        exit(0);
    }
    else if(strcmp(res,"f4")==0)
    {
        DEBUG_printf("Server respond  F4\n\n");
        exit(0);
    }
    else if(strcmp(res,"f5")==0)
    {
        DEBUG_printf("Server respond  F5\n\n");
    }
    else if(strcmp(res,"f6")==0)
    {
         DEBUG_printf("Server respond  F6\n\n");
         DEBUG_printf("F6 exit!!!!!!\n");
         exit(0);

    }
    else if(strcmp(res,"f7")==0)
    {
        DEBUG_printf("Server respond  F7\n\n");
        exit(0);
    }
    else
    {
        DEBUG_printf("respond rev : %s",res);
        exit(0);
    }
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
///////发送前6个字芿每次向服务器发送都�?///////////////
char send_head(char *cmd,unsigned int body_len)
{
 unsigned char body_len1;
 unsigned char body_len2;
 body_len1=body_len/256;DEBUG_printf("body_len1=%d\n",body_len1);
 body_len2=body_len%256;DEBUG_printf("body_len2=%d\n",body_len2);
//Packg inpackg 6 byte;
  if ((sendbytes = senddata(ssl,sockfd,"00",sizeof("00")-1)) == -1)
    {
     perror("send head");
     exit(1);
    }//perror("send");
  if ((sendbytes = senddata(ssl,sockfd,cmd,strlen(cmd))) == -1)
    {
      perror("send");
       exit(1);
    }//perror("send cmd");
    //DEBUG_printf("sizeof(cmd)=%d\n",strlen(cmd));
   if ((sendbytes = senddata(ssl,sockfd,&body_len1, sizeof(char))) == -1)
    {
     perror("send");
      exit(1);
    }
   if ((sendbytes = senddata(ssl,sockfd,&body_len2, sizeof(char))) == -1)
    {
     perror("send");
      exit(1);
    }
    return 1;
}

void send_connect_auth(char *cmd)
{
 char *total_packg=NULL;
 char *userid;
 char *mac_data;
 char *cuniqid;
 char uid_tmp[60];
 char type[10];
 memset(uid_tmp,0,sizeof(char)*60);
 memset(type,0,sizeof(char)*10);
 cuniqid= (char*)calloc(10,sizeof(char));
 mac_data=(char*)calloc(30,sizeof(char));
 userid=(char*)calloc(60,sizeof(char));
 #ifdef S3_IP
 strcpy( userid,"UID=");
 strcat( userid,read_luci_conf("user_id"));
 strcat( userid,",");
 strcpy(type,"TYPE=3");
 unsigned char authbody_size=strlen(gen_cuniqid(cuniqid))+strlen(get_mac(mac_data,"eth1"))+strlen(userid)+strlen(type);
 send_head(cmd,authbody_size);
 #endif
 #ifdef S4_IP
 strcpy( userid,"UID=");
 strcat( userid,read_s4_switch(s4_conf_dir,"api-uid",uid_tmp));
 strcat( userid,",");
 stpcpy(type,"TYPE=400");
  unsigned char authbody_size=strlen(gen_cuniqid(cuniqid))+strlen(get_mac(mac_data,"eth0"))+strlen(userid)+strlen(type);
 send_head(cmd,authbody_size);
 #endif
  #ifdef PC_IP
 strcpy( userid,"UID=");
 strcat( userid,read_s4_switch(s4_conf_dir,"api-uid",uid_tmp));
 strcat( userid,",");
 stpcpy(type,"TYPE=5");
 unsigned char authbody_size=strlen(gen_cuniqid(cuniqid))+strlen(get_mac(mac_data,"eth0"))+strlen(userid)+strlen(type);
 send_head(cmd,authbody_size);
 #endif
 total_packg=(char*)calloc((authbody_size+4),sizeof(char));
 strcpy(total_packg,cuniqid);
 strcat(total_packg,mac_data);
 strcat(total_packg,userid);
 strcat(total_packg,type);
 if ((sendbytes = senddata(ssl,sockfd,total_packg,authbody_size)) == -1)
  {
    perror("send");
    //exit(1);
  }
 DEBUG_printf("send pakge str :%s\n",total_packg);

 free(mac_data);
 free(userid);
 free(cuniqid);
 if(total_packg)
 free(total_packg);
}

////////////////////////////////////////////////////
////////////Send CONNECT_CC/////////////////////////
int send_connect_cc(char *cmd)
{
  int revbody_size=strlen(buf);
  DEBUG_printf("02revbody_size=%d\n",revbody_size);
  send_head(cmd,revbody_size);
  if ((sendbytes = senddata(ssl,sockfd,buf, revbody_size)) == -1)
  {
    perror("send");
    exit(1);
  }
DEBUG_printf("send what I rev:\n%s\n",buf);
return 1;
}


/////////////////////03--30--03--30////////////////////////////
///////////////////get_status_res//////////////////////////////
////////////get  cgminer  state&& send////////////////////
int send_getstatus_res(char *cmd_server_code)
{
    char *api_len=NULL;
    char *buf_data=NULL;
    char *cuniqid;
    int len,i;
    short int port;
    int statebody_size=0;
    char ipaddr[40];
    char *cmd_server;
    char *s4_dataformat_dir;
    char *api_command;
    for(i=0;i<COMMAND_SUM;i++)
    {
        if(strcmp(cmd_server_code,server_command_code[i])==0)
        {
            s4_dataformat_dir =(char *)malloc(51 * sizeof(char));
            memset(s4_dataformat_dir,0,51);
            strcpy(s4_dataformat_dir,dataformat_conf_dir[i]);

            api_command = (char *)malloc(11 * sizeof(char));
            memset(api_command,0,11);
            strcpy(api_command,server_command[i]);

            cmd_server = (char *)malloc(5 * sizeof(char));
            memset(cmd_server,0,5);
            strcpy(cmd_server,command_code[i]);
            break;
        }
    }

//  DEBUG_printf("conf_dir :%s\n\n\n",s4_dataformat_dir);
//  DEBUG_printf("apicommand:%s\n\n\n",api_command);
//  DEBUG_printf("api_server:%s\n\n",cmd_server);

    buf_data=(char*)calloc(4096,sizeof(char));
    cuniqid= (char*)calloc(10,sizeof(char));
    len=strlen(gen_cuniqid(cuniqid));
    //Get_Port
    //Get_IP
    #ifdef S3_IP
    //port=atoi(read_conf("port_local"));
    port=4028;
    if(!get_ip("eth1",ipaddr))
    {
        DEBUG_printf("get IP error\n");
    }
    #endif
    #ifdef S4_IP
    char port_str[10];
    port=atoi(read_s4_switch(s4_conf_dir,"port_local",port_str));
    if(!get_ip("eth0",ipaddr))
    {
        DEBUG_printf("get IP error\n");
    }
    #endif
    #ifdef PC_IP
    port=4028;
    strcpy(ipaddr,ip_pc);
    #endif

    api_len=callapi(api_command, ipaddr,port);

    if(strlen(api_len) == 0)
    {
        DEBUG_printf("nothing returned form callapi\n\n\n ");
        return 0;
    }
    DEBUG_printf("the result of callapi is %s\n",api_len);

    statebody_size=strlen(data_format(s4_dataformat_dir,buf_data,api_len));

    DEBUG_printf("state_len=%d\n",(statebody_size+len));
    send_head(cmd_server,(statebody_size+len));


    if ((sendbytes = senddata(ssl,sockfd,cuniqid, len)) ==-1)
    {
        perror("send");
        //exit(1);
    }
    DEBUG_printf("sendbytes1111111111=%d\n",sendbytes);
    DEBUG_printf("send myid :%s\n",cuniqid);


    if ( (sendbytes =senddata(ssl,sockfd,buf_data, (statebody_size))) ==-1)
    {
        perror("send");
        //exit(1);
    }
    DEBUG_printf("sendbytes222222222222222=%d\n",sendbytes);
    DEBUG_printf("send pakge str :%s\n",buf_data);

    if(api_len)
        free(api_len);
    free(buf_data);
    free(cuniqid);
    return 1;
}


//////////////////////////////////////////////////////重启5s后尝试链接cgminer,如果链接失败则，再等5s，尝试连�?�?
////////////设定矿机矿池参数//////////////////////////////
//write  cgminer.conf文件
int write_config_s4(PKG_Format pkg_struct)
{
    GETS4_CONFIG *new_pools_data;
    new_pools_data=creat_newpools(rev_pools(pkg_struct.pkg_body,&changed_pools),save_old_conf(s4_cgc_dir,&get_cgminer_conf));

    create_conf(new_pools_data,s4_cgc_dir);
    //restart_cgminer();
    return 1;
}
int write_config_s3(PKG_Format pkg_struct)
{
   GETS3_CONFIG *new_pools_data;
   new_pools_data=creat_s3_newpools(rev_pools(pkg_struct.pkg_body,&changed_pools),save_s3_webconf(&gets3_cgminer_conf));
   write_S3_conf(new_pools_data);
   //restart_cgminer();
   return 1;
}

char restart_cgminer()  //restart  cgminer
{
 struct hostent *ip;
 struct sockaddr_in serv;
 int sock;
 char ipaddr[40];

 short cg_port;
 #ifdef S3_IP
  if(!get_ip("eth1",ipaddr))
  {
     DEBUG_printf("get IP error\n");
  }
  //cg_port=atoi(read_conf("port_local"));
   cg_port=4028;
  #endif
  #ifdef S4_IP
   char port_str[10];
  cg_port=atoi(read_s4_switch(s4_conf_dir,"port_local",port_str));
  if(!get_ip("eth0",ipaddr))
  {
     DEBUG_printf("get IP error\n");
  }
  #endif
  #ifdef PC_IP
  cg_port=4028;
  //printf("cg_port= %d\n",cg_port);
  strcpy(ipaddr,ip_pc);
  #endif

 sigset_t sigs;
 sigemptyset(&sigs);
 sigaddset(&sigs,SIGPIPE);
 sigprocmask(SIG_BLOCK,&sigs,0);
 #ifdef S3_IP
 system("/etc/init.d/cgminer restart");
 #endif
 #ifdef S4_IP
 system("/etc/init.d/cgminer.sh restart");
 #endif
 #ifdef PC_IP
 system("/etc/init.d/cgminer.sh restart");
 #endif

 sigprocmask(SIG_UNBLOCK,&sigs,0);
  sleep(5);      //重启5s后尝试链接cgminer,如果链接失败则，再等5s，直到重启成�?
 ip = gethostbyname(ipaddr);
 if (!ip) {
        DEBUG_printf("Failed to resolve host \n");
        return 0;
    }
 sock = socket(AF_INET, SOCK_STREAM, 0);
 if (sock == -1) {
        DEBUG_printf("Socket initialisation failed: %s\n", strerror(errno));
        return 0;
    }
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr = *((struct in_addr *)ip->h_addr);
    serv.sin_port = htons(cg_port);
    int i=6;
    while (connect(sock, (struct sockaddr *)&serv, sizeof(struct sockaddr))<0 && (i--)>0)
    {
      DEBUG_printf("Cgminer!Socket connect failed: %s\n", strerror(errno));
      DEBUG_printf("restart not well!!\nwait next 5s......\n");
      sleep(5);
    }
    close(sock);
    if(i<=0)
    {
      DEBUG_printf("restart timeout!\n");
      return -1;
    }
    else
    {
        DEBUG_printf("restart OK!\n");
        return 1;
    }
}

// After reboot is complete, client send SET_CONFIG_RES
void send_setconfig_result(char *cmd,char *conf_stat)
{
  char *cuniqid;
  char *buf_data;
  int body_len;
  cuniqid= (char*)calloc(10,sizeof(char));
  buf_data=(char*)calloc(60,sizeof(char));
  strcpy(buf_data,gen_cuniqid(cuniqid));
  strcat(buf_data,"ID=");
  strcat(buf_data,rev_ID);
  strcat(buf_data,",result=");
  strcat(buf_data,conf_stat);
  body_len=strlen(buf_data);
  send_head(cmd, body_len);
  if((sendbytes = senddata(ssl,sockfd,buf_data, body_len)) == -1)
  {
    perror("send");
    //exit(1);
  }
DEBUG_printf("send cgminer state :%s\n",buf_data);
free(cuniqid);
free(buf_data);


}

////////////////////////////////////////////////////
//////////////升级Cgminer////////////////////////////
//通过libcurl获取url地址数据
void curl_get()
{
 CURL *curl;
 FILE *fp;
 size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
 {
 int written = fwrite(ptr, size, nmemb, (FILE *)fp);
 return written;
 }
 curl_global_init(CURL_GLOBAL_ALL);
 curl=curl_easy_init();
 curl_easy_setopt(curl, CURLOPT_URL, "www.baidu.com");

 if((fp=fopen("save.txt","w"))==NULL)
 {
  curl_easy_cleanup(curl);
  exit(1);
 }
 ////CURLOPT_WRITEFUNCTION 将后继的动作交给write_data函数处理
 curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
 curl_easy_perform(curl);
 curl_easy_cleanup(curl);
}

int updata()     //还没完成
{
     /*
     * 解析收到数据中的命令
     * ………………………………………�?
     */
return 1;
}


int deal_cmd(PKG_Format pkg_struct)
{
    if(strcmp(pkg_struct.pkg_cmd,"01") == 0)
    {
        send_connect_auth("10");   //对服务器命令的所有回应操�?
    }
    else if(strcmp(pkg_struct.pkg_cmd,"06") == 0)
    {
        #ifdef S4_WRITE
        write_config_s4(pkg_struct);
        #endif
        #ifdef S3_WRITE
        write_config_s3(pkg_struct);
        #endif
        if(restart_cgminer()==1)
        {
            send_setconfig_result("60","S");
            DEBUG_printf("restart cgminer OK OK \n");
        }
        else
        {
            send_setconfig_result("60","F");
            DEBUG_printf("restart cgminer FAILD \n");
            return 0;
        }
    }
    else if(send_getstatus_res(pkg_struct.pkg_cmd)==0)
    {
        DEBUG_printf("after send_getstatus_res()\n");
        return 0;
    }
    clear_recv(pkg_struct);
    return 1;
}


