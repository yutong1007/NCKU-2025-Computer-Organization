// See LICENSE for license details.

#ifndef _RISCV_CACHE_SIM_H
#define _RISCV_CACHE_SIM_H

#include "memtracer.h"
#include "common.h"
#include <cstring>
#include <string>
#include <map>
#include <cstdint>
#include <queue>
#include <vector>

class lfsr_t
{
 public:
  lfsr_t() : reg(1) {}
  lfsr_t(const lfsr_t& lfsr) : reg(lfsr.reg) {}
  uint32_t next() { return reg = (reg>>1)^(-(reg&1) & 0xd0000001); }
 private:
  uint32_t reg;
};

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

class cache_memtracer_t : public memtracer_t
{
 public:
  cache_memtracer_t(const char* config, const char* name)
  {
    cache = cache_sim_t::construct(config, name);
  }
  ~cache_memtracer_t()
  {
    delete cache;
  }
  void set_miss_handler(cache_sim_t* mh)
  {
    cache->set_miss_handler(mh);
  }
  void clean_invalidate(uint64_t addr, size_t bytes, bool clean, bool inval)
  {
    cache->clean_invalidate(addr, bytes, clean, inval);
  }
  void set_log(bool log)
  {
    cache->set_log(log);
  }
  void print_stats()
  {
    cache->print_stats();
  }

 protected:
  cache_sim_t* cache;
};

class icache_sim_t : public cache_memtracer_t
{
 public:
  icache_sim_t(const char* config, const char* name = "I$")
	  : cache_memtracer_t(config, name) {}
  bool interested_in_range(uint64_t UNUSED begin, uint64_t UNUSED end, access_type type)
  {
    return type == FETCH;
  }
  void trace(uint64_t addr, size_t bytes, access_type type)
  {
    if (type == FETCH) cache->access(addr, bytes, false);
  }
};

class dcache_sim_t : public cache_memtracer_t
{
 public:
  dcache_sim_t(const char* config, const char* name = "D$")
	  : cache_memtracer_t(config, name) {}
  bool interested_in_range(uint64_t UNUSED begin, uint64_t UNUSED end, access_type type)
  {
    return type == LOAD || type == STORE;
  }
  void trace(uint64_t addr, size_t bytes, access_type type)
  {
    if (type == LOAD || type == STORE) cache->access(addr, bytes, type == STORE);
  }
};

#endif
