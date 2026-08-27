<<<<<<< HEAD
---
title: 計組HW1

---

# 計組 HW1
C64111174 王宇桐

## Exercise 1. Array Bubble sort
Input:
1. array 大小
2. array 的每一個元素

Output:
1. 排序完的 array


### Bubble sort C code

```c
// array a bubble sort
for (int i = 0; i < arr_size - 1; i++) {
    for (int j = 0; j < arr_size - i -1; j++) {
        if (*(p_a + j) > *(p_a + j + 1)) {
            int tmp = *(p_a + j);
            *(p_a + j) = *(p_a + j + 1);
            *(p_a + j + 1) = tmp;
        }
    }
}
```
Bubble sort 是每次都將 array 中的值兩兩比較，將較小的值擺到 array 的左邊，總共做 array size - 1 次後就會把 array 從小到大排序。但是範例的 Bubble sort C code 是每次都把最大的值擺到 array 的右邊，所以最外層的 for 迴圈是做 array size - 1 次，內層 for 迴圈則是將 array element 中從左到右兩兩相比，只需要做 array size - i - 1 次是因為每次做完外層 for 迴圈都會固定好一個最右邊的值，可以不用比較。兩兩相比後如果左邊的數比較大，則和右邊的數交換。

### Bubble sort assembly code
```c
for (int i = 0; i < arr_size - 1; i++) {
        for (int j = 0; j < arr_size - i - 1; j++) {
            asm volatile(
                // Your code
                "");
        }
    }
}
```
只需要將下列這段 code 轉成 assembly code
```c
if (*(p_a + j) > *(p_a + j + 1)) {
    int tmp = *(p_a + j);
    *(p_a + j) = *(p_a + j + 1);
    *(p_a + j + 1) = tmp;
```

### 思考邏輯
先用 pointer 和 j 提取 *(p_a + j) 和 *(p_a + j + 1) 的值，將兩個值比較決定需不需要交換位置。所以需要兩個暫存器存 p_a + j 和 p_a + j + 1 這兩個 pointer 位置，交換位置時也需要用這兩個指標位置賦予值。 也需要兩個暫存器存這兩個 pointer 分別指向的值，另外一個暫存器負責存 j * 4（因為需要轉換成 byte），最後一個暫存器當作交換位置時的 temp。因為在 C code 中已經有 p_a 和 j 這兩個變數，直接在指令使用。

1. t0 = *(p_a + j) 的值
2. t1 = *(p_a + j + 1) 的值
3. t2 = j * 4
4. t3 = p_a + j
5. t4 = j + 4 （j+1）
6. t5 = p_a + j + 1
7. t6 = temp

#### 提取*(p_a + j)
* 用 slli 將 j 的 bit pattern 左移 2 代表乘 4
* p_a + j 存在 t3 中
* 用 t3 存的 pointer 位置讀取值後存在 t0
```c
"slli t2, %[j], 2\n" // bit left shift 2 (times 4)
"add t3, %[p_a], t2\n" // p_a + j
"lw t0, 0(t3)\n" // t0 = *(p_a + j) value
```

#### 提取*(p_a + j + 1)
* 直接將原本的 j 值加上 4 個 byte 做 + 1
* p_a + j + 1 存在 t5 中
* 用 t5 存的 pointer 位置讀取值後存在 t1
```c
"addi t4, t2, 4\n" // j + 1
"add t5, %[p_a], t4\n" // p_a + j +1    
"lw t1, 0(t5)\n" // t1 = *(p_a + j +1) value
```

#### 條件判斷
* ble t0m t1, 1f 代表如果 t0 <= t1 的話，會跳到 1:
* 反之 t0 > t1 的話會做以下指令：
* 將 t0 存的 *(p_a + j) 暫存到 t6 temp
* sw t1, 0(t3) 是將 t1 也就是 *(p_a + j + 1) 的值讀到 t3 也就是 (p_a + j) 這個 pointer 位置
* sw t6, 0(t5) 是將 t6 也就是 temp 暫存的值讀到 t5 也就是 (p_a + j + 1) 這個 pointer 位置
```c
"ble t0, t1, 1f\n" // if t0 <= t1, skip tp 1:
"mv t6, t0\n" // temp = *(p_a + j)
"sw t1, 0(t3)\n" // *(p_a + j) = *(p_a + j + 1)
"sw t6, 0(t5)\n" //*(p_a + j + 1) = temp
"1:\n" // if t0 <= t1, will not do anything
```

