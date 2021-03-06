#include <stdio.h>
#include <math.h>
#include <nlopt.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <fstream>
#include <iostream>
#include <map>

#include "scorer_types.h"
#ifndef _TDISCOUNTER_H
#define _TDISCOUNTER_H

#define NBR_KN_CONSTS 3

using namespace std;

typedef map<count_t, count_t> count_map_t  ;

class Discounter
{
public:
    typedef std::vector<double> param_array_t ;
    typedef  double (*xi_func_t)(const std::vector<double> &, score_t);
private:
    unsigned short n_params;
    param_array_t params;
    xi_func_t xi;
	
    static inline double xi_kn(const param_array_t &p,score_t x){
            return (x-p[MIN(x,NBR_KN_CONSTS)-1])/x;
    }

    static inline double xi_hyp1(const param_array_t &p,score_t x){
            return 1./(1+p[0]*x);
    }

    static inline double xi_hyp(const param_array_t &p,score_t x){
            return pow(1+x,-p[0]);
    }

    static inline double xi_log(const param_array_t &p,score_t x){
            return pow(1+log(1+x),-p[0]);
    }
    static inline double xi_log1(const param_array_t &p,score_t x){
            return 1./(1+p[0]*log(1+x));
    }

    static inline double xi_exp(const param_array_t &p,score_t x){
            return pow(1+p[0],-x);
    }

    static inline double xi_exp1(const param_array_t &p,score_t x){
            return 2./(1+pow(1+p[0],x));
    }

    static inline double xi_dhyp1(const param_array_t &p,score_t x){
            return 1./(1+p[0]/x);
    }

    static inline double xi_dhyp(const param_array_t &p,score_t x){
            return pow(1+1/x,-p[0]);
    }

    static inline double xi_dlog(const param_array_t &p,score_t x){
            return pow(1+log(1+1/x),-p[0]);
    }
    static inline double xi_dlog1(const param_array_t &p,score_t x){
            return 1./(1+p[0]*log(1+1/x));
    }

    static inline double xi_dexp(const param_array_t &p,score_t x){
            return pow(1+p[0],-1/x);
    }

    static inline double xi_dexp1(const param_array_t &p,score_t x){
            return 2./(1+pow(1+p[0],1/x));
    }

    static inline double xi_none(const param_array_t &p,score_t x){
            return 1;
    }
public:
    Discounter(string ="", bool =true);
    bool tune(const count_map_t & , score_t =-1);
    bool ptune(const count_map_t & , unsigned short,score_t =-1);
    bool ptune_kn(const count_map_t & );

    inline score_t discount(score_t x) const{
        return x*xi(params,x);
    }
    inline score_t discount(score_t x, count_t cooc) const{
        return x*xi(params,cooc);
    }
    inline score_t get_xi_val(score_t x) const{
        return xi(params,x);
    }
    friend ostream & operator << (ostream & , const Discounter& );

};

ostream & operator << (ostream & , const Discounter& );


class Likelihood_maximizer
{

    struct likelihood_data_t{
        Discounter::xi_func_t xi;
        const count_map_t * counts;
        bool loo;
        score_t n0;
        const vector<thread> * workers;
        vector<double> tmp;
        atomic<unsigned> n_comp;
        unsigned n_task;
        condition_variable cv;
//        atomic<unsigned> nstarted;
        mutex cv_m;
        bool done;
//        bool ready;
        likelihood_data_t(Discounter::xi_func_t xi, const count_map_t * counts,score_t n0,bool loo):xi(xi),
            counts(counts),loo(loo),n0(n0),workers(NULL),n_comp(-1u),n_task(0),done(false)//,ready(false)//,tmp(2,0.0)
        {

        }
    } ;
    likelihood_data_t likelihood_params;
    unsigned short n_params;


    inline static double objective(const std::vector<double> &, std::vector<double> &, void *);

    inline static double pobjective(const std::vector<double> &, std::vector<double> &, void *);
    inline static void master_peval_objective(const std::vector<double> &, Likelihood_maximizer::likelihood_data_t* );

    inline static void partialeval_objective(const std::vector<double> &, Likelihood_maximizer::likelihood_data_t* , count_map_t::const_iterator , unsigned long);
    static void teval_obj_wrap(const std::vector<double> *, Likelihood_maximizer::likelihood_data_t*,unsigned short,unsigned=0, unsigned long=-1 );



public:
    Likelihood_maximizer(Discounter::xi_func_t xi, unsigned short n_params, const count_map_t & counts,double n0, bool loo=true):likelihood_params(xi,&counts,n0,loo),n_params(n_params)
    {

    }
    bool maximize(Discounter::param_array_t & , double & );

    bool pmaximize(Discounter::param_array_t & , double & );
    void set_workers(const vector<thread> * ws)
    {
        likelihood_params.workers=ws;
    }


    friend class Discounter;

};


#endif // _TDISCOUNTER_H
