#ifndef _CORO_H_
#define _CORO_H_

typedef void (*CoroFunType)(const void*);

void coro_new(CoroFunType f, const void* arg);
void coro_yield();
void coro_main();

#endif
