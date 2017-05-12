#include "nemu.h"
#include <elf.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
extern int get_sym(char *var);
enum {
	NOTYPE = 256, ADD, SUB, MUL, DEV, L_P, R_P, EQ, NUM, HEX, DER, AND, OR, NOTEQ, NOT, REG, VAR,
	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// spaces
	{"\\+", ADD},					// plus
	{"\\-", SUB},
	{"\\*", MUL},
	{"\\/", DEV},
	{"\\(", L_P},
	{"\\)", R_P},
	{"==", EQ},
	{"&&", AND},
	{"\\|\\|", OR},
	{"!", NOT},
	{"!=", NOTEQ},
	{"[a-zA-Z_][a-zA-Z_0-9]*", VAR},
	{"\\$e[a|b|c|d|b|s|i][x|p|i]", REG},
	{"0[x|X][a-fA-F0-9]+",HEX},
	{"[0-9]+",NUM},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;
	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				//Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rule					 s[i]. Add codes
				 * to record the token in the array ``tokens''. 				   For certain 
				 * types of tokens, some extra actions should be 				   performed.
				 */
				
				switch(rules[i].token_type) {
					case NOTYPE:tokens[nr_token].type = NOTYPE;
						 break;
					case ADD:tokens[nr_token].type = ADD;
						 nr_token++;break;
					case SUB:tokens[nr_token].type = SUB;
						 nr_token++;break;
					case MUL:tokens[nr_token].type = MUL;
						 nr_token++;break;
					case DEV:tokens[nr_token].type = DEV;
						 nr_token++;break;
					case L_P:tokens[nr_token].type = L_P;
						 nr_token++;break;
					case R_P:tokens[nr_token].type = R_P;
						 nr_token++;break;
					case EQ:tokens[nr_token].type = EQ;
						 nr_token++;break;
					case AND:tokens[nr_token].type = AND;
						 nr_token++;break;
					case OR:tokens[nr_token].type = OR;
						 nr_token++;break;
					case NOT:tokens[nr_token].type = NOT;
						 nr_token++;break;
					case NOTEQ:tokens[nr_token].type = NOTEQ;
						 nr_token++;break;
					case VAR:tokens[nr_token].type = VAR;
						 strncpy(tokens[nr_token].str,substr_start,substr_len); 
						 nr_token++;break;
					case REG:tokens[nr_token].type = REG;
						 sscanf(substr_start,"%4s",tokens[nr_token].str);
						 nr_token++;break;
					case HEX:tokens[nr_token].type = HEX;
						 strcpy(tokens[nr_token].str,substr_start); 
						 nr_token++;break;
					case NUM:tokens[nr_token].type = NUM;
						 strcpy(tokens[nr_token].str,substr_start);
						 nr_token++;break;					                default: panic("please implement me");
			}

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

uint32_t eval(int p, int q);
bool check_parenthese(int p, int q);

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
	int i = 0;
	for(;i<nr_token;i++)
	if(tokens[i].type == MUL && (i == 0 || (tokens[i-1].type !=NUM && tokens[i-1].type != HEX)))
		{
			tokens[i].type = DER;

		}
	double result= eval(0, nr_token-1);
	return result;
}

uint32_t eval(int p, int q)
{
	if(p > q)
	{
		printf("This is a bad expression,please check.\n");
		return -1;
	}
	if(tokens[p].type == DER && (((tokens[q].type == HEX || tokens[q].type == VAR)&& p+1 == q)||check_parenthese(p+1,q) == true))
	{
		if(check_parenthese(p+1,q) == true)
		{
			//printf("%d\n",eval(p+1,q));			
			return swaddr_read(eval(p+1,q),4,R_DS);
		}
		else
		{
			int addr = 0;
			if(tokens[q].type != VAR)
				sscanf(tokens[q].str,"%x",&addr);
			else
				addr = get_sym(tokens[q].str);
			return swaddr_read(addr,4, R_DS);
		}
	}
	if(tokens[p].type == NOT &&(((tokens[q].type == NUM ||tokens[q].type == HEX) && p+1 == q)||check_parenthese(p+1,q) == true))
	{
		if(check_parenthese(p+1,q) == true)
			return !eval(p+1 , q);
		else
		{
			double result = 0;
			sscanf(tokens[q].str,"%lf",&result);
			return !result;
		}
	}
	
			
	if(p == q)
	{
		if(tokens[p].type == VAR)
		{
			int addr = get_sym(tokens[p].str);
			return addr;
		}
		if(tokens[p-1].type == DER&& p!=0)
		{
			int addr = 0;
			sscanf(tokens[p].str,"%x",&addr);
			return swaddr_read(addr,4,R_DS);
		}
		else if(tokens[p].type == REG)
		{
			if(strcmp(tokens[p].str,"$eax")==0)
				return cpu.eax;
			if(strcmp(tokens[p].str,"$ebx")==0)
				return cpu.ebx;
			if(strcmp(tokens[p].str,"$ecx")==0)
				return cpu.ecx;
			if(strcmp(tokens[p].str,"$edx")==0)
				return cpu.edx;
			if(strcmp(tokens[p].str,"$esp")==0)
				return cpu.esp;
			if(strcmp(tokens[p].str,"$ebp")==0)
				return cpu.ebp;
			if(strcmp(tokens[p].str,"$esi")==0)
				return cpu.esi;
			if(strcmp(tokens[p].str,"$edi")==0)
				return cpu.edi;
			if(strcmp(tokens[p].str,"$eip")==0)
				return cpu.eip;
			else
			{
				printf("Don't have this register!\n");
				return -1;
			}
		}
		else
		{
			double result = 0;
			sscanf(tokens[p].str,"%lf",&result);
			return result;
		}
	}
	if(check_parenthese(p,q))
	{
		return eval(p+1, q-1);
	}
	else
	{
		int op = 0;
		int power = 0;
		int i;
		for(i = p;i <= q; i++)
		{
			if(tokens[i].type != NUM&&tokens[i].type != HEX&&tokens[i].type != REG)
			{
				if(tokens[i].type == EQ)
				{	
					op = i;
					power+=2;
				}
				if(tokens[i].type == AND)
				{
					op = i;
					power+=2;
				}
				if(tokens[i].type == OR)
				{
					op = i;
					power+=2;
				}
				if(tokens[i].type == NOTEQ)
				{
					op = i;
					power+=2;
				}
				if(tokens[i].type == ADD && power<2)
				{	
					op = i;
					power++;
				}
				if(tokens[i].type == SUB && power<2)
				{	
					op = i;
					power++;
				}
				if(tokens[i].type == MUL && power<1)
					op = i;
				if(tokens[i].type == DEV && power<1)
					op = i;
				if(tokens[i].type == L_P)
				{
					for(;tokens[i].type != R_P;i++)
					{
						if(i > q)
						{
							printf("This is a bad expression!\n");
							return -1;
						}
					}
				}		
			}
		}
		uint32_t val1 = eval(p,op-1);
		uint32_t val2 = eval(op+1,q);
		switch(tokens[op].type)
		{
		case ADD:return val1+val2;
		case SUB:return val1-val2;
		case MUL:return val1*val2;
		case DEV:return val1/val2;
		case EQ:return val1 == val2;
		case AND:return val1 && val2;
		case OR:return val1 || val2;
		case NOTEQ:return val1 != val2;
		default:assert(0);
		}
	}
}

bool check_parenthese(int p, int q)
{
	if(tokens[p].type == L_P && tokens[q].type == R_P)
		return true;
	else
		return false;
}

