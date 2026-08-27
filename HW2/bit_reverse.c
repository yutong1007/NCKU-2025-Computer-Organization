"add t0, x0, x0\n\t" 
"add t1, %[m], x0\n\t" 
"addi %[add_cnt], %[add_cnt], 2\n\t"

"reverse:\n\t"
"beq t1, x0, reverse_end\n\t"
"slli t0, t0, 1\n\t"
"andi t2, %[b], 1\n\t"         
"or   t0, t0, t2\n\t"        
"srli %[b], %[b], 1\n\t"         
"addi %[others_cnt], %[others_cnt], 5\n\t"

"addi t1, t1, -1\n\t"
"addi %[add_cnt], %[add_cnt], 1\n\t"

"addi %[others_cnt], %[others_cnt], 1\n\t"
"jal x0, reverse\n\t"

"reverse_end:\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"add %[b], t0, x0\n\t"
"addi %[add_cnt], %[add_cnt], 1\n\t"