#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

#define SIZE 20

int generate_file ()
{
	int rnd, i, fd;

	srand (time (NULL));

	if ((fd = open ("input", O_WRONLY | O_CREAT, 0xFFFF)) == -1)
	{
		perror ("Error opening file\n");
		return 1;
	}
	for (i = 0; i < SIZE; i++)
	{
		rnd = rand () % 100;
		write (fd, &rnd, sizeof (int));
	}

	close (fd);

	return 0;
}

int print_file ()
{
	int a, fd;

	if ((fd = open ("input", O_RDONLY)) == -1)
	{
		perror ("Error opening file\n");
		return 1;
	}

	while (read (fd, &a, sizeof (int)) > 0)
		printf ("%d ", a);

	close (fd);

	return 0;
}

int sort_file ()
{
	int fd, flag, a, b;

	if ((fd = open ("input", O_RDWR)) == -1)
	{
		perror ("Error opening file\n");
		return 1;
	}

	flag = 1;
	while (flag)
	{
		flag = 0;
		if (read (fd, &a, sizeof (int)) == 0)
			return 0;
		while (read (fd, &b, sizeof (int)) > 0)
			if (a > b)
			{
				lseek (fd, -2*sizeof (int), SEEK_CUR);
				write (fd, &b, sizeof (int));
				write (fd, &a, sizeof (int));
				flag = 1;
			}
			else
				a = b;
		lseek (fd, 0, SEEK_SET);
	}

	close (fd);

	return 0;
}

int main ()
{
	if (generate_file ())
		return 1;
	if (sort_file ())
		return 1;
	if (print_file ())
		return 1;

	return 0;
}
