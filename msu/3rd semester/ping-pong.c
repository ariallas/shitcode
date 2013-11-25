#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int stop = 0;

void show_ball(int ball, char *who, int flag, int to, int from)
{
	char s[20] = "";
	close (from);
	close (to);
	if (flag)
		strcpy(s, " I have the ball!");
	printf("%s: %d%s\n", who, ball, s);
	exit(0);
}

void play(int to, int from, char *who)
{
	int ball = 0;
	while (1)
	{
		if (stop || read(from, &ball, sizeof(int)) < sizeof(int))
			show_ball(ball, who, 0, to, from);

		ball++;

		if (stop)
			show_ball(ball, who, 1, to, from);
		write(to, &ball, sizeof(int));
	}
}

void signal_handler(int sig)
{
	stop = 1;
}

/*    <--------------(2)------------------ *
 * father --(0)--> son1 --(1)--> son2 ---^ */

int main()
{
	int pipes[3][2], pid, ball = 0;
	char who[10];
	pipe(pipes[0]);
	pipe(pipes[1]);
	pipe(pipes[2]);

	signal(SIGINT, signal_handler);
	signal(SIGPIPE, signal_handler);

	if ((pid = fork()) < 0)
		return 1;
	else if (!pid)
	{
		/* son1 */
		close(pipes[0][1]);
		close(pipes[2][0]);
		close(pipes[2][1]);
		close(pipes[1][0]);
		strcpy(who, "Son 1");
		play(pipes[1][1], pipes[0][0], who);
	}

	if ((pid = fork()) < 0)
		return 1;
	else if (!pid)
	{
		/* son2 */
		close(pipes[2][0]);
		close(pipes[0][0]);
		close(pipes[0][1]);
		close(pipes[1][1]);
		strcpy(who, "Son 2");
		play(pipes[2][1], pipes[1][0], who);
	}

	/* father */
	close(pipes[0][0]);
	close(pipes[1][0]);
	close(pipes[1][1]);
	close(pipes[2][1]);
	strcpy(who, "Father");
	write(pipes[0][1], &ball, sizeof(int));
	play(pipes[0][1], pipes[2][0], who);

	return 0;
}