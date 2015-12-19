
/*
# # # # # # # # # # СГЕНЕРИРОВАНО ПРИ mount # # # # # # # # # #
### образ 16 Мб
mkfs.vfat -c -v -f2 -n FAT16 -r224 -F16 fatdisk
mkfs.fat 3.0.26 (2014-03-07)
fatdisk has 64 heads and 32 sectors per track,
hidden sectors 0x0000;
logical sector size is 512,
using 0xf8 media descriptor, with 32768 sectors;
drive number 0x80;
filesystem has 2 16-bit FATs and 4 sectors per cluster.
FAT size is 32 sectors, and provides 8171 clusters.
There is 1 reserved sector.
Root directory contains 256 slots and uses 16 sectors.
Volume ID is bab1abfd, volume label FAT16      .
*/

/*
 ls  -- на 428 строке
 cat -- 459
 cd  -- 502
 */

#include "ftools.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// !
DISK *disk;
#define BUF_SIZE 512

#define CHECK(condition, message)               \
  do {                                          \
    if(condition) {                             \
      fprintf(stdout, "Error: %s\n", message);  \
      exit(-1);                                 \
    }                                           \
  } while(0)

// считается нулевым кластером в FAT
#define ROOT_DIR_CLUSTER_NUMBER 0
// нумерация кластеров в области данных начинается с 2
#define START_NUMBERING_CLUSTERS 2

static void get_disk_parameters();
static unsigned int get_sector_size();
static unsigned int get_reserved_sectors_number();
static unsigned int get_sectors_number_per_cluster();
static unsigned int get_FATs_number();
static unsigned int get_sector_per_fat();
static unsigned int get_root_entries();
static void set_additional_disk_parameters();
//
static unsigned int get_cluster_number(size_t offset);
static void set_next_cluster(unsigned int next_cluster_number);
static unsigned int get_next_cluster_number(unsigned int cur_cluster_number);

void init(char *disk_name) {
  assert(disk_name);
  disk = (DISK *)malloc(sizeof(DISK));
  CHECK(!disk, "Out of memory, disk wasn't created\n");
  //
  disk->fp = (FILE*)fopen(disk_name, "r");
  CHECK(disk->fp == NULL, "open file");

  disk->buf = (char *)malloc(sizeof(char) * BUF_SIZE);
  CHECK(disk->buf == NULL, "doesn't allocate memory for buf");

  // get disk parameters
  get_disk_parameters();
  set_additional_disk_parameters();

  // set file position at the beginning of root_dir
  fseek(disk->fp, disk->root_dir_offset, SEEK_SET);
  disk->current_file_pos = disk->root_dir_offset;
}

void deinit() {
  CHECK(fclose(disk->fp) != 0, "close file");
  // free
  free(disk->buf);
  free(disk);
}

#define PRINT_DISK_INFO(info, n) printf("%s = %d\n", (info), (n))

static void get_disk_parameters() {
  disk->sector_size = get_sector_size();
  printf("sector size = %d bytes\n", disk->sector_size);
  disk->reserved_sectors_number = get_reserved_sectors_number();
  PRINT_DISK_INFO("reserved_sectors_number", disk->reserved_sectors_number);
  disk->sector_per_cluster = get_sectors_number_per_cluster();
  PRINT_DISK_INFO("sectors per cluster", disk->sector_per_cluster);
  disk->FATs_number = get_FATs_number();
  PRINT_DISK_INFO("FATs number", disk->FATs_number);
  disk->sector_per_fat = get_sector_per_fat();
  PRINT_DISK_INFO("sectors per 1 fat", disk->sector_per_fat);
  disk->root_entries = get_root_entries();
  PRINT_DISK_INFO("max_number_of_root_entries", disk->root_entries);
}
#undef PRINT_DISK_INFO

