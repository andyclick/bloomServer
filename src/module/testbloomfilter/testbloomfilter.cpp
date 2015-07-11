#include "bloom_filter.h"
#include "murmur.h"

#include "common_define.h"

void test_bloomfilter()
{
    char url[1024];
    blooms_init(400000000);
    char bloom_key[]="1";
    //blooms_add(bloom_key, 10000001, 0.00001);
    blooms_add("1", 40000001, 0.000001);
    int a=0,b=0,c=0;
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    size_t count = 0;
    fp = fopen("./ali_busin_url.csv", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    while ((read = getline(&line, &len, fp)) != -1) {
        //printf("Retrieved line of length %zu :\n", read);
        //printf("%s", line);
        int16_t ret = blooms_get(bloom_key, line); 
        if (ret == BLOOF_FILTER_SUBKEY_NOT_EXIST)
        {
            a = a + 1;
            blooms_set(bloom_key, line);
        } else if (ret == BLOOF_FILTER_SUBKEY_EXIST){
            printf("%s\n",line);
            b = b + 1;
        }else if (ret == BLOOF_FILTER_KEY_NOT_EXIST) {
            c = c + 1;
        }
        count++;
    }
#if 0
    for(int i = 0 ;i<=10000000; i++)
    {
        sprintf(url,"http://laputa.1688.com/offer/ajax/OfferDesc.do?offerId=%010d&callback=rnd",i);
        int16_t ret = blooms_get(bloom_key, url); 
        if (ret == BLOOF_FILTER_SUBKEY_NOT_EXIST)
        {
            a = a + 1;
            blooms_set(bloom_key, url);
        } else if (ret == BLOOF_FILTER_SUBKEY_EXIST){
            printf("%s\n",url);
            b = b + 1;
        }else if (ret == BLOOF_FILTER_KEY_NOT_EXIST) {
            c = c + 1;
        }
    }
    blooms_delete_all();
    printf("a=%d,b=%d,c=%d\r\n",a,b,c);
#endif
    printf("a=%d,b=%d,c=%d\r\n",a,b,c);
    printf("count:%d\r\n", count);
}

int main()
{
    test_bloomfilter();
    return 0;
}
