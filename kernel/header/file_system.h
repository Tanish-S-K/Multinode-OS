#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "mystdlib.h"

#define total_sectors 2048
#define bitmap_start 200
#define entry_start 210
#define entry_end 1000
#define data_start 1001

typedef struct {
    char name[16];
    uint8_t type;
    uint16_t parent;
    uint16_t child;
    uint16_t next_sib;
    uint16_t prev_sib;
    uint16_t data_sector;
    uint16_t size;
} DiskNode;

extern uint16_t cur_sec;
int init_file_system();
int check_bit(int sector);
void set_bit(int sector,uint16_t val);
void load_bitmap();
void set_user(uint16_t user_sector);
uint16_t find_dir(char* name);

uint16_t create_dir(char *name,int i);
uint16_t create_file(char* name,int i);
void write_file(char* name,char* content,int i);
int read_file(char* name,char *data,int i);
void delete_file(char* name,int i);
int copy_file(char *name);
void move_file(char *name);
void paste_file(char *name);
void godir(char *name,int i);
void curpos();
void goparent();
void format_disk();
void list();


#endif
