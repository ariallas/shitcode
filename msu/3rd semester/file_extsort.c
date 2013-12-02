#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <limits.h>

#define POSTFIX_LEN 4                   /* Postfix length of splitted files */
#define SIZE 1024 * 1024 / sizeof (int) /* How many int will be in generated file */
#define BUF_SIZE 1000                   /* Max size of ints array */
#define FILE_SIZE 1024 / sizeof (int)   /* How many ints our file can handle 	  */

void swap (int *x, int *y)
{
	int t = *x;
	*x = *y;
	*y = t;
}

int generate_file (char *name)
{
	int rnd, i, fd;

	srand (time (NULL));

	if ((fd = open (name, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
	{
		perror (name);
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

int print_file (char *name)
{
	int a, fd;

	if ((fd = open (name, O_RDONLY)) == -1)
	{
		perror (name);
		return 1;
	}

	while (read (fd, &a, sizeof (int)) >= sizeof (int))
		printf ("%d ", a);
	putchar ('\n');

	close (fd);
	return 0;
}

void generate_file_name (char *name, int n)
{
	int j, cnt = 1;
	name[0] = 'x';
	for (j = n; j > 9; j /= 10)
		cnt++;
	for (j = 1; j <= POSTFIX_LEN - cnt; j++)
		name[j] = '0';
	sprintf (name + j, "%d", n);
	name[POSTFIX_LEN + 1] = '\0';
}

int split_file (char *name, int cnt)
{
	int i, fd, fdp, bytesread, readtotal = 0, buf[BUF_SIZE];
	char spname[POSTFIX_LEN + 2];

	if ((fd = open (name, O_RDONLY)) == -1)
	{
		perror ("Error opening file");
		return 1;
	}

	for (i = 0; i < cnt; i++)
	{
		generate_file_name (spname, i);
		if ((fdp = open (spname, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
		{
			perror ("Error opening file");
			return 1;
		}
		while (BUF_SIZE * sizeof (int) + readtotal <= FILE_SIZE)
		{
			readtotal += bytesread = read (fd, &buf, BUF_SIZE * sizeof (int));
			write (fdp, &buf, bytesread);
		}
		bytesread = read (fd, &buf, FILE_SIZE * sizeof (int) - readtotal);
		write (fdp, &buf, bytesread);
		close (fdp);
	}

	close (fd);
	return 0;
}

int sort_file (char *name)
{
	int fd, cnt, i, j, arr[FILE_SIZE];

	if ((fd = open (name, O_RDWR)) == -1)
	{
		perror (name);
		return 1;
	}

	cnt = 0;
	cnt = read (fd, &arr, FILE_SIZE * sizeof (int)) / sizeof (int);

	for (i = 0; i < cnt; i++)
		for (j = 0; j < cnt - i - 1; j++)
			if (arr[j] > arr[j + 1])
				swap (&arr[j], &arr[j + 1]);

	lseek (fd, 0, SEEK_SET);
	write (fd, arr, cnt * sizeof (int));
	close (fd);

	return 0;
}

int merge_files (char *name,  int cnt)
{
	int i, j, fd1, fd2, fdt, a, b;
	char file1[POSTFIX_LEN + 2], file2[POSTFIX_LEN + 2];
	int buf[BUF_SIZE], bytesread, readfrom, bufcount;

	for (i = 1; i < cnt; i *= 2)
	{
		int ncnt;
		if (i > 1)
			ncnt = (cnt + 1) / i;
		else
			ncnt = cnt;
		for (j = 0; j < ncnt - 1; j += 2)
		{
			generate_file_name (file1, j);
			generate_file_name (file2, j + 1);
			if ((fd1 = open (file1, O_RDONLY)) == -1 || (fd2 = open (file2, O_RDONLY)) == -1
											|| (fdt = open ("temp.tmp", O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
			{
				perror ("Error opening file");
				return 1;
			}

			bytesread = read (fd1, &a, sizeof (int));
			if (bytesread >= sizeof (int))
				bytesread = read (fd2, &b, sizeof (int));
			readfrom = 2;
			bufcount = 0;

			while (bytesread >= sizeof (int))
			{
				if (b < a)
					swap (&a, &b);
				else if (readfrom == 1)
					readfrom = 2;
				else
					readfrom = 1;

				buf[bufcount++] = a;
				if (bufcount == BUF_SIZE)
				{
					write (fdt, buf, bufcount * sizeof (int));
					bufcount = 0;
				}
				a = b;
				if (readfrom == 1)
					bytesread = read (fd1, &b, sizeof (int));
				else
					bytesread = read (fd2, &b, sizeof (int));
			}

			if (readfrom == 1)
				swap (&fd1, &fd2);
			do
			{
				buf[bufcount++] = a;
				if (bufcount == BUF_SIZE)
				{
					write (fdt, buf, bufcount * sizeof (int));
					bufcount = 0;
				}
			}
			while (read (fd1, &a, sizeof (int)) >= sizeof (int));

			write (fdt, buf, bufcount * sizeof (int));
			bufcount = 0;

			close (fd1);
			close (fd2);
			close (fdt);

			remove (file1);
			remove (file2);
			generate_file_name (file1, j / 2);
			rename ("temp.tmp", file1);
		}

		if (ncnt % 2)
		{
			generate_file_name (file1, ncnt - 1);
			generate_file_name (file2, ncnt / 2);
			rename (file1, file2);
			remove (file1);
		}
	}

	generate_file_name (file1, 0);
	rename (file1, name);
	remove (file1);

	return 0;
}

int get_file_size (char *name)
{
	struct stat st;
	stat (name, &st);
	return st.st_size;
}

int main ()
{
	int i, cnt;
	char name[] = "input.txt", spname[POSTFIX_LEN + 2];

	if (generate_file (name))
		return 1;
	cnt = (get_file_size (name) - 1) / (FILE_SIZE * sizeof (int)) + 1;
	if (split_file (name, cnt))
		return 1;
	puts ("Done splitting files");

	for (i = 0; i < cnt; i++)
	{
		generate_file_name (spname, i);
		if (sort_file (spname))
			return 1;
	}
	puts ("Done sorting files");

	if (merge_files (name, cnt))
		return 1;
	puts ("Done merging files");

	return 0;
}
