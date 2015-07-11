struct element
{
    1: i16 is_exist,
    2: string url,
}

struct memInfo
{
    1: string key,
    2: i32 size,
}

service Serv{
     i16 add(1: string key, 2: i32 max_elements, 3: double false_rate),
     list<element> fill(1: string key, 2: list<element> vector_url),
     list<element> get(1: string key, 2: list<element> vector_url),
     list<memInfo> get_stats(),
     list<memInfo> get_blooms_stats(),
     list<memInfo> get_bloom_stats(1: string key),
     i16 set_mem(1: i32 size),
     i16 get_one(1: string key, 2: string url),
     i16 fill_one(1: string key, 2: string url),
     i16 delete_blooms(1: string key);
     i16 delete_blooms_all();
}
