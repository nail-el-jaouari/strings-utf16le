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

#include "unicode-length-utf8.h"
#include "vec-str.h"

#ifndef BUF_SZ
#define BUF_SZ (4096)
#endif

void usage(int err);
void print_buffer(iconv_t cd, char **buf, size_t size, size_t *bytes_scanned, error_t *err, size_t min, struct VecStr *line);
char *from_wc_str(iconv_t cd, const char **buf, size_t size, size_t *bytes_read, size_t *out_buf_len, error_t *err);
//char *from_str(const char *buf, size_t size, size_t *bytes_read);

int main(int argc, char *argv[])
{
    int ret = EXIT_SUCCESS;
    int hflag = 0;
    int lflag = 0;
    int nflag = 0;
    const char *locale = NULL;
    const char *min_arg = NULL;
    size_t min = 0;
    struct VecStr line;

    for (;;)
    {
        int c = getopt(argc, argv, "hl:n:");
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
            case 'n':
                nflag = 1;
                min_arg = optarg;
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

    if (nflag)
    {
        char *endptr = NULL;

        min = strtoul(min_arg, &endptr, 10);

        if (endptr != NULL && *endptr != '\0')
        {
            usage(1);
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

    vec_str_init(&line, MIN_CAP);

    FILE *f = fopen(argv[optind], "rb");

    if (f == NULL)
    {
        perror("fopen()");
        ret = EXIT_FAILURE;
        goto free_vec_str;
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
        memset(new_buffer, '\0', BUF_SZ - bytes_left);
        bytes_read = fread(new_buffer, sizeof *new_buffer, BUF_SZ - bytes_left, f);

        if (bytes_read == 0)
        {
            if (errno != 0)
            {
                perror("fread()");
            }
            break;
        }

        p = buffer;

        print_buffer(cd, &p, bytes_read, &bytes_scanned, &err, min, &line);

        if (err == EINVAL)
        {
            bytes_left = bytes_read - bytes_scanned;
            memmove(buffer, buffer + bytes_scanned, bytes_left);
        }
        else
        {
            bytes_left = 0;
            bytes_scanned = bytes_read;
        }

        if (new_buffer - buffer + bytes_read < sizeof buffer / sizeof *buffer)
        {
            keeping = 0;
        }
    }

    if (line.total_str_length >= min)
    {
        vec_str_print(&line);
    }

    iconv_close(cd);
close_file:
    fclose(f);
free_vec_str:
    vec_str_free(&line);
exit_program:
    return ret;
}

void usage(int err)
{
    FILE *console = (err != 0) ? stdout : stderr;

    fprintf(console, "usage: strings-utf16 [-h] [-l locale] [-n number] <file>\n");
}

/*
 * print_str print all c-strings in str separated by '\0'.
 */
void print_str(const char **str, size_t size, size_t min, struct VecStr *line, int wc_str)
{
    size_t position = 0;

    if (wc_str == 0)
    {
        if (line->total_str_length >= min)
        {
            vec_str_print(line);
        }

        vec_str_free(line);
        vec_str_init(line, MIN_CAP);

        char *elem = strdup(*str);
        vec_str_push(line, elem);
    }
    else
    {
        while (position < size)
        {
            size_t len = strlen(*str);
            position += len + 1;
            if (len > 0)
            {
                if (position < size)
                {
                    if (line->total_str_length >= min)
                    {
                        printf("\n");
                        vec_str_print(line);
                    }

                    vec_str_free(line);
                    vec_str_init(line, MIN_CAP);
                }
                else
                {
                    char *elem = strdup(*str);
                    vec_str_push(line, elem);
                }
            }

            *str += len + 1;
        }
    }

    fflush(stdout);
}

void print_buffer(iconv_t cd, char **buf, size_t size, size_t *bytes_scanned, error_t *err, size_t min, struct VecStr *line)
{
    char *shift_buf          = *buf;
    int crossing_boundary    = 0;
    size_t position          = 0;
    size_t out_buf_len       = 0;

    static int wc_str_pnt   = 1;

    while (position < size)
    {
        char *new_buf;
        char *old_buf;
    print_wc_str:
        new_buf = from_wc_str(cd, (const char **)&shift_buf, size - position, bytes_scanned, &out_buf_len, err);
        old_buf = new_buf;

        if (*bytes_scanned != 0 && new_buf != NULL)
        {
            crossing_boundary = (*bytes_scanned == (size - position)) ? 1 : 0;
            print_str((const char **)&new_buf, out_buf_len, min, line, wc_str_pnt);
            free(old_buf);

            wc_str_pnt   = 1;

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
            free(old_buf);
            goto skip_char;
        }

    skip_char:
        wc_str_pnt   = 0;
        position += 2;

        if (position >= size)
        {
            break;
        }

        *bytes_scanned = 2;
        shift_buf += 2;

        goto print_wc_str;
    }
}

static size_t wc_printable(const wchar_t *buf, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        const unsigned char *p = (const unsigned char *)buf;
        wchar_t w = {0};
        unsigned char *c = (unsigned char *)&w;
        c[0] = p[2 * i];
        c[1] = p[2 * i + 1];
        
        if (!iswprint(w))
        {
            return i;
        }
    }

    return size;
}

char *from_wc_str(iconv_t cd, const char **buf, size_t size, size_t *bytes_read, size_t *out_buf_len, error_t *err)
{
    size_t ret;
    char *inbuf = (char *)*buf;
    char *p = inbuf;
    size_t bytes_scanned;

    bytes_scanned = 2 * wc_printable((const wchar_t *)inbuf, size/2);

    if (bytes_scanned == 0)
    {
        *bytes_read = 0;
        *err = EILSEQ;
        return NULL;
    }

    char *u8str;
    char *q = NULL;

    size_t bytes_written;
    size_t buf_sz = 1024;

malloc_u8str:
    u8str = calloc((buf_sz + 1), sizeof *u8str);
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
            case EINVAL:
            {
                *err = errno;
                *bytes_read = (size_t)(p - inbuf);

                return u8str;
            }
            break;
            case E2BIG:
            {
                *err = errno;
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
        *err = 0;
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
