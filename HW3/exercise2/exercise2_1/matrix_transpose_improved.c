void matrix_transpose(int n, int *dst, int *src) {
    // Inplement your code here 
    int block = 8;

    // Double for loop for blocking
    for (int row = 0; row < n; row = row + block) {
        for (int column = 0; column < n; column = column + block) {
            // Double for loop for transpose
            // need to double check x/y < n
            for (int x = row; x < row + block && x < n; x++) {
                for (int y = column; y < column + block && y < n; y++) {
                    dst[y*n + x] = src[x*n + y]; // row - wise exchange         
                }
            }
        }
    }
}