#include <cpu.h>
#include <string.h>

#define DRAM_SIZE (1 << 20)

CPU cpu;
instruction current;
byte dram[DRAM_SIZE];
extern char *One_Byte_Op_Map[16][16];
void ramdisk_read(dword addr, dword len, byte *data);
void ramdisk_write(dword addr, dword len, byte *data);

void init_cpu(){
  cpu.eax.val = 0;
  cpu.ebx.val = 0;
  cpu.ecx.val = 0;
  cpu.edx.val = 0;
  cpu.edi.val = 0;
  cpu.esi.val = 0;
  cpu.esp.val = 0;
  cpu.eip = 0xfff0;
  cpu.eflags.CF = 0;
  cpu.eflags.PF = 0;
  cpu.eflags.AF = 0;
  cpu.eflags.ZF = 0;
  cpu.eflags.SF = 0;
  cpu.eflags.TF = 0;
  cpu.eflags.IF = 0;
  cpu.eflags.DF = 0;
  cpu.eflags.OF = 0;
  cpu.eflags.IO_PL = 0;
  cpu.eflags.NT = 0;
  cpu.eflags.RF = 0;
  cpu.eflags.VM = 0;
  cpu.cr0.PE = 0;
  cpu.cr0.MP = 0;
  cpu.cr0.EM = 0;
  cpu.cr0.TS = 0;
  cpu.cr0.ET = 0;
  cpu.cr0.PG = 0;
  cpu.cs.RPL = 0;
  cpu.cs.TI = 0;
  cpu.cs.INDEX = 0;
  cpu.ds.RPL = 0;
  cpu.ds.TI = 0;
  cpu.ds.INDEX = 0;
  cpu.es.RPL = 0;
  cpu.es.TI = 0;
  cpu.es.INDEX = 0;
  cpu.ss.RPL = 0;
  cpu.ss.TI = 0;
  cpu.ss.INDEX = 0;
  cpu.idtr.LIMIT = 0x3ff;
  cpu.idtr.BASE = 0;
  cpu.gdtr.LIMIT = 0;
  cpu.gdtr.BASE = 0;
}

dword instr_fetch(dword eip, instruction *i){
  byte data;
  ramdisk_read(eip,1,(void *)&data);
  char op[20];
  strcpy(op,One_Byte_Op_Map[(data >> 4) & 0xf][data & 0xf]);
  char instr_pattern[3][20];
  int j;
  int n = 0;
  for(j = 0; op[j] != '\0'; j++){
    if(op[j] == '_'){
      instr_pattern[n][j] = '\0';
      n++;
    }
    else
      instr_pattern[n][j] = op[j];
  }
  return 4;
}

void instr_decode(instruction *i){
  
}

void instr_exec(instruction *i){
}

void init_dram(){	//init dram to be zero
  dword start = 0;
  dword end = DRAM_SIZE - 1;
  dword i;
  for(i = start; i < end; i++){
    dram[i] = 0;
  }
}

void init_monitor(){
//for debug
}

byte dram_read(dword addr){ //once read a byte
  return dram[addr];
}

void dram_write(dword addr, byte data){  //once write a byte
  dram[addr] = data;
}

void ramdisk_read(dword addr, dword len, byte *data){	//read addr for length
  int i;
  for(i = 0; i < len; i++){
    data[i] = dram_read(addr + i);
  }
}

void ramdisk_write(dword addr, dword len, byte *data){	//write addr for length
  int i;
  for(i = 0; i < len; i++){
    dram_write(addr + i, data[i]);
  }
}

dword load_elf(){	//should have program to load
  return 0;
}

void system_start(dword entry){
  while(1){	//need to break
    dword instr_len = instr_fetch(entry, (void *)&current);
    instr_decode(&current);
    instr_exec(&current);
    entry += instr_len;
  };
}

void init_kernel(){
  dword entry = load_elf();
  system_start(entry);
}

int main(){
  init_cpu();
  init_dram();
  init_monitor();
  init_kernel();
  return 0;
}
