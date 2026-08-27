---
title: 計組 HW2

---

# 計組 HW2
C64111174 王宇桐

## Performance Modeling
要紀錄 instruction counts，已經定義好計數器去紀錄不同類型的指令次數（add_cnt、lw_cnt...），每種指令類型也有固定的CPI，已經在macro_define.h 定義好了，用 instruction count 和 CPI 算出 total cycles。

## Performance Characterization
* CPU Bound：程式的效能取決於 CPU 計算能力
* Memory Bound：程式的效能受限於 Memory 存取速度

assignment：用 instruction counts 算出 total cycles，根據公式：(clock cycles for the instructions other than load/store instructions)/(clock cycles for all the instructions) 算出 ratio，ratio > 0.5 是 CPU Bound，<0.5 是 Memory Bound。

## Performance Data Collection
所有計數器和變數都已經宣告好了，只需要在 assembly code 裡面執行每個 instruction 時， addi 該指令的 counter。

```
addi %[lw_cnt], %[lw_cnt], 1
```

```
Total cycles = add_cnt * add_CPI
             + sub_cnt * sub_CPI
             + mul_cnt * mul_CPI
             + div_cnt * div_CPI
             + lw_cnt * lw_CPI
             + sw_cnt * sw_CPI
             + others_cnt * others_CPI
             + (浮點的指令也要加)
```

```
CPU Time = Total cycles × cycle time

```

## Exercise 1 Fast Fourier Transform Calculation
實作傅立葉轉換，把 6 個 function：complex_add, complex_sub, complex_mul, log2, bit_reverse, pi 用 assembly code 完成，每行 instruction 都要 record 次數，最後在 macro_define.h 定義計算公式。

傅立葉轉換：將連續序號拆解成 sin 和 cos 的加總

Cooley - Tukey Fast Fourier Transform Algorithm：
1. 用 Bit-Reversal Reordering 重新排列 data，將原本 data 的 index 轉成二進位制後，再將 bit pattern 反轉，新的 bit pattern 再轉回十進制得到該筆 data 的新的 index。
2. 用 Butterfly Operation 將 data 兩兩一組，分別相加得到上層、相減乘 Twiddle Factor 𝑊 得到下層
3. Twiddle Factor：W = e^(−j2πk/N)), while e^(-j*θ) = cos(θ) - (j * sin(θ))。

### complex structure
```c
typedef struct {
    float Re;
    float Im;
} Complex;
```
第一個 element 是實數，第二個 element 是虛數，中間差 8 個 byte

### complex add
實數 + 實數，虛數 + 虛數
```c
Complex complex_add(Complex a, Complex b)
{
    Complex result;
    //result.Re = a.Re + b.Re;
    //result.Im = a.Im + b.Im;
    asm volatile(
        #include "complex_add.c"
        :[C_Re] "=f"(result.Re), [C_Im] "=f"(result.Im), [fadd_cnt] "+r"(fadd_cnt)
        :[A_Re] "f"(a.Re), [B_Re] "f"(b.Re), [A_Im] "f"(a.Im), [B_Im] "f"(b.Im));
    return result;
}

```
不用用到 register，直接用 c code 裡面有的 a.Re, b.Re, a.Im, b.Im 去相加。

### complex sub
實數 - 實數，虛數 - 虛數
```c
Complex complex_sub(Complex a, Complex b)
{
    Complex result;
    //result.Re = a.Re - b.Re;
    //result.Im = a.Im - b.Im;
    asm volatile(
        #include "complex_sub.c"
        :[C_Re] "=f"(result.Re), [C_Im] "=f"(result.Im), [fsub_cnt] "+r"(fsub_cnt)
        :[A_Re] "f"(a.Re), [B_Re] "f"(b.Re), [A_Im] "f"(a.Im), [B_Im] "f"(b.Im));
    return result;
}
```
和 add 做一樣的事，只是加法改成減法

