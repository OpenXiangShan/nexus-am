#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#define MY_DEBUG
WP* new_w;
void cpu_exec(uint32_t);

/* We use the ``readline'' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_si(char *args) {
	char *arg = strtok(NULL, " ");
	int i = 0;
	if(arg == NULL)
		cpu_exec(1);
	else
	{
		if(sscanf(arg, "%d" ,&i))
		cpu_exec(i);
	}
#ifdef MY_DEBUG
		printf("eax = 0x%x\n",cpu.eax);
		printf("edx = 0x%x\n",cpu.edx);
		printf("ecx = 0x%x\n",cpu.ecx);
		printf("ebx = 0x%x\n",cpu.ebx);
		printf("ebp = 0x%x\n",cpu.ebp);
		printf("esi = 0x%x\n",cpu.esi);
		printf("edi = 0x%x\n",cpu.edi);
		printf("esp = 0x%x\n",cpu.esp);
		printf("[eip] = 0x%x\n",cpu.eip);
		printf("CF = %d\n",cpu.EFLAGS.CF);
		printf("ZF = %d\n",cpu.EFLAGS.ZF);
		printf("OF = %d\n",cpu.EFLAGS.OF);
		printf("SF = %d\n",cpu.EFLAGS.SF);
		printf("CS = %x\n",cpu.CS.val);
		printf("DS = %x\n",cpu.DS.val);
		printf("SS = %x\n",cpu.SS.val);
		printf("ES = %x\n",cpu.ES.val);
#endif
	return 0;
}
static int cmd_info(char *args){
	char *arg = strtok(NULL, " ");
	if(strcmp(arg,"r") == 0)
	{
		printf("eax = 0x%x\n",cpu.eax);
		printf("edx = 0x%x\n",cpu.edx);
		printf("ecx = 0x%x\n",cpu.ecx);
		printf("ebx = 0x%x\n",cpu.ebx);
		printf("ebp = 0x%x\n",cpu.ebp);
		printf("esi = 0x%x\n",cpu.esi);
		printf("edi = 0x%x\n",cpu.edi);
		printf("esp = 0x%x\n",cpu.esp);
		/*printf("ax = 0x%x\n",cpu.gpr[0]._16);
		printf("dx = 0x%x\n",cpu.gpr[1]._16);
		printf("cx = 0x%x\n",cpu.gpr[2]._16);
		printf("bx = 0x%x\n",cpu.gpr[3]._16);
		printf("bp = 0x%x\n",cpu.gpr[4]._16);
		printf("si = 0x%x\n",cpu.gpr[5]._16);
		printf("di = 0x%x\n",cpu.gpr[6]._16);
		printf("sp = 0x%x\n",cpu.gpr[7]._16);
		printf("al = 0x%x\n",cpu.gpr[0]._8[0]);
		printf("dl = 0x%x\n",cpu.gpr[1]._8[0]);
		printf("cl = 0x%x\n",cpu.gpr[2]._8[0]);
		printf("bl = 0x%x\n",cpu.gpr[3]._8[0]);
		printf("ah = 0x%x\n",cpu.gpr[0]._8[1]);
		printf("dh = 0x%x\n",cpu.gpr[1]._8[1]);
		printf("ch = 0x%x\n",cpu.gpr[2]._8[1]);
		printf("bh = 0x%x\n",cpu.gpr[3]._8[1]);*/
		//printf("CF = %d\n",cpu.EFLAGS.CF);
		//printf("ZF = %d\n",cpu.EFLAGS.ZF);
		//printf("OF = %d\n",cpu.EFLAGS.OF);
		//printf("SF = %d\n",cpu.EFLAGS.SF);
		printf("CS = %x\n",cpu.CS.val);
		printf("DS = %x\n",cpu.DS.val);
		printf("SS = %x\n",cpu.SS.val);
		printf("ES = %x\n",cpu.ES.val);
	}
	else if(strcmp(arg,"w")==0){
		printf("The assignment of the varible is %x\n",new_w->result);
}
	else
	{
		assert(0);
	}
	return 0;
}

static int cmd_x(char *args)
{
	char *n = strtok(NULL, " ");
	int i = 0;
	char *EXPR = strtok(NULL, " ");
	uint32_t addr = 0;
	uint32_t addr_10 = 0;
	sscanf(EXPR, "%*c%*c%x", &addr);
	sscanf(EXPR, "%*c%*c%d", &addr_10);
	for(;i < atoi(n);i++ )
	{
		int j = 0;
		printf("%d\t", addr_10);
		for(;j < 5;j++ )
		{
			printf("0x%02x  " ,swaddr_read(addr,1,R_DS));
			addr++;
			addr_10++;
		}
		printf("\n");
	}
	printf("\n");
	return 0;
}

static int cmd_p(char *args)
{
	bool *flag = false;
	uint32_t result = expr(args, flag);
	printf("The answer of the expression is %d.\n",result);
	return 0;
}

static int cmd_w(char *args)
{
	new_w = new_wp();
	strcpy(new_w->expr, args);
	bool *flag = false;
	new_w->result = expr(new_w->expr, flag);
	return 0;
}

static int cmd_d(char *args)
{
	WP *current = new_w;
	for(;current!= NULL;current = current->next)
	{
		if(strcmp(current->expr, args) == 0)
		{	
			free_wp(current);
			break;
		}
	}
	return 0;
}

static int cmd_bt(char *args)
{
	swaddr_t i;
	for(i = cpu.ebp; i != 0; i = swaddr_read(i,4,R_SS))
	{
		get_funcname(swaddr_read(i+4,4,R_SS)+1);
		int j;
		for(j = 0; j < 4; j++)
			printf("args[%d]:%x\n",j,swaddr_read(i+4*(j+2),4,R_SS));
	}
	return 0;
}

static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "Single-step implement", cmd_si },
	{ "info", "Get the information of program", cmd_info},
	{ "x", "Scan momery", cmd_x},
	{ "p", "Get the answer to the expression", cmd_p},
	{ "w", "Set a watchpoint", cmd_w},
	{ "d", "Delete a watchpoint", cmd_d},
	{ "bt", "Print stack", cmd_bt},
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
