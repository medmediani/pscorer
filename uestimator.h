#include <mpi.h>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include "score.h"

using namespace std;

// typedef unsigned long count_t;
// typedef map<count_t, count_t> count_map_t  ;
// typedef double score_t;

// #define COUNT_MPI_TYPE MPI::UNSIGNED_LONG

typedef vector<count_t> count_vec_t;
#define MK_COUNT_VECTOR(cc,v,h) count_vec_t v(h,0); mk_metacount_vec(cc,v,h)
#ifdef ROOT_MSG 
    #undef ROOT_MSG

#endif
#define ROOT_MSG if(MPI::COMM_WORLD.Get_rank()==0)
    
class Unseen_estimator
{
public:
    typedef  score_t (*estimator_t)(const count_map_t &, count_t, count_t);
private:
	static inline void mk_metacount_vec(const count_map_t & cc , count_vec_t & vec, count_t highest_count)
	{
// 		vec.reserve(highest_count);
		
		count_map_t::const_iterator it;
		for (unsigned i=1;i<=highest_count;++i){
			it=cc.find(i);
			if (it != cc.end()){
// 				cerr<<MPI::COMM_WORLD.Get_rank()<<": found n"<<i<<"="<<it->second<<endl;
				vec[i-1]=it->second;
				
			}
// 			else{
// 				vec.push_back(0);				
// 			}
		}
// 		cerr<<MPI::COMM_WORLD.Get_rank()<<": size="<<vec.size()<<endl;
		MPI::COMM_WORLD.Allreduce(MPI::IN_PLACE,&vec.front(),highest_count,COUNT_MPI_TYPE,MPI::SUM);
		
	}
	static inline score_t chao(const count_map_t & cc, count_t n, count_t un){
		MK_COUNT_VECTOR(cc,vec,2);
// 		cerr<<MPI::COMM_WORLD.Get_rank()<<": n1="<<vec[0]<<endl;
		return (double( vec[0])/vec[1])*vec[0]/2.0;
	}
	
	static inline score_t bcorrected_chao(const count_map_t & cc, count_t n, count_t un){
		MK_COUNT_VECTOR(cc,vec,2);
		return (double( vec[0])/(vec[1]+1))*(vec[0]-1)/2.0;
	}
	
	
	static inline score_t np_lindley(const count_map_t & cc, count_t n, count_t un){
		MK_COUNT_VECTOR(cc,vec,3);
		double a=(vec[1]+vec[0]/3.0-vec[2])/(vec[2]+vec[1]/3.0)        ;
        return 2*(a+1)*vec[1]-(2-a)*vec[0];		
	}
	
	
	static inline score_t mix_exp(const count_map_t & cc, count_t n, count_t un){
		
        return (un)/(double(n)/un-1);
	}
	
	
	static inline score_t gt(const count_map_t & cc, count_t n, count_t un){
		MK_COUNT_VECTOR(cc,vec,1);
		
        return 1./(1./vec[0]-1./un);
        
	}
    
    static inline score_t none(const count_map_t & cc, count_t n, count_t un){
        return -1;        
    }
public:
	
	estimator_t estimate;
	Unseen_estimator(string estimator=""):estimate(Unseen_estimator::none)
	{
        if (estimator=="chao" || estimator=="c"){
            ROOT_MSG cout<<endl<<"Estimating the number of unseen events using 'Chao estimator'"<<endl;
            estimate=Unseen_estimator::chao;
        }else if (estimator=="bias-corrected-chao" || estimator=="b"){
            ROOT_MSG cout<<endl<<"Estimating the number of unseen events using 'Bias-corrected-Chao estimator'"<<endl;
			estimate=Unseen_estimator::bcorrected_chao;
		}else if (estimator=="nonparametric-lindley" || estimator=="l"){
            ROOT_MSG cout<<endl<<"Estimating the number of unseen events using 'Mixed Poisson-Lindley estimator'"<<endl;
			estimate=Unseen_estimator::np_lindley;
		}else if (estimator=="geometric" || estimator=="g"){
            ROOT_MSG cout<<endl<<"Estimating the number of unseen events using 'Mixed Exponential estimator'"<<endl;
			estimate=Unseen_estimator::mix_exp;
		}else if (estimator=="good-turing" || estimator=="gt"){
            ROOT_MSG cout<<endl<<"Estimating the number of unseen events using 'Good-Turing estimator'"<<endl;
			estimate=Unseen_estimator::gt;
		}else{
            ROOT_MSG cout<<endl<<"Estimating the number of unseen events using 'Leaving-One-Out'"<<endl;
        }
		
	}
	
};

// int main(int argc, char **argv)
// {
// //
//     int rank, size;
// 
// //     MPI::Init_thread(argc, argv, MPI_THREAD_MULTIPLE);
//     MPI::Init(argc, argv);
//     rank = MPI::COMM_WORLD.Get_rank();
//     size = MPI::COMM_WORLD.Get_size();
// 
// 
// 
//     string fn="c.4."+to_string(rank);//"counts.1";//
// //    if (rank==1)
// //        fn="c.1
// 
// 	ifstream fin(fn.c_str());
//     string line;
// 	count_map_t xini;
// 
//     if(!fin){
//         return false;
//     }
// 
// 	unsigned long val;
// 	unsigned long n=0, un=0;
// 
// 
//     while (fin >>val) {
// 		un++;
// 		n+=val;
// //		cout<< "Val= "<<val<<endl;
// 		xini[val]+=1;
// 
// 	}
// 	count_map_t::const_iterator it=xini.begin();
// 
// 	for(int i=0;i<10;i++,++it){
// 		cout<<"Occurrnces of "<<it->first<<" = "<<it->second<<endl;
// 	}
// 
// 
// 	MPI::COMM_WORLD.Allreduce(MPI::IN_PLACE,&n,1,MPI::UNSIGNED_LONG,MPI::SUM);
// 		
// 	MPI::COMM_WORLD.Allreduce(MPI::IN_PLACE,&un,1,MPI::UNSIGNED_LONG,MPI::SUM);
// // 	cerr<<MPI::COMM_WORLD.Get_rank()<<": n="<<n<<endl;
// // 	cerr<<MPI::COMM_WORLD.Get_rank()<<": un="<<un<<endl;
// // 	cerr<<MPI::COMM_WORLD.Get_rank()<<": n1="<<xini[1]<<endl;
// 	
// 	cerr<<MPI::COMM_WORLD.Get_rank()<<": Estimated unseen: "<<Unseen_estimator("g").estimate(xini,n,un)<<endl;
// //	Crono timer;
// //	timer.start();
// //	timer.stop();
// //	cerr<<MPI::COMM_WORLD.Get_rank()<<": "<<timer.formatted_span()<<endl;
// //
// //	nlopt::opt opt(nlopt::LN_COBYLA, N_PARAMS);
// //
// //	vector<double> lb;
// //	for(int i=0;i<N_PARAMS;++i)
// //		lb.push_back(0);
// //
// //
// //	opt.set_lower_bounds(lb);
// //	likelihood_data_t d={xi_exp1, &xini};
// //	opt.set_max_objective(myvfunc, &d);
// //
// //	opt.set_xtol_rel(1e-10);
// //
// //	std::vector<double> x(N_PARAMS,1.0);
// //
// //	double minf;
// //	nlopt::result result = opt.optimize(x, minf);
// //
// //	if (0) {
// //		printf("nlopt failed!\n");
// //	}
// //	else {
// //		printf("found max at f(%g) = %0.10g\n", x[0], minf);
// //	}
//     MPI::Finalize();
// 
//     return (EXIT_SUCCESS);
// }
// 