#define PRINT_ADD_INFO(info, n) printf("%s = %zu\n", (info), (n))
// 
static void set_additional_disk_parameters() {
  printf("###### Additional disk parameters ######\n");
  // 1.
  disk->FAT_offset = disk->reserved_sectors_number * disk->sector_size;
  PRINT_ADD_INFO("first_FAT_offset", disk->FAT_offset);
  // 2.
  disk->FAT_size = disk->sector_per_fat * disk->sector_size;
  PRINT_ADD_INFO("FAT_size", disk->FAT_size);
  // 3.
  disk->root_dir_offset = disk->FAT_offset + (disk->FATs_number * disk->FAT_size);
  PRINT_ADD_INFO("root_dir_offset", disk->root_dir_offset);

  // 4.
  disk->root_dir_size = disk->root_entries * ENTRY_SIZE;
  PRINT_ADD_INFO("root_dir_size", disk->root_dir_size);
  // 5.
  disk->data_offset = disk->root_dir_offset + disk->root_dir_size;
  PRINT_ADD_INFO("data_offset", disk->data_offset);
  // 6.
  disk->cluster_size = disk->sector_per_cluster * disk->sector_size;
  PRINT_ADD_INFO("cluster_size", disk->cluster_size);
}

// offset относительно position
// position can be
//   SEEK_SET (beginning of a file)
//   SEEK_CUR (current file position indicator)
//   SEEK_END (end of file)
#define SET_FILE_POSITION(offset, position) \
  do {                                      \
    fseek(disk->fp, (offset), (position));  \
  } while(0)

// this function is used to read system parameters (sector_size, FATs number, etc)
static unsigned int get_data(unsigned int offset, unsigned int length) {
  unsigned int data;

  SET_FILE_POSITION(offset, SEEK_SET);
  // read 1 element of data, each (length) bytes long
  fread(&data, length, 1, disk->fp);
  return data;
}

//
static unsigned int get_sector_size() {
#define SECTOR_SIZE_OFFSET 0xB
#define LENGTH 2
  return get_data(SECTOR_SIZE_OFFSET, LENGTH);
}
#undef SECTOR_SIZE_OFFSET
#undef LENGTH

static unsigned int get_reserved_sectors_number() {
#define OFFSET 0xE
#define LENGTH 2
  return get_data(OFFSET, LENGTH);
}
#undef OFFSET
#undef LENGTH

//
static unsigned int get_sectors_number_per_cluster() {
#define OFFSET 0xD
#define LENGTH 1
  return get_data(OFFSET, LENGTH);
}
#undef OFFSET
#undef LENGTH

//
static unsigned int get_FATs_number() {
#define OFFSET 0x10
#define LENGTH 1
  return get_data(OFFSET, LENGTH);
}
#undef OFFSET
#undef LENGTH

static unsigned int get_sector_per_fat() {
#define OFFSET 0x16
#define LENGTH 2
  return get_data(OFFSET, LENGTH);
}
#undef OFFSET
#undef LENGTH

static unsigned int get_root_entries() {
#define OFFSET 0x11
#define LENGTH 2
  return get_data(OFFSET, LENGTH);
}
#undef OFFSET
#undef LENGTH

/* * * * * * * * * * * * * * * */
/* * * * * * *  ls * * * * * * */
/* * * * * * * * * * * * * * * */
#define SET_CURRENT_DIR_POS   fseek(disk->fp, disk->current_file_pos, SEEK_SET)

// используется функцией ls
static void print_entry_info(ENTRY *entry) {
  int i;

  // после удаления файла FAT помечает первый байт имени e5
  // поэтому эти записи не надо выводить
  if(entry->name[0] == 0xffffffe5)
    return;
  fprintf(stdout, "%s", disk->buf);

  switch(entry->attr) {
    case 0x01:
      fprintf(stdout, "  read-only");
      break;
    case 0x10:
      fprintf(stdout, "  DIR");
      break;
    case 0x08:
      // файл метки тома может находиться только в рутовой директории
      fprintf(stdout, "  Volume_ID");
      break;
    default:
      fprintf(stdout, "  attr=0x%x", entry->attr);
  }
  fprintf(stdout, "  fsize=%u", entry->fsize); 
  fprintf(stdout, "  first_clstr=%u\n", entry->first_cluster_number);
}

