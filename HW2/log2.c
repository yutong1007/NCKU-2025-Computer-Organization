"addi t0, %[N], 0\n\t"
"addi t1, %[log], 0\n\t"
"addi %[add_cnt], %[add_cnt], 2\n\t"

"log:\n\t"
"srli t0, t0, 1\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"beq t0, x0, log_end\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"

"addi t1, t1, 1\n\t"
"addi %[add_cnt], %[add_cnt], 1\n\t"

"addi %[others_cnt], %[others_cnt], 1\n\t"
"jal x0, log\n\t"

"log_end:\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"addi %[log], t1, 0\n\t"
"addi %[add_cnt], %[add_cnt], 1\n\t"