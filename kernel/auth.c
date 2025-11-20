#include <stdint.h>
#include "header/mystdlib.h"
#include "header/file_system.h"

#define user_start 911
#define user_end 1000

typedef struct {
	char username[100];
	char password[100];
} crendentials;

void create_user(){
	char buffer[512];
	crendentials *node = (crendentials *)mem(sizeof(crendentials));
	cur_sec = entry_start;
	for(int i=user_start;i<user_end;i++){
    	if(!check_bit(i)) {
    		print("Set Username: ");
			input(node->username);
			print("\nSet Password: ");
			input(node->password);
			print("\n");
			mem_cpy(buffer,node,sizeof(crendentials));
			write_sector(i,(uint16_t *)buffer);
			set_bit(i,1);
			int sec = create_dir(node->username,0);
			godir(node->username,0);
			set_user(sec);
			return;
    	}
    }
    print("Too many users exist!!!\n");
	
}
int authenticate(){
	
	if(!check_bit(911)){
		create_user();
		return 911;
	}
	char buffer[512];
	crendentials *node = (crendentials *)mem(sizeof(crendentials));
	char username[100],password[100];

	print("\nEnter Username: ");
	input(username);
	print("\nEnter Password: ");
	input(password);
	print("\n");

    for(int i=user_start;i<user_end;i++){
    	read_sector(i,(uint16_t *)buffer);
    	mem_cpy(node,buffer,sizeof(crendentials));
    	
    	if (cmp(node->username,username) && cmp(node->password,password)){
    		int sec = find_dir(node->username);
    		godir(node->username,0);
    		set_user(sec);
    		return i;
    	}
    }
    return 0;
}
