#include <iostream>
#include "BloomfilterConnectionPool.h"

BloomfilterConnectionPool pool(100);
void *get_thread(void * arg)
{
    ServClient* get = (ServClient*)(arg);
    std::vector<element> url_elements;
    element url_element;
    url_element.is_exist = -2;
    url_element.url = "http://laputa.1688.com/offer/ajax/OfferDesc.do?offerId=110&callback=rnd";
    element url_element1;
    url_element1.is_exist = -2;
    url_element1.url = "http://laputa.1688.com/offer/ajax/OfferDesc.do?offerId=120&callback=rnd";
    url_elements.push_back(url_element);
    url_elements.push_back(url_element1);
    std::vector<element> url_elements_return;
    while(1)
    {
        try
        {
            get->get(url_elements_return, "alibaba", url_elements);
            get->fill(url_elements_return, "alibaba", url_elements);
            pool.returnConnection(get);
            get = pool.getConnection();
            //printf("get............\n");
        }
        catch(...)
        {
            delete get;
            //pool.returnConnection(get);
            printf("get return success\n");
            pool.closeConnectionPool();
            printf("get close success\n");
            while(pool.createPool("localhost", 9090) != 0)
            {
                sleep(2);
            }
            printf("get create success......\n");
           get = pool.getConnection();
        }
    }
    return (void*)1;
}

void *fill_thread(void * arg)
{
    ServClient* get = (ServClient*)(arg);
    std::vector<element> url_elements;
    element url_element;
    url_element.is_exist = -2;
    url_element.url = "http://laputa.1688.com/offer/ajax/OfferDesc.do?offerId=110&callback=rnd";
    element url_element1;
    url_element1.is_exist = -2;
    url_element1.url = "http://laputa.1688.com/offer/ajax/OfferDesc.do?offerId=120&callback=rnd";
    url_elements.push_back(url_element);
    url_elements.push_back(url_element1);
    std::vector<element> url_elements_return;
    while(1)
    {
        try
        {
            get->fill(url_elements_return, "alibaba", url_elements);
            printf("fill............\n");
        }
        catch(...)
        {
            pool.returnConnection(get);
            printf("fill return success\n");
            pool.closeConnectionPool();
            printf("fill close success\n");
            while(pool.createPool("localhost", 9091) != 0)
            {
                sleep(2);
                printf("fill create........\n");
            }
           get = pool.getConnection();
        }
    }
    return (void*)1;
}

int main(int argc, char** argv)
{
    // 三个参数依次为连接池的大小，服务ip，端口号。

    // 返回-1表示创建连接池失败，0表示创建连接池成功。

    while(pool.createPool("localhost", 9090) != 0)
    {
        sleep(2);
        printf("2222\n");
    }

    //printf("^^^^^^^^^^^^^^^^^^^^^\n");

    // 返回NULL表示连接池还没有创建。非NULL表示返回可用连接。
    ServClient* testClient = pool.getConnection();
    ServClient* testClient1 = pool.getConnection();

    //增加一个bloomfilter，三个参数依次为bloomfilter名称，数据量大小，理论碰撞率。返回1表示该名称的bloomfilter已经存在，返回2表示创建失败，0表示成功创建。
    testClient->add("alibaba", 10000001, 0.00001);
    testClient1->add("baidu", 10000001, 0.00001);
    int num = 100;
    int i;
    pthread_t *tid = (pthread_t *)malloc(num * sizeof(pthread_t));
    for (i = 0; i < num; i++) {
        //pthread_create(&tid[i], NULL, get_thread, (void*)testClient);
        pthread_create(&tid[i], NULL, get_thread, (void*)(pool.getConnection()));
    }
    /*pthread_t *tid = (pthread_t*)malloc(1 * sizeof(pthread_t));
    pthread_create(tid, NULL, get_thread, (void*)testClient);

    pthread_t *tid1 = (pthread_t*)malloc(1 * sizeof(pthread_t));
    pthread_create(tid1, NULL, fill_thread, (void*)testClient1);
    */
    sleep(500);

    // element结构体有整型数据is_exist和string型url。
    /*std::vector<element> url_elements;
      element url_element;
      url_element.is_exist = -2;
      url_element.url = "http://laputa.1688.com/offer/ajax/OfferDesc.do?offerId=110&callback=rnd";
      element url_element1;
      url_element1.is_exist = -2;
      url_element1.url = "http://laputa.1688.com/offer/ajax/OfferDesc.do?offerId=120&callback=rnd";
      url_elements.push_back(url_element);
      url_elements.push_back(url_element1);
      std::vector<element> url_elements_return;
      */
    // 将vector中的所有元素进行查询，查询结果存放到url_elements_return里，可以通过查看vector里每个元素的is_exist值来查看是否存在，-1表示要查询的过滤器不存在，0表示要查询的字符串不存在，1表示要查询的字符串存在。
    //testClient->get(url_elements_return, "alibaba", url_elements);
    //url_elements_return.clear();

    // 将vector中的所有元素尝试写入，写入结果存放到url_elements_return里，可以通过查看vector里每一个元素的is_exist值来查看是否写入成功，1表示要写入的过滤器不存在，0表示写入成功。
    //testClient->fill(url_elements_return, "alibaba", url_elements);

    // 删除某一个bloomfilter,返回1表示要删除的过滤器不存在，0表示成功删除。
    //testClient1->delete_blooms("baidu");

    // 使用完一个连接池中的一个连接之后，将连接返回，供需要者使用。
    pool.returnConnection(testClient);
    pool.returnConnection(testClient1);

    // 关闭连接池。
    pool.closeConnectionPool();

    // 关闭之后可以通过createPool函数重新创建。
    pool.createPool("localhost", 9091);
    pool.closeConnectionPool();
    return 0;


}
