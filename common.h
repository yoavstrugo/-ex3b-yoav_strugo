#include <stdio.h>
#include <libgen.h>

#ifndef EX3B_YOAV_STRUGO_COMMON_H
#define EX3B_YOAV_STRUGO_COMMON_H

#define ERROR_OPEN_FILE_FMT "Error: Failed to open file %s.\n"

#define PROGRAM_NAME_ARG_INDEX  0

#define DECIMAL_BASE            10

/**
 * @brief Prints the usage message for the program
 * @param program_name The program's name. should be in argv[0]
 */
void usage (char *program_name, const char *format);

#endif //EX3B_YOAV_STRUGO_COMMON_H
