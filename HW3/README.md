---
title: 計組 HW3

---

# 計組 HW3
C64111174 王宇桐


## Exercise 1. Implement FIFO Data Cache Replacement Policy
實現 FIFO(First in First out) 的 cache
用 queue 這個資料結構裝 block 的 way index，每次有 block 進來 cache 時，如果已經滿了的話從 queue 取出最早加入的 way index 丟掉，取代成新進來的 block way index

### cachesim.h
將 Set-Associative Cache (cache_sim_t) 的 class 多加入一個成員變數 fifo_queues，記錄每個 set 中各個 block 被放入的先後順序
```cpp
class cache_sim_t 
{
public:
    cache_sim_t(size_t sets, size_t ways, size_t linesz, const char* name);
    cache_sim_t(const cache_sim_t& rhs);
    virtual ~cache_sim_t();

    void access(uint64_t addr, size_t bytes, bool store);
    void clean_invalidate(uint64_t addr, size_t bytes, bool clean, bool inval);
    void print_stats();
    void set_miss_handler(cache_sim_t* mh) { miss_handler = mh; }
    void set_log(bool log_) { log = log_; }
    static cache_sim_t* construct(const char* config, const char* name);

protected:
    static const uint64_t VALID = 1ULL << 63;
    static const uint64_t DIRTY = 1ULL << 62;

    virtual uint64_t* check_tag(uint64_t addr);
    virtual uint64_t victimize(uint64_t addr);

    cache_sim_t* miss_handler;
    size_t sets;
    size_t ways;
    size_t linesz;
    size_t idx_shift;
    uint64_t* tags;

    uint64_t read_accesses;
    uint64_t read_misses;
    uint64_t bytes_read;
    uint64_t write_accesses;
    uint64_t write_misses;
    uint64_t bytes_written;
    uint64_t writebacks;

    std::string name;
    bool log;

    void init();

    // 加上 FIFO 的 queues 裝每個 block 的 ways
    std::vector<std::queue<size_t>> fifo_queues;
};
```

將 Fully-Associative Cache (fa_cache_sim_t)的 class 多加入一個成員變數 fifo_queue，因為沒有分 set，所以只需要一個 queue，記錄 tag 插入的順序
```cpp
class fa_cache_sim_t : public cache_sim_t
{
 public:
  fa_cache_sim_t(size_t ways, size_t linesz, const char* name);
  uint64_t* check_tag(uint64_t addr);
  uint64_t victimize(uint64_t addr);

 private:
  std::map<uint64_t, uint64_t> tags;

  // 加上 FIFO 的 queue 裝每個 block 的 ways (只有一個 set)
  std::queue<uint64_t> fifo_queue;              
};
```

### cachesim.cc
在 init() 幫每個 set 建立一個 queue，並寫初始化每個 fifo queues
在 cache_sim_t::cache_sim_t(const cache_sim_t& rhs) 加上複製這個 cache 時也要複製 fifo queues
```cpp
void cache_sim_t::init()
{
  if (sets == 0 || (sets & (sets-1)))
    help();
  if (linesz < 8 || (linesz & (linesz-1)))
    help();

  idx_shift = 0;
  for (size_t x = linesz; x>1; x >>= 1)
    idx_shift++;

  tags = new uint64_t[sets*ways]();

  // 初始化每個 queue
  fifo_queues.resize(sets);

  read_accesses = 0;
  read_misses = 0;
  bytes_read = 0;
  write_accesses = 0;
  write_misses = 0;
  bytes_written = 0;
  writebacks = 0;

  miss_handler = NULL;
}

cache_sim_t::cache_sim_t(const cache_sim_t& rhs)
 : sets(rhs.sets), ways(rhs.ways), linesz(rhs.linesz),
   idx_shift(rhs.idx_shift), name(rhs.name), log(false)
{
  tags = new uint64_t[sets*ways];
  memcpy(tags, rhs.tags, sets*ways*sizeof(uint64_t));

  // copy cache 時也要 copy fifo queues
  fifo_queues = rhs.fifo_queues;
}
```

