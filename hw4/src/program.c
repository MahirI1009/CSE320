#include <stdlib.h>
#include <stdio.h>

#include "mush.h"
#include "debug.h"

typedef struct node {
    STMT stmt;
    struct node *next;
}node;

node *head;

int pCtr = -1;
int listExists = 0;
STMT stmt;

void makelist() {
    listExists = 1;
    head = malloc(sizeof(node));
    head->next = NULL;
}

/*
 * This is the "program store" module for Mush.
 * It maintains a set of numbered statements, along with a "program counter"
 * that indicates the current point of execution, which is either before all
 * statements, after all statements, or in between two statements.
 * There should be no fixed limit on the number of statements that the program
 * store can hold.
 */

/**
 * @brief  Output a listing of the current contents of the program store.
 * @details  This function outputs a listing of the current contents of the
 * program store.  Statements are listed in increasing order of their line
 * number.  The current position of the program counter is indicated by
 * a line containing only the string "-->" at the current program counter
 * position.
 *
 * @param out  The stream to which to output the listing.
 * @return  0 if successful, -1 if any error occurred.
 */
int prog_list(FILE *out) {
    if (listExists == 0) {makelist();}
    if (head == NULL) {return -1;}

    node *curr = head->next;
    while (curr != NULL) { 
        show_stmt(out, &(curr->stmt));
        curr = curr->next;
    }
    return 0;
}

/**
 * @brief  Insert a new statement into the program store.
 * @details  This function inserts a new statement into the program store.
 * The statement must have a line number.  If the line number is the same as
 * that of an existing statement, that statement is replaced.
 * The program store assumes the responsibility for ultimately freeing any
 * statement that is inserted using this function.
 * Insertion of new statements preserves the value of the program counter:
 * if the position of the program counter was just before a particular statement
 * before insertion of a new statement, it will still be before that statement
 * after insertion, and if the position of the program counter was after all
 * statements before insertion of a new statement, then it will still be after
 * all statements after insertion.
 *
 * @param stmt  The statement to be inserted.
 * @return  0 if successful, -1 if any error occurred.
 */
int prog_insert(STMT *stmt) {
    if (listExists == 0) {makelist();}         //initialize list for first time
    
    //insert to front of list
    if (head->next == NULL) {
        node *newStmt = malloc(sizeof(node));
        newStmt->stmt = *stmt;
        newStmt->next = NULL;
        head->next = newStmt;
        return 0;
    }
    node *curr = head->next, *prev = head;  
    //go through linkedlist to find a place to insert node
    while (curr != NULL) {
        //insert a new node
        if (stmt->lineno < curr->stmt.lineno) {
            node *newStmt = malloc(sizeof(node));
            newStmt->stmt = *stmt;
            newStmt->next = curr;
            prev->next = newStmt;
            return 0;
        }
        //replace a node with the same line #
        if (stmt->lineno == curr->stmt.lineno) {
            free_stmt(&(curr->stmt));
            curr->stmt = *stmt;
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    //insert at end of list
    if (curr == NULL) {
        node *newStmt = malloc(sizeof(node));
        newStmt->stmt = *stmt;
        newStmt->next = NULL;
        prev->next = newStmt;
    }
    return 0;
}

/**
 * @brief  Delete statements from the program store.
 * @details  This function deletes from the program store statements whose
 * line numbers fall in a specified range.  Any deleted statements are freed.
 * Deletion of statements preserves the value of the program counter:
 * if before deletion the program counter pointed to a position just before
 * a statement that was not among those to be deleted, then after deletion the
 * program counter will still point the position just before that same statement.
 * If before deletion the program counter pointed to a position just before
 * a statement that was among those to be deleted, then after deletion the
 * program counter will point to the first statement beyond those deleted,
 * if such a statement exists, otherwise the program counter will point to
 * the end of the program.
 *
 * @param min  Lower end of the range of line numbers to be deleted.
 * @param max  Upper end of the range of line numbers to be deleted.
 */
int prog_delete(int min, int max) {
    if (listExists == 0) {makelist();}
    if (head == NULL) {return -1;}
    
    node *curr = head->next;
    node* next = curr->next;
    if (curr == NULL || next == NULL) {return -1;}

    while (curr != NULL && next != NULL) {
        if (next->stmt.lineno >= min && next->stmt.lineno <= max) {
            node* del = next;
            next = del->next;
            curr->next = next;
            free(del);
        } else if (curr->stmt.lineno > max) {return 0;}
        else {
            curr = curr->next;
            next = curr->next;
        }
    }
    return 0;
}

/**
 * @brief  Reset the program counter to the beginning of the program.
 * @details  This function resets the program counter to point just
 * before the first statement in the program.
 */
void prog_reset(void) { 
    node* curr = head->next;
    if (curr != NULL) { pCtr = (curr->stmt.lineno) - 1; }
}

/**
 * @brief  Fetch the next program statement.
 * @details  This function fetches and returns the first program
 * statement after the current program counter position.  The program
 * counter position is not modified.  The returned pointer should not
 * be used after any subsequent call to prog_delete that deletes the
 * statement from the program store.
 *
 * @return  The first program statement after the current program
 * counter position, if any, otherwise NULL.
 */
STMT *prog_fetch(void) {
    if (listExists == 0) {makelist();}
    if (head == NULL) {return NULL;}

    node *curr = head->next;
    while (curr != NULL) { 
        if (curr->stmt.lineno == pCtr) { 
            stmt = curr->next->stmt;
            return &stmt;
        } 
        curr = curr->next;
    }
    return NULL;
}

/**
 * @brief  Advance the program counter to the next existing statement.
 * @details  This function advances the program counter by one statement
 * from its original position and returns the statement just after the
 * new position.  The returned pointer should not be used after any
 * subsequent call to prog_delete that deletes the statement from the
 * program store.
 *
 * @return The first program statement after the new program counter
 * position, if any, otherwise NULL.
 */
STMT *prog_next() {
    if (listExists == 0) {makelist();}
    if (head == NULL) {return NULL;}

    node *curr = head->next;
    while (curr != NULL) { 
        if (curr->stmt.lineno == pCtr) { 
            pCtr = curr->next->stmt.lineno;
            stmt = curr->next->stmt;
            return &stmt;
        }
        curr = curr->next;
    }
    return NULL;
}

/**
 * @brief  Perform a "go to" operation on the program store.
 * @details  This function performs a "go to" operation on the program
 * store, by resetting the program counter to point to the position just
 * before the statement with the specified line number.
 * The statement pointed at by the new program counter is returned.
 * If there is no statement with the specified line number, then no
 * change is made to the program counter and NULL is returned.
 * Any returned statement should only be regarded as valid as long
 * as no calls to prog_delete are made that delete that statement from
 * the program store.
 *
 * @return  The statement having the specified line number, if such a
 * statement exists, otherwise NULL.
 */
STMT *prog_goto(int lineno) {
    if (listExists == 0) {makelist();}
    if (head == NULL) {return NULL;}

    node *curr = head->next;
    while (curr != NULL && curr->next != NULL) { 
        if (lineno == curr->next->stmt.lineno) {
            lineno = curr->stmt.lineno;
            stmt = curr->stmt;
            return &stmt;
        }
        curr = curr->next;
    }
    return NULL;
}