static void print_cluster_content(size_t file_content_offset) {
  size_t i;
  char character;

  SET_FILE_POSITION(file_content_offset, SEEK_SET);
  for(i = 0; i < disk->cluster_size; i++) {
    fread(&character, sizeof(char), 1, disk->fp);
    if(character == 0)
      break;
    fprintf(stdout, "%c", character);
  }
}


#define LFN_MARK 0x40
#define ONE_LFN_SYMBOLS_NUMBER 13
// данная функция считывает long file name в disk->buf
// и возвращает кол-во прочитанных 32-байтовых LFN entries
static unsigned int read_LFN_to_buf() {
  unsigned int read_entries = 0;
  long starting_file_pos_indicator;
  unsigned char attr_byte, first_byte, byte;
  unsigned int entry_number;
  int is_lfn_read_finished = 0;  // not last
  unsigned int entry_pos, i;
  size_t lfn_length = 0;

#define ATTR_BYTE_OFFSET 0xB
  // 
  starting_file_pos_indicator = ftell(disk->fp);
  SET_FILE_POSITION(ATTR_BYTE_OFFSET, SEEK_CUR);
  fread(&attr_byte, sizeof(char), 1, disk->fp);
  SET_FILE_POSITION(starting_file_pos_indicator, SEEK_SET);
  //fprintf(stdout," attrbyte = %x\n", attr_byte);
  if(attr_byte != 0x0f) {
    // this is NOT LFN entry (for example, Volume ID entry)
    return 0;
  }
  fread(&first_byte, sizeof(char), 1, disk->fp);
  while(first_byte != 0 && !is_lfn_read_finished) {
    read_entries++;
    // if this file was removed, utility should ignore it
    if(first_byte == 0xe5) {
      SET_FILE_POSITION(ENTRY_SIZE - 1, SEEK_CUR);
      fread(&first_byte, sizeof(char), 1, disk->fp);
      continue;
    }
    //
    entry_number = (unsigned int)first_byte;
    if(first_byte > LFN_MARK && first_byte <= (LFN_MARK + 0x20)) {
      entry_number -= LFN_MARK;
    }
    if(entry_number == 0x01)
      is_lfn_read_finished = 1;
    // read
    entry_pos = 1;  // начинаем с 1, т.к. уже считали первый байт
    i = 0;
    while(entry_pos < ENTRY_SIZE) {
      if(entry_pos >= 1 && entry_pos < 0xA || entry_pos >= 0xE && entry_pos < 0x19 || entry_pos >= 0x1C && entry_pos < 0x1F) {
        fread(&byte, sizeof(char), 1, disk->fp);
        SET_FILE_POSITION(1, SEEK_CUR);
        entry_pos += 2;
      } else if(entry_pos == 0xB) {
        SET_FILE_POSITION(3, SEEK_CUR);
        entry_pos += 3;
        continue;
      } else if(entry_pos == 0x1A) {
        fread(&byte, sizeof(char), 1, disk->fp);
        SET_FILE_POSITION(1, SEEK_CUR);
        entry_pos += 2;
      } else {
        printf("while(entry_pos ...) ERROR!");
        exit(-1);
      }
      if(byte == 0 || byte == 0xff || byte == 0xf)
        continue;
      disk->buf[ONE_LFN_SYMBOLS_NUMBER * (entry_number-1) + i] = (char)byte;
      lfn_length++;
      i++;
    }
    fread(&first_byte, sizeof(char), 1, disk->fp);
  }
  
  //
  SET_FILE_POSITION(starting_file_pos_indicator + read_entries * ENTRY_SIZE, SEEK_SET);
  //
  disk->buf[lfn_length] = '\0';
  return read_entries;
}
#undef ATTR_BYTE_OFFSET
#undef LFN_MARK
#undef ONE_LFN_SYMBOLS_NUMBER

