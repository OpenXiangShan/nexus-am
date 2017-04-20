#ifndef __NPC_DEBUG_H__
#define __NPC_DEBUG_H__


union debug_que_entry {
  unsigned int part[4];
  struct {
    unsigned int data_to_mig_low_word;
    unsigned int data_from_mig_low_word;
    unsigned int addr_to_mig;

    unsigned int cache_status     : 3;
    unsigned int cache_counter    : 3;
    unsigned int mid_rdy          : 1;
    unsigned int mig_wdf_dry      : 1;
    unsigned int mig_ddr_inited   : 1;
    unsigned int mig_data_end     : 1;
    unsigned int mig_data_valid   : 1;
    unsigned int ddr_ctrl_reading : 1;
    unsigned int ddr_ctrl_writing : 1;
    unsigned int miss_count       : 7;
    unsigned int pad              : 12;
  };
};


#endif //__NPC_DEBUG_H__
