/*********************/
/* par.c             */
/* for Par 3.20      */
/* Copyright 1993 by */
/* Adam M. Costello  */
/*********************/

/* This is ANSI C code. */


#include "errmsg.h"
#include "buffer.h"    /* Also includes <stddef.h>. */
#include "reformat.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>

#undef NULL
#define NULL ((void *) 0)


const char * const progname = "par";
const char * const version = "3.20";


static int digtoint(char c)

/* Returns the value represented by the digit c,   */
/* or -1 if c is not a digit. Does not use errmsg. */
{
  return c == '0' ? 0 :
         c == '1' ? 1 :
         c == '2' ? 2 :
         c == '3' ? 3 :
         c == '4' ? 4 :
         c == '5' ? 5 :
         c == '6' ? 6 :
         c == '7' ? 7 :
         c == '8' ? 8 :
         c == '9' ? 9 :
         -1;

  /* We can't simply return c - '0' because this is ANSI  */
  /* C code, so it has to work for any character set, not */
  /* just ones which put the digits together in order.    */
}


static int strtoudec(const char *s, int *pn)

/* Puts the decimal value of the string s into *pn, returning */
/* 1 on success. If s is empty, or contains non-digits,       */
/* or represents an integer greater than 9999, then *pn       */
/* is not changed and 0 is returned. Does not use errmsg.     */
{
  int n = 0;

  if (!*s) return 0;

  do {
    if (n >= 1000 || !isdigit(*s)) return 0;
    n = 10 * n + digtoint(*s);
  } while (*++s);

  *pn = n;

  return 1;
}

static void parse_opt_2(int *pwidth, int *pprefix, int *psuffix, int *phang, int *plast, int *pmin, int argc, char * const *argv) {

  const char* options = "w:p:s:h::l::m::";

  static struct option long_options[] = {
    {"version", no_argument, 0, 'v'},
    {"width", required_argument, 0, 'w'},
    {"prefix", required_argument, 0, 'p'},
    {"suffix", required_argument, 0, 's'},
    {"hang", optional_argument, 0, 'h'},
    {"last", no_argument, 0, 'L'},
    {"no-last", no_argument, 0, 'k'},
    {"min", no_argument, 0, 'M'},
    {"no-min", no_argument, 0, 'n'},
    {0,0,0,0}
  };

  int dummy = 0;
  int option = getopt_long(argc, argv, options, long_options, NULL);
  while (option != -1) {
      switch(option) {
        case 'v':
          sprintf("%s %s\n", progname, version);
          exit(EXIT_SUCCESS);

        case 'w':
          if(strtoudec(optarg, &dummy) == 0) {set_error("Bad option: %.149s\n");}
          else *pwidth = dummy;
          break;

        case 'p':
          if(strtoudec(optarg, &dummy) == 0) {set_error("Bad option: %.149s\n");}
          else *pprefix = dummy;
          break;
        
        case 's':
          if(strtoudec(optarg, &dummy) == 0) {set_error("Bad option: %.149s\n");}
          else *psuffix = dummy;
          break;

        case 'h':
          if(strtoudec(optarg, &dummy) == 0) {set_error("Bad option: %.149s\n");}
          else *phang = dummy;
          break;

        case 'l':
          if(strtoudec(optarg, &dummy) == 0) {set_error("Bad option: %.149s\n");}
          else *plast = dummy;
          break;


        case 'L':
          *plast = 1;
          break;

        case 'k':
          *plast = 0;
          break;

        case 'm':
          if(strtoudec(optarg, &dummy) == 0) {set_error("Bad option: %.149s\n");}
          else *pmin = dummy;
          break;

        case 'M':
          *pmin = 1;
          break;

        case 'n':
          *pmin = 0;
          break;
      }
  }
}

static char **readlines(void) {

/* Reads lines from stdin until EOF, or until a blank line is encountered, */
/* in which case the newline is pushed back onto the input stream. Returns */
/* a NULL-terminated array of pointers to individual lines, stripped of    */
/* their newline characters. Uses errmsg, and returns NULL on failure.     */

  struct buffer *cbuf = NULL, *pbuf = NULL;
  int c, blank;
  char ch, *ln, *nullline = NULL, nullchar = '\0', **lines = NULL;

  cbuf = newbuffer(sizeof (char));
  if (is_error()) goto rlcleanup;
  pbuf = newbuffer(sizeof (char *));
  if (is_error()) goto rlcleanup;

  for (blank = 1;  ; ) {
    c = getchar();
    if (c == EOF) break;
    if (c == '\n') {
      if (blank) {
        ungetc(c,stdin);
        break;
      }
      additem(cbuf, &nullchar);
      if(is_error()) goto rlcleanup;
      ln = copyitems(cbuf);
      if(is_error()) goto rlcleanup;
      additem(pbuf, &ln);
      if(is_error()) goto rlcleanup;
      clearbuffer(cbuf);
      blank = 1;
    }
    else {
      if (!isspace(c)) blank = 0;
      ch = c;
      additem(cbuf, &ch);
      if (is_error()) goto rlcleanup;
    }
  }

  if (!blank) {
    additem(cbuf, &nullchar);
    if(is_error()) goto rlcleanup;
    ln = copyitems(cbuf);
    if(is_error()) goto rlcleanup;
    additem(pbuf, &ln);
    if(is_error()) goto rlcleanup;
  }

  additem(pbuf, &nullline);
  if(is_error()) goto rlcleanup;
  lines = copyitems(pbuf);

rlcleanup:

  if (cbuf) freebuffer(cbuf);
  if (pbuf) {
    if (!lines) {
      for (;;) {
        lines = nextitem(pbuf);
        if (!lines) break;
        free(*lines);
      }
    }
      freebuffer(pbuf);
  }

  return lines;
}