### complex mul
(a.Re+a.Im)×(b.Re+b.Im)=(a.Re * b.Re − a.Im * b.Im)+(a.Re * b.Im + a.Im * b.Re)i
```c
Complex complex_mul(Complex a, Complex b)
{
    Complex result;
    asm volatile(
        #include "complex_mul.c"
        : [C_Re] "=f"(result.Re), [C_Im] "=f"(result.Im), [fmul_cnt] "+r"(fmul_cnt), [fsub_cnt] "+r"(fsub_cnt), [fadd_cnt] "+r"(fadd_cnt)
        : [A_Re] "f"(a.Re), [B_Re] "f"(b.Re),[A_Im] "f"(a.Im), [B_Im] "f"(b.Im)
        : "f1", "f2", "f3", "f4"
    );
    return result;
}
```
用 4 個暫存器分別存 a.Re * b.Re, a.Im * b.Im, a.Re * b.Im,  a.Im * b.Re，用 fadd.s 和 fsub.s 寫入 result 的 Re 和 Im。

1. f1 = a.Re * b.Re
2. f2 = a.Im * b.Im
3. f3 = a.Re * b.Im
4. f4 = a.Im * b.Re

### log2
取 log2 整數，先初始化 log = 0，當 N > 1 時一直右移一位（除以2），然後 log++。
```c
uint32_t Log2(uint32_t N)
{
    uint32_t log = 0;
    while (N > 1) {
        N = N >> 1;
        log++;       
    }
    asm volatile(
        #include "log2.c"
        : [log] "+r"(log), [N] "+r"(N), [add_cnt] "+r"(add_cnt), [others_cnt] "+r"(others_cnt)
        :
        : "t0", "t1"
    );
    return log;
}
```
用 2 個暫存器分別存 log counter 跟現在 N 的值，持續 while 迴圈count log。
```c
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
```
* 用 beqz 看 N 是否 = 0 ，如果 = 0 的話迴圈結束跳到 end label
* 用 srli 將 N 右移一個 bit 做除以 2
* 迴圈結束後將 t1 存的 log 值存回變數

### Pi
用一個 while 迴圈，每次都做 pi + sign 1/(1, 3, 5, 7...)，每次 sign 都要變號，當 count 大於 iter（已經定義）時結束迴圈
```c
float PI(void)
{
    /* description: Gregory-Leibniz series
    * 
    *    we use Leibniz formula to approximate Pi
    * 
    *    pi/4 = (1 - 1/3 + 1/5 - 1/7 + 1/9 - ...)
    *    pi   = 4(1 - 1/3 + 1/5 - 1/7 + 1/9 - ...)
    * 
    */
    float pi = 0;
    int sign = 1;
    int denom = 1;
    int count = 0;
    
    while (count < iter) {
        pi += (float)sign / denom;
        sign = -sign;     
        denom += 2;         
        count++;             
    }
    asm volatile(
        #include "pi.c"
        : [add_cnt] "+r"(add_cnt), [fadd_cnt] "+r"(fadd_cnt), [sub_cnt] "+r"(sub_cnt), [fsub_cnt] "+r"(fsub_cnt), [mul_cnt] "+r"(mul_cnt), [div_cnt] "+r"(div_cnt), [fdiv_cnt] "+r"(fdiv_cnt), [lw_cnt] "+r"(lw_cnt), [sw_cnt] "+r"(sw_cnt), [others_cnt] "+r"(others_cnt), [pi] "+f"(pi)
        : [N] "r"(iter)
        : "f1", "f2", "t1", "t2", "t3", "t4"
    );
    pi = 4 * pi;
    return pi;
}
```
```c
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
```
* t1 = counter
* t2 = sign
* t3 = denom
* t4 = -1（改變 sign 正負號） 
* 每次迴圈先判斷 counter 是否 < N，如果 >= 的話跳到 end label
* 用 f1, f2 存 fcvt.s.w 將 sign 和 denom 轉成的 float，用 f3 存相除的結果
* 和原本的 pi 相加後寫回去 pi 變數
* 最後更新 sign, denom, counter 的值

