//
// Created by Administrator on 25-4-8.
//
#include "MemoryPool.h"
#include<string.h>
struct Data {
    char* ptr;
    FILE * pfile;
};
void func1(void *p) {
    auto p1 =static_cast<char *>(p);
    std::cout<<"free ptr mem"<<std::endl;
    free(p1);
}
void func2(void *p) {
    auto p1 =static_cast<FILE *>(p);
    std::cout<<"close file"<<std::endl;
    fclose(p1);
}

int main() {
    MemoryPool ngx_pool(512);
    void *p1 = ngx_pool.ngx_palloc(128);
    if (!p1) {
        std::cout<<"ngx_palloc 128 bytes fail"<<std::endl;
    }

    Data *p2 =static_cast<Data *>(ngx_pool.ngx_palloc(512));
    if (!p2) {
        std::cout<<"ngx_palloc 512 bytes fail"<<std::endl;
    }
    p2->ptr = static_cast<char *>(malloc(12));
    strcpy(p2->ptr,"hello world");
    p2->pfile=fopen("data.txt","w");


    ngx_pool_cleanup_t *c1 = ngx_pool.ngx_pool_cleanup_add(sizeof(char*));
    c1->handler = func1;
    c1->data = p2->ptr;
    ngx_pool_cleanup_t *c2 = ngx_pool.ngx_pool_cleanup_add(sizeof(FILE*));
    c2->handler = func2;
    c2->data = p2->pfile;

    ngx_pool.ngx_destroy_pool();
    return 0;
}