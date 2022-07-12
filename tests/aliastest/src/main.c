#include <aliastest.h>

void map_kernel_page() {

}

int main() {
    printf("Alias test shall pass when difftest gives no error\n");
    do_alias_init();
    return 0;
}
