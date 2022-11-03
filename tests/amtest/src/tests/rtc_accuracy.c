#include <amtest.h>
#include <nemu.h>

void rtc_accuracy_test() {
	printf("RTC test\n");
	// read rtc, get start
	volatile uint64_t start = *(uint64_t *)(RTC_ADDR);
	// do some work
	volatile uint64_t a = 0;
	for (int i = 0; i < 20000; i++) {
		a += 1;
	}
	// read rtc, get end
	volatile uint64_t end = *(uint64_t *)(RTC_ADDR);
	printf("RTC time %llu\n", end - start);
	if (end - start > 10000) printf("RTC tick seems to be too fast!\n");
}
