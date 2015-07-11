#ifndef _OCTOPUS_B_SERVER_BLOOMFILTER_MANAGER_H_
#define _OCTOPUS_B_SERVER_BLOOMFILTER_MANAGER_H_

#include <string>
#include <vector>
#include <queue>
#include <stdlib.h>
#include <string.h>
#include "BaseManager.h"


namespace octopus {
  namespace b_server {
    //class CoreManager;
    class BloomFilterManager: public octopus::common::BaseManager {
    public:
      BloomFilterManager();
      virtual ~BloomFilterManager();

      virtual int16_t initialize();
      virtual int16_t wait_for_shutdown();
      virtual int16_t shutdown();
      virtual int16_t immediate_shutdown();
      virtual int16_t start();

    public:
      int16_t init(size_t);
      int16_t add (char *key, int64_t n, double e);
      int16_t add(uint32_t key, int64_t n, double e);
      int16_t get(char *bloom_key, char *key);
      int16_t get(uint32_t bloom_key, char *key);
      int16_t set(char *bloom_key, char *key);
      int16_t set(uint32_t bloom_key, char *key);
      void status(char *);
      int16_t remove(char *bloom_key);
      int16_t remove(uint32_t bloom_key);
      int16_t remove_all();

    private:
      //CoreManager *core_manager_;

    };
  }
}

#endif

