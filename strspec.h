#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <vector>
#include "scorer_types.h"

#ifndef _STRSPEC_H
#define _STRSPEC_H
typedef vector<string> str_vec;

void tokenize_in_vec(char * ,str_vec * );

void flip_around1(char * str,char c,int =-1,int =-1);

void flip_around(char * str,char c,unsigned =0);


void get_alignments(char * ,aligns * );
        
void invert_alignment(char * );

void reverse_chars(char * str,int =-1,int =-1);
inline const char * first_part(const char * pair)
{
    return pair;
}
inline const char * last_part(const char * pair)
{
    return pair+strlen(pair)+1;
}

inline void break_pair(char * pair)
{
//    #pragma omp critical(output)
//         {
//            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread: "<<omp_get_thread_num()<<" breaking: '"<<pair<<"'"<<endl;
//         }
    *strchr(pair,STR_SEP)='\0';
//    #pragma omp critical(output)
//         {
//            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread: "<<omp_get_thread_num()<<" brook"<<endl;
//         }
}
#endif
