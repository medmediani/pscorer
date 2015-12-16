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

#ifndef _DISCOUNTER_H
#define _DISCOUNTER_H


using namespace std;

typedef map<unsigned long, unsigned long> count_map_t  ;

typedef vector<pair<unsigned long, unsigned long> > count_vec_t  ;

class Discounter
{
public:
    typedef std::vector<double> param_array_t ;
    typedef  double (*xi_func_t)(const std::vector<double> &, double);
private:
    unsigned short n_params;
    param_array_t params;
    xi_func_t xi;

    static inline double xi_hyp1(const param_array_t &p,double x){
            return 1./(1+p[0]*x);
    }

    static inline double xi_hyp(const param_array_t &p,double x){
            return pow(1+x,-p[0]);
    }

    static inline double xi_log(const param_array_t &p,double x){
            return pow(1+log(1+x),-p[0]);
    }
    static inline double xi_log1(const param_array_t &p,double x){
            return 1./(1+p[0]*log(1+x));
    }

    static inline double xi_exp(const param_array_t &p,double x){
            return pow(1+p[0],-x);
    }

    static inline double xi_exp1(const param_array_t &p,double x){
            return 2./(1+pow(1+p[0],x));
    }

    static inline double xi_dhyp1(const param_array_t &p,double x){
            return 1./(1+p[0]/x);
    }

    static inline double xi_dhyp(const param_array_t &p,double x){
            return pow(1+1/x,-p[0]);
    }

    static inline double xi_dlog(const param_array_t &p,double x){
            return pow(1+log(1+1/x),-p[0]);
    }
    static inline double xi_dlog1(const param_array_t &p,double x){
            return 1./(1+p[0]*log(1+1/x));
    }

    static inline double xi_dexp(const param_array_t &p,double x){
            return pow(1+p[0],-1/x);
    }

    static inline double xi_dexp1(const param_array_t &p,double x){
            return 2./(1+pow(1+p[0],1/x));
    }

public:
    Discounter(string ="", bool =true);
    bool tune(const count_map_t & , double =-1);
    bool ptune(const count_map_t & , double =-1);
    bool ttune(const count_map_t & , unsigned short,double =-1);

    inline double discount(double x){
        return x*xi(params,x);
    }
    inline double get_xi_val(double x){
        return xi(params,x);
    }

};



class Likelihood_maximizer
{

    struct likelihood_data_t{
        Discounter::xi_func_t xi;
        const count_map_t * counts;
        bool loo;
        double n0;
        const vector<thread> * workers;
        vector<double> tmp;
        atomic<unsigned> n_comp;
        unsigned n_task;
        condition_variable cv;
//        atomic<unsigned> nstarted;
        mutex cv_m;
        bool done;
//        bool ready;
        likelihood_data_t(Discounter::xi_func_t xi, const count_map_t * counts,double n0,bool loo):xi(xi),
            counts(counts),loo(loo),n0(n0),workers(NULL),n_comp(-1u),n_task(0),done(false)//,ready(false)//,tmp(2,0.0)
        {

        }
    } ;
    likelihood_data_t likelihood_params;
    unsigned short n_params;


    static double objective(const std::vector<double> &, std::vector<double> &, void *);

    static double pobjective(const std::vector<double> &, std::vector<double> &, void *);
    static double tobjective(const std::vector<double> &, std::vector<double> &, void *);

    static void master_peval_objective(const std::vector<double> &, Likelihood_maximizer::likelihood_data_t* );

    static void partialeval_objective(const std::vector<double> &, Likelihood_maximizer::likelihood_data_t* , count_map_t::const_iterator , unsigned long);
    static void teval_obj_wrap(const std::vector<double> *, Likelihood_maximizer::likelihood_data_t*,unsigned short,unsigned=0, unsigned long=-1 );


public:
    Likelihood_maximizer(Discounter::xi_func_t xi, unsigned short n_params, const count_map_t & counts,double n0, bool loo=true):likelihood_params(xi,&counts,n0,loo),n_params(n_params)
    {

    }
    bool maximize(Discounter::param_array_t & , double & );
    bool pmaximize(Discounter::param_array_t & , double & );

    bool tmaximize(Discounter::param_array_t & , double & );
    void set_workers(const vector<thread> * ws)
    {
        likelihood_params.workers=ws;
    }


    friend class Discounter;

};


#endif // _DISCOUNTER_H
