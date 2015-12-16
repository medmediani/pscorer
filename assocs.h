/* 
 * File:   assocs.h
 * Author: mmediani
 *
 * Created on February 21, 2011, 1:51 PM
 */
#include <cmath>
#include <string> 

#include "scorer_types.h"
#ifndef _ASSOCS_H
#define	_ASSOCS_H

#define ZERO 1e-70
#define ODDS_BIAS_CORRECTION 0.5
#define DEFAULT_BIAS 1e-5


using namespace std;

struct Assoc_calculator
{
	typedef score_t (Assoc_calculator::*assoc_func)(count_t, count_t, count_t, count_t);
// 	typedef score_t (Assoc_calculator::*corrected_assoc_func)(count_t, count_t, count_t, count_t);
	
	
	assoc_func assoc;
	assoc_func raw_assoc;
	
	score_t bias;
// 	corrected_assoc_func corrected_assoc;
	
	static inline score_t xlogx(score_t x){
		return x>0? x*log(x): 0;
	}
	static inline score_t lfact(count_t x){
		return lgamma(x+1);
	}
	static inline score_t lcomb(count_t n, count_t r){

		return r<=n ? Assoc_calculator::lfact(n)-Assoc_calculator::lfact(r)-Assoc_calculator::lfact(n-r): 0;
	}
	static inline score_t comb(count_t n, count_t r){
		return exp(Assoc_calculator::lcomb(n,r));
		
	}
	static inline score_t sqr(score_t x){
		return x*x;
	}
	inline score_t correct_neg(count_t cooc, count_t socc, count_t tocc, count_t all){
		score_t val=(this->*raw_assoc)(cooc,socc,tocc,all);
		if (val<0)
			return bias/(1-val);
		return val+bias;
	}
	
	inline score_t correct_two_sided(count_t cooc, count_t socc, count_t tocc, count_t all){
		score_t val=(this->*raw_assoc)(cooc,socc,tocc,all);
		if(is_pos(cooc, socc, tocc, all) && val >0 ){
			return val+bias;
		}
		return bias/(1+abs(val));		
	}
    inline bool is_two_sided(string method)
	{
		return method == "multinomial_likelihood" || method =="mn-l" 
						|| 
			   method == "binomial_likelihood" || method =="bn-l"
						||
				method == "poisson_likelihood" || method =="p-l"
						|| 
				method == "poisson_stirling" || method =="p-s"
						||
				method == "hypergeometric_likelihood" || method =="hg-l"		
						||				
				method == "chi-squre-ind" || method =="chi2-i"
						||
				method == "chi-square" || method =="chi2"
						||
				method == "log-likelihood-ratio" || method =="llr"
						||
				method == "log-likelihood-ratio-dunning" || method =="llr-d"
		;
	}
	