## Exercise 2. Array Search
Input:
1. array 大小
2. array 的每一個元素
3. 要在 array 中找的值

Output:
1. target value 的 index

### Function C code
```c
int arraySearch(int *p_a, int arr_size, int target)
{
    int result = -1;
    
    //Original C code
    for (int i = 0; i < arr_size; i++) {
        if (*(p_a + i) == target) {
            result = i;
            break;
        }
    }
    asm volatile(
        // Your code
        "");

    return result;
}
```
輸入 *p_a = array, arr size, target value，result 是 target 在 array 中的 index，如果沒找到的話是 -1。

### 思考邏輯
這段 code 要寫的 asm volatile 沒有寫在 for 迴圈內，所以必須寫 迴圈的指令來遍歷 array 中的全部元素，找出 target 在 array 中的 index。用一個暫存器存 for 迴圈的變數 i，同時這個暫存器存的也代表 index，用一個暫存器存 i * 4（因為需要轉換成 byte），用一個暫存器存 pointer (p_a + i) 的位置，最後一個暫存器存這個 pointer 指向的值，也就是 array 中的元素
1. t0 = index for for loop
2. t1 = i * 4
3. t2 = p_a + i
4. t3 = *(p_a + i)

#### for loop
* 用 for: 這個 label 來當作 for 迴圈
* 同時用 bge t0, %[arr_size], loop_end 來判斷，當 t0 也就是 i > array size 時，for 迴圈結束，跳到 loop_end label
* 用 slli 將 j 的 bit pattern 左移 2 代表乘 4
* p_a + i 存在 t2 中
* 用 t2 存的 pointer 位置讀取值後存在 t3 

```c
"for:\n"
"bge t0, %[arr_size], loop_end\n" // if i > arr_size, loop end
"slli t1, t0, 2\n" // bit left shift 2 (times 4)
"add t2, %[p_a], t1\n" // p_a + i
"lw t3, 0(t2)\n" // *(p_a + i)
```

#### 如果找到 target
* beq t3, %[target], break 代表如果 t3 = target 的話，會跳到 break label，反之繼續 for loop
* 繼續 for loop 就是 i = i + 1，然後跳回 for label 

```c
// if (*(p_a + i) == target
// "beq t3, %[target], break\n" // if found, return. else i = i + 1 and keep looping
"add t0, t0, 1\n" // i = i + 1
"j for\n" // jump to for loop, keep looping
```

#### loop end
* break 時，i 的值就是 target 的 index ，所以把 C code 中的變數 result 改成 t0 存的值

```c
"break:\n"
"mv %[result], t0\n" // result = index

"loop_end:\n" // loop break, will not do anything
```

## Exercise 3. Linked-List Merge Sort
Input:
1. array 大小
2. array 的每一個元素

Output:
1. 排序完的 array

### 複習

#### linkded list
structure Node 裡面有兩個元素，分別是這個 node 存的 data 和一個 pointer 指向下一個 node
```c
typedef struct Node {
    int data;
    struct Node *next;
} Node;
```

#### merge sort
將一個 linked list 拆成兩個 linked list，持續對兩個 linked list 遞迴做 merge sort 再合併成完整 sorted 的 linked list


### Block A: Split list
將一個 linked list 從中間拆成兩個 linked list
```c
// Split the linked list into two parts
void splitList(Node *head, Node **firstHalf, Node **secondHalf)
{
    asm volatile(
        /*
        Block A (splitList), which splits the linked list into two halves
        */
        "");
}
```

