/* This one should be launched second */
#include <stdio.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <signal.h>

typedef unsigned long long int tball;
int stop = 0;

void signal_handler(int sig)
{
	stop = 1;
	signal(SIGINT, signal_handler);
}

int main()
{
	struct sembuf semdown = { 0, -1, 0 };
	int shmid, semid;
	struct tbuf
	{
		tball ball;
		int endflag;
	} *buf;

	key_t key = ftok("/bin/ls", 'x');
	if ((shmid = shmget(key, sizeof(struct tbuf), 0666)) == -1 ||
		(semid = semget(key, 1, 0666)) == -1)
	{
		perror(NULL);
		return 1;
	}
	signal(SIGINT, signal_handler);

	if ((buf = shmat(shmid, NULL, 0)) == (void *)(-1))
	{
		perror(NULL);
		return 2;
	}

	while (1)
	{
		semop(semid, &semdown, 1);
		if (stop)
		{
			buf->endflag = 1;
			printf("\nSIGINT recieved, exiting. My ball is %llu\n", buf->ball + 1);
			semop(semid, &semdown, 1);
			break;
		}
		else if (buf->endflag)
		{
			printf("Exiting. My ball is %llu\n", buf->ball);
			semop(semid, &semdown, 1);
			break;
		}
		++buf->ball;
		semop(semid, &semdown, 1);
	}

	shmdt(buf);
	return 0;
}