### bit reverse
將 b 的前 m 個 bit 反轉，用一個新的變數 result 存結果，for 迴圈做 m 次，每次都將這個變數左移一位，然後將 b 的最低位取出放到 result，再將 b 右移一位改變最低位。
```c
uint32_t bit_reverse(uint32_t b, uint32_t m)
{
    uint32_t result = 0;
    for (uint32_t i = 0; i < m; i++) {
        result <<= 1;
        result |= (b & 1); 
        b >>= 1;           
    }
    b = result;            
    return b;
    asm volatile (
        #include "bit_reverse.c"
        : [b] "+r"(b), [lw_cnt] "+r"(lw_cnt), [add_cnt] "+r"(add_cnt), [sub_cnt] "+r"(sub_cnt), [sw_cnt] "+r"(sw_cnt), [others_cnt] "+r"(others_cnt)
        : [m] "r"(m)
        : "t0", "t1", "t2"
    );
    return b;
}
```
```c
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
```
* t0 = result 
* t1 = m
* t2 = b 的最低位
* 用 m 當作迴圈 counter，直到 m = 0 時迴圈結束
* 每次迴圈先將 t1 左移一位（因為要放 b 的最低位進來），將 b 和 1 做 AND 取最低位（因為和 1 的 bit pattern 做 AND 的話除了最後一位以外都一定是 0，最後一位則是看原本的 b 該位是 0 or 1），取完最低位後用 or 放到 result(t1)的最後一位，最後把 b 再右移一位更新 last bit
* 迴圈結束後將 t1 重新存回變數 b


## Exercise 2.1 Array Multiplication without V Extension
實作以下 c code
```c
void arraymul_baseline(){
    float *p_h = h;
    float *p_x = x;
    float *p_y = y;
    float id = student_id;// id = your_student_id % 100;
    int arr_length = arr_size;
    /* original C code
    for (int i = 0; i < arr_size; i++){
	    p_y[i] = p_h[i] * p_x[i] + id;
    }
    */
    asm volatile(
        #include "arraymul_baseline.c"
    : [h] "+r"(p_h), [x] "+r"(p_x), [y] "+r"(p_y), [add_cnt] "+r"(add_cnt), [mul_cnt] "+r"(mul_cnt), [fadd_cnt] "+r"(fadd_cnt), [fmul_cnt] "+r"(fmul_cnt), [flw_cnt] "+r"(flw_cnt), [fsw_cnt] "+r"(fsw_cnt), [lw_cnt] "+r"(lw_cnt), [sw_cnt] "+r"(sw_cnt), [others_cnt] "+r"(others_cnt), [arr_size] "+r"(arr_length)
    : [id] "f"(id)
    : "f0", "f1"
    );
```
因為只能用 f0, f1 兩個 register，所以直接使用 arr_length(arr_size) 當作迴圈的 counter，每次迴圈都將兩個 array 的值 load 到 f0 和 f1 中，相乘後 + id（都存在 f0）再 store 回另外一個 array，每次迴圈後將 array pointer 加上 offset。

* f0 = p_h[i] 以及之後相乘 + id 的結果
* f1 = p_x[i]

```c
"for:\n\t"
"beq %[arr_size], x0, for_end\n\t"
"flw f0, 0(%[h])\n\t"
"flw f1, 0(%[x])\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"addi %[flw_cnt], %[flw_cnt], 2\n\t"

"fmul.s f0, f0, f1\n\t"
"fadd.s f0, f0, %[id]\n\t"
"addi %[fmul_cnt], %[fmul_cnt], 1\n\t"
"addi %[fadd_cnt], %[fadd_cnt], 1\n\t"

"fsw f0, 0(%[y])\n\t"
"addi %[fsw_cnt], %[fsw_cnt], 1\n\t"

"addi %[h], %[h], 4\n\t"
"addi %[x], %[x], 4\n\t"
"addi %[y], %[y], 4\n\t"
"addi %[arr_size], %[arr_size], -1\n\t"
"addi %[add_cnt], %[add_cnt], 4\n\t"

"addi %[others_cnt], %[others_cnt], 1\n\t"
"jal x0, for\n\t"

"for_end:"
"addi %[others_cnt], %[others_cnt], 1\n\t"
```
* 題目沒有給能用 int register，所以直接用 arr_size 當作 for loop counter，每次迴圈後 arr_size - 1，當 = 0 時跳出迴圈
* 每次迴圈都 flw 兩個 array 中的 float 到 f0 和 f1 中，分別相乘再加上 id 後都存在 f0，最後再將 f0 存的值 fsw 回去新的 array
* 每次迴圈結束後要更新下一個 array 的位置，因為是操作 pointer，array 中每個元素都差 4 個 byte，所以 pointer 加上 offset(4) 到下一個位置。 


