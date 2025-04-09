//
// Created by Administrator on 25-4-8.
//

#include "MemoryPool.h"

MemoryPool::MemoryPool(const size_t size) {
   ngx_create_pool(size);
    if (!pool_) {
        throw std::runtime_error("MemoryPool 创建失败");
    }
}

void MemoryPool::ngx_create_pool(const size_t size) {
    auto *p = static_cast<ngx_pool_s *>(malloc(size));
    if (!p) {
        std::cerr << "内存池开辟失败" << std::endl;
        return;
    }
    p->d.last = reinterpret_cast<u_char *>(p)+ sizeof(ngx_pool_s);
    p->d.end = reinterpret_cast<u_char *>(p)+size;
    p->d.failed=0;
    p->d.next = nullptr;
    p->current = p;
    p->cleanup =nullptr;
    p->large = nullptr;
    p->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL;
    pool_= p;
}

void * MemoryPool::ngx_palloc(const size_t size){
    if (size>pool_->max) {
        return  ngx_palloc_large(size);
    }
    return ngx_palloc_small(size,1);
}

void MemoryPool::ngx_destroy_pool(){
    ngx_pool_s *p = pool_;
    if (!p) return;

    for (const ngx_pool_cleanup_t *c = pool_->cleanup;c;c=c->next) {
        if (c->handler) {
            c->handler(c->data);
        }
    }
    for (const ngx_pool_large_s *l = pool_->large;l;l = l->next) {
        free(l->alloc);
    }

    while (p) {
        ngx_pool_s *n = p->d.next;
        free(p);
        p = n;
    }

}
void MemoryPool::ngx_reset_pool() {
    if (!pool_) {
        std::cerr << "Warning: 尝试重置空的内存池指针 pool_" << std::endl;
        return;
    }
    for (const ngx_pool_large_s *l = pool_->large; l; l = l->next) {
        if (l->alloc) {
            free(l->alloc);
        }
    }

    for (ngx_pool_s *p = pool_; p; p = p->d.next) {
        p->d.last = reinterpret_cast<u_char *>(p) + sizeof(ngx_pool_s);
        p->d.failed = 0;
    }

    pool_->current = pool_;
    pool_->large = nullptr;
}

void MemoryPool::ngx_pfree(const void *p) {
    for (ngx_pool_large_s *l = pool_->large; l; l = l->next) {
        if (p == l->alloc) {
            free(l->alloc);
            l->alloc = nullptr;
        }
    }
}


void * MemoryPool::ngx_palloc_small(const size_t size, const ngx_uint_t align) {
    ngx_pool_s *p = pool_->current;
    do {
        u_char *m = p->d.last;
        if (align){
            m = ngx_align_ptr(m,NGX_POOL_ALIGNMENT);
        }
        if (static_cast<size_t>(p->d.end-m)>size) {
            p->d.last = m + size;
            return m;
        }
        p=p->d.next;
    }while (p);
    return ngx_palloc_block(size);
}

void * MemoryPool::ngx_palloc_large(const size_t size) {
    ngx_uint_t         n=0;
    void *p = malloc(size);
    ngx_pool_large_s *large;
    if (!p) {
        return nullptr;
    }
    for (large = pool_->large;large;large=large->next) {
        if (large->alloc == nullptr) {
            large->alloc = p;
            return p;
        }
        n++;
        if (n>3) {
            break;
        }
    }

    large = static_cast<ngx_pool_large_s *>(ngx_palloc_small(sizeof(ngx_pool_large_s), 1));
    if (large == nullptr) {
        free(p);
        return  nullptr;
    }
    large->alloc=p;
    large->next=pool_->large;
    pool_->large=large;

    return p;

}

void * MemoryPool::ngx_palloc_block(const size_t size) {
    const auto psize=static_cast<size_t>(pool_->d.end-reinterpret_cast<u_char *>(pool_));
    ngx_pool_s  *p;
    auto *m = static_cast<u_char *>(malloc(psize));

    if (!m) {
        return  nullptr;
    }

    auto *new_chunk = reinterpret_cast<ngx_pool_s *>(m);
    new_chunk->d.end=m+psize;
    new_chunk->d.next=nullptr;
    new_chunk->d.failed=0;
    m += sizeof(ngx_pool_data_t);
    m = ngx_align_ptr(m, NGX_POOL_ALIGNMENT);
    new_chunk->d.last = m + size;

    for (p=pool_->current;p->d.next;p=p->d.next) {
        if (p->d.failed++>4) {
            pool_->current = p->d.next;
        }
    }

    p->d.next = new_chunk;
    return m;
}

ngx_pool_cleanup_t * MemoryPool::ngx_pool_cleanup_add(const size_t size) {
    auto *c = static_cast<ngx_pool_cleanup_t *>(ngx_palloc(sizeof(ngx_pool_cleanup_t)));
    if (c == nullptr) {
        return nullptr;
    }
    if (size) {
        c->data = ngx_palloc(size);
        if (c->data == nullptr) {
            return nullptr;
        }
    } else {
        c->data = nullptr;
    }
    c->handler = nullptr;
    c->next = pool_->cleanup;
    pool_->cleanup = c;
    return c;
}
