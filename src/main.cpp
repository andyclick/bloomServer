#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "util.h"
#include "InfoCrawler.h"

static void disconnect();
static void usage();

int main(int ac, char **av)
{
    int			c;
    FILE *logfp = NULL;
    //extern char		*optarg;
    //
    //
    // Parse command line arguments
    //
    bool deamon = true;
    bool onlyeexternal = false;
    while ((c = getopt(ac, av, "ade")) != -1)
    {
        switch (c)
        {
            case 'd':
                deamon = false;
                break;
            case 'e':
                onlyeexternal = true;
                break;
            case 'a':
                break;
            case '?':
                usage();
                break;
            default:
                printf("testsetset");
                break;
        }
    }
    
    int fork_result = -1;
    int fork_try;

    if (deamon) {
        disconnect(); 
        fork_result = fork();
    } else {
        fork_result  = 0;
    }
    if (fork_result == 0) { // children process
        //start server
        InfoCrawler *infocrawler = NULL;
        if (onlyeexternal) {
        } else {
            infocrawler = InfoCrawler::getInstance(START_TYPE_DEFAULT);
        }
        infocrawler->startServer();
    } else if (fork_result == -1) { //error
       // LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
        //mylog_error(m_pLogGlobalCtrl->errorlog, "Server starting error!!! - %s:%s:%d:%d\n", ERROR_LOG_SUFFIX);
    }
}
//
// Display usage information for the kurobot program
//
void usage()
{
    printf("usage: infocrawler\n");
    exit(0);
}

static void disconnect()
{
	int pid, fd;

#ifdef SIGTTOU
	signal(SIGTTOU, SIG_IGN);
#endif
#ifdef SIGTTIN
	signal(SIGTTIN, SIG_IGN);
#endif
#ifdef SIGTSTP
	signal(SIGTSTP, SIG_IGN);
#endif
    
    //LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
	if ((pid = fork()) < 0) {
        //mylog_error(m_pLogGlobalCtrl->errorlog, "fork - %s:%s:%d:%d\n", ERROR_LOG_SUFFIX);
		return;
	}
	if (pid) {		/* parent */
		exit(0);	/* quietly exit to let child do the work */
	}
	(void) setsid();
	/* Logging beneath here will not work */
	/* Close all file descriptors */
	close_all_fds(0);

	/* Redirect the stdin, stdout, and stderr to /dev/null */
	if ((fd = open("/dev/null", O_RDWR)) < 0) {
		exit(1);
	}
	(void) dup2(fd, 0);
	(void) dup2(fd, 1);
	(void) dup2(fd, 2);
    
}


