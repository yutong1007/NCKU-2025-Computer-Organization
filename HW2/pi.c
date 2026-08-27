"addi t1, x0, 0\n\t"
"addi t2, x0, 1\n\t"
"addi t3, x0, 1\n\t" 
"addi t4, x0, -1\n\t" 
"addi %[add_cnt], %[add_cnt], 4\n\t"

"fcvt.s.w f1, t1\n\t"
"fadd.s %[pi], f1, f1\n\t"
"addi %[fadd_cnt], %[fadd_cnt], 1\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"

"Pi:\n\t"
"bge t1, %[N], end_Pi\n\t"
"fcvt.s.w f1, t2\n\t"
"fcvt.s.w f2, t3\n\t"
"addi %[others_cnt], %[others_cnt], 3\n\t"

"fdiv.s f3, f1, f2\n\t"
"addi %[fdiv_cnt], %[fdiv_cnt], 1\n\t"
"fadd.s %[pi], %[pi], f3\n\t"
"addi %[fadd_cnt], %[fadd_cnt], 1\n\t"

"mul t2, t2, t4\n\t"
"addi %[mul_cnt], %[mul_cnt], 1\n\t"

"addi t3, t3, 2\n\t"
"addi t1, t1, 1\n\t"
"addi %[add_cnt], %[add_cnt], 2\n\t"

"addi %[others_cnt], %[others_cnt], 1\n\t"
"jal x0, Pi\n\t"

"end_Pi:\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"