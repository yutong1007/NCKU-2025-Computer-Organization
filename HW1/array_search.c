#include <stdio.h>

int arraySearch(int *p_a, int arr_size, int target)
{
    int result = -1;
    /*Original C code
    for (int i = 0; i < arr_size; i++) {
        if (*(p_a + i) == target) {
            result = i;
            break;
        }
    }
    */

    asm volatile(
	/*
	t0 = index for for loop
    t1 = i * 4 (4byte)
	t2 = p_a + i
	t3 = *(p_a + i)	
	*/

	"li t0, 0\n" // i = 0
	
	// for loop	
    "for:\n"
	"bge t0, %[arr_size], loop_end\n" // if i > arr_size, loop end
	"slli t1, t0, 2\n" // bit left shift 2 (times 4)
	"add t2, %[p_a], t1\n" // p_a + i
	"lw t3, 0(t2)\n" // *(p_a + i)
	
	// if (*(p_a + i) == target
	"beq t3, %[target], break\n" // if found, return. else i = i + 1 and keep looping
	"add t0, t0, 1\n" // i = i + 1
	"j for\n" // jump to for loop, keep looping

	"break:\n"
	"mv %[result], t0\n" // result = index

	"loop_end:\n" // loop break, will not do anything	
	: [result] "=r" (result) // output = result
	: [arr_size] "r" (arr_size), [p_a] "r" (p_a), [target] "r" (target)
	: "t0", "t1", "t2", "t3", "memory"
    );
    return result;
}

// Main function to test the implementation
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
    int target;
    fscanf(input, "%d", &target);
    fclose(input);

    int *p_a = &arr[0];

    int index = arraySearch(p_a, arr_size, target);

    // Print the result
    printf("%d ", index);
    printf("\n");

    return 0;
}