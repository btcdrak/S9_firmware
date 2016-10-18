#include <linux/input.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef enum{false=0,true=!false} bool ;

/*
#define RECOVERY_KEY (gpio221)
#define RED_LED     (hps_led0)
#define GREEN_LED   (hps_led2)
*/


void rg_on(void)
{
    system("echo 1 > /sys/class/leds/hps_led0/brightness");
    system("echo 1 > /sys/class/leds/hps_led2/brightness");
}

void rg_off(void)
{
    system("echo 0 > /sys/class/leds/hps_led0/brightness");
    system("echo 0 > /sys/class/leds/hps_led2/brightness");
}

//To determine whether a file exists
int isExist(char *filename)
{
    return (access(filename, 0) == 0);
}

bool get_gpio(FILE *pFile)
{
	unsigned int value;
	FILE *pRreco = NULL;
	
    if(!isExist("/sys/class/gpio/gpio221"))
    {
        unsigned int value;
        FILE *pRreco = NULL;
        if((pRreco = fopen("/sys/class/gpio/export", "w")) == NULL)
        {
            printf("Open read gpio/export\n");
            return -1;
        }

        if(fwrite("221", strlen("221"), 1, pRreco) != 1)
        printf("File Write Error!\n");
        fclose(pRreco);
    }

    if((pRreco = fopen("/sys/class/gpio/gpio221/value", "r")) == NULL)
	{
	    printf("Open read recovery button failure\n");
		return -1;
	}
	fscanf(pRreco,"%d",&value);
	close(pRreco);
	
    printf("%s: value = %d\n", __FUNCTION__, value);    

    if(value)
		return true;
	else
		return false;
}

int main(int argc, char **argv)
{
	int count = 0;
	int state = 0;
	FILE *pRreco = NULL;
	struct timeval start,end;
	printf("monitor-recobtn compile %s--%s\n", __DATE__,__TIME__);
	while(1)
	{
		if(get_gpio(pRreco) == false)
		{
			gettimeofday( &start, NULL );
			rg_on();
			printf("Detect recovery button push off\n");
			while(1)
			{
				if(get_gpio(pRreco) == true)
				{
					rg_off();
					break;
				}
				gettimeofday( &end, NULL );
				//3S
				if((1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec) > (1000000 * 3))
				{
					printf("recovery button off over 3S, wait push on\n");
					while(get_gpio(pRreco) == false)
					{
						rg_on();
						sleep(1);
						rg_off();
						sleep(1);
					}
					printf("recovery button on\n");
					pid_t pid = fork();
					if ( pid < 0) /* Èç¹û³ö´í */
						printf("error occured.\n");
					else if (pid == 0) {
						rg_off();
						//dup2(1, 0);
						if(argc >= 2)
							execvp(argv[1], argv+1);
						else
						{
							sleep(1);
						}
						exit(0);
					}
					else if (pid > 0)
					{
						wait(NULL);
						printf("child complete\n");
						for(count = 0; count < 100; count++)
						{
							rg_on();
							usleep(100*1000);
							rg_off();
							usleep(100*1000);
						}
						printf("recovery factory complete\n");
					}
				}
				else
					usleep(500*1000);
			}
		}
		else
			sleep(1);
	}
}