    inline bool maybe_neg(string method)
	{
		return method == "z-score" || method =="z"
						|| 
			   method == "t-score" || method =="t"
						;
	}
	Assoc_calculator(string method="",bool correct=true,score_t bias=DEFAULT_BIAS): assoc(NULL),raw_assoc(NULL),bias(bias){
		
		if (method == "multinomial_likelihood" || method =="mn-l"){
			raw_assoc=&Assoc_calculator::multinomial_likelihood;
		}else if (method == "binomial_likelihood" || method =="bn-l"){
			raw_assoc=&Assoc_calculator::binomial_likelihood;
		}else if (method == "poisson_likelihood" || method =="p-l"){
			raw_assoc=&Assoc_calculator::poisson_likelihood;
		}else if (method == "poisson_stirling" || method =="p-s"){
			raw_assoc=&Assoc_calculator::poisson_stirling;
		}else if (method == "hypergeometric_likelihood" || method =="hg-l"){
			raw_assoc=&Assoc_calculator::hypergeometric_likelihood;
		}else if (method == "z-score" || method =="z"){
			raw_assoc=&Assoc_calculator::z_score;
		}else if (method == "t-score" || method =="t"){
			raw_assoc=&Assoc_calculator::t_score;
		}else if (method == "chi-squre-ind" || method =="chi2-i"){
			raw_assoc=&Assoc_calculator::chi_square_ind;
		}else if (method == "chi-square" || method =="chi2"){
			raw_assoc=&Assoc_calculator::chi_square;
		}else if (method == "log-likelihood-ratio" || method =="llr"){
			raw_assoc=&Assoc_calculator::llr;
		}else if (method == "log-likelihood-ratio-dunning" || method =="llr-d"){
			raw_assoc=&Assoc_calculator::llr_dunning;
		}else if (method == "mutual-info" || method =="mi"){
			raw_assoc=&Assoc_calculator::mi;
		}else if (method == "mutual-info2" || method =="mi2"){
			raw_assoc=&Assoc_calculator::mi2;
		}else if (method == "mutual-info3" || method =="mi3"){
			raw_assoc=&Assoc_calculator::mi3;
		}else if (method == "log-mutual-info" || method =="lmi"){
			raw_assoc=&Assoc_calculator::lmi;
		}else if (method == "average-mutual-info" || method =="ami"){
			raw_assoc=&Assoc_calculator::ami;
		}else if (method == "odds-ratio" || method =="or"){
			raw_assoc=&Assoc_calculator::odds_ratio;
		}else if (method == "geometric-mean" || method =="gm"){
			raw_assoc=&Assoc_calculator::gmean;
		}else if (method == "dice" || method =="d"){
			raw_assoc=&Assoc_calculator::dice;
		}else if (method == "jaccard" || method =="j"){
			raw_assoc=&Assoc_calculator::jaccard;
		}else if (method == "arithmetic-mean" || method =="amean"){
			raw_assoc=&Assoc_calculator::amean;
		}else if (method == "rho" || method =="r"){
			raw_assoc=&Assoc_calculator::rho;
		}
		assoc=raw_assoc;
		if(correct)
			if(is_two_sided(method))
				assoc=&Assoc_calculator::correct_two_sided;
			else if(maybe_neg(method))
				assoc=&Assoc_calculator::correct_neg;
			
	}
	inline score_t multinomial_likelihood(count_t cooc, count_t socc, count_t tocc, count_t all){
		return -(
				lfact(all)-lfact(cooc)-lfact(socc-cooc)-lfact(tocc-cooc)-lfact(all+cooc-tocc-socc)+xlogx(socc)+
				xlogx(tocc)+xlogx(all-socc)+xlogx(all-tocc)-2*xlogx(all)
		);
	}
	inline score_t binomial_likelihood(count_t cooc, count_t socc, count_t tocc, count_t all){
		return -(
			lcomb(all, cooc)+cooc*log((score_t(socc)/all)*(score_t(tocc)/all))+(all-cooc)*log(1-(score_t(socc)/all)*(score_t(tocc)/all))
		);
	}
	
	
	inline score_t poisson_likelihood(count_t cooc, count_t socc, count_t tocc, count_t all){
		return -(-(score_t(socc)/all)*tocc + cooc*log((score_t(socc)/all)*tocc)-lfact(cooc));
	}
	inline score_t poisson_stirling(count_t cooc, count_t socc, count_t tocc, count_t all){
		return cooc*(log((score_t(cooc)/tocc)*(score_t(all)/socc))-1);
	}
	
	inline score_t hypergeometric_likelihood(count_t cooc, count_t socc, count_t tocc, count_t all){
		return -(lcomb(tocc, cooc)+lcomb(all-tocc, socc-cooc)-lcomb(all, socc));
	}
	inline score_t z_score(count_t cooc, count_t socc, count_t tocc, count_t all){
		return (cooc-(score_t(socc)/all)*tocc)/sqrt((score_t(socc)/all)*tocc) ;
	}
	
	inline score_t t_score(count_t cooc, count_t socc, count_t tocc, count_t all){
		return (cooc-(score_t(socc)/all)*tocc)/sqrt(cooc) ;
	}
	inline score_t chi_square_ind(count_t cooc, count_t socc, count_t tocc, count_t all){
		return ( ( (score_t(all)/(socc))/tocc )*sqr(cooc-(score_t(socc)/all)*tocc)) / (1-score_t(tocc)/all) / (1-score_t(socc)/all);
	}
	
