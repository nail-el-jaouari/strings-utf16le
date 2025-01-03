#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <iconv.h>
#include <errno.h>
#include <string.h>

#ifndef BUF_SZ
#define BUF_SZ (8)
#endif

void usage(int err);
void print_buffer(iconv_t cd, char **buf, size_t size, size_t *bytes_scanned, int lflag, error_t *err);
char *from_wc_str(iconv_t cd, const char **buf, size_t size, int lflag, size_t *bytes_read, size_t *out_buf_len, error_t *err);
//char *from_str(const char *buf, size_t size, size_t *bytes_read);

int main(int argc, char *argv[])
{
    int ret = EXIT_SUCCESS;
    int hflag = 0;
    int lflag = 0;
    const char *locale = NULL;



    for (;;)
    {
        int c = getopt(argc, argv, "hl:");
        if (c == -1)
        {
            break;
        }
        switch (c)
        {
	case 'h':
	    hflag = 1;
	    break;
	case 'l':
	    lflag = 1;
	    locale = optarg;
	    break;
	default:
	    usage(1);
	    return EXIT_FAILURE;
        }
    }
    if (hflag)
    {
        usage(0);
        goto exit_program;
    }

    if (lflag)
    {
        const char *ret_locale = setlocale(LC_ALL, locale);
        if (ret_locale == NULL)
        {
            perror("setlocale()");
            ret = EXIT_FAILURE;
            goto exit_program;
        }
    }

    if (optind == argc)
    {
        usage(1);
        ret = EXIT_FAILURE;
        goto exit_program;
    }

    FILE *f = fopen(argv[optind], "r");

    if (f == NULL)
    {
        perror("fopen()");
        ret = EXIT_FAILURE;
        goto exit_program;
    }

    iconv_t cd = iconv_open("UTF-8", "UTF-16LE");

    if (cd == (iconv_t) -1)
    {
        perror("iconv_open");
        ret = EXIT_FAILURE;
        goto close_file;
    }

    char buffer[BUF_SZ];
    char *p = buffer; /* this pointer is needed because if we pass address of buffer array to function print_buffer it can't change its value*/
    char *new_buffer;
    int keeping = 1;
    size_t bytes_read;
    size_t bytes_scanned = BUF_SZ;
    size_t bytes_left = 0;
    for (;;)
    {
        error_t err = 0;

        if (keeping == 0)
        {
            break;
        }

        new_buffer = buffer + bytes_left;
        memset(new_buffer, '\0', bytes_scanned);
        bytes_read = fread(new_buffer, sizeof *new_buffer, bytes_scanned, f);

        if (bytes_read == 0)
        {
            if (errno != 0)
            {
                perror("fread()");
            }
            break;
        }

        p = buffer;

        print_buffer(cd, (char **)&p, bytes_read, &bytes_scanned, lflag, &err);

        if (err == EINVAL)
        {
            bytes_left = sizeof buffer / sizeof *buffer - bytes_scanned;
            memmove(buffer, buffer + bytes_scanned, bytes_left);
        }
        else
        {
            bytes_left = 0;
            bytes_scanned = sizeof buffer / sizeof *buffer;
        }

        if (new_buffer - buffer + bytes_read < sizeof buffer / sizeof *buffer)
        {
            keeping = 0;
        }
    }

    printf("\n");

    iconv_close(cd);
close_file:
    fclose(f);
exit_program:
    return ret;
}

void usage(int err)
{
    FILE *console = (err != 0) ? stdout : stderr;

    fprintf(console, "usage: strings-utf16 [-h] [-l locale] <file>\n");
}

/*
 * print_str print all c-strings in str separated by '\0'.
 */
void print_str(const char **str, size_t size)
{
    size_t position = 0;

    while (position < size)
    {
        int len = strlen(*str);
        position += len + 1;
        if (len > 0)
        {
            if (position < size)
            {
                printf("%s\n", *str);
            }
            else
            {
                printf("%s", *str);
            }
        }

        *str += len + 1;
    }
    fflush(stdout);
}

