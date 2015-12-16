/*
 * File:   score.h
 * Author: mmediani
 *
 * Created on February 21, 2011, 11:09 AM
 */
//#include "global.h"
#include <queue>
#include <algorithm>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <map>
#include "scorer_types.h"
#include "processalign.h"

#ifndef _SCORE_H
#define	_SCORE_H

#define TRANSFER 15
#define MIN_TR_TAG 100

typedef pair<score_t,align_info> lex_align_pair_t;

typedef vector<lex_align_pair_t> lex_align_vec_t;


struct Smoothing_att_calculator
{
	typedef count_t (*accumulator_func)(count_t , count_t, score_t);
	
	accumulator_func accumulator;
	
	Smoothing_att_calculator(string smoothing=""):accumulator(Smoothing_att_calculator::lower_order_accumulator)
	{
		if (smoothing=="unigram" || smoothing == "g"){
			accumulator=Smoothing_att_calculator::unigram_accumulator;
		}
		if (smoothing=="aunigram" || smoothing == "ag"){
			accumulator=Smoothing_att_calculator::aunigram_accumulator;
		}
	}
	
	static inline count_t unigram_accumulator(count_t sum, count_t uniq,score_t s)
	{
		return sum;
	}
	
	static inline count_t lower_order_accumulator(count_t sum,count_t uniq,score_t s)
	{
		return uniq;
	}
	
	static inline count_t aunigram_accumulator(count_t sum, count_t uniq, score_t asum)
	{
		return asum;
	}
	
};

struct Lexical_aggregator
{
	typedef lex_align_pair_t (*aggregator_t)(const lex_align_vec_t &);
	
	aggregator_t aggregate;

    Lexical_aggregator(string aggregation="max"):aggregate(NULL)
    {
        if(aggregation=="max" || aggregation=="m"){
            aggregate=Lexical_aggregator::max;
        }else if (aggregation=="average" || aggregation== "a"){
            aggregate=Lexical_aggregator::average;
        }else if (aggregation=="max-occur" || aggregation=="o"){
			aggregate=Lexical_aggregator::max_occur;
        }

    }

		
	static inline  lex_align_pair_t max(const lex_align_vec_t & vec)
	{
		lex_align_vec_t::const_iterator max_ret=vec.begin();
		for(lex_align_vec_t::const_iterator i=vec.begin()+1; i!=vec.end(); ++i)
		{
			if(i->first>max_ret->first)
				max_ret=i;
		}
		return *max_ret;
	}
	static inline lex_align_pair_t average(const lex_align_vec_t & vec)
	{
		score_t avg=vec.begin()->first;
		lex_align_vec_t::const_iterator lind=vec.begin();    
		for(lex_align_vec_t::const_iterator i=vec.begin()+1; i!=vec.end(); ++i){
			avg+=i->first;
			if(lind->second.nb_points<i->second.nb_points)
				lind=i;
		}

		
		return lex_align_pair_t(avg/vec.size(),lind->second);
	}

	static inline lex_align_pair_t max_occur(const lex_align_vec_t & vec)
	{
		typedef map<packed_align_t,unsigned> freqs_t;
		freqs_t freqs;
		
		unsigned max_f=0;
		

		for(lex_align_vec_t::const_iterator i=vec.begin(); i!=vec.end(); ++i)
		{
			freqs[i->second.alignment]+=1;
			if(max_f<freqs[i->second.alignment])
				max_f=freqs[i->second.alignment];
	//         if(!freqs.insert(pair<score_t,unsigned>(*i,0)).second)
	//             freqs[*i]+=1;
		}
		lex_align_vec_t::const_iterator  ret=vec.begin();
		score_t max_s=0;
		
		for(lex_align_vec_t::const_iterator i=vec.begin()+1; i!=vec.end(); ++i)
			if(freqs[i->second.alignment]==max_f)
				if(max_s<i->first){
					max_s=i->first;
					ret=i;
				}

		return *ret;
	}


};


struct proc_queue{
    queue<int> ranks;
    pthread_mutex_t pq_lock;
    int num_procs;
    proc_queue():
            num_procs(0)
    {
        pthread_mutex_init(&pq_lock, NULL);
    }
    ~proc_queue()
    {
        pthread_mutex_destroy(&pq_lock);
    }

};

struct thread_args{
    //synchronization
    pthread_mutex_t mutex;
    pthread_mutex_t *gtag_mutex;
    pthread_cond_t  cond;
    bool waiting;
    bool done;
    proc_queue * ready;
    int tag;
    int * glob_tag;

    int dest_proc;

    //data
    void * data;

    thread_args():
    data(NULL),waiting(false),
    dest_proc(-1)
    ,tag(-1),glob_tag(NULL),gtag_mutex(NULL)
    ,done(false)
    {
        pthread_mutex_init(&mutex, NULL);
//        pthread_mutex_init(&gtag_mutex, NULL);
        pthread_cond_init (&cond, NULL);
    }
    ~thread_args()
    {
        pthread_mutex_destroy(&mutex);
//        pthread_mutex_destroy(&gtag_mutex);
        pthread_cond_destroy(&cond);
    }
};
//
//void score_one_source(element_vector *,element_vector *,element_vector::const_iterator *, aggregation_func=NULL);
//void score_one_source(element_vector *,element_vector *,unsigned long * ,unsigned long * ,
//                        unsigned long * ,unsigned long * ,unsigned long * ,
//                        element_vector::const_iterator *, string );


typedef map<count_t, count_t> count_map_t  ;
void copy_to_global_and_count(local_vector * , element_vector * , count_map_t * , omp_lock_t * );
unsigned long score(element_vector ** ,global_t* ,const Options * );
void do_sort(element_vector * ,element_vector *,unsigned long =MEM_BLOCK );
#endif	/* _SCORE_H */

