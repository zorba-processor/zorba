// malloc_count: counts the malloc-family calls of a process and the bytes they
// request, and prints the totals to stderr at exit. glibc only. Build and use:
//   cc -O2 -shared -fPIC -o malloc_count.so malloc_count.c
//   LD_PRELOAD=./malloc_count.so zorba ...
// Prints one line:
//   MALLOC_COUNT malloc=M calloc=C realloc=R free=F bytes=B sizes=H16/H32/.../Hbig
// where H16 counts requests of at most 16 bytes, H32 of 17 to 32 bytes, etc.
#define _GNU_SOURCE
#include <stddef.h>
#include <stdio.h>

extern void *__libc_malloc( size_t );
extern void *__libc_calloc( size_t, size_t );
extern void *__libc_realloc( void*, size_t );
extern void __libc_free( void* );

static unsigned long long n_malloc, n_calloc, n_realloc, n_free, n_bytes;
static unsigned long long sizes[8];     // <=16, <=32, <=64, <=128, <=256, <=1k, <=4k, more

static void count_size( size_t n ) {
  int const i = n <= 16 ? 0 : n <= 32 ? 1 : n <= 64 ? 2 : n <= 128 ? 3 :
                n <= 256 ? 4 : n <= 1024 ? 5 : n <= 4096 ? 6 : 7;
  ++sizes[i];
  n_bytes += n;
}

void* malloc( size_t n ) {
  ++n_malloc;
  count_size( n );
  return __libc_malloc( n );
}

void* calloc( size_t m, size_t n ) {
  ++n_calloc;
  count_size( m * n );
  return __libc_calloc( m, n );
}

void* realloc( void *p, size_t n ) {
  ++n_realloc;
  count_size( n );
  return __libc_realloc( p, n );
}

void free( void *p ) {
  if ( p )
    ++n_free;
  __libc_free( p );
}

__attribute__((destructor)) static void report( void ) {
  char buf[512];
  int const len = snprintf( buf, sizeof buf,
    "MALLOC_COUNT malloc=%llu calloc=%llu realloc=%llu free=%llu bytes=%llu "
    "sizes=%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu\n",
    n_malloc, n_calloc, n_realloc, n_free, n_bytes,
    sizes[0], sizes[1], sizes[2], sizes[3], sizes[4], sizes[5], sizes[6], sizes[7] );
  if ( len > 0 )
    fwrite( buf, 1, (size_t)len, stderr );
}
