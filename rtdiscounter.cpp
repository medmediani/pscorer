#include "rtdiscounter.h"
#include <omp.h>
#include <mpi.h>


bool Discounter::tune(const count_map_t & counts, double n0)
{

    Likelihood_maximizer maximizer(xi, n_params, counts, n0<0? counts.find(1)->second:n0, n0<0);
    double best;
    maximizer.maximize(params,best);
    cout<<"Best params: "<<params[0]<<", Best value: "<< best<<endl;

}
bool Discounter::ttune(const count_map_t & counts, unsigned short nth,double n0)
{

    Likelihood_maximizer maximizer(xi, n_params, counts, n0<0? counts.find(1)->second:n0, n0<0);
    double best;

    unsigned long chunk_size=counts.size()/nth;

    cout<<"Chunk size" <<chunk_size<<endl;


    vector<thread> workers;
//    maximizer.likelihood_params.nstarted=nth-1;
    for(unsigned i=0;i<nth-1;++i)
        workers.push_back(thread(&Likelihood_maximizer::teval_obj_wrap,&params,&maximizer.likelihood_params,nth,i,chunk_size));

//    cout<<MPI::COMM_WORLD.Get_rank()<<":threads started" <<endl;
    maximizer.set_workers(&workers);

    maximizer.tmaximize(params,best);

    cout<<"Best params: "<<params[0]<<", Best value: "<< best<<endl;

    maximizer.likelihood_params.done=true;
    maximizer.likelihood_params.n_task++;

//    maximizer.likelihood_params.ready=true;
    maximizer.likelihood_params.cv.notify_all();

    for(auto &  t : workers){
        t.join();
    }


    cout<<"ALL DONE," <<endl;

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

            Likelihood_maximizer::master_peval_objective(params, &maximizer.likelihood_params);

            cout<<endl<<MPI::COMM_WORLD.Get_rank()<<": ll="<<maximizer.likelihood_params.tmp[0]<<"; ll0="<<maximizer.likelihood_params.tmp[1]<<endl;


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

void Likelihood_maximizer::partialeval_objective(const std::vector<double> &x, Likelihood_maximizer::likelihood_data_t* data,
                                                  count_map_t::const_iterator start, unsigned long n_items)
{
//    cout<<endl<<MPI::COMM_WORLD.Get_rank()<<": evaluating for params: "<<x[0]<<endl


    unsigned long processed=0;
    double xi_val,
            x_i;
    double ll=0,
            ll0=0;
    if(data->loo){
//        #pragma omp parallel for
//        for(i=0; i< data->counts->size(); ++i)
        for(; processed<n_items; ++start,++processed)
        {
            if(start->first<=1)
                continue;
            x_i=start->first-1;
            xi_val=data->xi(x,x_i);
            //		cout<<"Xi-val= "<<xi_val<<endl;
            //                if (isnan(xi_val) || isinf(xi_val)){
            //                    return -1e100;
            //                }
            ll+=start->second*start->first*log(xi_val);
            ll0+=start->second*x_i*(1-xi_val);
        }
    }else{
//        #pragma omp parallel for
//        for(i=0; i< data->counts->size(); ++i)
        for(; processed<n_items; ++start,++processed)
        {

            xi_val=data->xi(x,start->first);

            ll+=start->second*start->first*log(xi_val);
            ll0+=start->second*start->first*(1-xi_val);
        }
    }
    {
        data->cv_m.lock();

        data->tmp[0]+=ll;
        data->tmp[1]+=ll0;
        data->cv_m.unlock();
    }
        data->n_comp++;
//    return ll+(data->n0)*log(ll0);
}

void Likelihood_maximizer::teval_obj_wrap(const std::vector<double> * x, Likelihood_maximizer::likelihood_data_t* data,unsigned short nth,unsigned  myID, unsigned long chunk_s)
{

    //cout<<MPI::COMM_WORLD.Get_rank()<<": Thread: "<<myID <<"Chunk size:"<<chunk_s<<endl;
    count_map_t::const_iterator it=data->counts->begin();
    unsigned long my_start=myID*chunk_s+
            data->counts->size()-
            (data->counts->size()/nth)*
            (nth-1);
    advance(it,my_start);
    unsigned next_task=1;
//    string mymsg_s="Process "+to_string(MPI::COMM_WORLD.Get_rank())+": Thread "+to_string(myID)+" EVALUATING\n";

//    string mymsg_e="Process "+to_string(MPI::COMM_WORLD.Get_rank())+": Thread "+to_string(myID)+" DONE EVALUATING\n";
    //cout<<MPI::COMM_WORLD.Get_rank()<<": Thread: "<<myID<<" Starting the loop" <<endl;
    for(;;){
//        data->nstarted--;
        {
//            cout<<MPI::COMM_WORLD.Get_rank()<<": Thread: "<<myID<<" SLEEpING" <<endl;
            unique_lock<mutex> lk(data->cv_m);

            data->cv.wait(lk, [&]{return data->n_task==next_task;});
        }
//        data->nstarted++;

        //cout<<MPI::COMM_WORLD.Get_rank()<<": Thread: "<<myID<<" evaluating for " <<(*x)[0]<<endl;
        if(data->done)
            break;
//        cout<<mymsg_s;
        partialeval_objective(*x,data,it,chunk_s);
//        cout<<mymsg_e;
        next_task++;
//        while( data->n_comp<=nth)
//        {
//            string msg="Process "+to_string(MPI::COMM_WORLD.Get_rank())+
//                ": Thread "+to_string(myID)+" COMP="+to_string(data->n_comp)+" Param="+to_string((*x)[0])+"\n";
//            cout<<msg;
//
//        }
//            ;
       // if(myID==0)
        //cout<<MPI::COMM_WORLD.Get_rank()<<": Thread: "<<myID<<" done evaluating for " <<(*x)[0]<<endl;
    }
//    cout<<MPI::COMM_WORLD.Get_rank()<<": Thread: "<<myID<<" is done" <<endl;
}


void Likelihood_maximizer::master_peval_objective(const std::vector<double> &x,  Likelihood_maximizer::likelihood_data_t* data)
{

//    cout<<MPI::COMM_WORLD.Get_rank()<<": in master eval" <<endl;
    unsigned n_w=data->workers->size();
    data->tmp.assign(2,0.0);
    data->cv_m.lock();
    data->n_task++;
    data->cv_m.unlock();
    data->n_comp=0;
//    data->ready=true;
    //Notify workers
    data->cv.notify_all();


//    cout<<MPI::COMM_WORLD.Get_rank()<<":Master has notified the workers" <<endl;
    //do my work
    unsigned long master_chunk=data->counts->size()-
            (data->counts->size()/(n_w+1))*
            n_w;

//    cout<<MPI::COMM_WORLD.Get_rank()<<":Master chunk="<<master_chunk <<" N counts"<<data->counts->size()<<" N threads="<<data->workers->size()+1<<endl;
//    do my work
    partialeval_objective(x,data,data->counts->begin(),master_chunk);

//    cout<<MPI::COMM_WORLD.Get_rank()<<":Finished computed parts="<<data->n_comp<<endl;
    while(data->n_comp<= n_w)
//    {
//        string msg="Process "+to_string(MPI::COMM_WORLD.Get_rank())+
//                ": Master"+" COMP="+to_string(data->n_comp)+" Param="+to_string((x)[0])+"\n";
//        cout<<msg;
//
//    }

        ;
//    cout<<MPI::COMM_WORLD.Get_rank()<<": Master done waiting"<<endl;


//    data->ready=false;

    //Collect results

}

double Likelihood_maximizer::tobjective(const std::vector<double> &x, std::vector<double> &grad, void *my_func_data)
{

    //cout<<MPI::COMM_WORLD.Get_rank()<<"Master evaluation parameter:"<<x[0] <<endl;
    likelihood_data_t* data= reinterpret_cast<likelihood_data_t*>(my_func_data);

    master_peval_objective(x,data);
//    cout<<endl<<MPI::COMM_WORLD.Get_rank()<<": objective before reduce: "<<obj<<endl;

    //cout<<endl<<MPI::COMM_WORLD.Get_rank()<<"Before reduce obj="<<data->tmp[0]+data->n0*log(data->tmp[1])<<endl;

    //cout<<endl<<MPI::COMM_WORLD.Get_rank()<<" Before reduce ll="<<data->tmp[0]<<"; ll0="<<(data->tmp[1])<<endl;


//    cout<<endl<<MPI::COMM_WORLD.Get_rank()<<" after reduce ll="<<data->tmp[0]<<"; ll0="<<(data->tmp[1])<<endl;

//    cout<<endl<<MPI::COMM_WORLD.Get_rank()<<"after reduce obj="<<data->tmp[0]+data->n0*log(data->tmp[1])<<endl;
    //cout<<endl<<MPI::COMM_WORLD.Get_rank()<<": objective after reduce: "<< data->tmp[0]+data->n0*log(data->tmp[1])<<endl;
    return data->tmp[0]+data->n0*log(data->tmp[1]);

}

double Likelihood_maximizer::pobjective(const std::vector<double> &x, std::vector<double> &grad, void *my_func_data)
{
    likelihood_data_t* data= reinterpret_cast<likelihood_data_t*>(my_func_data);
    std::vector<double> & x_ref= const_cast<std::vector<double> &>(x);
    MPI::COMM_WORLD.Bcast(&x_ref.front(), x.size(),MPI::DOUBLE, 0) ;

    master_peval_objective(x,data);
//    cout<<endl<<MPI::COMM_WORLD.Get_rank()<<": objective before reduce: "<<obj<<endl;

    cout<<endl<<MPI::COMM_WORLD.Get_rank()<<"Before reduce ll="<<data->tmp[0]<<"; ll0="<<(data->tmp[1])<<endl;

    MPI::COMM_WORLD.Reduce(MPI::IN_PLACE, &data->tmp.front(), 2, MPI::DOUBLE, MPI::SUM, 0);
//    cout<<endl<<MPI::COMM_WORLD.Get_rank()<<"after reduce obj="<<data->tmp[0]+data->n0*log(data->tmp[1])<<endl;
    cout<<endl<<MPI::COMM_WORLD.Get_rank()<<": After reduce ll="<<data->tmp[0]<<"; ll0="<<(data->tmp[1])<<endl;
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


bool Likelihood_maximizer::tmaximize(Discounter::param_array_t & p, double & best_val)
{


    nlopt::opt opt(nlopt::LN_COBYLA, n_params);

    vector<double> lb(n_params,0);

    opt.set_lower_bounds(lb);
//        likelihood_data_t d={xi, counts};
    opt.set_max_objective(Likelihood_maximizer::tobjective, &likelihood_params);

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
#define SEND_SIZE_TAG 3000
#define SEND_MAP_TAG 3001

void send_map(const count_map_t & counts,unsigned short dest)
{
    //serialize the map into a vector
    unsigned long size=counts.size();
    MPI::COMM_WORLD.Send(&size, 1, MPI::UNSIGNED_LONG, dest, SEND_SIZE_TAG);
    if(!size)
        return;

    count_vec_t sbuf;
    copy (counts.begin(),counts.end(),back_inserter(sbuf));

    MPI::COMM_WORLD.Send(&sbuf.front(), 2*sbuf.size(), MPI::UNSIGNED_LONG, dest, SEND_MAP_TAG);

}

void receive_reduce_map(count_map_t & counts,unsigned short source)
{
    unsigned long n_pairs;
    MPI::COMM_WORLD.Recv(&n_pairs, 1, MPI::UNSIGNED_LONG, source, SEND_SIZE_TAG);
    if(!n_pairs)
        return;
    count_vec_t rbuf;
    rbuf.reserve(n_pairs);
    rbuf.resize(n_pairs);
    MPI::COMM_WORLD.Recv(&rbuf.front(), 2*n_pairs, MPI::UNSIGNED_LONG, source, SEND_MAP_TAG);

    for(count_vec_t::const_iterator it=rbuf.begin();it != rbuf.end(); ++it)
        counts[it->first]+=it->second;

}
void reduce_maps(count_map_t & counts)
{
    int rank, size;
    unsigned n_loops;

    rank = MPI::COMM_WORLD.Get_rank();
    size = MPI::COMM_WORLD.Get_size();
    unsigned step;

    for(n_loops=0;1<<n_loops < size;++n_loops){
        step=1<<n_loops;

        if(rank % (step) ==0 ){
            if((rank/step) %2 ==0){ //I receive
                if(rank+step < size){

                    cout<<"Iteration "<<n_loops<<": Process "<< rank<<" receiving from process "<<rank+step<<endl;
                    receive_reduce_map(counts, rank+step);
                }else{
                    cout<<"Iteration "<<n_loops<<": Process "<< rank<<" doing nothing in this iteration,, waiting for the next"<<endl;
                }
            }else{//I send
                cout<<"Iteration "<<n_loops<<": Process "<< rank<<" sending to process "<<rank-step<<endl;
                send_map(counts,rank-step);
            }

        }
        else{
            cout<<"Iteration "<<n_loops<<": Process "<<rank<<" leaving!!"<<endl;
            break;
        }

//        cout<<"Process "<<rank<<" did it all until the end!"<<endl;
    }


}
int main(int argc, char **argv)
{
//
    int rank, size;

    MPI::Init_thread(argc, argv, MPI_THREAD_MULTIPLE);
//    MPI::Init(argc, argv);
    rank = MPI::COMM_WORLD.Get_rank();
    size = MPI::COMM_WORLD.Get_size();


    string fn="c.4."+to_string(rank);//"counts.1";//
//    if (rank==1)
//        fn="c.1";

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

	reduce_maps(xini);
	count_map_t::const_iterator it=xini.begin();

	for(int i=0;i<10;i++,++it){
		cout<<MPI::COMM_WORLD.Get_rank()<<": Occurrnces of "<<it->first<<" = "<<it->second<<endl;
	}
	if (rank==0){
	    Discounter d;
	    cout<<"Tuning to data"<<endl;
	    d.ttune(xini,4);
	}
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
