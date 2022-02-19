#include <stdlib.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */

int validargs(int argc, char **argv) {
    /*
    The input "bin/argo -h" is an array of strings of size 2. "bin/argo" and "-h" are the 2 elements of thr array.
    argc is the number of arguments passed, which is basically the size of the input array. 
    argv is the array itself, given the nature of variables in C, argv points to the first variable of the array. argv = argv[0].
    */

    if (argc < 2 || argc > 4) {
        global_options=0x0;
        return -1;
    }

    if (argc == 2) {                                // If argc = 2, then the possible valid input strings are "bin/argo -h" and "bin/argo -v"
        if (*(*(argv+1)) == '-') {                  //Go to argv[1][0] which should '-'     
            if (*(*(argv+1)+1) == 'h') {            //Go to argv[1] which can be 'h' or 'v', check for 'h' or 'v'
                if (*(*(argv+1)+2) == '\0') {
                    global_options=HELP_OPTION;
                    return 0;
                } else {
                    global_options=0x0;
                    return -1;
                }
            } else if (*(*(argv+1)+1) == 'v') {
                if (*(*(argv+1)+2) == '\0') {
                    global_options=VALIDATE_OPTION;
                    return 0;
                } else {
                    global_options=0x0;
                    return -1;
                }
            } else if (*(*(argv+1)+1) == 'c') {
                if (*(*(argv+1)+2) == '\0') {
                    global_options=CANONICALIZE_OPTION;
                    return 0;
                } else {
                    global_options=0x0;
                    return -1;
                }
            } else {
                global_options=0x0;
                return -1;
            }
        }
    }

    if (argc == 3) {                                
        if (*(*(argv+1)) == '-') {                  
            if (*(*(argv+1)+1) == 'c') {            
                if (*(*(argv+1)+2) == '\0') {
                    if (*(*(argv+2)) == '-') {                    
                        if (*(*(argv+2)+1) == 'p') {
                            if (*(*(argv+2)+2) == '\0') {
                                global_options=CANONICALIZE_OPTION + PRETTY_PRINT_OPTION + 4;
                                return 0;
                            }
                        }
                    }
                } else {
                    global_options=0x0;
                    return -1;
                }
            } else {
                global_options=0x0;
                return -1;
            }
        }
    }

    if (argc == 4) {                                
        if (*(*(argv+1)) == '-') {                  
            if (*(*(argv+1)+1) == 'c') {            
                if (*(*(argv+1)+2) == '\0') {
                    if (*(*(argv+2)) == '-') {                  
                        if (*(*(argv+2)+1) == 'p') {
                            if (*(*(argv+2)+2) == '\0') {
                                if (*(*(argv+3)) > 0) { 
                                    int indent = *(*(argv+3)) -'0';
                                    for (int i = 1; *(*(argv+3)+i) != '\0'; i++) {
                                        indent *= 10;
                                        indent += (*(*(argv+3)+i) -'0');
                                    }
                                    global_options=CANONICALIZE_OPTION + PRETTY_PRINT_OPTION + indent;
                                    return 0; 
                                } else {
                                    global_options=0x0;
                                    return -1;
                                }
                            }
                        }
                    }
                } else {
                    global_options=0x0;
                    return -1;
                }
            } else {
                global_options=0x0;
                return -1;
            }
        }
    }

    return 0;
}
