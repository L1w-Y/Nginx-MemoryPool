//
// Created by Administrator on 25-4-8.
//

#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H
#include <cstdint>
#include<functional>
#include<memory>
#include<iostream>
#include <errno.h>


using u_char = unsigned char;
using ngx_uint_t = unsigned int;
struct ngx_pool_s;
struct ngx_pool_large_s;

constexpr int ngx_pagesize = 4096;
constexpr int NGX_MAX_ALLOC_FROM_POOL=ngx_pagesize - 1;

constexpr int NGX_DEFAULT_POOL_SIZE = 16 * 1024;

constexpr size_t NGX_POOL_ALIGNMENT=16;  //内存对齐：16字节


// 对整数进行字节对齐
template <typename T>
constexpr T ngx_align(T d, T a) noexcept {
    return (d + (a - 1)) & ~(a - 1);
}


// 对指针进行字节对齐，返回原始类型指针
template <typename T>
constexpr T* ngx_align_ptr(T* p, uintptr_t a) {
    return reinterpret_cast<T*>(
        (reinterpret_cast<uintptr_t>(p) + (a - 1)) & ~(a - 1)
    );
}

typedef void (*ngx_pool_cleanup_pt)(void *data);
struct ngx_pool_cleanup_t {
    ngx_pool_cleanup_pt   handler;
    void                 *data;
    ngx_pool_cleanup_t   *next;
};


struct ngx_pool_large_s {
    ngx_pool_large_s     *next;
    void                 *alloc;
};

struct ngx_pool_data_t{
    u_char               *last;
    u_char               *end;
    ngx_pool_s           *next;
    ngx_uint_t            failed;
};

//内存池头部
struct ngx_pool_s {
    ngx_pool_data_t       d;
    size_t                max;
    ngx_pool_s           *current;
    ngx_pool_large_s     *large;
    ngx_pool_cleanup_t   *cleanup;
};

constexpr int NGX_MIN_POOL_SIZE =ngx_align((sizeof(ngx_pool_s) + 2 * sizeof(ngx_pool_large_s)),NGX_POOL_ALIGNMENT);

class MemoryPool{
public:
    void ngx_create_pool(size_t size);//创建内存池
    void* ngx_palloc(size_t size);//内存对齐的内存分配
    //void* ngx_pnalloc(size_t size);//不考虑内存对齐的内存分配
    //void* ngx_pcalloc(size_t size);//内存对齐，初始化为0
    void ngx_destroy_pool() ;//调用cleanup回调函数
    void ngx_reset_pool();//重置内存池，先释放大块内存
    void ngx_pfree(const void *p);//释放大块内存
    ngx_pool_cleanup_t *ngx_pool_cleanup_add(size_t size);
    explicit MemoryPool(size_t size);
private:
	ngx_pool_s *pool_;//内存池入口指针
    void* ngx_palloc_small(size_t size,ngx_uint_t align);
    void* ngx_palloc_large(size_t size) ;
    void* ngx_palloc_block(size_t size) ;
};


#endif //MEMORYPOOL_H
