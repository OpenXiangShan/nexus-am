#include<am.h>
#include<klib.h>
void assert (int expression){
	if(!expression)_halt(0);
}
