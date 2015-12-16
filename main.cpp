#include <stdio.h>
#include <math.h>
#include <nlopt.hpp>

#include <fstream>
#include <iostream>
#include <map>

using namespace std;
#define N_PARAMS 1

typedef map<unsigned long, unsigned long> count_map_t  ;

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
    Discounter(string method="", bool decreasing=true):n_params(1),xi(Discounter::xi_dhyp1),params(1,1.0){
        if (method=="xi_hyp1"){
            if (decreasing)
                xi=Discounter::xi_dhyp1;
            else
                xi=Discounter::xi_hyp1;

        }else if (method=="xi_hyp"){
             if (decreasing)
                xi=Discounter::xi_dhyp;
            else
                xi=Discounter::xi_hyp;

        }else if (method=="xi_log"){
             if (decreasing)
                xi=Discounter::xi_dlog;
            else
                xi=Discounter::xi_log;
        }else if (method=="xi_log1"){
             if (decreasing)
                xi=Discounter::xi_dlog1;
            else
                xi=Discounter::xi_log1;
        }else if (method=="xi_exp"){
             if (decreasing)
                xi=Discounter::xi_dexp;
            else
                xi=Discounter::xi_exp;
        }else if (method=="xi_exp1"){
             if (decreasing)
                xi=Discounter::xi_dexp1;
            else
                xi=Discounter::xi_exp1;
        }


    }
    bool tune(const count_map_t & counts);

    double discount(double x){
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
        likelihood_data_t(Discounter::xi_func_t xi, const count_map_t * counts):xi(xi),counts(counts)
        {

        }
    } ;
    likelihood_data_t likelihood_params;
    unsigned short n_params;


    static double objective(const std::vector<double> &x, std::vector<double> &grad, void *my_func_data)
    {
        likelihood_data_t* data= reinterpret_cast<likelihood_data_t*>(my_func_data);
        double ll=0,
                ll0=0;
        count_map_t::const_iterator it=data->counts->begin();
        double xi_val,
                x_i;
        for(;it!=data->counts->end();++it){
            if(it->first<=1)
                continue;
            x_i=it->first-1;
            xi_val=data->xi(x,x_i);
    //		cout<<"Xi-val= "<<xi_val<<endl;
    //                if (isnan(xi_val) || isinf(xi_val)){
    //                    return -1e100;
    //                }
            ll+=it->second*it->first*log(xi_val);
            ll0+=it->second*x_i*(1-xi_val);
        }
        return ll+(data->counts->find(1)->second)*log(ll0);

    }


public:
    Likelihood_maximizer(Discounter::xi_func_t xi, unsigned short n_params, const count_map_t & counts):likelihood_params(xi,&counts),n_params(n_params)
    {

    }
    bool maximize(Discounter::param_array_t & p, double & best_val)
    {


        nlopt::opt opt(nlopt::LN_COBYLA, n_params);

        vector<double> lb(n_params,0);

        opt.set_lower_bounds(lb);
//        likelihood_data_t d={xi, counts};
        opt.set_max_objective(Likelihood_maximizer::objective, &likelihood_params);

        /*my_constraint_data data[2] = { {2,0}, {-1,1} };
        opt.add_inequality_constraint(myvconstraint, &data[0], 1e-8);
        opt.add_inequality_constraint(myvconstraint, &data[1], 1e-8);*/

        opt.set_xtol_rel(1e-10);



//        double minf;
        return opt.optimize(p, best_val);

//        if (result>0) {
//            return true;
//            printf("nlopt failed!\n");
//        }
//        else {
//            return false;
//            printf("found max at f(%g) = %0.10g\n", x[0], minf);
//        }


    }




};

bool Discounter::tune(const count_map_t & counts)
{

    Likelihood_maximizer maximizer(xi, n_params, counts);
    double best;
    maximizer.maximize(params,best);
    cout<<"Best params: "<<params[0]<<", Best value: "<< best<<endl;

}


int main(int argc, char **argv)
{


	ifstream fin("counts.1");
    string line;
	count_map_t xini;

    if(!fin){
        return false;
    }

	unsigned long val;


	vector<pair<int,int> > v;
	v.push_back(pair<int,int>(1,20));
	v.push_back(pair<int,int>(2,30));
	v.push_back(pair<int,int>(3,40));
	v.push_back(pair<int,int>(4,50));
	cout << "@0="<<*((int *)&v.front())<<"; @1="<<*((int *)&v.front()+1)<<"@2="<<*((int *)&v.front()+2)<<"; @3="<<*((int *)&v.front()+3)<<endl;


    while (fin >>val) {
//		cout<< "Val= "<<val<<endl;
		xini[val]+=1;

	}
	count_map_t::const_iterator it=xini.begin();

	for(int i=0;i<10;i++,++it){
		cout<<"Occurrnces of "<<it->first<<" = "<<it->second<<endl;
	}

	Discounter d;
	cout<<"Tuning to data"<<endl;
	d.tune(xini);
//
//	nlopt::opt opt(nlopt::LN_COBYLA, N_PARAMS);
//
//	vector<double> lb;
//	for(int i=0;i<N_PARAMS;++i)
//		lb.push_back(0);
//
//
//	opt.set_lower_bounds(lb);
//	likelihood_data_t d={xi_exp1, &xini};
//	opt.set_max_objective(myvfunc, &d);
//
//	/*my_constraint_data data[2] = { {2,0}, {-1,1} };
//	opt.add_inequality_constraint(myvconstraint, &data[0], 1e-8);
//	opt.add_inequality_constraint(myvconstraint, &data[1], 1e-8);*/
//
//	opt.set_xtol_rel(1e-10);
//
//	std::vector<double> x(N_PARAMS,1.0);
//
//	double minf;
//	nlopt::result result = opt.optimize(x, minf);
//
//	if (0) {
//		printf("nlopt failed!\n");
//	}
//	else {
//		printf("found max at f(%g) = %0.10g\n", x[0], minf);
//	}

	return 0;
}
