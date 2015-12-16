/*
 * File:   out.h
 * Author: mmediani
 *
 * Created on February 22, 2011, 2:48 PM
 */
#include <fcntl.h>
#include "scorer_types.h"
#include "strspec.h"

//#include "global.h"
#ifndef _OUT_H
#define	_OUT_H
//#ifdef File
//#undef  File
//#endif
#define MOSES_SEP " ||| "
#define STTK_SEP " # "

#define SHARE_SIZE 5000
#define PRECISION 20
#define PENALTY exp(1)
//typedef MPI::File FileMPI ;



struct Prob_calculator
{

    typedef  score_t (*smoothing_func)(const pair_t * p,const global_t * g);
    smoothing_func back_prob,
                    for_prob;

    Prob_calculator(string smoothing="lower-order")
    {
        if(smoothing=="lower-order" || smoothing=="l"){
            back_prob=Prob_calculator::back_prob_lorder;
            for_prob=Prob_calculator::for_prob_lorder;
        }else if (smoothing=="uniform" || smoothing == "f"){
//             cout<< "######################### UNIFORM ############"<<endl;
            back_prob=Prob_calculator::back_prob_uniform;
            for_prob=Prob_calculator::for_prob_uniform;
        }else if (smoothing=="unigram" || smoothing == "g"){
//             cout<< "######################### UNIFORM ############"<<endl;
            back_prob=Prob_calculator::back_prob_unigram;
            for_prob=Prob_calculator::for_prob_unigram;
        }else if (smoothing=="aunigram" || smoothing == "ag"){
//             cout<< "######################### UNIFORM ############"<<endl;
            back_prob=Prob_calculator::back_prob_aunigram;
            for_prob=Prob_calculator::for_prob_aunigram;
        }else if (smoothing=="none" || smoothing == "n"){
            back_prob=Prob_calculator::back_prob_none;
            for_prob=Prob_calculator::for_prob_none;
        }

    }


    static inline score_t back_prob_lorder(const pair_t * p,const global_t * g)
    {/*
        if(p->back_score+(p->sum_t*p->us)/g->un > 1)
        {
            string out="BACK_SCOR="+to_string(p->back_score)+"; SUM_T="+to_string(p->sum_t)+"; US="+to_string(p->us)+"; UN="+to_string(g->un)+"\n";
#pragma omp sritical(output)
            {
                cout<<out;
            }
        }*/
        return p->back_score+(p->sum_t*p->us)/g->un;

    }
    static inline score_t for_prob_lorder(const pair_t * p,const global_t * g)
    {
        return p->for_score+(p->sum_s*p->ut)/g->un;
    }

    static inline score_t back_prob_unigram(const pair_t * p,const global_t * g)
    {/*
        if(p->back_score+(p->sum_t*p->us)/g->un > 1)
        {
            string out="BACK_SCOR="+to_string(p->back_score)+"; SUM_T="+to_string(p->sum_t)+"; US="+to_string(p->us)+"; UN="+to_string(g->un)+"\n";
#pragma omp sritical(output)
            {
                cout<<out;
            }
        }*/
        return p->back_score+(p->sum_t*p->us)/g->n;

    }
    static inline score_t for_prob_unigram(const pair_t * p,const global_t * g)
    {
        return p->for_score+(p->sum_s*p->ut)/g->n;
    }

    static inline score_t back_prob_aunigram(const pair_t * p,const global_t * g)
    {/*
        if(p->back_score+(p->sum_t*p->us)/g->un > 1)
        {
            string out="BACK_SCOR="+to_string(p->back_score)+"; SUM_T="+to_string(p->sum_t)+"; US="+to_string(p->us)+"; UN="+to_string(g->un)+"\n";
#pragma omp sritical(output)
            {
                cout<<out;
            }
        }*/
        return p->back_score+(p->sum_t*p->us)/g->sum;

    }
    static inline score_t for_prob_aunigram(const pair_t * p,const global_t * g)
    {
        return p->for_score+(p->sum_s*p->ut)/g->sum;
    }
    
    static inline score_t back_prob_uniform(const pair_t * p,const global_t * g)
    {
        return p->back_score+p->sum_t/g->us;
    }
    static inline score_t for_prob_uniform(const pair_t * p,const global_t * g)
    {

        return p->for_score+p->sum_s/g->ut;

    }

    static inline score_t back_prob_none(const pair_t * p,const global_t * g)
    {
        return p->back_score;
    }
    static inline score_t for_prob_none(const pair_t * p,const global_t * g)
    {

        return p->for_score;

    }

};

struct Entry_fomatter
{

	typedef  void (*format_func)(pair_t * ,const global_t * ,const Prob_calculator * ,ostringstream *);
    
	format_func format;
    unsigned add_per_entry;

    Entry_fomatter(string formatting="sttk")
	{
	    if(formatting=="sttk"){
	        format=Entry_fomatter::format_sttk;
            add_per_entry=3+5*strlen(STTK_SEP)+4*(6+PRECISION+1);//assuming exponent is in two digits, 4 probabilities,+spaces after each score

	    }else if (formatting=="moses"){
	        format=Entry_fomatter::format_moses;
            add_per_entry=4*strlen(MOSES_SEP)+5*(6+PRECISION+1) ; 
	    }

	}


	static inline void format_sttk(pair_t * p,const global_t * g, const Prob_calculator * pc, ostringstream * ret)
	{
//         ostringstream ret;

//        cooc_type::key_t key;
//        strcpy(key,cooc._key);
        break_pair(p->_key);
         
        *ret<<setprecision(PRECISION)<<scientific<<
                        //Phrases
                "@PT"<< STTK_SEP<<first_part(p->_key)<<
                        STTK_SEP<<last_part(p->_key)<<
                        //scores
                        STTK_SEP<<pc->back_prob(p,g)<<" "<<p->back_lex_score<<
                             " "<<pc->for_prob(p,g)<<" "<<p->for_lex_score<<                             
                        //alignments
                        STTK_SEP<<p->align.source_align()<<
                        STTK_SEP<<p->align.target_align()<<
                        
                        endl;


    //    ret.str(string());
//         return ret.str();

	}

	static inline void format_moses(pair_t * p,const global_t * g, const Prob_calculator * pc,ostringstream * ret)
	{
//         ostringstream ret;

//        pair_t::key_t key;
    //    strcpy(key,cooc->_key);
        break_pair(p->_key);
        *ret<<setprecision(PRECISION)<<scientific<<
        first_part(p->_key)<<
                        MOSES_SEP<<last_part(p->_key)<<
                        MOSES_SEP<<p->align.source_align()<<
                        MOSES_SEP<<p->align.target_align()<<
                        MOSES_SEP<< pc->back_prob(p,g)<<" "<<p->back_lex_score<<
                              " "<<pc->for_prob(p,g)<<" "<<p->for_lex_score<<" "
                                 <<PENALTY<<endl;
//         return ret.str();

	}

};

//string format_entry(pair_t,global_t * gs, string ="sttk");
//void save_to_file(File,unsigned long ,element_vector * ,global_t* , string ="sttk");
void psave_to_file(string,unsigned long ,element_vector * ,global_t* , const Options *);

typedef  std::vector<value_type> local_vector;

// #define File FILE *
#endif	/* _OUT_H */

