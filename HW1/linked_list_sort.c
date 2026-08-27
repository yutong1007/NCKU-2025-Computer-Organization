#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

// Split the linked list into two parts
void splitList(Node *head, Node **firstHalf, Node **secondHalf)
{
    /*Original C code
    //count number of nodes
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
    */
    
    asm volatile (
	    /*
        Block A (splitList), which splits the linked list into two halves
        */

        /*	
        t0 = counter
        t1 = temp
        t2 = 2 // riscv no divi
        t3 = mid
        t4 = i for for loop
        t5 = cur
	    t6 = cur->next
        */
	
        // count number of nodes
        "li t0, 0\n" // counter = 0
	    "mv t1, %[head]\n" // temp = head
	        
        "count:\n"
        "beq t1, x0, count_end\n" // if temp == 0, skip to count_end
    	"addi t0, t0, 1\n" // count++
        "ld t1, 8(t1)\n" // temp = temp->next (offset = 8)
	    "j count\n" // keep loop

        "count_end:\n"
	
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
        
        // return memory location of node pointer to **firstHalf and **secondHalf
        "ld t6, 8(t5)\n" // cur = cur->next
        "sd x0, 8(t5)\n" // cur->next = NULL
        "sd %[head], 0(%[firstHalf])\n" // *firstHalf = head
        "sd t6, 0(%[secondHalf])\n" // *secondHalf = cur
        : 
        : [head] "r" (head), [firstHalf] "r" (firstHalf), [secondHalf] "r" (secondHalf)
        : "t0", "t1", "t2", "t3", "t4", "t5", "t6",  "memory"
	);
}

// Merge two sorted linked lists
Node *mergeSortedLists(Node *a, Node *b)
{
    Node *result = NULL;
    Node *tail = NULL;

    /*Original C code
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
    */

    asm volatile(
        /*
        Block B (mergeSortedList), which merges two sorted lists into one
        */
	
        /*
        t0 = a->data
        t1 = b->data
        */

	
        // decide the head of the result
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

        // combine two sorted linked lists
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
        : [result] "=r" (result), [tail] "+r" (tail), [a] "+r" (a), [b] "+r" (b)
        : 
        : "t0", "t1", "memory"
        );
    return result;
}

// Merge Sort function for linked list
Node *mergeSort(Node *head)
{
    if (!head || !head->next)
        return head; // Return directly if there is only one node

    Node *firstHalf, *secondHalf;
    splitList(head, &firstHalf,
              &secondHalf); // Split the list into two sublists

    firstHalf = mergeSort(firstHalf);   // Recursively sort the left half
    secondHalf = mergeSort(secondHalf); // Recursively sort the right half

    return mergeSortedLists(firstHalf, secondHalf); // Merge the sorted sublists
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    if (!input) {
        fprintf(stderr, "Error opening file: %s\n", argv[1]);
        return 1;
    }
    int list_size;
    fscanf(input, "%d", &list_size);
    Node *head = (list_size > 0) ? (Node *)malloc(sizeof(Node)) : NULL;
    Node *cur = head;
    for (int i = 0; i < list_size; i++) {
        fscanf(input, "%d", &(cur->data));
        if (i + 1 < list_size)
            cur->next = (Node *)malloc(sizeof(Node));
        cur = cur->next;   
    }
    
    fclose(input);

    // Linked list sort
    head = mergeSort(head);

    cur = head;

    while (cur) {
        printf("%d ", cur->data);
        
        /*Original C code
        cur = cur->next
        */

        asm volatile(
            /*
            Block C (Move to the next node), which updates the pointer to
            traverse the linked list
            */
        
            "ld %[cur], 8(%[cur])\n" // cur = second element in node structure (next node pointer)
            : [cur] "+r" (cur) // write in to cur variable
            : // no regester
            : "memory"
            );
    }
    printf("\n");
    return 0;
}