// в VFAT введена LFN (long file name), поэтому один файл может занимать
// не менее 2 entries (entries, связанные с LFN и сама entry об этом файле)
// (кроме метки тома, и директорий ".", ".." -- по 1 entry)
// возвращает, сколько записей было прочитано (то есть LFN_entries + 1 SFN_entry)
static unsigned int read_entry_info(ENTRY *entry) {
  unsigned int read_entries = 0;

  //
  memset(disk->buf, 0, BUF_SIZE);
  //
  read_entries = read_LFN_to_buf();
  fread(entry->name, sizeof(char), FNAME_SIZE, disk->fp);
  fread(entry->type, sizeof(char), FTYPE_SIZE, disk->fp);
  if(read_entries == 0) {
    strncpy(disk->buf, entry->name, FNAME_SIZE + FTYPE_SIZE);
    disk->buf[FNAME_SIZE + FTYPE_SIZE] = '\0';
  }
#define ATTR_BYTES_NUMBER 1
  fread(&(entry->attr), sizeof(char), ATTR_BYTES_NUMBER, disk->fp);
#define FIRST_CLUSTER_NUMBER_OFFSET 0x1A
  SET_FILE_POSITION(FIRST_CLUSTER_NUMBER_OFFSET - (FNAME_SIZE + FTYPE_SIZE + ATTR_BYTES_NUMBER), SEEK_CUR);
  // reads 1 element of data (number of first cluster, where this file is located), each 2 bytes long
  fread(&(entry->first_cluster_number), 2, 1, disk->fp);
  // reads 1 element of data (file_size in bytes), each 4 bytes long
  fread(&(entry->fsize), 4, 1, disk->fp);
  // прочитали еще 1 entry, поэтому:
  read_entries++;
  return read_entries;
}
#undef FIRST_CLUSTER_NUMBER_OFFSET
#undef ATTR_BYTES_NUMBER

// для чтения entries в директории на одном кластере
#define READ_CLUSTER_ENTRIES(entry, entry_number, max_entries_number) \
  do {                                                                \
    while(entry_number < max_entries_number) {                        \
      entry_number += read_entry_info(&entry);                        \
      if(entry.name[0] == 0)                                          \
        break;                                                        \
      print_entry_info(&entry);                                       \
    }                                                                 \
  } while(0)

/*
 * Данная функция читает содержимое кластеров
 * общая функция для функций ls, cat
 * 
 * offset -- offset (от начала fatdisk) интересуемого кластера
 * is_content  -- некоторый флаг: если 1, то читаем содержимое, как данные
 *                                если 0, то читаем содержимое, как entries
 */
static void read_clusters_content(size_t offset, int is_content) {
  ENTRY entry;
  unsigned int entry_number = 0;
  int is_there_next_cluster = 0;
  unsigned int next_cluster_number;
  unsigned int current_cluster_number;
  unsigned int max_entries_number;

  current_cluster_number = get_cluster_number(offset);
  if(current_cluster_number == ROOT_DIR_CLUSTER_NUMBER) {
    max_entries_number = disk->root_entries;
  } else {
    max_entries_number = disk->cluster_size / ENTRY_SIZE;
  }
  do {
    if(is_content) {
      // читать содержимое, как данные
      print_cluster_content(offset);
    } else {
      // читать содержимое, как entries
      READ_CLUSTER_ENTRIES(entry, entry_number, max_entries_number);
    }

    // get next cluster
    next_cluster_number = get_next_cluster_number(current_cluster_number);
    fprintf(stdout, "current_cluster_number=%d\n", current_cluster_number);
    fprintf(stdout, "next_cluster_number=%x\n", next_cluster_number);

    CHECK(next_cluster_number == 0, "this cluster must NOT be free!");
    CHECK(next_cluster_number >= 0xFFF0 && next_cluster_number <= 0xFFF6, "that's reserved cluster!");
    CHECK(next_cluster_number == 0xFFF7, "defective cluster!");
    if(next_cluster_number >= 0xFFF8 && next_cluster_number <= 0xFFFF) {
      // последний в цепочке
      is_there_next_cluster = 0;
    } else if(next_cluster_number >= 0x2 && next_cluster_number <= 0xFFEF) {
      // следующий кластер
      is_there_next_cluster = 1;
      offset = disk->data_offset + (next_cluster_number - START_NUMBERING_CLUSTERS) * disk->cluster_size;
      SET_FILE_POSITION(offset, SEEK_SET);
      current_cluster_number = next_cluster_number;
    }
  } while(is_there_next_cluster);
  SET_CURRENT_DIR_POS;
}

