#include "header/mystdlib.h"
#include "header/file_system.h"

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
        else if (!cmp(parse,"exit")){
            print("Only Use My Commands.. lol...\n");
        }
        print("\n");
    } while(!cmp(parse,"exit"));
    print("Out of the System");
}