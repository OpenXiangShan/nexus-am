#include "trap.h"
#include "stdio.h"

void swap(volatile int arr[], int i, int j) {
	int temp = arr[i];
	arr[i] = arr[j];
	arr[j] = temp;
}
// low-high in increasing order
void quickSort3Ways(volatile int arr[], int lowIndex, int highIndex) {
	if(lowIndex >= highIndex) return;
	int i = lowIndex + 1;
	int pivot_value = arr[lowIndex];
	int lt = lowIndex;
	int gt = highIndex;
	while(i <= gt) {
		if(arr[i] < pivot_value) {
			swap(arr, i++, lt++);
		}
		else if(arr[i] > pivot_value) {
			swap(arr, i, gt--);
		}
		else {
			i ++;
		}
	}
	quickSort3Ways(arr, lowIndex, lt - 1);
	quickSort3Ways(arr, gt + 1, highIndex);
}

#define VMEM ((char *)0xc0000000)

int main() {
    char* vga = VMEM + 320 + 80;
	volatile int arr[100];
	int j;
	put_hex((int )&arr[0], vga);
	vga += 160;
	for(j = 0; j < 100; j += 1) {
		arr[j] = 200 - j;  
	}
	quickSort3Ways(arr, 0, 99);

	for(j = 0; j < 50; j ++) {
        put_hex(arr[j], vga);
        vga += 160;
	}
    vga += 210;
	for(;j < 100; j ++) {
        put_hex(arr[j], vga);
        vga += 160;
	}
    good();
    return good();
}
