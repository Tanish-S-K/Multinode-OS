#include "header/mystdlib.h"

#define total_sectors 2048
#define bitmap_start 200
#define entry_start 210
#define entry_end 1000
#define data_start 1001

void load_bitmap();
void save_bitmap();
void format_disk();
uint16_t find_dir(char* name);

typedef struct {
    char name[19];
    uint8_t type;
    uint16_t parent;
    uint16_t child;
    uint16_t next_sib;
    uint16_t prev_sib;
    uint16_t data_sector;
    uint16_t size;
} DiskNode;

uint16_t bitmap[total_sectors+1];
uint16_t cur_sec;
char copy_buffer[512]="\0";

void read_node(uint16_t sector, DiskNode* node) {
    uint16_t buffer[256];
    read_sector(sector, buffer);
    mem_cpy(node, buffer, sizeof(DiskNode));
}

void write_node(uint16_t sector, DiskNode* node) {
    uint16_t buffer[256] = {0};
    mem_cpy(buffer, node, sizeof(DiskNode));
    write_sector(sector, buffer);
}

void print_bit(){
    load_bitmap();
    for (int i=190;i<entry_start+10;i++) {
        if (bitmap[i] == 1)
            print("1");
        else
            print("0");
    }
}


void load_bitmap() {
    uint16_t buffer[256];
    for (int i = 0; i < total_sectors / 256; i++) {
        read_sector(bitmap_start+i, buffer);
        mem_cpy(bitmap + i*256, buffer, sizeof(buffer));
    }
}


void save_bitmap() {
    uint16_t buffer[256];
    for (int i=0;i<total_sectors/256;i++) {
        mem_cpy(buffer, bitmap+i*256, sizeof(buffer));
        write_sector(bitmap_start+i, buffer);
    }
}


uint16_t salloc() {
    load_bitmap();

    for (uint16_t i=entry_start; i<total_sectors; i++) {
        if (bitmap[i] != 1) {
            bitmap[i] = 1;
            save_bitmap();
            return i;
        }
    }
    return 0;
}


int init_file_system() {
    load_bitmap();

    cur_sec = entry_start;
    if (bitmap[entry_start] == 1) {
        return 0;
    }
    format_disk();
    for(int i=0;i<entry_start+1;i++){
        bitmap[i] = 1;
    }
    save_bitmap();

    DiskNode root;
    cpy(root.name, "/",0);
    root.type = 1;
    root.parent = 0;
    root.child = 0;
    root.next_sib = 0;
    root.prev_sib = 0;
    root.data_sector = 0;
    root.size = 0;

    write_node(entry_start, &root);
    return 1;
}

uint16_t create_dir_cur(char *name) {
    uint16_t sec = salloc();
    if (!sec) {
        print("Disk full!\n");
        return -1;
    }

    DiskNode dir;
    cpy(dir.name, name,0);
    dir.type = 1;
    dir.parent = cur_sec;
    dir.child = 0;
    dir.next_sib = 0;
    dir.prev_sib = 0;
    dir.data_sector = 0;
    dir.size = 0;

    DiskNode parent;
    read_node(cur_sec, &parent);
    if (parent.child == 0) {
        parent.child = sec;
        write_node(cur_sec, &parent);
    } else {
        DiskNode ptr;
        uint16_t ptr_sector = parent.child;
        read_node(ptr_sector, &ptr);
        while (ptr.next_sib) {
            ptr_sector = ptr.next_sib;
            read_node(ptr_sector, &ptr);
        }
        ptr.next_sib = sec;
        write_node(ptr_sector, &ptr);
        dir.prev_sib = ptr_sector;
    }
    write_node(sec, &dir);
    return sec;
}

void list() {
    DiskNode current;
    read_node(cur_sec, &current);
    if (current.child == 0) {
        print("No files or directories\n");
        return;
    }
    uint16_t ptr_sector = current.child;
    DiskNode ptr;
    read_node(ptr_sector, &ptr);

    while (1) {
        print(ptr.name);
        print(ptr.type == 1 ? " [DIR]\n" : " [FILE]\n");
        if (ptr.next_sib == 0) break;
        ptr_sector = ptr.next_sib;
        read_node(ptr_sector, &ptr);
    }
}


// file functions 