## Exercise 2.2 Array Multiplication with V Extension
和 2.1 一樣的題目，只是可以用 vector 來實作
```c
void improved_version(){
    float *p_h = h;
    float *p_x = x;
    float *p_y = y;
    float id = student_id;// id = your_student_id % 100;
    /* original C code
    for (int i = 0; i < arr_size; i++){
    	p_y[i] = p_h[i] * p_x[i] + id;
    }
    */
    int arr_length = arr_size;
    asm volatile(
        #include "arraymul_improved.c"
        : [h] "+r"(p_h), [x] "+r"(p_x), [y] "+r"(p_y), [add_cnt] "+r"(add_cnt), [sub_cnt] "+r"(sub_cnt), [mul_cnt] "+r"(mul_cnt), [div_cnt] "+r"(div_cnt), [lw_cnt] "+r"(lw_cnt), [sw_cnt] "+r"(sw_cnt), [fadd_cnt] "+r"(fadd_cnt), [fmul_cnt] "+r"(fmul_cnt), [others_cnt] "+r"(others_cnt), [arr_size] "+r"(arr_length)
        : [id] "f"(id)
        : "t0", "v0", "v1", "v2"
    );
```

### Vector
一次處理多筆 data 的方式，例如說當 vector 長度是 4 的話，可以一次 load 4 個 float 後相乘相加，儲存 4 個運算後的結果，可以提高效率。
此題可能用到指令：
* vle32.v：讀一堆 float
* vfmul.vv：讓整個 vector 兩兩相乘
* vfadd.vf：把整個 vector 加上 id（scalar）
* vse32.v：存回整堆 float

將 vector length 設為 4，因為已知 array elements 是 2 的倍數，一次處理 array 中的 4 個值相乘加上 id，迴圈結束後 arr_size要 - 4，pointer 也要加上 4 個 offset

* t0 = vector length(4)
* v0 = vector register 存 h array 的 4 個 float
* v1 = vector register 存 x array 的 4 個 float
* v2 = vector register 存 array 相乘加上 id 後的結果

```c
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
```
* vector 一次裝 4 個 float，一樣用 arr_size 當作 for loop 的 counter，每次迴圈都先定義 vector 大小，接著把兩個 array 的 4 個 float load 進 vector（雖然是 load 4 個數，但只需要算一次 lw_cnt
* 接著兩個 vector 相乘再加上 id，存入 v2 register(一樣 instruction 只需要算一次)
* 最後把 v2 的值 store 回去
* 因為一次做 4 個 float，所以移動 array pointer 的話要加上 4 個 offset，也就是 16 個 byte，arr_size 也要一次減 4

## Exercise 3.1 Single Floating-point Multiplication
單精度浮點數的乘法總積

每次將兩個 array 的 float 相乘進 single_floating_result 這個變數
```c
void arraymul_float(){
    float *p_h = h;
    float *p_x = x;
    int arr_length = arr_size;
    /* original C code
    for (int i = 0; i < arr_size; i++){
    	single_floating_result = single_floating_result * p_h[i] * p_x[i];
    }
    */
    asm volatile(
        #include "arraymul_float.c"
        : [h] "+r"(p_h), [x] "+r"(p_x), [result] "+f"(single_floating_result), [add_cnt] "+r"(add_cnt), [fadd_cnt] "+r"(fadd_cnt), [fmul_cnt] "+r"(fmul_cnt), [flw_cnt] "+r"(flw_cnt), [fsw_cnt] "+r"(fsw_cnt), [others_cnt] "+r"(others_cnt), [arr_size] "+r"(arr_length)
        : 
        : "t0", "f0", "f1", "f2", "f3"
    );
}
```
* t0 = 1 for single_floating_result
* f0 = single_floating_result
* f1 = p_h[i]
* f2 = p_x[i]
* f3 = p_h[i] * p_x[i]

