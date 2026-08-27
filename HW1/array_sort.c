#include <stdio.h>

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
    int arr_size;
    fscanf(input, "%d", &arr_size);
    int arr[arr_size];

    // Read integers from input file into the array
    for (int i = 0; i < arr_size; i++) {
        int data;
        fscanf(input, "%d", &data);
        arr[i] = data;
    }
    fclose(input);

    int *p_a = &arr[0];

    // array a bubble sort
    /* Original C code segment
    for (int i = 0; i < arr_size - 1; i++) {
        for (int j = 0; j < arr_size - i -1; j++) {
            if (*(p_a + j) > *(p_a + j + 1)) {
                int tmp = *(p_a + j);
                *(p_a + j) = *(p_a + j + 1);
                *(p_a + j + 1) = tmp;
            }
        }
    }
    */

    /*translate
    if (*(p_a + j) > *(p_a + j + 1)) {
        int tmp = *(p_a + j);
        *(p_a + j) = *(p_a + j + 1);
        *(p_a + j + 1) = tmp;
     to assembly code
    */
    for (int i = 0; i < arr_size - 1; i++) {
        for (int j = 0; j < arr_size - i - 1; j++) {
            asm volatile(
		/*
		to get *(p_a + j) value
		t0 = *(p_a + j) value
		t2 = j * 4 value (1 int = 4 byte)
		t3 = p_a + j value
		*/
		
		"slli t2, %[j], 2\n" // bit left shift 2 (times 4)
		"add t3, %[p_a], t2\n" // p_a + j
		"lw t0, 0(t3)\n" // t0 = *(p_a + j) value
		
		/*
		Do not use original regester to store pointer
		to get *(p_a + j + 1) value
	        t1 = *(p_a + j + 1) value
		t4 = j + 4 value
		t5 = p_a + j + 1 value
		*/

		"addi t4, t2, 4\n" // j + 1
	        "add t5, %[p_a], t4\n" // p_a + j +1	
		"lw t1, 0(t5)\n" // t1 = *(p_a + j +1) value
	    	
		/*
		compare t0 t1, if t0 > t1 = if t0 <= t1
		change t0 t1
		t6 = temp
		*/
		"ble t0, t1, 1f\n" // if t0 <= t1, skip tp 1:
		"mv t6, t0\n" // temp = *(p_a + j)
		"sw t1, 0(t3)\n" // *(p_a + j) = *(p_a + j + 1)
		"sw t6, 0(t5)\n" //*(p_a + j + 1) = temp
		"1:\n" // if t0 <= t1, will not do anything
		: // no output
		:[p_a] "r" (p_a), [j] "r" (j)
		:"t0", "t1", "t2", "t3", "t4", "t5", "t6", "memory"
            );
        }
    }
    p_a = &arr[0];
    for (int i = 0; i < arr_size; i++)
        printf("%d ", *p_a++);
    printf("\n");
    return 0;
}