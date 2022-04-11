#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * This is the "data store" module for Mush.
 * It maintains a mapping from variable names to values.
 * The values of variables are stored as strings.
 * However, the module provides functions for setting and retrieving
 * the value of a variable as an integer.  Setting a variable to
 * an integer value causes the value of the variable to be set to
 * a string representation of that integer.  Retrieving the value of
 * a variable as an integer is possible if the current value of the
 * variable is the string representation of an integer.
 */

typedef struct node {
    char* name;
    char* value;
    struct node *next;
}node;

int listExists = 0;
node* head;

void makeList() {
    listExists = 1;
    head = malloc(sizeof(node));
    head->next = NULL;
}

/**
 * @brief  Get the current value of a variable as a string.
 * @details  This function retrieves the current value of a variable
 * as a string.  If the variable has no value, then NULL is returned.
 * Any string returned remains "owned" by the data store module;
 * the caller should not attempt to free the string or to use it
 * after any subsequent call that would modify the value of the variable
 * whose value was retrieved.  If the caller needs to use the string for
 * an indefinite period, a copy should be made immediately.
 *
 * @param  var  The variable whose value is to be retrieved.
 * @return  A string that is the current value of the variable, if any,
 * otherwise NULL.
 */
char *store_get_string(char *var) {
    if (var == "") {return -1;}
    if (listExists == 0) {makeList();}
    node* curr = head->next;
    while (curr != NULL) {
        if (curr->name == var) {return curr->value;}
        else curr = curr->next;
    }
    return NULL;
}

/**
 * @brief  Get the current value of a variable as an integer.
 * @details  This retrieves the current value of a variable and
 * attempts to interpret it as an integer.  If this is possible,
 * then the integer value is stored at the pointer provided by
 * the caller.
 *
 * @param  var  The variable whose value is to be retrieved.
 * @param  valp  Pointer at which the returned value is to be stored.
 * @return  If the specified variable has no value or the value
 * cannot be interpreted as an integer, then -1 is returned,
 * otherwise 0 is returned.
 */
int store_get_int(char *var, long *valp) {
    if (var == "") {return -1;}
    if (listExists == 0) {makeList();}
    node *curr = head->next;
    while (curr != NULL) {
        if (curr->name == var) {
            valp = atoi(curr->value);
            if (valp == 0) {return -1;}
            else return 0;
        }
        else curr = curr->next;
    }
    return -1;
}

/**
 * @brief  Set the value of a variable as a string.
 * @details  This function sets the current value of a specified
 * variable to be a specified string.  If the variable already
 * has a value, then that value is replaced.  If the specified
 * value is NULL, then any existing value of the variable is removed
 * and the variable becomes un-set.  Ownership of the variable and
 * the value strings is not transferred to the data store module as
 * a result of this call; the data store module makes such copies of
 * these strings as it may require.
 *
 * @param  var  The variable whose value is to be set.
 * @param  val  The value to set, or NULL if the variable is to become
 * un-set.
 */
int store_set_string(char *var, char *val) {
    if (var == "") {return -1;}
    if (listExists == 0) {makeList();}
    node *curr = head->next;
    while (curr != NULL) {
        if (curr->name == var) {
            curr->value = val;
            return 0;
        }
        else curr = curr->next;
    }
    return -1;
}

/**
 * @brief  Set the value of a variable as an integer.
 * @details  This function sets the current value of a specified
 * variable to be a specified integer.  If the variable already
 * has a value, then that value is replaced.  Ownership of the variable
 * string is not transferred to the data store module as a result of
 * this call; the data store module makes such copies of this string
 * as it may require.
 *
 * @param  var  The variable whose value is to be set.
 * @param  val  The value to set.
 */
int store_set_int(char *var, long val) {
    if (var == "") {return -1;}
    if (listExists == 0) {makeList();}
    node *curr = head->next;
    while (curr != NULL) {
        if (curr->name == var) {
            char* str;
            sprintf(str, "%ld", val);
            if (str != 0) {
                curr->value = str;
                return 0;
            } else return -1;
        } else curr = curr->next;
    }
    return -1;
}

/**
 * @brief  Print the current contents of the data store.
 * @details  This function prints the current contents of the data store
 * to the specified output stream.  The format is not specified; this
 * function is intended to be used for debugging purposes.
 *
 * @param f  The stream to which the store contents are to be printed.
 */
void store_show(FILE *f) {
    if (listExists == 0) {makeList();}
    node *curr = head->next;
    while (curr != NULL) { 
        fprintf(f, curr->name);
        fprintf(f, " ");
        fprintf(f, curr->value);
        fprintf(f, "\n");
        curr = curr->next;
    }
}
