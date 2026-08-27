void matrix_multiply(int *A, int *B, int *Output, int i,
                           int k, int j) {
    // Inplement your code here 
    int block = 8;

    // three for loop for blocking
    for (int a_row = 0; a_row < i; a_row = a_row + block) {
        for (int a_column = 0; a_column < k; a_column = a_column + block) { // also b_row
            for (int b_column = 0; b_column < j; b_column = b_column + block) {
                // Multiplaction
                for (int x = a_row; x < a_row + block && x < i; x++) {
                    for (int y = b_column; y < b_column + block && y < j; y++) {
                        int sum = 0;
                        for (int z = a_column ; z < a_column + block && z < k; z++) {
                            sum = sum + A[x * k + z] * B[z * j + y];
                        }
                        Output[x * j + y] = Output[x * j + y] + sum; // add all blocks sum
                     }
                }
            }
        }
    }
}
