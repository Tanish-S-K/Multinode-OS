#include "header/mystdlib.h"
#include "header/file_system.h"
#include "header/auth.h"
#include "header/cli.h"

void kernel_main() {
    clear_screen();
    init_file_system();
    
    int ans;
    load_bitmap();
    do {
        ans = authenticate();
        if (ans!=0){

            break;
        } else{
            print("\nWrong!!\n");
        }
    } while (1);

    print("Entered the kernal-tan\n");
    cli();

    while (1) {}
}
