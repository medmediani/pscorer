#include "discounter.h"
#include <omp.h>
#include <mpi.h>
#include <iomanip>
#include "crono.h"

bool Discounter::tune(const count_map_t & counts, double n0)
{

    Likelihood_maximizer maximizer(xi, n_params, counts, n0<0? counts.find(1)->second:n0, n0<0);
    double best;
    maximizer.maximize(params,best);
    cout<<"Best params: "<<params[0]<<", Best value: "<< best<<endl;

}


bool Discounter::ptune(const count_map_t & counts, double n0)
{

    if(n0<0){
        unsigned long in0,on0;
        count_map_t::const_iterator one=counts.find(1);
        if (one == counts.end())
            in0=0;
        else
            in0=one->second;


        MPI::COMM_WORLD.Reduce(&in0,&on0,1,MPI::UNSIGNED_LONG,MPI::SUM,0);
//        n0=*(unsigned long *)(&n0);
        n0=on0;

    }
//    cout<<endl<<MPI::COMM_WORLD.Get_rank()<<": Current value of n0: "<<n0<<endl;

    Likelihood_maximizer maximizer(xi, n_params, counts, n0);
    if(MPI::COMM_WORLD.Get_rank()==0){
        double best;
        vector<double> done(n_params,-1);
        maximizer.pmaximize(params,best);

        MPI::COMM_WORLD.Bcast(&done.front(), n_params,MPI::DOUBLE, 0) ;
        cout<<"Best params: "<<params[0]<<", Best value: "<< best<<endl;
        return true;
    }else{

        for(;;){


            MPI::COMM_WORLD.Bcast(&params.front(), n_params,MPI::DOUBLE, 0) ;


            if (params[0]<0)
                return true ;
            //compute local objective

            Likelihood_maximizer::peval_objective(params, &maximizer.likelihood_params);

//            cout<<endl<<MPI::COMM_WORLD.Get_rank()<<": ll="<<maximizer.likelihood_params.tmp[0]<<"; ll0="<<maximizer.likelihood_params.tmp[1]<<endl;


            MPI::COMM_WORLD.Reduce(&maximizer.likelihood_params.tmp.front(), NULL, 2, MPI::DOUBLE, MPI::SUM, 0);
        }
    }

}


Discounter::Discounter(string method, bool decreasing):n_params(1),xi(Discounter::xi_dhyp1),params(1,1.0)
{
    if (method=="xi_hyp1")
    {
        if (decreasing)
            xi=Discounter::xi_dhyp1;
        else
            xi=Discounter::xi_hyp1;

    }
    else if (method=="xi_hyp")
    {
        if (decreasing)
            xi=Discounter::xi_dhyp;
        else
            xi=Discounter::xi_hyp;

    }
    else if (method=="xi_log")
    {
        if (decreasing)
            xi=Discounter::xi_dlog;
        else
            xi=Discounter::xi_log;
    }
    else if (method=="xi_log1")
    {
        if (decreasing)
            xi=Discounter::xi_dlog1;
        else
            xi=Discounter::xi_log1;
    }
    else if (method=="xi_exp")
    {
        if (decreasing)
            xi=Discounter::xi_dexp;
        else
            xi=Discounter::xi_exp;
    }
    else if (method=="xi_exp1")
    {
        if (decreasing)
            xi=Discounter::xi_dexp1;
        else
            xi=Discounter::xi_exp1;
    }


}


