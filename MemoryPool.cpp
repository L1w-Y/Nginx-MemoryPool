//
// Created by Administrator on 25-4-8.
//

#include "MemoryPool.h"

void* MemoryPool::ngx_create_pool(size_t size) {
    ngx_pool_s* p;
    p = static_cast<ngx_pool_s*>(malloc(size));
    if (!p) {
        return nullptr;
    }

    p->d.last = reinterpret_cast<u_char *>(p)+ sizeof(ngx_pool_s);
    p->d.end = reinterpret_cast<u_char *>(p)+size;
    p->d.failed=NULL;
    p->d.next = nullptr;
    p->current = p;
    p->cleanup =nullptr;
    p->large = nullptr;
    p->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL;
    return p;
}

void * MemoryPool::ngx_palloc(const size_t size) {
    if (size>pool_->max) {
        return  ngx_palloc_large(size);
    }
    return ngx_palloc_small(size,1);
}

void * MemoryPool::ngx_pnalloc(size_t size) {
}

void * MemoryPool::ngx_pcalloc(size_t size) {
}

void MemoryPool::ngx_destroy_pool() {
}

void MemoryPool::ngx_reset_pool() {
}

void MemoryPool::ngx_pfree(void *p) {
}

void * MemoryPool::ngx_alloc(size_t size) {
}

void * MemoryPool::ngx_calloc(size_t size) {
}

void * MemoryPool::ngx_palloc_small(size_t size, ngx_uint_t align) {
}

void * MemoryPool::ngx_palloc_large(size_t size) {
}

void * MemoryPool::ngx_palloc_block(size_t size) {
}