/*
 *   Если находимся в не root_dir, то так как эта dir считается как файл (все содержимое в области данных)
 *   возможно, эта директория со всем своим содержимым (то есть со всеми записями, которые она содержит)
 *   находится не на одном кластере => надо проверять таблицу FAT
 */
void ls() {
  // флаг = 0, т.к. хотим прочитать содержимое кластера, как entries
  read_clusters_content(disk->current_file_pos, 0);
}

////////////////////////////////////////
#define SEARCH_ENTRY(fname, file_entry_offset, file_entry)                                \
  do {                                                                                    \
    (file_entry_offset) = search_entry(fname, get_cluster_number(disk->current_file_pos));\
    if(file_entry_offset == 0) {                                                          \
      fprintf(stdout, "entry with name %s doesn't exist\n", fname);                       \
      SET_CURRENT_DIR_POS;                                                                \
      return;                                                                             \
    }                                                                                     \
    SET_FILE_POSITION(file_entry_offset, SEEK_SET);                                       \
    read_entry_info(file_entry);                                                          \
  } while(0)

/* * * * * * * * * * * * * * * */
/* * * * * * *  cd * * * * * * */
/* * * * * * * * * * * * * * * */
static size_t search_entry(char *entry_name_and_type, unsigned int cur_cluster_number);
static int copy_entry_name_and_type_to_buf(char *dirname);


// 1. Проверить, что директория с таким именем существует в текущей директории
//      Если нет, то сообщаем об этом
// 2. Если нашли (т.е. существует entry с именем dirname)
//      Надо SET_FILE_POSITION на нужное место
//        т.е. узнать номер кластера (0-ой кластер - это root_dir, кластеры в области данных нумеруются, начиная с 2)
//      Изменить disk->current_file_pos
void cd(char *dirname) {
  ENTRY dir_entry;
  unsigned int entry_number = 0;
  size_t dir_entry_offset;
  size_t dir_offset;

  // 1.
  memset(disk->buf, 0, BUF_SIZE);
  SEARCH_ENTRY(dirname, dir_entry_offset, &dir_entry);

  // 2
  // дополн. проверить, что это директория по attr_byte
  if(dir_entry.attr != 0x10) {
    SET_CURRENT_DIR_POS;
    fprintf(stdout, "%s is NOT a directory\n", dirname);
    return;
  }

  // 3.
  if(dir_entry.first_cluster_number == ROOT_DIR_CLUSTER_NUMBER) {
    // то есть нулевой кластер (рутовая директория)
    dir_offset = disk->root_dir_offset;
  } else {
    dir_offset = disk->data_offset + (dir_entry.first_cluster_number - START_NUMBERING_CLUSTERS) * disk->cluster_size;
  }
  // 4. обновляем property current_file_pos
  disk->current_file_pos = dir_offset;
  // и ставим file indicator в fatdisk
  SET_FILE_POSITION(dir_offset, SEEK_SET);
}

/* * * * * * * * * * * * * * * */
/* * * * * * * cat * * * * * * */
/* * * * * * * * * * * * * * * */
#define POSSIBLE_ERROR_IN_CAT(condition, message)  \
  do {                                              \
    if(condition) {                                 \
      fprintf(stdout, "%s\n", message);             \
      SET_CURRENT_DIR_POS;                          \
      return;                                       \
    }                                               \
  } while(0)