	inline score_t chi_square(count_t cooc, count_t socc, count_t tocc, count_t all){
		return (sqr(cooc-(socc*tocc)/all)/((socc*tocc)*((1-score_t(socc)/all)*(1-score_t(tocc)/all))))*all;
	}
	
	
	inline score_t llr(count_t cooc, count_t socc, count_t tocc, count_t all){
		return 2*(
			xlogx(cooc)+xlogx(all)-xlogx(socc)-xlogx(tocc)+xlogx(socc-cooc)+xlogx(tocc-cooc)
			-xlogx(all-tocc)-xlogx(all-socc) +xlogx(all+cooc -socc-tocc)
		);
	}
	
	inline score_t llr_dunning(count_t cooc, count_t socc, count_t tocc, count_t all){
		score_t  	f3= socc>cooc ? (socc-cooc)*log(score_t(socc-cooc)/(all-tocc)): 0,
					f2= tocc>cooc ? (tocc-cooc)*log(1-score_t(cooc)/tocc) :0,
					f4= all +cooc> tocc+socc ? (all+cooc-socc-tocc)*log(1-score_t(socc-cooc)/(all-tocc)) : 0,
					f1= all > socc ? (all-socc)*log(1-score_t(socc)/all) : 0;
		return -2*(
			socc*log(score_t(socc)/all) +f1 - cooc*log(score_t(cooc)/tocc)- f2-f3-f4
		);
	}
	
	inline score_t mi(count_t cooc, count_t socc, count_t tocc, count_t all){
		return log(1+ (score_t(all)/socc)*(score_t(cooc)/tocc ) );
	}
	inline score_t mi2(count_t cooc, count_t socc, count_t tocc, count_t all){
		return log(1+ (score_t(all)/socc)*(sqr(cooc)/tocc));
	}
	
	inline score_t mi3(count_t cooc, count_t socc, count_t tocc, count_t all){
		return log(1+ (score_t(all)/socc)*(cooc*sqr(cooc)/tocc));
	}
	
	inline score_t lmi(count_t cooc, count_t socc, count_t tocc, count_t all){
		return cooc*log(1+(score_t(all)/socc)*(score_t(cooc)/tocc));
	}
	
	inline score_t ami(count_t cooc, count_t socc, count_t tocc, count_t all){
		return (xlogx(cooc)+xlogx(all)-xlogx(socc)-xlogx(tocc)+xlogx(socc-cooc)+
		       xlogx(tocc-cooc)-xlogx(all-tocc)-xlogx(all-socc)+xlogx(all+cooc-socc-tocc));
	}
	
	inline score_t odds_ratio(count_t cooc, count_t socc, count_t tocc, count_t all){
		return log(1+((cooc+ODDS_BIAS_CORRECTION)/(socc-cooc+ODDS_BIAS_CORRECTION))*(all+cooc-socc-tocc+ODDS_BIAS_CORRECTION)/(tocc-cooc+ODDS_BIAS_CORRECTION) );
	}

	
	
	inline score_t gmean(count_t cooc, count_t socc, count_t tocc, count_t all){
		return score_t(cooc)/sqrt((socc)*(tocc));
	}
	
	inline score_t dice(count_t cooc, count_t socc, count_t tocc, count_t all){
		return 2*score_t(cooc)/((socc)+(tocc));
	}
	
	inline score_t jaccard(count_t cooc, count_t socc, count_t tocc, count_t all){
		return score_t(cooc)/((socc)+(tocc)-cooc);
	}
	
	inline score_t amean(count_t cooc, count_t socc, count_t tocc, count_t all){
		return (cooc)*(0.5/socc+0.5/tocc);
	}
	
	inline score_t rho(count_t cooc, count_t socc, count_t tocc, count_t all){
		return 2-sqrt((2-score_t(cooc)/socc)*(2-score_t(cooc)/tocc));
	}
	/***************/
	inline bool is_pos(count_t cooc, count_t socc, count_t tocc, count_t all){
		return cooc*all> socc*tocc;
	}
	
};



#endif	/* _ASSOCS_H */