double Likelihood_maximizer::objective(const std::vector<double> &x, std::vector<double> &grad, void *my_func_data)
{
    likelihood_data_t* data= reinterpret_cast<likelihood_data_t*>(my_func_data);
    double ll=0,
              ll0=0;
    count_map_t::const_iterator it=data->counts->begin();
    double xi_val,
    x_i;
    if(data->loo){
        for(; it!=data->counts->end(); ++it)
        {
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
    }else{

        for(; it!=data->counts->end(); ++it)
        {
            xi_val=data->xi(x,it->first);

            ll+=it->second*it->first*log(xi_val);
            ll0+=it->second*it->first*(1-xi_val);
        }
    }
//    cout<<"Objective="<<ll+(data->n0)*log(ll0)<<endl;
    return ll+(data->n0)*log(ll0);

}

void Likelihood_maximizer::peval_objective(const std::vector<double> &x,  Likelihood_maximizer::likelihood_data_t* data)
{
//    cout<<endl<<MPI::COMM_WORLD.Get_rank()<<": evaluating for params: "<<x[0]<<endl;
    data->tmp.assign(2,0.0);

    count_map_t::const_iterator it=data->counts->begin();
    unsigned long i;
    double xi_val,
            x_i;
    if(data->loo){
//        #pragma omp parallel for
//        for(i=0; i< data->counts->size(); ++i)
        for(; it!=data->counts->end(); ++it)
        {
            if(it->first<=1)
                continue;
            x_i=it->first-1;
            xi_val=data->xi(x,x_i);
            //		cout<<"Xi-val= "<<xi_val<<endl;
            //                if (isnan(xi_val) || isinf(xi_val)){
            //                    return -1e100;
            //                }
            data->tmp[0]+=it->second*it->first*log(xi_val);
            data->tmp[1]+=it->second*x_i*(1-xi_val);
        }
    }else{
//        #pragma omp parallel for
//        for(i=0; i< data->counts->size(); ++i)
        for(; it!=data->counts->end(); ++it)
        {

            xi_val=data->xi(x,it->first);

            data->tmp[0]+=it->second*it->first*log(xi_val);
            data->tmp[1]+=it->second*it->first*(1-xi_val);
        }
    }

//    return ll+(data->n0)*log(ll0);
}

double Likelihood_maximizer::pobjective(const std::vector<double> &x, std::vector<double> &grad, void *my_func_data)
{
    likelihood_data_t* data= reinterpret_cast<likelihood_data_t*>(my_func_data);
    std::vector<double> & x_ref= const_cast<std::vector<double> &>(x);
    MPI::COMM_WORLD.Bcast(&x_ref.front(), x.size(),MPI::DOUBLE, 0) ;

    peval_objective(x,data);
//    cout<<endl<<MPI::COMM_WORLD.Get_rank()<<": objective before reduce: "<<obj<<endl;

//    cout<<endl<<MPI::COMM_WORLD.Get_rank()<<"Before reduce ll="<<data->tmp[0]<<"; ll0="<<(data->tmp[1])<<endl;

    MPI::COMM_WORLD.Reduce(MPI::IN_PLACE, &data->tmp.front(), 2, MPI::DOUBLE, MPI::SUM, 0);
//    cout<<endl<<MPI::COMM_WORLD.Get_rank()<<"after reduce obj="<<data->tmp[0]+data->n0*log(data->tmp[1])<<endl;
//    cout<<endl<<MPI::COMM_WORLD.Get_rank()<<": After reduce ll="<<data->tmp[0]<<"; ll0="<<(data->tmp[1])<<endl;
    return data->tmp[0]+data->n0*log(data->tmp[1]);

}
bool Likelihood_maximizer::maximize(Discounter::param_array_t & p, double & best_val)
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

bool Likelihood_maximizer::pmaximize(Discounter::param_array_t & p, double & best_val)
{


    nlopt::opt opt(nlopt::LN_COBYLA, n_params);

    vector<double> lb(n_params,0);

    opt.set_lower_bounds(lb);
//        likelihood_data_t d={xi, counts};
    opt.set_max_objective(Likelihood_maximizer::pobjective, &likelihood_params);

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
//
//void send_map(unsigned short dest)
//{
//
//}
//void pmerge_count_maps()
//{
//
//}

int main(int argc, char **argv)
{
//
    int rank, size;

    MPI::Init_thread(argc, argv, MPI_THREAD_MULTIPLE);
//    MPI::Init(argc, argv);
    rank = MPI::COMM_WORLD.Get_rank();
    size = MPI::COMM_WORLD.Get_size();

    cout<<setprecision(4)<<scientific<<double(1.2343754)<<endl;

    MPI::Finalize();

    return (EXIT_SUCCESS);

    string fn="c.4."+to_string(rank);//"counts.1";//

	ifstream fin(fn.c_str());
    string line;
	count_map_t xini;

    if(!fin){
        return false;
    }

	unsigned long val;


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
//	Crono timer;
//	timer.start();
	d.ptune(xini);
//	timer.stop();
//	cerr<<MPI::COMM_WORLD.Get_rank()<<": "<<timer.formatted_span()<<endl;
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
    MPI::Finalize();

    return (EXIT_SUCCESS);
}