uint16_t create_file_cur(char* name) {
    uint16_t sec = salloc();
    if (!sec) {
        print("Disk full!\n");
        return 0;
    }

    DiskNode file;
    cpy(file.name, name,0);
    file.type = 0;
    file.parent = cur_sec;
    file.child = 0;
    file.next_sib = 0;
    file.prev_sib = 0;
    file.data_sector = 0;
    file.size = 0;

    DiskNode parent;
    read_node(cur_sec, &parent);
    if (parent.child == 0) {
        parent.child = sec;
        write_node(cur_sec, &parent);
    } else {
        DiskNode ptr;
        uint16_t ptr_sector = parent.child;
        read_node(ptr_sector, &ptr);
        while (ptr.next_sib) {
            ptr_sector = ptr.next_sib;
            read_node(ptr_sector, &ptr);
        }
        ptr.next_sib = sec;
        write_node(ptr_sector, &ptr);
        file.prev_sib = ptr_sector;
    }

    write_node(sec, &file);
    return sec;
}

<<<<<<< Updated upstream
uint16_t find_file(char* name){
    DiskNode cur;
    for(int i=entry_start;i<entry_end+1;i++){
        if (bitmap[i]){
            read_node(i,&cur);
            if(cur.type==0 && cmp(cur.name,name)){
                return i;
            }
        }
    }
    return 0;
}

uint16_t find_dir(char* name){
    DiskNode cur;
    for(int i=entry_start;i<entry_end;i++){
        if (bitmap[i]){
            read_node(i,&cur);
            if(cur.type==1 && cmp(cur.name,name)){
                return i;
            }
        }
    }
    return 0;
}

