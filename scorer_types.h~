/*
 * File:   scorer_types.h
 * Author: mmediani
 *
 * Created on February 10, 2011, 2:06 PM
 */

#include <mpi.h>
#include <iostream>
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <string.h>
#include <stxxl/io>
#include <stxxl/mng>
#include <stxxl/ksort>
#include <stxxl/sort>
#include <stxxl/vector>
#include <string>
#include <string.h>
#include <map>
#include <climits>
//#include <zlib.h>

#include <demsort/distributed_external_sort.h>

#include "processalign.h"


#ifndef _SCORER_TYPES_H
#define	_SCORER_TYPES_H



//using std::cout;
//using std::endl;
//using std::cerr;

#define MEM_BLOCK (1024ull*1024*1024)
#define MAX_THREAD_MEM (64ul*1024*1024)
//(512 * 1024 * 1024)
#define DEFAULT_MEM_BLOCK 1024
#define MAXL 1000
#define FIELD_SEP " ||| "
#define ENV_DIR "maps"
#define TASK_DONE "#"
#define MAX_ALIGNMENTS 8
#define UNALIGNED "NULL"
#define STR_SEP  '\x01'
//#define STTK_SEP  "#"
#define DEFAULT_LEX "model/lex.0-0.e2f"
#define DEFAULT_REVLEX "model/lex.0-0.f2e"
#define DEFAULT_EXTRACT "model/extract.0-0"
#define DEFAULT_STAT "phrase-table.0-0.stat.half.f2e"
#define DEFAULT_TMP "/tmp"
#define DEFAULT_TMP_SIZE 0
#define STXXL_EXT ".stxxl"
#define STXXL_CFG_ENV_VAR "STXXLCFG"
#define STXXL_LOG_ENV_VAR "STXXLLOGFILE"
#define STXXL_ERR_ENV_VAR "STXXLERRLOGFILE"

#define STXXL_CFG_EXT ".cfg"
#define STXXL_LOG_EXT ".log"
#define STXXL_ERR_EXT ".err"

#define PATH_SEP '/'
#define MIN(X,Y) ((X)<=(Y)?(X):(Y))
#define MAX(X,Y) ((Y)<=(X)?(X):(Y))
#define NB_WORKERS omp_get_max_threads()
//omp_get_num_procs()
#define REAL_ZERO 1e-100
#define FACTOR 512
//#define MAX_STR_LEN 256
#ifndef MAX_STR
	#define MAX_STR 250
#endif

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif
#define BUF_SIZE (2*1024*1024ull)
#define EMPTY_RESIDUE 1
#define NON_EMPTY_RESIDUE 2
#define NON_EMPTY_BORDER_KEYS 3
#define EMPTY_BORDER_KEYS 4

#define LINE_SEP '\n'



#define ROOT_MSG if(MPI::COMM_WORLD.Get_rank()==0)

using namespace std;



//	typedef map<string, long > PhraseMap;
//	typedef map<long, string > PhraseIdMap;
typedef map<string,double > LexMap;
//	#define RegisterMap(map)


#ifdef USE_GZIP
    #undef fopen
    #undef fgets
    #undef fclose
    #undef fputs
    #undef fprintf
    #undef feof
    #undef FILE
    #undef fflush
    #undef fread
    #undef fwrite
    #undef fseek
    #undef ftell

    #define File gzFile
    #define fopen gzopen
    #define fdopen gzdopen
    #define fgets(X,Y,Z) gzgets(Z,X,Y)
    #define fclose gzclose
    #define fputs(X,Y) gzputs(Y,X)
    #define fprintf gzprintf
    #define feof gzeof
    #define fflush(X) gzflush(X, Z_SYNC_FLUSH)
    #define fread(W,X,Y,Z) gzread(Z,W,X*Y)
    #define fwrite(W,X,Y,Z) gzwrite(Z,W,X*Y)
    #define fseek gzseek
    #define ftell gztell
#else
    #define File FILE *


#endif


typedef unsigned long count_t;
typedef double score_t;
#define ZERO_SCORE numeric_limits<double>::epsilon()
typedef vector<string> str_vec_t;
#define COUNT_MAX ULONG_MAX
#define COUNT_MPI_TYPE MPI::UNSIGNED_LONG
#define SCORE_MPI_TYPE MPI::DOUBLE

struct global_t
{
	count_t 	n,//number of all extracted phrases
					un,//number of unique extracted phrases
					us,//number of unique source phrases
					ut;//number of unique target phrases
	score_t sum;
	global_t(count_t n=0,
                           count_t un=0,
                           count_t us=0,
                           count_t ut=0,
						   score_t sum=0
                          ):n(n),un(un),us(us),ut(ut),sum(sum)
    {
    }

};

struct pair_t
{
    typedef char key_t[MAX_STR];
    key_t _key;

	count_t cooc;
	score_t us,
            ut;

    score_t sum_s,
            sum_t;

	score_t for_lex_score,back_lex_score;
	score_t for_score,back_score;
	align_info align;

    char * key() //const
    {
        return _key; //src_id;
    }

	pair_t():cooc(0),sum_s(0.0),sum_t(0.0),us(0.),ut(0.),for_lex_score(0.0),back_lex_score(0.0),for_score(0.0),back_score(0.0){
		*_key='\0';
	}
	pair_t( const char * pair)://src_id(sid),_trgt_id(tid),
		cooc(0),sum_s(0.),sum_t(0.),us(0.),ut(0.),for_lex_score(0.0),back_lex_score(0.0),for_score(0.0),back_score(0.0){
			strcpy(_key,pair);
	}
	pair_t( const char * pair,score_t src_score,score_t tr_score,align_info * al_inf)://_src_id(sid),_trgt_id(tid),
		cooc(1),sum_s(1),sum_t(1),us(1),ut(1),for_lex_score(src_score),back_lex_score(tr_score),for_score(0.0),back_score(0.0){
			strcpy(_key,pair);
			if(al_inf)
				memcpy(&align,al_inf,sizeof(align_info));
			else
				align=align_info();
	}

