
/*
 *
 */

#include "command_interface.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define COMMAND_LENGTH 8
#define ARG_LENGTH 256

#define PRINT_ERROR(condition, message)   \
  do {                                    \
    if(condition) {                       \
      fprintf(stdout, message);           \
      exit(-1);                           \
    }                                     \
  } while(0);

CMND *cmnd_interf_init() {
  CMND *cmnd = (CMND *)malloc(sizeof(CMND));
  PRINT_ERROR(cmnd == NULL, "Out of memory, command_interface wasn't created");
  cmnd->command = (char *)malloc(sizeof(char) * COMMAND_LENGTH);
  cmnd->arg = (char *)malloc(sizeof(char) * ARG_LENGTH);
  PRINT_ERROR(cmnd->command == NULL || cmnd->arg == NULL, "out of memory, command_interface wasn't created");

  return cmnd;
}

void cmnd_interf_deinit(CMND *cmnd) {
  free(cmnd->command);
  free(cmnd->arg);
  free(cmnd);
}

int enter_command(CMND *cmnd) {
  fscanf(stdin, "%s", cmnd->command);
  if(strcmp(cmnd->command, "exit") == 0 || strcmp(cmnd->command, "quit") == 0)
    return -1;
  if(strcmp(cmnd->command, "ls") != 0 && strcmp(cmnd->command, "cd") != 0 && strcmp(cmnd->command, "cat")!=0) {
    fprintf(stdout, "%s: unknown command\n", cmnd->command);
    return 0;
  }
  if(strcmp(cmnd->command, "ls") == 0)
    return 1;
  fscanf(stdin, "%s", cmnd->arg);

  return 1;
}