void print_buffer(iconv_t cd, char **buf, size_t size, size_t *bytes_scanned, int lflag, error_t *err)
{
    char *shift_buf          = *buf;
    int crossing_boundary    = 0;
    size_t position          = 0;
    size_t out_buf_len       = 0;

    static int wc_str_pnt   = 0;
    //static int char_skipped = 0;

    while (position < size)
    {
        char *new_buf;
        char *old_buf;
    print_wc_str:
        new_buf = from_wc_str(cd, (const char **)&shift_buf, size - position, lflag, bytes_scanned, &out_buf_len, err);
        old_buf = new_buf;

        if (*bytes_scanned != 0 && new_buf != NULL)
        {
            crossing_boundary = (*bytes_scanned == (size - position)) ? 1 : 0;
            if (wc_str_pnt == 1)
            {
                print_str((const char **)&new_buf, out_buf_len);
            }
            else
            {
                printf("\n");
                print_str((const char **)&new_buf, out_buf_len);
            }

            free(old_buf);

            wc_str_pnt   = 1;
            //char_skipped = 0;

            position += *bytes_scanned;

            if (position >= size)
            {
                break;
            }

            shift_buf += *bytes_scanned;


            if (crossing_boundary == 1)
            {
                goto print_wc_str;
            }
            else
            {
                goto skip_char;
            }
        }
        else
        {
            goto skip_char;
        }

    skip_char:
        wc_str_pnt   = 0;
        //char_skipped = 1;

        position++;

        if (position >= size)
        {
            break;
        }

        *bytes_scanned = 1;
        shift_buf += 1;

        goto print_wc_str;
    }
}

static size_t wc_printable(const wchar_t *buf, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        const unsigned char *p = (const unsigned char *)buf;
        unsigned char c[2];
        c[0] = p[2 * i];
        c[1] = p[2 * i + 1];
        wchar_t w = (c[0]) | (c[1] << 8);
        if (!iswprint(w))
        {
            return i;
        }
    }

    return size;
}

char *from_wc_str(iconv_t cd, const char **buf, size_t size, int lflag, size_t *bytes_read, size_t *out_buf_len, error_t *err)
{
    size_t ret;
    char *inbuf = (char *)*buf;
    char *p = inbuf;
    size_t bytes_scanned;

    if (lflag)
    {
        bytes_scanned = 2 * wc_printable((const wchar_t *)inbuf, size/2);
        if (bytes_scanned == 0)
        {
            *bytes_read = 0;
            // *err ??
            return NULL;
        }
    }
    else
    {
        bytes_scanned = size;
    }

    char *u8str;
    char *q = NULL;

    size_t bytes_written;
    size_t buf_sz = 1024;

malloc_u8str:
    u8str = calloc(sizeof *u8str, (buf_sz + 1));
    q = u8str;
    bytes_written = sizeof *u8str * buf_sz;

    //iconv(cd, NULL, NULL, &q, &bytes_written);
    iconv(cd, NULL, NULL, NULL, NULL);
    ret = iconv(cd, &p, &bytes_scanned, &q, &bytes_written);
    *out_buf_len = q - u8str;

    if (ret == (size_t) -1)
    {
        switch (errno)
        {
	case EILSEQ:
	{
	    *err = EILSEQ;
	    *bytes_read = (size_t)(p - inbuf);

	    return u8str;
	}
	break;
	case EINVAL:
	{
	    *err = EINVAL;
	    *bytes_read = (size_t)(p - inbuf);

	    return u8str;
	}
	break;
	case E2BIG:
	{
	    free(u8str);
	    buf_sz *= 2;
	    goto malloc_u8str;
	}
	break;
	default:
	{
	    return u8str;
	}
        }
    }
    else
    {
        *bytes_read = (size_t)(p - inbuf);

        return u8str;
    }
}

/*
  static size_t str_printable(const char *buf, size_t size)
  {
  for (size_t i = 0; i < size; i++)
  {
  if (isprint(buf[i]) == 0)
  {
  return i;
  }
  }

  return size;
  }
*/

/*
  char *from_str(const char *buf, size_t size, size_t *bytes_read)
  {
  size_t position = str_printable(buf, size);

  if (position == 0)
  {
  *bytes_read = 0;
  return NULL;
  }

  char *str = malloc(position * sizeof *str + 1);

  if (str == NULL)
  {
  *bytes_read = 0;
  return NULL;
  }

  strncpy(str, buf, position);
  str[position] = '\0';
  *bytes_read = position;
  return str;
  }
*/
