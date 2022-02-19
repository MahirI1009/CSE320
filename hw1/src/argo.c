#include <stdlib.h>
#include <stdio.h>

#include "argo.h"
#include "global.h"
#include "debug.h"
int argo_write_object();
int argo_write_array();
int argo_write_basic();


/**
 * @brief  Read JSON input from a specified input stream, parse it,
 * and return a data structure representing the corresponding value.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON value,
 * according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.  See the assignment handout for
 * information on the JSON syntax standard and how parsing can be
 * accomplished.  As discussed in the assignment handout, the returned
 * pointer must be to one of the elements of the argo_value_storage
 * array that is defined in the const.h header file.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  A valid pointer if the operation is completely successful,
 * NULL if there is any error.
 */
// ARGO_VALUE *argo_read_value(FILE *f) { abort(); }

/**
 * @brief  Read JSON input from a specified input stream, attempt to
 * parse it as a JSON string literal, and return a data structure
 * representing the corresponding string.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON string
 * literal, according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_read_string(ARGO_STRING *s, FILE *f) {
    // TO BE IMPLEMENTED.
    abort();
}

/**
 * @brief  Read JSON input from a specified input stream, attempt to
 * parse it as a JSON number, and return a data structure representing
 * the corresponding number.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON numeric
 * literal, according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.  The returned value must contain
 * (1) a string consisting of the actual sequence of characters read from
 * the input stream; (2) a floating point representation of the corresponding
 * value; and (3) an integer representation of the corresponding value,
 * in case the input literal did not contain any fraction or exponent parts.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_read_number(ARGO_NUMBER *n, FILE *f) {
    // TO BE IMPLEMENTED.
    abort();
}

/**
 * @brief  Write canonical JSON representing a specified value to
 * a specified output stream.
 * @details  Write canonical JSON representing a specified value
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.
 *
 * @param v  Data structure representing a value.
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */



int argo_write_value(ARGO_VALUE *v, FILE *f) { 
    int x = v -> type;   
    switch(x) {
        case ARGO_OBJECT_TYPE:
            argo_write_object(&v -> content.object, f);
            break;
        case ARGO_ARRAY_TYPE:
            argo_write_array(&v -> content.array, f);
            break;
        case ARGO_STRING_TYPE:
            argo_write_string(&v -> content.string, f);
            break;
        case ARGO_NUMBER_TYPE:
            argo_write_number(&v -> content.number, f);
            break;
        case ARGO_BASIC_TYPE:
            argo_write_basic(&v -> content.basic, f);
            break;
    } 
    return 0;
}

int argo_write_object(ARGO_OBJECT *o, FILE *f) {
    ARGO_VALUE* head = o -> member_list;
    ARGO_VALUE* curr = head -> next;

    int p_print = 0;
    int no_indent = 0, indent = 0;
    if (global_options == CANONICALIZE_OPTION) {no_indent = 1;}
    if (global_options > (CANONICALIZE_OPTION + PRETTY_PRINT_OPTION)) { 
        p_print = 1; 
        indent = global_options - (CANONICALIZE_OPTION + PRETTY_PRINT_OPTION);
    }
    if (global_options == (CANONICALIZE_OPTION + PRETTY_PRINT_OPTION)) { 
        no_indent = 0; 
        p_print = 1;
    }

    fputc(ARGO_LBRACE, f);
    if (p_print == 1) {
        fputs("\n", f);
        indent_level++;
    }
    int i;
    if (no_indent == 0 && indent != 0) { for (i = 0; i <= (indent * indent_level); i++) {fputc(ARGO_SPACE, f); }}
    while (curr != head) {
        argo_write_string(&(curr->name), f);
        fputs(": ", f);
        argo_write_value(curr, f);
        curr = curr -> next;
        if (curr != head) { fputc(ARGO_COMMA, f); }
        else indent_level--;
        if (p_print == 1) { fputs("\n", f); }
        if (no_indent == 0  && indent != 0) { for (i = 0; i <= (indent * indent_level); i++) {fputc(ARGO_SPACE, f); }}
    }    
    fputc(ARGO_RBRACE, f);
    if (p_print == 1) {
        fputs("\n", f);
    }
    if (no_indent == 0  && indent != 0) { for (i = 0; i <= (indent * indent_level); i++) {fputc(ARGO_SPACE, f); }}

    return 0;
}