void cat(char *fname) {
  ENTRY file_entry;
  size_t file_entry_offset;
  size_t file_content_offset;
  int is_there_next_cluster = 0;
  unsigned int next_cluster_number;
  unsigned int current_cluster_number;

  // 1.
  SEARCH_ENTRY(fname, file_entry_offset, &file_entry);
  // 2.
  POSSIBLE_ERROR_IN_CAT(file_entry.attr == 0x10, "It is directory!");
  POSSIBLE_ERROR_IN_CAT(file_entry.attr == 0x08, "It is Volume_ID!");

  // 3.
  file_content_offset = disk->data_offset + (file_entry.first_cluster_number - START_NUMBERING_CLUSTERS) * disk->cluster_size;
  // флаг = 1, т.к. хотим прочитать содержимое кластера, как данные
  read_clusters_content(file_content_offset, 1);
}
#undef POSSIBLE_ERROR_IN_CAT
#undef SEARCH_ENTRY

/* * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * */

#define CHECK_DOTS(byte, sign)                  \
  do {                                          \
    fread(&byte, sizeof(char), 1, disk->fp);    \
    if(byte != sign) {                          \
      SET_CURRENT_DIR_POS;                      \
      return 0;                                 \
    }                                           \
  } while(0)

#define DOT         1
#define DOUBLE_DOT  2
// эта функция используется search_entry(...) функцией
static unsigned int dot_or_double_dot_directory(char *entry_name_and_type) {
  char byte;
  int i, is_dot, is_double_dot;
  unsigned int cur_cluster_number = get_cluster_number((size_t)ftell(disk->fp));
  unsigned int offset, length;

  if(cur_cluster_number == ROOT_DIR_CLUSTER_NUMBER)
    return 0;
  is_dot = is_double_dot = 0;
  offset = length = 0;
  if(strcmp(entry_name_and_type, ".") == 0) {
    is_dot = 1;
    offset = 0;
    length = 1; // strlen(".")
  } else if(strcmp(entry_name_and_type, "..") == 0) {
    is_double_dot = 1;
    offset = ENTRY_SIZE;
    length = 2; // strlen("..")
  }
  // проверим, что других символов нет (т.е. что точно "." или "..")
  SET_FILE_POSITION(offset, SEEK_CUR);
  for(i = 0; i < length; i++) {
    CHECK_DOTS(byte, '.');
  }
  for(i = length; i < FNAME_SIZE; i++) {
    CHECK_DOTS(byte, ' ');
  }
  for(i = 0; i < FTYPE_SIZE; i++) {
    CHECK_DOTS(byte, ' ');
  }
  SET_CURRENT_DIR_POS;
  if(is_dot)
    return DOT;
  else if(is_double_dot)
    return DOUBLE_DOT;
}
#undef CHECK_DOTS

static int str_contains_only_space_characters(char *str, unsigned int length) {
  unsigned int i;

  for(i = 0; i < length; i++) {
    if(str[i] != ' ')
      return 0;
  }
  return 1;
}

