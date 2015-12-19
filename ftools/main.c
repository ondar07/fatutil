#include <stdio.h>
#include <string.h>
#include "ftools.h"
#include "command_interface.h"

int main() {
  init("../fatdisk");
  CMND *cmnd = cmnd_interf_init();
  int res;

  fprintf(stdout, "Enter your commands (ls, cat, cd); type \'exit\' or \'quit\' to finish work\n");
  do {
    res = enter_command(cmnd);
    if(!res)
      continue;
    if(strcmp(cmnd->command, "ls") == 0)
      ls();
    if(strcmp(cmnd->command, "cd") == 0)
      cd(cmnd->arg);
    if(strcmp(cmnd->command, "cat") == 0)
      cat(cmnd->arg);
  } while(res >= 0);

  cmnd_interf_deinit(cmnd);
  deinit();
  return 0;
}
