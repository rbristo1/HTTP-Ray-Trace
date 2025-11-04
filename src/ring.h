/**
 * @file rbuf.h
 * @author Stephen Marz (sgm@utk.edu)
 * @brief Ring buffer prototypes.
 * @version 0.1
 * @date 2023-05-07
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once

#include <stdbool.h>
#include <stdlib.h>

struct RingBuffer;
typedef struct RingBuffer RingBuffer;

RingBuffer *rb_new(size_t capacity);
void rb_free(RingBuffer *rb);

// Ring buffer components
size_t  rb_at(const RingBuffer *rb);
size_t  rb_size(const RingBuffer *rb);
size_t  rb_capacity(const RingBuffer *rb);
bool    rb_push(RingBuffer *rb, char data);
bool    rb_pop(RingBuffer *rb, char *data);
char    rb_peek(const RingBuffer *rb);
void    rb_ignore(RingBuffer *rb, size_t num);
void    rb_clear(RingBuffer *rb);
size_t  rb_read(RingBuffer *rb, char *buf, size_t max_bytes);
size_t  rb_write(RingBuffer *rb, const char *buf, size_t bytes);

