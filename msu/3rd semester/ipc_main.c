/* This process should be launched first */
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
	signal(SIGINT, signal_handler);
	stop = 1;
}

int main()
{
	struct sembuf semup = { 0, 2, 0 }, semwait = { 0, 0, 0 };
	union semnum
	{
		int val;
	} semset;
	int shmid, semid;
	struct tbuf
	{
		tball ball;
		int endflag;
	} *buf;

	key_t key = ftok("/bin/ls", 'x');
	if ((shmid = shmget(key, sizeof(struct tbuf), 0666 | IPC_CREAT | IPC_EXCL)) == -1)
	{
		perror(NULL);
		return 1;
	}
	if ((semid = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL)) == -1)
	{
		perror(NULL);
		shmctl(shmid, IPC_RMID, NULL);
		return 2;
	}
	signal(SIGINT, signal_handler);
	if ((buf = shmat(shmid, NULL, 0)) == (void *)(-1))
	{
		perror(NULL);
		shmctl(shmid, IPC_RMID, NULL);
		semctl(semid, 0, IPC_RMID);
		return 3;
	}

	buf->ball = 0;
	buf->endflag = 0;
	semset.val = 0;
	semctl(semid, 0, SETVAL, semset);

	while (1)
	{
		semop(semid, &semwait, 1);
		if (stop)
		{
			buf->endflag = 1;
			printf("\nSIGINT recieved, exiting. My ball is %llu\n", buf->ball + 1);
			semctl(semid, 0, SETVAL, semset);
			semop(semid, &semup, 1);
			semop(semid, &semwait, 1);
			break;
		}
		else if (buf->endflag)
		{
			printf("Exiting. My ball is %llu\n", buf->ball);
			break;
		}
		++buf->ball;
		semop(semid, &semup, 1);
	}

	shmdt(buf);
	shmctl(shmid, IPC_RMID, NULL);
	semctl(semid, 0, IPC_RMID);
	return 0;
}