```c
"addi t0, t0, 1\n\t"
"fcvt.s.w f0, t0\n\t"
"addi %[add_cnt], %[add_cnt], 1\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"

"for:\n\t"
"beq %[arr_size], x0, for_end\n\t"
"flw f1, 0(%[h])\n\t"
"flw f2, 0(%[x])\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"addi %[flw_cnt], %[flw_cnt], 2\n\t"

"fmul.s f3, f1, f2\n\t"
"fmul.s f0, f0, f3\n\t"
"addi %[fmul_cnt], %[fmul_cnt], 2\n\t"

"addi %[h], %[h], 4\n\t"
"addi %[x], %[x], 4\n\t"
"addi %[arr_size], %[arr_size], -1\n\t"
"addi %[add_cnt], %[add_cnt], 3\n\t"

"addi %[others_cnt], %[others_cnt], 1\n\t"
"jal x0, for\n\t"

"for_end:\n\t"
"addi t0, x0, 0\n\t"
"fcvt.s.w f1, t0\n\t"
"fadd.s %[result], f0, f1\n\t"
"addi %[others_cnt], %[others_cnt], 2\n\t"
"addi %[add_cnt], %[add_cnt], 1\n\t"
"addi %[fadd_cnt], %[fadd_cnt], 1\n\t"
```
* t0 = 1 是初始化，再用這個 t0 轉成單精度 float 後存進 f0 當作single_floating_result 這個變數
* 用 arr_size 當 for loop counter，當 arr_size = 0 時跳出迴圈
* 每次迴圈將兩個 array 的 float load 到 f1, f2，將 f1, f2 相乘後存進 f3，再將 f3 和 single_floating_result 相乘存進 single_floating_result
* 更新 array pointer 到下一個位置，arr_size - 1
* 最後再將 f0 存的值給回到 c code 中的變數，因為不能用 mv，所以先把 f1 再拿來用（存 0），然後用 adds f0 + 0 的方法把 f0 存回 result

## Exercise 3.2 Double Floating-point Multiplication
雙精度浮點數乘法總積

```c
void arraymul_double(){
    double *p_h = u;
    double *p_x = v;
    int arr_length = arr_size;
    /* original C code
    for (int i = 0; i < arr_size; i++){
    	double_floating_result = double_floating_result * p_h[i] * p_x[i];
    }
    */
   asm volatile(
    #include "arraymul_double.c"
    : [h] "+r"(p_h), [x] "+r"(p_x), [result] "+f"(double_floating_result), [add_cnt] "+r"(add_cnt), [dadd_cnt] "+r"(dadd_cnt), [dmul_cnt] "+r"(dmul_cnt), [dlw_cnt] "+r"(dlw_cnt), [dsw_cnt] "+r"(dsw_cnt), [others_cnt] "+r"(others_cnt), [arr_size] "+r"(arr_length)
    :
    : "t0", "f0", "f1", "f2", "f3"
);
```

* t0 = 1 for double_floating_result
* f0 = double_floating_result
* f1 = p_h[i]
* f2 = p_x[i]
* f3 = p_h[i] * p_x[i]

```c
"addi t0, t0, 1\n\t"
"fcvt.d.w f0, t0\n\t"
"addi %[add_cnt], %[add_cnt], 1\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"

"for:\n\t"
"beq %[arr_size], x0, for_end\n\t"
"fld f1, 0(%[h])\n\t"
"fld f2, 0(%[x])\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"addi %[dlw_cnt], %[dlw_cnt], 2\n\t"

"fmul.d f3, f1, f2\n\t"
"fmul.d f0, f0, f3\n\t"
"addi %[dmul_cnt], %[dmul_cnt], 2\n\t"

"addi %[h], %[h], 8\n\t"
"addi %[x], %[x], 8\n\t"
"addi %[arr_size], %[arr_size], -1\n\t"
"addi %[add_cnt], %[add_cnt], 3\n\t"

"addi %[others_cnt], %[others_cnt], 1\n\t"
"jal x0, for\n\t"

"for_end:\n\t"
"addi t0, x0, 0\n\t"
"fcvt.d.w f1, t0\n\t"
"fadd.d %[result], f0, f1\n\t"
"addi %[others_cnt], %[others_cnt], 2\n\t"
"addi %[add_cnt], %[add_cnt], 1\n\t"
"addi %[dadd_cnt], %[dadd_cnt], 1\n\t"
```
* 基本上和 Single Floating-point Multiplication 的 code 架構邏輯一樣，只是改 float instruction 改成 double 的指令，同時改 instruction counter，還有改 array 每個 element 間的 offset = 8