	pair_t(const pair_t  & other)://_src_id(other._src_id),_trgt_id(other._trgt_id),
		cooc(other.cooc),sum_s(other.sum_s),sum_t(other.sum_t),us(other.us),ut(other.ut),for_lex_score(other.for_lex_score),back_lex_score(other.back_lex_score),
		for_score(other.for_score),back_score(other.back_score){
//         cout<<"Calling default constructor, Other="<<other._key<<endl;
			strcpy(_key,other._key);
//         cout<<"Leaving default constructor, this="<<_key<<endl;
			memcpy(&align,&other.align,sizeof(align_info));
	}

    static pair_t min_value()
    {
        pair_t m;
        memset(m._key,0,sizeof m._key);
        return m;
    }
    static pair_t max_value()
    {
        pair_t m;
        memset(m._key,0xff,sizeof m._key);
        m._key[sizeof(m._key)-1]='\x0';
        return m;
    }

	bool operator ==(const pair_t & other) const
	{
		return !strcmp(_key,other._key); //(_src_id==other._src_id) && (_trgt_id==other._trgt_id);}
	}
	bool operator !=(const pair_t& other) const
	{
		return strcmp(_key,other._key); //(_src_id!=other._src_id) || (_trgt_id!=other._trgt_id);}
	}

};
/*
struct count_counts_t
{
    pair<count_t, count_t> _c;
    count_counts_t(count_t k): _c(k,1)
    {

    }
    count_counts_t(count_t k,count_t cc): _c(k,cc)
    {

    }
    count_counts_t(const count_counts_t & other): _c(other._c)
    {

    }

    static count_counts_t min_value()
    {
        return count_counts_t(0,0);
    }
    static count_counts_t max_value()
    {
        return count_counts_t(COUNT_MAX,COUNT_MAX);
    }

	bool operator ==(const count_counts_t& other) const
	{
		return other._c ==_c;
	}
	bool operator !=(const count_counts_t& other) const
	{
		return other._c != _c;
	}

};*/

struct key_count_t
{
    pair_t::key_t key;
    count_t count;
    key_count_t(const pair_t & c):
                    count(0)
    {
        strcpy(key,c._key);
    }
};

bool operator < (const pair_t & a,const pair_t & b);
// bool operator < (const count_counts_t & a,const count_counts_t & b);
//
//bool cmp_by_key(const cooc_type & a, const cooc_type & b)
//{
//	if(a.key()==b.key())
//		return a.val() < b.val();
//	return a.key()<b.key();
//}
//
//bool cmp_by_val(const cooc_type & a, const cooc_type & b)
//{
//	if(a.val()==b.val())
//		return a.key() < b.key();
//	return a.val()<b.val();
//}

struct CmpKey: public std::binary_function<pair_t, pair_t, bool>
{
    bool operator () (const pair_t& a, const pair_t& b) const
    {
		return strcmp(a._key,b._key)<0;//a.key() < b.key();
    }
    static pair_t min_value()
    {
        return pair_t::min_value();
    }
    static pair_t max_value()
    {
        return pair_t::max_value();
    }

    bool is_forbidden(const pair_t& key) const
    {
        return (key == pair_t::min_value() || key == pair_t::max_value());
    }
};

/*
struct CmpCount: public std::binary_function<count_counts_t, count_counts_t, bool>
{
    bool operator () (const count_counts_t& a, const count_counts_t& b) const
    {
		return  a._c.first < b._c.first;
    }
    static count_counts_t min_value()
    {
        return count_counts_t::min_value();
    }
    static count_counts_t max_value()
    {
        return count_counts_t::max_value();
    }

    bool is_forbidden(const count_counts_t & key) const
    {
        return (key == count_counts_t::min_value() || key == count_counts_t::max_value());
    }
};*/



std::ostream & operator << (std::ostream & o, const pair_t& obj);

const global_ext_element_index block_size_bytes = sizeof(pair_t) * 4096 * 12;

// const global_ext_element_index block_size_count_bytes = sizeof(count_counts_t) * 4096 * 12;


typedef pair_t value_type;
typedef stxxl::VECTOR_GENERATOR<value_type, 1, 1, block_size_bytes, stxxl::striping>::result element_vector;

typedef  std::vector<value_type> local_vector;

// typedef stxxl::VECTOR_GENERATOR<count_counts_t, 1, 1, block_size_count_bytes, stxxl::striping>::result count_counts_vector;


class VectorOverloaded:public exception
{
	virtual const char* what() const throw()
	{
		return "A thread vector reached a critical size";
	}

};


struct Options
{
	string output_fmt,
			smoothing_dist,
			xi_func,
			lex_aggergator,
			lex_scoring,
            unseen_estimator,
			assoc_method;
	bool smoothed_lex;
	unsigned long mem;
	Options(string output_fmt="moses",
			string smoothing_dist="lower-order",
			string xi_func="kn",
			string lex_aggergator="max",
			string lex_scoring="average",
            string unseen_estimator="",
			string assoc_method="cooc",
			bool smoothed_lex=false,
			unsigned long mem=MEM_BLOCK
		   ):output_fmt(output_fmt),smoothing_dist(smoothing_dist),xi_func(xi_func),lex_aggergator(lex_aggergator),
		   lex_scoring(lex_scoring),
		   unseen_estimator(unseen_estimator),
		   smoothed_lex(smoothed_lex),
		   assoc_method(assoc_method),
		   mem(mem)
		   
	{
	}
};

#endif	/* _SCORER_TYPES_H */

