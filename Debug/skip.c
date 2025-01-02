#include <stdio.h>
//#include <strings.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int skip = atoi(argv[1]);
	FILE *in = fopen(argv[2], "r");
	FILE *out = fopen(argv[3], "w");

	char buffer[1024];
	//int fseek(FILE *stream, long offset, int whence);
	//size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
	// size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
	size_t bytes_read = 0;
	off_t offset = 0LL;
	
	fseek(in, (long)skip, SEEK_SET);

	/* printf("buffer size %lu\n", sizeof buffer / sizeof *buffer); */
	/* printf("buffer elem size %lu\n", sizeof *buffer); */
	/* printf("skip %d\n", skip); */

	while ((bytes_read = fread(buffer, sizeof *buffer, sizeof buffer / sizeof *buffer, in)) > 0)
	{
		fwrite(buffer, sizeof *buffer, bytes_read, out);
	}


	return 0;
}