// соответствие имени entry_name_and_type и entry->name & entry->type для SFN (not LFN)
static int is_needed_SFN_entry(ENTRY *entry, char *entry_name_and_type) {
  unsigned int i, cur_pos;
  unsigned char with_type = 0;    // флаг, что есть тип (считана '.')
  unsigned int length = (unsigned int)strlen(entry_name_and_type);

  // it is LFN
  if(length > FTYPE_SIZE + FNAME_SIZE)
    return 0;
  for(i = 0; i < FNAME_SIZE && i < length; i++) {
    if(entry_name_and_type[i] == '.') {
      with_type = 1;
      // проверить, что оставшиеся в entry->name[i+1, ... FNAME_SIZE-1] == '.'
      if(!str_contains_only_space_characters(entry->name + i, FNAME_SIZE - i))
        return 0;
      else
        // идем к чтению type
        break;
    }
    // в FAT названия файлов и типов -- заглавные буквы (SFN)
    if(!(entry->name[i] == entry_name_and_type[i] || entry->name[i] == entry_name_and_type[i] + 'A'-'a'))
      return 0;
  }
  if(!with_type) {
    // т.е. еще не прочитали '.'
    if(i == length) {
      // прочитать оставшиеся, и если там ' ', то return 1; иначе return 0
      if(str_contains_only_space_characters(entry->name + i, FNAME_SIZE - i)) {
        if(str_contains_only_space_characters(entry->type, FTYPE_SIZE))
          return 1;
        else
          return 0;
      } else
        return 0;
      // примеры -- FAT16, mydir (DIR)
    } else {
      // еще есть несчитанные символы
      if(entry_name_and_type[++i] != '.') {
        // this is LFN
        return 0;
      }
    }
  }
  // до этого места дойдем, если считали предыдущим символом '.'
  cur_pos = i + 1;    // указывает на первый символ после точки
  for(i = 0; i < FTYPE_SIZE && i + cur_pos < length; i++) {
    // ascii for type, too
    if(!(entry->type[i] == entry_name_and_type[cur_pos+i] || entry->type[i] == entry_name_and_type[cur_pos+i] + 'A'-'a'))
      return 0;
  }
  if(i < 3) {
    // проверить оставшиеся на ' '
    // если все нормально, то вернуть 1
    if(str_contains_only_space_characters(entry->type + i, FTYPE_SIZE - i))
      return 1;
    else
      return 0;
  } else if(i + cur_pos < length) {
      // asdf.tttt; после точки максимум 3 символа
      return 0;
  } else {
    // i == 3
    return 1;
  }
}

// offset <- from beginning of fatdisk
static unsigned int get_cluster_number(size_t offset) {
  unsigned int cluster_number;
  size_t offset_from_data;

  if(offset == disk->root_dir_offset) {
    return ROOT_DIR_CLUSTER_NUMBER;
  }
  // not root dir
  offset_from_data = offset - disk->data_offset;
  cluster_number = START_NUMBERING_CLUSTERS + offset_from_data / (disk->sector_per_cluster * disk->sector_size);
  return cluster_number;
}

// в FAT таблице каждая строка занимает 2 байта
#define FAT_ENTRY_SIZE 2

static unsigned int get_next_cluster_number(unsigned int cur_cluster_number) {
  unsigned int next_cluster_number;

  if(cur_cluster_number == ROOT_DIR_CLUSTER_NUMBER) {
    next_cluster_number = 0xFFFF;
  } else {
    // Идем в FAT, в соответствующую строчку
    SET_FILE_POSITION(disk->FAT_offset + cur_cluster_number * FAT_ENTRY_SIZE, SEEK_SET);
    fread(&next_cluster_number, FAT_ENTRY_SIZE, 1, disk->fp);
    SET_CURRENT_DIR_POS;
  }
  return next_cluster_number;
}

// установить позицию чтения (file indicator in fatdisk) на начало чтения следующего кластера
static void set_next_cluster(unsigned int next_cluster_number) {
  // for debugging
  CHECK(next_cluster_number == 0, "set_next_cluster_function tries to set position to root_dir_cluster");
  SET_FILE_POSITION(disk->data_offset + (next_cluster_number - START_NUMBERING_CLUSTERS) * disk->cluster_size, SEEK_SET );
}

