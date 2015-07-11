#include "BloomFilterManager.h"
#include "CoreManager.h"
#include "bloom_filter.h"
#include "murmur.h"
//#include "GFactory.h"

using namespace octopus::common;

namespace octopus {
  namespace b_server {

    BloomFilterManager::BloomFilterManager() {
      //this->core_manager_ = core_manager;
    }

    BloomFilterManager::~BloomFilterManager() {

    }

    int16_t BloomFilterManager::initialize() {
      //init(GFactory::get_server_config().bloomfilter_config_.total_amount_of_elements);
      return MY_STATE_OK;
    }

    int16_t BloomFilterManager::wait_for_shutdown() {
      return MY_STATE_OK;
    }

    int16_t BloomFilterManager::shutdown() {
      remove_all();;
      return MY_STATE_OK;
    }

    int16_t BloomFilterManager::immediate_shutdown() {
      return MY_STATE_OK;
    }

    int16_t BloomFilterManager::start() {
      return MY_STATE_OK;
    }

    int16_t BloomFilterManager::init(size_t max) {
      if (blooms_init(max) == 0) {
        return MY_STATE_OK;
      }
      return MY_STATE_NOT_OK;
    }

    /**
     *1 key exist
     *2 bloom add fail
     *0 success
     *lock
     */
    int16_t BloomFilterManager::add(char *key, int64_t n, double e) {
      return blooms_add(key, n, e);
    }

    int16_t BloomFilterManager::add(uint32_t key, int64_t n, double e) {
      char bloom_key_str[64];
      sprintf(bloom_key_str, "%d", key);
      return blooms_add(bloom_key_str, n, e);
    }
    
    int16_t BloomFilterManager::get(char *bloom_key, char *key) {
      return blooms_get(bloom_key, key); 
    }

    int16_t BloomFilterManager::set(char *bloom_key, char *key) {
      if (blooms_set(bloom_key, key) == 0) {
        return MY_STATE_OK;
      }
      return MY_STATE_NOT_OK;
    }

    int16_t BloomFilterManager::get(uint32_t bloom_key, char *key) {
      char bloom_key_str[64];
      sprintf(bloom_key_str, "%d", bloom_key);
      return blooms_get(bloom_key_str, key); 
    }

    int16_t BloomFilterManager::set(uint32_t bloom_key, char *key) {
      char bloom_key_str[64];
      sprintf(bloom_key_str, "%d", bloom_key);
      if (blooms_set(bloom_key_str, key) == 0) {
        return MY_STATE_OK;
      }
      return MY_STATE_NOT_OK;
    }


    int16_t BloomFilterManager::remove(char *bloom_key) {
      if (blooms_delete(bloom_key) == 0) {
        return MY_STATE_OK;
      }
      return MY_STATE_NOT_OK;
    }

    int16_t BloomFilterManager::remove(uint32_t bloom_key) {
      char bloom_key_str[64];
      sprintf(bloom_key_str, "%d", bloom_key);
      if (blooms_delete(bloom_key_str) == 0) {
        return MY_STATE_OK;
      }
      return MY_STATE_NOT_OK;
    }

    int16_t BloomFilterManager::remove_all() {
      blooms_delete_all();
      return MY_STATE_OK;
    }

  }
}

