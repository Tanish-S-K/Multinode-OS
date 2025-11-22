#include "header/mystdlib.h"
#include "header/file_system.h"
#include "header/auth.h"

void shutdown();
void restart();

void cli(){
    char buffer[100],parse[100];
    int i,status;
    do {
        input(buffer);
        i=0;

        i = nextarg(buffer,i,parse,' ');
        
        if (cmp(parse,"pos")){
            curpos();
        }
        else if(cmp(parse,"ndir")){
            status = nextarg(buffer,i,parse,' ');
            if (status!=-1)
            create_dir(parse,0);
        }
        else if(cmp(parse,"list")){
            list();
        }
        else if (cmp(parse,"goto")){
            status = nextarg(buffer,i,parse,' ');
            if (status!=-1)
            godir(parse,0);
        }
        else if(cmp(parse,"clear")){
            clear_screen();
        }
        else if(cmp(parse,"nfile")){
            status = nextarg(buffer,i,parse,' ');
            if (status!=-1)
            create_file(parse,0);
        }
        else if (cmp(parse,"wfile")){
            status = nextarg(buffer,i,parse,' ');
            if (status!=-1){
                char content[100];
                status = nextarg(buffer,status,content,'\0');
                if(status!=-1)
                write_file(parse,content,0);
            }
        }
        else if (cmp(parse,"rfile")){
            status = nextarg(buffer,i,parse,' ');
            if (status!=-1){
                char content[100];
                read_file(parse,content,0);
                print(content);
                print("\n");
            }
        }
        else if (cmp(parse,"cpy")){
            status = nextarg(buffer,i,parse,' ');
            if(status!=-1){
                copy_file(parse);
            }
        }
        else if (cmp(parse,"mov")){
            status = nextarg(buffer,i,parse,' ');
            if(status!=-1){
                move_file(parse);
            }
        }
        else if (cmp(parse,"paste")){
            status = nextarg(buffer,i,parse,' ');
            if(status!=-1){
                paste_file(parse);
            }
        }
        else if (cmp(parse,"dfile")){
            status = nextarg(buffer,i,parse,' ');
            if (status!=-1){
                delete_file(parse,0);
            }
        }
        else if (cmp(parse,"dformat")){
            format_disk();
        }
        else if (cmp(parse,"nuser")){
            create_user();
        }
        else if (cmp(parse,"shutdown")) {
            shutdown();
            return;
        }

        else if (cmp(parse,"restart")) {
            restart();
            return;
        }
        else{
            print("Use only my command... hehehe...");
        }
        print("\n");
    } while(1);
}

void shutdown() {
    outw(0x604, 0x2000);
}

void restart() {
    uint8_t temp;

    asm volatile("cli");

    do {
        temp = inb(0x64);
    } while (temp & 0x02);

    outb(0x64, 0xFE);
    asm volatile ("lidt (0)");
    asm volatile ("int $3");
}
