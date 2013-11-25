#include <stdio.h>
#include <string.h>

int clear (const char *src, char *dst, int wordlen)
{
	int i, dstlen = 0, len, maxlen, srclen = strlen (src);

	if (wordlen < 0)
		return -1;

	i = 0;
	while (i < srclen)
	{
		int repfound = 0;
		if ( (srclen - i)/2 > wordlen )
			maxlen = wordlen;
		else
			maxlen = (srclen - i)/2;
		for (len = maxlen; len > 0; len--)
			if (!strncmp (src + i, src + i + len, len))
			{
				repfound = 1;
				break;
			}
		if (repfound)
			i += len*2;
		else
		{
			dst[dstlen] = src[i];
			dstlen++;
			i++;
		}
	}

	dst[dstlen] = '\0';

	return 0;
}

int main ()
{
	char a[50], b[50];
	scanf ("%s", a);
	clear (a, b, 3);
	puts (b);

	return 0;
}