static void setdefaults(const char * const *inlines, int *pwidth, int *pprefix, int *psuffix, int *phang, int *plast, int *pmin) {
 /* If any of *pwidth, *pprefix, *psuffix, *phang, *plast, *pmin are     */
/* less than 0, sets them to default values based on inlines, according */
/* to "par.doc". Does not use errmsg because it always succeeds.        */
  int numlines;
  const char *start, *end, * const *line, *p1, *p2;

  if (*pwidth < 0) *pwidth = 72;
  if (*phang < 0) *phang = 0;
  if (*plast < 0) *plast = 0;
  if (*pmin < 0) *pmin = *plast;

  for (line = inlines;  *line;  ++line);
  numlines = line - inlines;

  if (*pprefix < 0) {
    if (numlines <= *phang + 1) {*pprefix = 0;}
    else {
      start = inlines[*phang];
      for (end = start;  *end;  ++end);
      for (line = inlines + *phang + 1;  *line;  ++line) {
        for (p1 = start, p2 = *line;  p1 < end && *p1 == *p2;  ++p1, ++p2);
        end = p1;
      }
      *pprefix = end - start;
    }
  }

  if (*psuffix < 0) {
    if (numlines <= 1) {*psuffix = 0;}
    else {
      start = *inlines;
      for (end = start;  *end;  ++end);
      for (line = inlines + 1;  *line;  ++line) {
        for (p2 = *line;  *p2;  ++p2);
        for (p1 = end;
             p1 > start && p2 > *line && p1[-1] == p2[-1];
             --p1, --p2);
        start = p1;
      }
      while (end - start >= 2 && isspace(*start) && isspace(start[1])) ++start;
      *psuffix = end - start;
    }
  }
}


static void freelines(char **lines)
/* Frees the strings pointed to in the NULL-terminated array lines, then */
/* frees the array. Does not use errmsg because it always succeeds.      */
{
  char **line;

  for (line = lines;  *line;  ++line)
    free(*line);

  free(lines);
}


int original_main(int argc, char * const *argv) {
  int width, widthbak = -1, prefix, prefixbak = -1, suffix, suffixbak = -1, hang, hangbak = -1, last, lastbak = -1, min, minbak = -1, c;
  char *parinit, *picopy = NULL, *opt, **inlines = NULL, **outlines = NULL, **line;
  const char * const whitechars = " \f\n\r\t\v";

  parinit = getenv("PARINIT");
  if (parinit) {
    picopy = malloc((strlen(parinit) + 1) * sizeof (char));
    if (!picopy) {
      strcpy(err_msg,outofmem);
      goto parcleanup;
    }
    strcpy(picopy,parinit);
    opt = strtok(picopy,whitechars);
    while (opt) {
      parse_opt_2(&widthbak, &prefixbak, &suffixbak, &hangbak, &lastbak, &minbak, argc, argv);
      if(is_error()) goto parcleanup;
      opt = strtok(NULL,whitechars);
    }
    free(picopy);
    picopy = NULL;
  }

  while (*++argv) {
    parse_opt_2(&widthbak, &prefixbak, &suffixbak, &hangbak, &lastbak, &minbak, argc, argv);
    if(is_error()) goto parcleanup;
  }

  for (;;) {
    for (;;) {
      c = getchar();
      if (c != '\n') break;
      putchar(c);
    }
    if (c == EOF) break;
    ungetc(c,stdin);

    inlines = readlines();
    if(is_error()) goto parcleanup;
    if (!*inlines) {
      free(inlines);
      inlines = NULL;
      continue;
    }

    width = widthbak;  prefix = prefixbak;  suffix = suffixbak;
    hang = hangbak;  last = lastbak;  min = minbak;
    setdefaults((const char * const *) inlines, &width, &prefix, &suffix, &hang, &last, &min);

    outlines = reformat((const char * const *) inlines, width, prefix, suffix, hang, last, min);
    if(is_error()) goto parcleanup;

    freelines(inlines);
    inlines = NULL;

    for (line = outlines;  *line;  ++line)
      puts(*line);

    freelines(outlines);
    outlines = NULL;
  }

parcleanup:

  if (picopy) free(picopy);
  if (inlines) freelines(inlines);
  if (outlines) freelines(outlines);

  if(is_error()) {
    fprintf(stderr, "%.163s", err_msg);
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
