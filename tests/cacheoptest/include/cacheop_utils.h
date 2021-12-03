clear_cop_csrs:
    csrw cop_finish, zero
    csrw cop_level, zero
    csrw cop_way, zero
    csrw cop_index, zero
    csrw cop_bank, zero
    csrw cop_tag_ecc, zero
    csrw cop_tag_bits, zero
    csrw cop_tag_data, zero
    csrw cop_tag_data_h, zero
    csrw cop_ecc_width, zero
    csrw cop_data_ecc, zero
    csrw cop_data_0, zero
    csrw cop_data_1, zero
    csrw cop_data_2, zero
    csrw cop_data_3, zero
    csrw cop_data_4, zero
    csrw cop_data_5, zero
    csrw cop_data_6, zero
    csrw cop_data_7, zero
    ret

wait_until_cop_finish_or_timeout:
    li t1, 0x0
    li t0, 0x100
keep_waiting:
    addi t1, t1, 1
    beq t0, t1, end_waiting
    csrr t0, cop_finish
    beq t0, zero, keep_waiting
end_waiting:
    ret

wait_until_cop_finish:
    csrr t0, cop_finish
    beq t0, zero, wait_until_cop_finish
    ret