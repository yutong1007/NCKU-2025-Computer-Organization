"addi t0, x0, 4\n\t"
"addi %[add_cnt], %[add_cnt], 1\n\t"

"for:\n\t"
"beq %[arr_size], x0, for_end\n\t"
"vsetvli t0, t0, e32\n\t"
"vle32.v v0, (%[h])\n\t"
"vle32.v v1, (%[x])\n\t"
"addi %[others_cnt], %[others_cnt], 2\n\t"
"addi %[lw_cnt], %[lw_cnt], 2\n\t"

"vfmul.vv v2, v0, v1\n\t"
"vfadd.vf v2, v2, %[id]\n\t"
"vse32.v v2, (%[y])\n\t"
"addi %[fmul_cnt], %[fmul_cnt], 1\n\t"
"addi %[fadd_cnt], %[fadd_cnt], 1\n\t"
"addi %[sw_cnt], %[sw_cnt], 1\n\t"

"addi %[h], %[h], 16\n\t"
"addi %[x], %[x], 16\n\t"
"addi %[y], %[y], 16\n\t"
"addi %[arr_size], %[arr_size], -4\n\t"
"addi %[add_cnt], %[add_cnt], 4\n\t"

"addi %[others_cnt], %[others_cnt], 1\n\t"
"jal x0, for\n\t"

"for_end:"
"addi %[others_cnt], %[others_cnt], 1\n\t"