void write_file(char* name, char* content) {
=======
uint16_t create_file(char* name,int i){
    char cur[1024];
    i = nextarg(name,i,cur,'/');
    if (name[i]=='\0') {
        return create_file_cur(cur);
    }
    int ret = cur_sec;
    int posi = find_dir(cur);
    if (posi == 0) return 0;
    cur_sec = posi;
    int res = create_file(name,i+1);
    cur_sec = ret;
    return res;
}

uint16_t create_dir(char* name,int i){
    char cur[1024];
    i = nextarg(name,i,cur,'/');
    if (name[i]=='\0') {
        return create_dir_cur(cur);
    }
    int ret = cur_sec;
    int posi = find_dir(cur);
    if (posi == 0) return 0;
    cur_sec = posi;
    int res = create_dir(name,i+1);
    cur_sec = ret;
    return res;
}

uint16_t search_file(char* name,uint16_t root){
    DiskNode cur;
    read_node(root,&cur);

    if (cur.type==0 && cmp(name,cur.name)) return root;

    if (cur.child != 0){
        int status= search_file(name,cur.child);
        if (status!=0) return status;
    }

    if (cur.next_sib!=0){
        int status= search_file(name,cur.next_sib);
        if (status!=0) return status;
    }

    return 0;
}

uint16_t search_dir(char* name,uint16_t root){
    DiskNode cur;
    read_node(root,&cur);

    if (cur.type==1 && cmp(name,cur.name)) return root;

    if (cur.child != 0){
        int status= search_dir(name,cur.child);
        if (status!=0) return status;
    }
    
    if (cur.next_sib!=0){
        int status= search_dir(name,cur.next_sib);
        if (status!=0) return status;
    }

    return 0;
}

uint16_t find_file(char* name){
    DiskNode cur;
    read_node(cur_sec,&cur);
    if (cmp(name,"..")) return cur.parent;

    int sector = cur.child;

    while (sector!=0){
        read_node(sector,&cur);
        if (cur.type==0 && cmp(cur.name,name)) return sector;
        sector = cur.next_sib;
    }
    return 0;
}

uint16_t find_dir(char* name){
    DiskNode cur;
    read_node(cur_sec,&cur);
    if (cmp(name,"..")) return cur.parent;

    int sector = cur.child;

    while (sector!=0){
        read_node(sector,&cur);
        if (cur.type == 1 && cmp(cur.name,name)) return sector;
        sector = cur.next_sib;
    }
    return 0;
}

void write_file_cur(char* name, char* content) {
>>>>>>> Stashed changes
    uint16_t file_sec = find_file(name);
    if (!file_sec) {
        print("No Such File Found!!\n");
        return;
    }
    DiskNode file;
    read_node(file_sec,&file);

    for(int i=data_start;i<total_sectors;i++){
        if (!bitmap[i]){
            bitmap[i] = 1;
            write_sector(i,(uint16_t *)content);
            file.data_sector = i;
            write_node(file_sec,&file);
            return;
        }
    }
}

<<<<<<< Updated upstream
int read_file(char* name,char *data) {
=======
void write_file(char* name,char* content,int i){
    char cur[1024];
    i = nextarg(name,i,cur,'/');
    if (name[i]=='\0') {
        write_file_cur(cur,content);
        return;
    }
    int ret = cur_sec;
    int posi = find_dir(cur);
    if (posi == 0) {
        print("Path is unknown!!\n");
        return;
    }
    cur_sec = posi;
    write_file(name,content,i+1);
    cur_sec = ret;
}

int read_file_cur(char* name,char *data) {
>>>>>>> Stashed changes
    uint16_t file_sec = find_file(name);
    if (!file_sec) {
        print("No Such File Found!!\n");
        return 0;
    }
    DiskNode file;
    read_node(file_sec,&file);
    int data_sec = file.data_sector;
    if (data_sec){
        read_sector(data_sec,(uint16_t*)data);
    }
    else{
        print("Empty File!!\n");
    }
    return 1;
}

<<<<<<< Updated upstream
void delete_file(char* name) {
=======
int read_file(char* name,char* content,int i){
    char cur[1024];
    i = nextarg(name,i,cur,'/');
    if (name[i]=='\0') {
        return read_file_cur(cur,content);
    }
    int ret = cur_sec;
    int posi = find_dir(cur);
    if (posi == 0) {
        print("Path is unknown!!\n");
        return 0;
    }
    cur_sec = posi;
    int res = read_file(name,content,i+1);
    cur_sec = ret;
    return res;
}

void delete_file_cur(char* name) {
>>>>>>> Stashed changes
    uint16_t file_sec = find_file(name);
    if (!file_sec) {
        print("No Such File Exist!!!\n");
        return;
    }

    DiskNode file;
    read_node(file_sec, &file);

    DiskNode parent;
    read_node(file.parent, &parent);

    if (parent.child == file_sec) {
        parent.child = file.next_sib;
        write_node(file.parent, &parent);
        if (file.next_sib) {
            DiskNode sib;
            read_node(file.next_sib, &sib);
            sib.prev_sib = 0;
            write_node(file.next_sib, &sib);
        }
    } else {
        if (file.prev_sib) {
            DiskNode prev;
            read_node(file.prev_sib, &prev);
            prev.next_sib = file.next_sib;
            write_node(file.prev_sib, &prev);
        }
        if (file.next_sib) {
            DiskNode next;
            read_node(file.next_sib, &next);
            next.prev_sib = file.prev_sib;
            write_node(file.next_sib, &next);
        }
    }
    bitmap[file_sec] = 0;
    clean_sector(file_sec);
    if (file.data_sector) {
        bitmap[file.data_sector] = 0;
        clean_sector(file.data_sector);
    }
    save_bitmap();
    print("File deleted successfully\n");
}

void delete_file(char* name,int i){
    char cur[1024];
    i = nextarg(name,i,cur,'/');
    if (name[i]=='\0') {
        delete_file_cur(cur);
        return;
    }
    int ret = cur_sec;
    int posi = find_dir(cur);
    if (posi == 0) {
        print("Path is unknown!!\n");
        return;
    }
    cur_sec = posi;
    delete_file(name,i+1);
    cur_sec = ret;
}

int copy_file(char *name){
    int status = read_file(name,copy_buffer,0);
    return status;
}

void move_file(char *name){
    if (copy_file(name))
    delete_file(name,0);
}

void paste_file(char *name){
    if (copy_buffer[0]){
        create_file(name,0);
        write_file(name,copy_buffer,0);
        return;
    }
    print("No Data Available!!\n");
}

void curpos(){
    DiskNode node;
    read_node(cur_sec,&node);

    if (cmp(node.name,"/")){
        print("/");
    } else{
        uint16_t cur = cur_sec;
        cur_sec = node.parent;
        curpos();
        print(node.name);
        print("/");
        cur_sec = cur;
    }
}

void goparent(){
    DiskNode node;
    read_node(cur_sec,&node);
    if (!node.parent){
        print("NO Parent Exist!!!\n");
    }
    cur_sec = node.parent;
}

void godir(char *name,int i){
    char cur[100];
    i = nextarg(name,i,cur,'/');

    if (len(cur)==0){
        // nothing hehehe..
    }   else{

        if (cmp(cur,"..")){
            goparent();
        } else{
            uint16_t sec= find_dir(cur);
            if (!sec){
                print("No Such Directory Exits!!!!\n");
                return;
            }
            cur_sec = sec;
        }
        godir(name,i+1);
    }
}

void format_disk(){
    for(int i=entry_start;i<total_sectors;i++){
        clean_sector(i);
    }
}


