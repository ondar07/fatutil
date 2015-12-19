
#ifndef _FTOOLS_H
#define _FTOOLS_H

#include <stdio.h>


typedef struct {
  FILE *fp;
  char *buf;
  size_t disk_size;
  unsigned int sector_size;
  unsigned int sector_per_cluster;
  unsigned int reserved_sectors_number; // number of boot sectors
  unsigned int FATs_number;
  unsigned int sector_per_fat;  // => FAT size = sector_per_fat * sector_size (bytes)
  unsigned int root_entries;    // max number of elements in root directory

  // additional parameters
  size_t FAT_offset;        // offset of the first FAT
  size_t FAT_size;
  size_t root_dir_offset;
  size_t root_dir_size;
  size_t data_offset;
  size_t cluster_size;

  // в файле образа fatdisk
  size_t current_file_pos;
} DISK;

#define ENTRY_SIZE 32
#define FNAME_SIZE 8
#define FTYPE_SIZE 3

typedef struct {
  char name[FNAME_SIZE];
  char type[FTYPE_SIZE];
  unsigned char attr;
  unsigned int first_cluster_number;
  unsigned int fsize;
} ENTRY;

void init(char *disk_name);
void deinit();
void ls();
void cat(char *fname);
void cd(char *dirname);

#endif
