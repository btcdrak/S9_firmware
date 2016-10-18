#ifndef _READCONF_FILE_H
#define _READCONF_FILE_H
#include <stdint.h>
#include <stdio.h>
#include <syslog.h>
#define    MAX_PATH_LEN         (512)
#define    MAX_FILE_NAME_LEN    (128)
char *conf_directory;
char *luci_conf_directory;
char *luci_cgminer_dir;
//////////////////////////////////////////////////
/////////////向client.c 传递读到的配置文件的各个项
////////主要通过下面的两个函数完成。
//// 在client.c中用read_conf(char *item)函数实现这个功能。
/* 将配置文件中的配置项解析后存放到全局数组中 */
int parse_luciconf_file(char *path_to_config_file);
int parse_myconf_file(char *path_to_config_file);
/* 从全局数组中读取一个个配置项。例如  get_config_var("var_name1");  */
char * get_config_var(char *var_name);
char *read_luci_conf(char *item);
char *read_conf(char *item);
char *read_cgminer_conf(char *item);
#endif