在 victimize() 實作當發生 cache miss 時要把哪個 block 換掉
* 先算出這個 block 屬於哪個 set，把 address 的 block offset bits 去掉 (addr >> idx_shift)，在和 set 取餘數得到 index
* 再來算出新進來的 block 的 tag，取得 tags bit 後在最前面加上 valid bit
* 檢查 set 裡面還有沒有位置，for 迴圈檢查 set 中的每一個 way，(tags[idx * ways + i] & VALID) 把這個 way 的 tags 取出再取出 valid bit，如果是 0 的話代表這個 block 還沒被填入 data，還可以把 block 放進去，所以把這個 block 設為新進來的 block tags，並且把這個 block way index 放進 queue
* 如果 block 滿了，把最早進來的 way(block) 在 queue 中丟掉，用新進來 block 的 tags 取代這個 block 的 tags，最後把這個 block way index 放進 queue
```cpp
uint64_t cache_sim_t::victimize(uint64_t addr)
{
  // set index 看這個 block 要放在哪個 set
  size_t idx = (addr >> idx_shift) & (sets-1);
  //std::cerr << "FIFO" << std::endl;

  // 新進來 block 的 tag
  uint64_t new_tag = (addr >> idx_shift) | VALID;
  
  // for loop 看這個 set 裡面還有沒有空的 way
  for (size_t i = 0; i < ways; i++) {
    // 如果這個 set 中有 way 的 tags valid bit 為 0，代表這個 way 還沒被使用過
    if (!(tags[idx * ways + i] & VALID)) {
      // 把新的 block 放進這個空的 way，同時要更新這個 set 的 queue 中的 block way index
      tags[idx * ways + i] = new_tag;
      fifo_queues[idx].push(i);
      return 0;
    }
  }

  // 如果 block 滿了，要把一個 block 丟掉 (FIFO)
  // 找到這個 set queue 中最先（最裡面）進來的 way，移出 queue，同時存這個被換掉的 block 的 tag
  size_t way = fifo_queues[idx].front();
  fifo_queues[idx].pop();
  uint64_t victim = tags[idx*ways + way];
  
  // 將這個被換掉的 way 的 tag 改成新進 block tag，並將這個 block way indes 放入 queue
  tags[idx*ways + way] = new_tag;
  fifo_queues[idx].push(way);
  return victim;
}
```

同時改 fully-associative cache 成 FIFO 的 block replacement
* 只有一個 set，不用算 set index
* 一樣算新進來的 block 的 tag
* 初始化要被 replace 的 tag = 0
* 如果 cache 滿了，從 queue 中找出最早進來的 block index，移出 queue並從 tags map 中刪掉，最後把這個要被 replace 的 block tag 存在 replece_tag
* 將新進來的 block tag 放入 tags map，同時也更新到 queue 中
```cpp
uint64_t fa_cache_sim_t::victimize(uint64_t addr)
{
  //std::cerr << "FIFO" << std::endl;

  // 新進來 block 的 tag
  uint64_t new_tag = (addr >> idx_shift) | VALID;

  // 初始化要存被 replace 的 block tag 的變數		
  uint64_t old_tag = 0;
  
  // 如果 cache 滿了
  if (fifo_queue.size() == ways)
  {
    // 在 queue 中找到最先進來的 block，移出 queue，從 tags map 中刪掉
    // 把被 replace 的 block tag 存起來
    uint64_t oldest_addr = fifo_queue.front();
    fifo_queue.pop();
    old_tag = tags[oldest_addr];
    tags.erase(oldest_addr);
  }

  // 將新的 block tag 加入 tags map 和 queue
  tags[addr >> idx_shift] = new_tag;
  fifo_queue.push(addr >> idx_shift);
  return old_tag;
}
```
## Exercise 2. Enhancing Software Programs to Reduce Memory Access Overhead
優化程式來減少 Memory access

### cache design
L1 data cache configuration 和前面實作的 FIFO cache replacement policy
* 4 ways
* 8 sets
* 32-byte cacheline
* Cache hit latency = 1 cycle
* Cache miss penalty = 100 cycles