// чтобы искать entries (dir, files) в текущей директории
// (будет использоваться в cd, cat функциях)
// если находим, то возвращаем соответствующий offset 
//    для short file name entry от начала fatdisk
// иначе
//    0
static size_t search_entry(char *entry_name_and_type, unsigned int cur_cluster_number) {
  ENTRY entry;
  size_t offset = 0;
  unsigned int current_dir_size;  // in bytes
  unsigned int max_dir_entries_number;
#define IS_NOT_FOUND_YET  2
#define WAS_FOUND         1
#define WAS_NOT_FOUND     0
  unsigned int n = 0; //
  int search_state = IS_NOT_FOUND_YET;
  unsigned int next_cluster_number;
  unsigned int dot_or_double_dot_dir;
  
  // 0. Т.к. директории '.' и '..' -- SFN, то придется отдельно проверить
  if((dot_or_double_dot_dir = dot_or_double_dot_directory(entry_name_and_type)) == DOT) {
    return ftell(disk->fp);
  } else if(dot_or_double_dot_dir == DOUBLE_DOT) {
    return ftell(disk->fp) + ENTRY_SIZE;
  }

  // 1.
  if(cur_cluster_number == ROOT_DIR_CLUSTER_NUMBER) {
    // root_dir
    current_dir_size = disk->root_dir_size;
  } else {
    // 1 cluster
    current_dir_size = disk->cluster_size;
  }
  
  // 2.
  max_dir_entries_number = current_dir_size / ENTRY_SIZE;
  while(n < max_dir_entries_number) {
    /* после */
    n += read_entry_info(&entry);
    if(entry.name[0] == 0) {
      // уже закончились все записи, значит, не нашли
      search_state = WAS_NOT_FOUND;
      break;
    }
    if(strcmp(entry_name_and_type, disk->buf) == 0 || is_needed_SFN_entry(&entry, entry_name_and_type) ) {
      search_state = WAS_FOUND;
      break;
    }
  }
  // after reading entries
  SET_CURRENT_DIR_POS;

  //
  if(search_state == IS_NOT_FOUND_YET && cur_cluster_number == ROOT_DIR_CLUSTER_NUMBER) {
    return WAS_NOT_FOUND;
  }

  if(search_state == WAS_NOT_FOUND) {
    return WAS_NOT_FOUND;
  } else if(search_state == WAS_FOUND) {
    if(cur_cluster_number == ROOT_DIR_CLUSTER_NUMBER) {
      offset = disk->root_dir_offset + (n - 1) * ENTRY_SIZE;
    } else {
      // смещение до нужного кластера
      offset = disk->data_offset + (cur_cluster_number - START_NUMBERING_CLUSTERS)*disk->sector_per_cluster * disk->sector_size;
      // смещение внутри этого кластера до нужного entry
      offset += (n-1) * ENTRY_SIZE;
    }
    return offset;
  }

  //
  if(search_state == IS_NOT_FOUND_YET && cur_cluster_number != ROOT_DIR_CLUSTER_NUMBER) {
    // т.е. текущий cluster не рутовая директория, и не нашли пока в нем
    // следовательно, надо проверить в следующих кластерах, т.е. прочитать содержимое FAT таблицы
    next_cluster_number = get_next_cluster_number(cur_cluster_number);

    CHECK(next_cluster_number == 0, "this cluster must NOT be free!");
    CHECK(next_cluster_number >= 0xFFF0 && next_cluster_number <= 0xFFF6, "that's reserved cluster!");
    CHECK(next_cluster_number == 0xFFF7, "defective cluster!");
    if(next_cluster_number >= 0xFFF8 && next_cluster_number <= 0xFFFF) {
      // последний в цепочке
      SET_CURRENT_DIR_POS;
      return WAS_NOT_FOUND;
    } else if(next_cluster_number >= 0x2 && next_cluster_number <= 0xFFEF) {
      // следующий кластер
      // ставим позицию в файле на начало следующего кластера
      SET_FILE_POSITION(disk->data_offset + (next_cluster_number - START_NUMBERING_CLUSTERS) * disk->cluster_size, SEEK_SET);
      // рекурсивно вызываем эту функцию
      return search_entry(entry_name_and_type, next_cluster_number);
    }
  }
}


#undef IS_NOT_FOUND_YET
#undef WAS_FOUND
#undef WAS_NOT_FOUND
#undef FAT_ENTRY_SIZE
#undef DOT
#undef DOUBLE_DOT

#undef CHECK
#undef SET_CURRENT_DIR_POS
#undef SET_FILE_POSITION
#undef ROOT_DIR_CLUSTER_NUMBER
#undef START_NUMBERING_CLUSTERS
#undef BUF_SIZE
