#include <cstdio>
#include <iomanip>
#include <vector>
#include <sstream>
#include <stdarg.h>

#include <stxxl/io>
#include <stxxl/aligned_alloc>
#include <unistd.h>
#include "scorer_types.h"

#ifndef _PREPARE_SPACE_H
#define _PREPARE_SPACE_H

#ifdef BLOCK_ALIGN
 #undef BLOCK_ALIGN
#endif

#define BLOCK_ALIGN  4096

#define NOREAD

//#define DO_ONLY_READ

#define POLL_DELAY 1000

#define RAW_ACCESS

//#define WATCH_TIMES

#define CHECK_AFTER_READ 0
string tmp_name(string =".",bool =true);
void remove_tmp(const str_vec_t &);//(const char * );
void prepare(stxxl::int64,const str_vec_t &);//const char *);
void create_space(stxxl::int64 , const char * ,... );

#endif