## Exercise 2-1: Matrix Transpose Optimization
做 Matrix Transpose，也就是將矩陣的 row/column 對調
**Original C code**
```c
void matrix_transpose(int n, int *dst, int *src) {
        for (int x = 0; x < n; x++) {
            for (int y = 0; y < n; y++) {
                dst[y + x * n] = src[x + y * n];
            }
        }
    }
```
n 代表 matrix 大小，而 dst 和 src 則是 transpose 前後的矩陣，2D matrix 用 1D array 表示， src[x + y*n] 代表第 x row 的第 y 個 column，迭代這個 matrix 的每個 row 的每個 column，把這個位置 (x, y) 的值轉換到新的 matrix 的 (y, x) 位置來達到 matrix transpose

**問題：**
原始 code 的方式是固定 x 後迭代每個 column，但是 array 中的記憶體只有 row-wise 會是連續位置的，如果每次都跳到下一個 column，記憶體位置會跳 n 個位置，前一次 memory access 抓到 cache 的 memory block 是這個 array(row) 的連續記憶體，很有可能沒有包含到下一個 array(也就是下一個 column) 的位置，導致 cache miss

**Improved version**
```c
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
```
* 用 blocking 的方式，將 matrix 拆成 small size 的 submatrix，每次針對這個 submatrix 做 transpose。可以使 block 內的 memory location 因為短期內重複被用到 + memory 連續性，使這部分一直保留在 cache 中，提高 memory locality，減少 cache miss
* 前兩個 for loop 用來定義這次要執行的 block 的範圍，接下來的兩個 for loop 就是利用前面的 block range 做 transpose，要多 check x/y 是否 < n 是因為 matrix size 不一定能剛好依照 block size 平均，最後一個 block size 的 matrix 可能會超出原始 matrix 的範圍，會發生 segmatation fault
* Transpose 時改以 row-wise，每次處理完整個 row 上每個 column 的 transpose 後再跳到下一個 row，所以 code 會改成 dst[y*n + x] = src[x*n + y]

## Exercise 2-2: Matrix Multiplication Optimization
做 Matrix Multiplication，也就是將兩個 Matrix 對應的 row 和 column 相乘的總和

**Original C code**
```c
void matrix_multiply(int *A, int *B, int *Output, int i, int k, int j) {
    for (int x = 0; x < i; x++) {
        for (int y = 0; y < j; y++) {
            int sum = 0;
            for (int z = 0; z < k; z++) {
                sum += A[x * k + z] * B[z * j + y];
            }
            Output[x * j + y] = sum;
        }
    }
}
```
A, B, Output 分別是三個 matrix，A 是 i X k matrix，B 是 k X j matrix，所以兩個矩陣相乘的 Output 就是 i X j matrix，所以這段 code 就是針對 A 的 x row 和 B 的 y column 每個元素相乘的總合，放進 Output 的 第 x row/ y column

### 問題
因為矩陣乘法是 row X column，對於 A 來說一樣是 row - wise，所以使用連續的記憶體，cache miss 較低。但是 B 的話是 column - wise，因為每次要跳到下一個 column，不在同一個 array 上所以記憶體不連續，cache miss 較高

**Improved version**
```c
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
```
* 一樣用 blocking 的方式將兩個 matrix A/B 拆成 8X8 的 submatrix，用這兩個 submatrix 做矩陣乘法
* 前三個 for loop 用來定義這次要執行的 block 的範圍，也就是 matrix A 的這個 matrix 從哪個 row 到哪個 column，matrix B 的這個 matrix 從哪個 row 到哪個 column。因為矩陣相乘，所以 A submatrix 的 column = B submatrix row
* 接下來三個 for loop 是對 submatrix 做相乘，對 A submatrix 的每個 row 和 B submatrix 的每個 column，把 A submatrix 這個 row 上的每一個 column 和對應的 B submatrix column 的每個 row 相乘加總
* 最後放進 Output 對應位置時是改成 Output[x * j + y] = Output[x * j + y] + sum; 因為矩陣相乘的結果是要看整個矩陣的運算，每個 block 其實只計算了一部分矩陣相乘的結果，要把全部 block 的結果相加才會是 matrix A row 上全部元素和 matrix B column 上全部元素相乘加總的結果