int argo_write_array(ARGO_ARRAY *a, FILE *f) {
    ARGO_VALUE* head = a -> element_list;
    ARGO_VALUE* curr = head -> next;

    int p_print = 0;
    int no_indent = 0, indent = 0;
    if (global_options == CANONICALIZE_OPTION) {no_indent = 1;}
    if (global_options > (CANONICALIZE_OPTION + PRETTY_PRINT_OPTION)) { 
        p_print = 1; 
        indent = global_options - (CANONICALIZE_OPTION + PRETTY_PRINT_OPTION);
    }
    if (global_options == (CANONICALIZE_OPTION + PRETTY_PRINT_OPTION)) { 
        no_indent = 0; 
        p_print = 1;
    }

    fputc(ARGO_LBRACK, f);
    if (p_print == 1) {
        fputs("\n", f);
        indent_level++;
    }
    int i;
    if (no_indent == 0  && indent != 0) { for (i = 0; i <= (indent * indent_level); i++) {fputc(ARGO_SPACE, f); }}
    while (curr != head) {
        argo_write_value(curr, f);
        curr = curr -> next;
        if (curr != head) { fputc(ARGO_COMMA, f); }
        else indent_level--;
        if (p_print == 1) { fputs("\n", f); }
        if (no_indent == 0  && indent != 0) { for (i = 0; i <= (indent * indent_level); i++) {fputc(ARGO_SPACE, f); }}
    }    
    fputc(ARGO_RBRACK, f);
    if (p_print == 1) {
        fputs("\n", f);
    }
    if (no_indent == 0 && indent != 0) { for (i = 0; i <= (indent * indent_level); i++) {fputc(ARGO_SPACE, f); }}

    return 0;
}


/**
 * @brief  Write canonical JSON representing a specified string
 * to a specified output stream.
 * @details  Write canonical JSON representing a specified string
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.  The argument string may contain any sequence of
 * Unicode code points and the output is a JSON string literal,
 * represented using only 8-bit bytes.  Therefore, any Unicode code
 * with a value greater than or equal to U+00FF cannot appear directly
 * in the output and must be represented by an escape sequence.
 * There are other requirements on the use of escape sequences;
 * see the assignment handout for details.
 *
 * @param v  Data structure representing a string (a sequence of
 * Unicode code points).
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_write_string(ARGO_STRING *s, FILE *f) {
    int len = s -> length;
    ARGO_CHAR* str = s -> content;

    int i;
    fputc(ARGO_QUOTE, f);
    for (i = 0; i < len; i++) {
        char c = *(str + i);
        if (c == 0x005c) { fputc(ARGO_BSLASH, f); }
        if (c == 0x0022) { fputc(ARGO_BSLASH, f); }
        if (c == 0x002f) { fputc(ARGO_FSLASH, f); }
        if  (!argo_is_control(c)) { fputc(c, f); }
        else if (c == 0x0008) { fputs("\\b", f); }
        else if (c == 0x000c) { fputs("\\f", f); }
        else if (c == 0x000a) { fputs("\\n", f); }
        else if (c == 0x0009) { fputs("\\r", f); }
        else if (c == 0x000d) { fputs("\\t", f); }
        else { 
            fputc(ARGO_BSLASH, f);
            fputc(ARGO_U, f);
            if (c < 0x001f && c > 0x0001) { fprintf(f, "000"); }
            if (c < 0x01ff && c > 0x001f) { fprintf(f, "00"); }
            if (c < 0x1fff && c > 0x01ff) { fprintf(f, "0"); }
            fprintf(f, "%x", c); 
        }
    }
    fputc(ARGO_QUOTE, f);
    return 0;
}

/**
 * @brief  Write canonical JSON representing a specified number
 * to a specified output stream.
 * @details  Write canonical JSON representing a specified number
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.  The argument number may contain representations
 * of the number as any or all of: string conforming to the
 * specification for a JSON number (but not necessarily canonical),
 * integer value, or floating point value.  This function should
 * be able to work properly regardless of which subset of these
 * representations is present.
 *
 * @param v  Data structure representing a number.
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_write_number(ARGO_NUMBER *n, FILE *f) {
    char v_s = n -> valid_string; 
    char v_i = n -> valid_int;
    char v_f = n -> valid_float;
    long num = n -> int_value;
    double fl = n -> float_value;
    if (v_s != 0) {
        //fflush(f);
        if (v_i != 0) { fprintf(f, "%ld", num); }
        else if (v_f != 0 && fl == 0.0) { fputs("0.0", f);}
        else if (v_f != 0 && fl == 1.0) { fputs("0.1e1", f);}
        else if (v_f != 0 && fl == -1.0) { fputs("-0.1e1", f);}
        else if (v_f != 0) { 
            if(fl < 0.0) {
                fl *= -1;
                fputc('-', f);
            }
            int exp = 0;
            if (fl > 1.0) {
                while (fl > 1.0) {
                    fl /= 10;
                    exp++;    
                }
            }
            if (fl < 0.1) {
                while (fl < 0.1) {
                    fl *= 10;
                    exp--;
                }
            }
            fputs("0.", f);
            int i;
            for (i = 0; i < 15; i++) { 
                fl *= 10; 
                long f_fl =  (long) fl;
                f_fl %= 10;
                f_fl += 48;
                fputc(f_fl, f);
            }
            fputc(ARGO_E, f);
            fprintf(f, "%d", exp);
        }
        else return -1;
    }
    else return -1;

    return 0;
}

int argo_write_basic(ARGO_BASIC *b, FILE *f) {
    int basic = *b;
    switch(basic) {
        case 0:
            fputs(ARGO_NULL_TOKEN, f);
            break;
        case 1: 
            fputs(ARGO_TRUE_TOKEN, f);
            break;
        case 2: 
            fputs(ARGO_FALSE_TOKEN, f);
            break;
    }
    return 0;
}