#### C code
用一個 counter 算這個 linked list 有幾個 node，再用這個 counter 將 linked list 拆成兩個（firstHalf 指到 head，secondHalf 指到 counter + 1 的 node）
```c
// count number of nodes
int counter = 0;
Node* temp = head;
while (temp) {
    temp = temp->next;
    counter++;
}

// split the linked list
int mid = counter / 2;
*firstHalf = head;

Node* cur = head;
for (int i = 0; i < mid - 1; i++) {
    cur = cur->next;
}

*secondHalf = cur->next;
cur->next = NULL;
```

### 思考邏輯
先算出 list 長度，所以用兩個暫存器分別存 counter 和要用來遍歷整個 list 的 temp(head)，算完後再用兩個暫存器完成除以 2 後得到這個 list 的中點，最後用三個暫存器分別存 for 迴圈要用的 i 和遍歷 list 的 cur(head) 以及 cur->next，最後將兩個指標的記憶體位置給回兩個二級指標。
1. t0 = counter
2. t1 = temp
3. t2 = 2 // riscv no divi
4. t3 = mid
5. t4 = i for for loop
6. t5 = cur
7. t6 = cur->next

#### Count list length
* 用 t0 暫存 counter，t1 暫存 head 也就是 temp 的值
* beq t1, x0, count_end 當 temp 不是 NULL 時，代表還沒到 list 尾部，持續做 temp = temp->next, counter++，當 temp = NULL 時離開迴圈
```c
// count number of nodes
"li t0, 0\n" // counter = 0
"mv t1, %[head]\n" // temp = head
	        
"count:\n"
"beq t1, x0, count_end\n" // if temp == 0, skip to count_end
"addi t0, t0, 1\n" // count++
"ld t1, 8(t1)\n" // temp = temp->next (offset = 8)
"j count\n" // keep loop

"count_end:\n"
```

#### Move to mid of list
* 先用 counter 除以 2 加上 -1 後得到 mid - 1
* bge t4, t3, loop_end 用 for 迴圈做 mid - 1 次的 cur = cur->next 後的 cur 就會指向 list 的中間的 node 
```c
"li t2, 2\n"
"div t3, t0, t2\n"
"addi t3, t3, -1\n" // get mid-1

"li t4, 0\n"              // i = 0
"mv t5, %[head]\n"        // cur = head

// move to mid of list
"loop:\n"
"bge t4, t3, loop_end\n"  // if i >= mid-1, skip to loop_end
"ld t5, 8(t5)\n"          // cur = cur->next
"addi t4, t4, 1\n"        // i++
"j loop\n" // keep loop

"loop_end:\n"
```

#### return value to **pointer
* 多做一次 cur = cur->next，並斷開 list cur->next = NULL
* 用 sd 將兩個 pointer 的位置賦予到二級指標指向的位置
```c
// return memory location of node pointer to **firstHalf and **secondHalf
"ld t6, 8(t5)\n" // cur = cur->next
"sd x0, 8(t5)\n" // cur->next = NULL
"sd %[head], 0(%[firstHalf])\n" // *firstHalf = head
"sd t6, 0(%[secondHalf])\n" // *secondHalf = cur
```
### Block B: Merge Sort
將兩個 sorted 的 linked list combine
```c
// Merge two sorted linked lists
Node *mergeSortedLists(Node *a, Node *b)
{
    Node *result = NULL;
    Node *tail = NULL;

    asm volatile(
        /*
        Block B (mergeSortedList), which merges two sorted lists into one
        */
        "");

    return result;
}
```

#### C code
同時迭代兩個 sorted 的 linked list，看哪個 list 的值比較小就把該 node 接上我們正在合併的 linked list。

```c
// decide the head of the result
if (a->data <= b->data) {
    result = tail = a;
    a = a->next;
} 
else {
    result = tail = b;
    b = b->next;
}

// combine two sorted linked lists
while (a && b) {
    if (a->data <= b->data) {
        tail->next = a;
        a = a->next;
    } 
    else {
        tail->next = b;
        b = b->next;
    }
    tail = tail->next;
}

// connect
if (a) tail->next = a;
if (b) tail->next = b;
```

