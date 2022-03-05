/*********************/
/* errmsg.c          */
/* for Par 3.20      */
/* Copyright 1993 by */
/* Adam M. Costello  */
/*********************/

/* This is ANSI C code. */


#include "errmsg.h"  /* Makes sure we're consistent with the declarations. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* err_msg = NULL;

void clear_error() { free(err_msg); }

void set_error(char * msg) { 
    clear_error();
    err_msg = strdup(msg); 
}

int is_error() {
    if (err_msg != NULL) return 1;
    else return 0;
}

int report_error(FILE *file) {
    if(is_error() == 1) {
        fprintf(file, "%s", err_msg);
        return 0;
    }
    else if (is_error() == 0) return 0;
    else return 1;
}

const char * const outofmem = "Out of memory.\n";
