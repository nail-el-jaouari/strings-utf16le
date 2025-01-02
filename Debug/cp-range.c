#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>

#define BUF_SIZE 4096

void usage(int err)
{
    FILE* f = err ? stderr : stdout;

    fprintf(f, "usage cp [-s <skip>] [-t <til>] -i <input_file> -o <output_file>\n");
}

int main(int argc, char *argv[])
{
    int skip_f;
    int til_f;
    int infile_f;
    int outfile_f;
    off_t skip;
    off_t til;
    const char *in_name;
    const char *out_name;
   
    skip = 0;
    til = 0;
    in_name = NULL;
    out_name = NULL;

    for (;;)
    {
        int opt = getopt(argc, argv, "s:t:i:o:");
        if (opt == -1)
        {
            break;
        }
        switch (opt)
        {
            case 's':
            {
                skip = atoll(optarg);
                skip_f = 1;
            }
            break;
            case 't':
            {
                til = atoll(optarg);
                til_f = 1;
            }
            break;
            case 'i':
            {
                in_name = optarg;
                infile_f = 1;
            }
            break;
            case 'o':
            {
                out_name = optarg;
                outfile_f = 1;
            }
            break;
            default:
            {
                usage(1);
                return EXIT_FAILURE;
            }
        }
    }

    if (infile_f == 0 || outfile_f == 0)
    {
        usage(1);
        return EXIT_FAILURE;
    }

    int fdin;
    int fdout;
    int ret = 0;

    fdin = open(in_name, O_RDONLY);
    if (fdin < 0)
    {
        ret = 1;
        goto EXIT;
    }

    fdout = open(out_name, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR);
    if (fdout < 0)
    {
        ret = 1;
        goto CLOSE_IN;
    }

    struct stat st;
    fstat(fdin,&st);

    if ( til_f == 1)
    {
        if (til > st.st_size)
        {
            fprintf(stderr, "til value must be less or equal <file_in> size\n");
            return EXIT_FAILURE;
        }
    }
    else
    {
        til = st.st_size;
    }

    
    if (skip > til)
    {
        fprintf(stderr, "skip value must be less or equal til value\n");
        return EXIT_FAILURE;
    }

    if ( skip_f == 0)
    {
        skip = 0;
    }

    char buffer[BUF_SIZE];
    
	
	lseek(fdin, (long)skip, SEEK_SET);

    off_t offset = 0;
    off_t old_offset = 0;

	for (;;)
	{
        ssize_t bytes_read = read(fdin, buffer, sizeof *buffer);

        if (bytes_read <= 0 || (offset >= (til - skip)))
        {
            break;
        };

        old_offset = offset;
        offset += bytes_read;

        bytes_read = (offset > til - skip) ? (til - skip - old_offset) : bytes_read;
        
        // write to file file_out
        for (;;)
        {
            size_t bytes_written;
            char *p = buffer;

            if (bytes_read <= 0)
            {
                break;
            }

            bytes_written = write(fdout, buffer, bytes_read);

            if (bytes_written > 0)
            {
                bytes_read -= bytes_written;
                p += bytes_written;
            }
        }
	}

CLOSE_OUT:
    close(fdout);
CLOSE_IN:
    close(fdin);
EXIT:
	return ret;
}