### 思考邏輯
只需要兩個暫存器分別存兩個 list 的某個 element 的 data，用這兩個值比大小，將比較小的值接上合併的 list 後跳到該 list 的下一個 element，持續迭代直到接完全部 element。
1. t0 = a->data
2. t1 = b->data

#### Decide the head of the result
* 看 a b 兩個 list 哪個開頭的值比較小就當做合併 list 的開頭
* 如果 a > b，b 就是 result 的開頭和 tail，並且 b 要從下一個 element 開始和 a 比較，所以用 mv 改變 result, tail 的值，並且 b = b->next
* 反之 b > a 亦然
```c
"lw t0, 0(%[a])\n"
"lw t1, 0(%[b])\n"

"ble t0, t1, a_small\n" // if a <= b, skip to a_small
"mv %[result], %[b]\n" // result = b
"mv %[tail], %[b]\n" // tail = b
"ld %[b], 8(%[b])\n" // b = b->next (8 byte)
"j merge\n"

"a_small:\n"
"mv %[result], %[a]\n"
"mv %[tail], %[a]\n" 
"ld %[a], 8(%[a])\n"
```

#### Combine two sorted linked lists
* 用一個 while 迴圈持續把 a b list 中比較小的 element 接上去
* 先用 a b 兩個 pointer 判斷其中某個 list 是不是 = NULL，代表用完 elements 了，如果是的話可以把另外一個 list 全部接上去
* lw 更新 a b element 的 data
* 比較 a b 哪個大，如果 a > b 的話，將 tail->next = b，並且 b list 要位移到下一個 element b = b->next
* update_tail 是要更新 tail 這個 pointer tail = tail->next，才可以持續接上新的 node
* 最後的 connect 就是當 a b 其中一個 list 已經都排進去時，將另外一個 list 直接接上去 tail->next
```c
"merge:\n"
"beq %[a], x0, connect_b\n" // if a is null, skip to connect b
"beq %[b], x0, connect_a\n" // if b is null, skip to connect a

"lw t0, 0(%[a])\n" // update a->data
"lw t1, 0(%[b])\n" // update b->data
"ble t0, t1, a_smaller\n" // if a <= b, skip to a_smaller
"sd %[b], 8(%[tail])\n"
"ld %[b], 8(%[b])\n" 
"j update_tail\n" 

"a_smaller:\n"
"sd %[a], 8(%[tail])\n" // tail->next = a
"ld %[a], 8(%[a])\n" // a = a->next
"j update_tail\n"

"update_tail:\n" // tail = tail->next
"ld %[tail], 8(%[tail])\n"
"j merge\n" // keep loop

"connect_a:\n" // tail->next = a
"sd %[a], 8(%[tail])\n"
"j end_merge\n"

"connect_b:\n" // tail->next = b
"sd %[b], 8(%[tail])\n"
"j end_merge\n"

"end_merge:\n"
```
### Block C: Move to next node （先做）
這段 code 是已經 sort 完 linked list 後要輸出結果
```c
// Linked list sort
head = mergeSort(head);

cur = head;
while (cur) {
    printf("%d ", cur->data);
    asm volatile(
        /*
        Block C (Move to the next node), which updates the pointer to
        traverse the linked list
        */
        "");
}
```

#### C code
其實 move to next node 就是要實現
```c
cur = cur -> next
```

### 思考邏輯
C code 中已經有 cur 這個指向 node 的 pointer，pointer 指到的因為 Node 的 structure已經被定義好了，第一個 element 是這個 node 存的 data，第二個 element 是下一個 node 的 pointer，所以我們需要將 cur 這個變數改為它指向的第二個 element。

```c
typedef struct Node {
    int data;
    struct Node *next;
} Node;
```

而 linux 虛擬機是 64 bit，所以前面的 int data 佔了 8 個 byte，所以位移 8 byte 後到給到第二個 element。
```c
"ld %[cur], 8(%[cur])
```


=======
# Computer-Organization-2025
>>>>>>> ccf6c433605146688f779bc920d9c8070c243753
