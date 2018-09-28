#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <math.h>
#include <inttypes.h>
#include <pthread.h>

#include "utlist.h"

typedef struct el {
	struct in_addr DstIp;
	char hostname[256];
	struct el *next, *prev;
} el;


pthread_t ThreadsResolve[1000];
void explode(const char *src, const char *tokens, char ***list, size_t *len)
{
	if(src == NULL || list == NULL || len == NULL)
		return;

	char *str, *copy, **_list = NULL, **tmp;
	*list = NULL;
	*len  = 0;

	copy = strdup(src);
	if(copy == NULL)
		return;

	str = strtok(copy, tokens);
	if(str == NULL)
		goto free_and_exit;

	_list = realloc(NULL, sizeof *_list);
	if(_list == NULL)
		goto free_and_exit;

	_list[*len] = strdup(str);
	if(_list[*len] == NULL)
		goto free_and_exit;
	(*len)++;


	while((str = strtok(NULL, tokens)))
	{
		tmp = realloc(_list, (sizeof *_list) * (*len + 1));
		if(tmp == NULL)
			goto free_and_exit;

		_list = tmp;

		_list[*len] = strdup(str);
		if(_list[*len] == NULL)
			goto free_and_exit;
		(*len)++;
	}


free_and_exit:
	*list = _list;
	free(copy);
}


void *ResolveAddressThread(void *argc){

	el *el=argc;
	int buflen=2048;
	char *buf=malloc(buflen);
	int rc, err;

	struct hostent hbuf;
	struct hostent *result;

	while ((rc = gethostbyname_r(el->hostname, &hbuf, buf, buflen, &result, &err)) == ERANGE) {
		printf("WTF !? reallocs");
		buflen *= 2;
		void *tmp = realloc(buf, buflen);
		if (NULL == tmp) {
			free(buf);
			perror("realloc");
		}
		else{
			buf = tmp;
		}
	}	

	if (0 != rc || NULL == result) {
		memset(&el->DstIp,0,4);
	} else 
		memcpy(&el->DstIp,result->h_addr_list[0],4);
	free(buf);
	return NULL;
}

void ResolveAddresses(el *head){

	int t=0,i=0;
	el *el;
	void *status;

	CDL_FOREACH(head,el){
		pthread_create(&ThreadsResolve[t],NULL,ResolveAddressThread,(void *)el);
		t++;
	}
	for(i=0;i<t;i++)  pthread_join(ThreadsResolve[i],&status);
}

int main (int argc, char **argv){

	el *NewDst=NULL,*Head=NULL;
	char **explode_list;
	size_t explode_len;
	int i,j;

	explode(argv[1], ",", &explode_list, &explode_len);
	for(j = 0; j < explode_len; ++j) {
		NewDst= (el*)malloc(sizeof(el));
		strcpy(NewDst->hostname,explode_list[j]);
		CDL_APPEND(Head, NewDst);
	}
	ResolveAddresses(Head); 
}