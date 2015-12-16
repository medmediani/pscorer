#include <demsort/distributed_external_sort.h>
#include <algorithm>


#include "scorer_types.h"
#include "strspec.h"
//#include "demsort.h"
#include "fix_borders.h"
#include "score.assoc.h"
#include "out.h"
#include "settings.h"
#include "load.h"
#include "crono.h"
#include "uestimator.h"

// void  Score_calculator::uniqify_cooc(local_vector * res_vec,element_vector * orig,element_vector::const_iterator * merger, count_t * ut_ptr, const Lexical_aggregator * agg,
// 	const Smoothing_att_calculator *acc)
// {
// 
//     std::vector<pair_t>  tmp_vec;
// //    set<unsigned> tids;
// //#pragma omp critical(output)
// //         {
// //            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread: "<<omp_get_thread_num()<<" getting share"<<endl;
// //         }
//     pair_t::key_t new_src,cur_src;
// //    pair<string,string> new_key,cur_key;
// #pragma omp critical(merger_access)
//     {
// 
//         if(*merger!=orig->end())
//         {
//             strcpy(new_src,(*merger)->_key);
//             //strcpy(cur_src,(*merger)->key());
// 
//             break_pair(new_src);//,STR_SEP)='\0';
//             strcpy(cur_src,new_src);
//             //memcpy(cur_src,new_src,strlen((*merger)->key())+1);
//             while(!strcmp(new_src,cur_src))
//             {
// 
// 
//                 tmp_vec.push_back((**merger));
//                 ++(*merger);
//                 if(*merger!=orig->end())
//                 {
// //                    orig->flush();
//                     strcpy(cur_src,(*merger)->_key);
// //                                      #pragma omp critical(output)
// //         {
// //            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread "<<omp_get_thread_num()<<" popped the key: '"<<(*merger)->get_key()<<"'\n";
// //         }
//                     break_pair(cur_src);//cur_key=split_phrase((*merger)->first);
//                 }
//                 else
//                     break;
//             }
// 
//         }
//     }
// 
// 
//     if(tmp_vec.size()==0  )
//         return;
// 
//     ++(*ut_ptr);
// 
// //      #pragma omp critical(output)
// //         {
// //            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread: "<<omp_get_thread_num()<<" got its share from the container"<<endl;
// //         }
// 
//     std::vector<pair_t>::iterator local_itr,
//     ref_itr;
// 
//     count_t 	coocs,ut
// //                ,n[3]
//     ;
//     score_t sum_s, sum_t;
// //    int category;
//     string key;
// 
//     ut=0;
//     sum_t=0;
// //    n[0]=n[1]=n[2]=0;
//     local_itr=tmp_vec.begin();
// 
//     lex_align_vec_t tr_lex,
//     src_lex;
//     while(local_itr!=tmp_vec.end())
//     {
//         ref_itr=local_itr;
// //            category=-1;
//         coocs=0;
//         sum_s=0;
//         tr_lex.clear();
//         src_lex.clear();
// //            max_tr_lex=local_itr->tr_lex_score;
// //            max_src_lex=local_itr->src_lex_score;
// //
//         while(!strcmp(ref_itr->_key,local_itr->_key)) //first==local_itr->first){//for all similar pairs
//         {
//             coocs+=local_itr->cooc;
//             sum_s+=local_itr->sum_s;
// //                    if(tids.insert(local_itr->table_id).second)
//             sum_t+=local_itr->sum_t;
//             //Merging lexical scores
//             if (agg->aggregate)
//             {
//                 tr_lex.push_back(lex_align_pair_t(local_itr->back_lex_score,local_itr->align));
//                 src_lex.push_back(lex_align_pair_t(local_itr->for_lex_score,local_itr->align));
// //                        ref_itr->tr_lex_score=func(ref_itr->tr_lex_score,local_itr->tr_lex_score);
// //                        ref_itr->src_lex_score=func(ref_itr->src_lex_score,local_itr->src_lex_score);
//             }
// //                    if(local_itr->tr_lex_score!=ref_itr->tr_lex_score)
// //                        cout<<"Different target lexical scores for similar pairs: "<<
// //                                local_itr->tr_lex_score<<"!="<<ref_itr->tr_lex_score<<endl;
// //                    if(local_itr->src_lex_score!=ref_itr->src_lex_score)
// //                        cout<<"Different source lexical scores for similar pairs: "<<
// //                                local_itr->src_lex_score<<"!="<<ref_itr->src_lex_score<<endl;
// 
// //                    ++category;
//             ++local_itr;
//             if(local_itr==tmp_vec.end())
//                 break;
//         }
//         //cout<<"Finished run\n";
//         ref_itr->cooc=coocs;
//         ref_itr->sum_s=sum_s;
//         if (agg->aggregate)
//         {
//             if(src_lex.size()>1){
//                 lex_align_pair_t for_lex=agg->aggregate(src_lex);
//                 ref_itr->for_lex_score=for_lex.first ;
//                 ref_itr->align=for_lex.second;
//                     /*
//                 string out=to_string( MPI::COMM_WORLD.Get_rank())+":"+to_string(omp_get_thread_num());
//                 for(lex_align_vec_t::const_iterator it=src_lex.begin();it!=src_lex.end();++it)
//                     out+=" "+to_string(it->second.alignment);
//                 out+="==> Max="+to_string(for_lex.second.alignment)+"\n";
//                 #pragma omp critical(output)
//                 {
//                     cout<<out;
//                     
//                 }*/
//             }
//             
//             if(tr_lex.size()>1){
//                 lex_align_pair_t back_lex=agg->aggregate(tr_lex);
//                 ref_itr->back_lex_score=back_lex.first;
//                 if(src_lex.size()<=1)
//                     ref_itr->align=back_lex.second;
//             }                            
//              //(src_lex);
//             // func(tr_lex);
//             
//         }
// 
//         ++ut;
// 
// 
// //            n[category<=2? category:2]++;
//     }
// //    #pragma omp critical(output)
// //         {
// //            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread: "<<omp_get_thread_num()<<" finished the first step"<<endl;
// //         }
// //   //
//     local_itr=tmp_vec.begin();
//     while(local_itr!=tmp_vec.end())
//     {
//         //update
//         local_itr->sum_t=sum_t;
//         local_itr->ut=acc->accumulator(sum_t,ut);
// //            local_itr->invn1=n[0];
// //            local_itr->invn2=n[1];
// //            local_itr->invn3p=n[2];
//         //The source becomes target and vice versa
//         /*
//         cooc_type::key_t new_pair;
//         break_pair(local_itr->key());
//         strcpy(new_pair,last_part(local_itr->key()));
//         strcpy(last_part(new_pair),first_part(local_itr->key()));
//         *(new_pair+strlen(new_pair))=STR_SEP;
//          */
//         key=string(local_itr->_key);
// //                 #pragma omp critical(output)
// //         {
// //            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread "<<omp_get_thread_num()<<" is flipping the key\n";
// //         }
// 
// 
//         /*
//         key= local_itr->first.second;
//         key.append(1,STR_SEP).append(local_itr->first.first);
// 
//          */ //save to the new map
// 
// //            #pragma omp critical(insert)
//         {
//             res_vec->push_back(*local_itr);//cooc_type(key,src_lex_score,tr_lex_score,&al));
//         }
// 
// 
// //             #pragma omp critical(output)
// //         {
// //            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread "<<omp_get_thread_num()<<" successfully wrote to the new vec\n";
// //         }
// //            ref_itr=local_itr;
//         do
//         {
//             ++local_itr;
//             if(local_itr==tmp_vec.end())
//                 break;
//         }
//         while(!strcmp(key.c_str(),local_itr->_key)); //skip all similar pairs
// 
// //              #pragma omp critical(output)
// //         {
// //            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread "<<omp_get_thread_num()<<" skipped identicals and will continue\n";
// //         }
// //             #pragma omp critical(print)
// //         {
// //            cout<<omp_get_thread_num()<<" skipped all similar pairs\n";
// //         }
//     }
// //              #pragma omp critical(output)
// //         {
// //            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread "<<omp_get_thread_num()<<" returning from scoring one source\n";
// //         }
// 
//     if(res_vec->size()*sizeof(value_type)>=MAX_THREAD_MEM)
//         throw VectorOverloaded();
// }
/*************************************************/

void uniqify_assoc(local_vector * res_vec,element_vector * orig,element_vector::const_iterator * merger ,
				   count_t * ut_ptr, 
const Lexical_aggregator * agg
// 	 )
	,const Smoothing_att_calculator *acc)
{

    std::vector<pair_t>  tmp_vec;
    pair_t::key_t new_src,cur_src;
#pragma omp critical(merger_access)
    {

        if(*merger!=orig->end())
        {
            strcpy(new_src,(*merger)->_key);
            break_pair(new_src);//,STR_SEP)='\0';
            strcpy(cur_src,new_src);
            while(!strcmp(new_src,cur_src))
            {


                tmp_vec.push_back((**merger));
                ++(*merger);
                if(*merger!=orig->end())
                {
//                    orig->flush();
                    strcpy(cur_src,(*merger)->_key);
                    break_pair(cur_src);//cur_key=split_phrase((*merger)->first);
                }
                else
                    break;
            }

        }
    }


    if(tmp_vec.size()==0  )
        return;

    ++(*ut_ptr);

    std::vector<pair_t>::iterator local_itr,
    ref_itr;

    count_t 	coocs
    ,ut
    ;
    score_t sum_s, sum_t;
    string key;

//     ut=0;
    sum_t=0;
    local_itr=tmp_vec.begin();

    lex_align_vec_t tr_lex,
    src_lex;
    while(local_itr!=tmp_vec.end())
    {
        ref_itr=local_itr;
//            category=-1;
        coocs=0;
        sum_s=0;
        tr_lex.clear();
        src_lex.clear();
//
        while(!strcmp(ref_itr->_key,local_itr->_key)) //first==local_itr->first){//for all similar pairs
        {
            coocs+=local_itr->cooc;
            sum_s+=local_itr->sum_s;
//                    if(tids.insert(local_itr->table_id).second)
            sum_t+=local_itr->sum_t;
            //Merging lexical scores
            if (agg->aggregate)
            {
                tr_lex.push_back(lex_align_pair_t(local_itr->back_lex_score,local_itr->align));
                src_lex.push_back(lex_align_pair_t(local_itr->for_lex_score,local_itr->align));
            }
            ++local_itr;
            if(local_itr==tmp_vec.end())
                break;
        }
        //cout<<"Finished run\n";
        ref_itr->cooc=coocs;
        ref_itr->sum_s=sum_s;
        if (agg->aggregate)
        {
            if(src_lex.size()>1){
                lex_align_pair_t for_lex=agg->aggregate(src_lex);
                ref_itr->for_lex_score=for_lex.first ;
                ref_itr->align=for_lex.second;
            }
            
            if(tr_lex.size()>1){
                lex_align_pair_t back_lex=agg->aggregate(tr_lex);
                ref_itr->back_lex_score=back_lex.first;
                if(src_lex.size()<=1)
                    ref_itr->align=back_lex.second;
            }
            
        }

        ++ut;

    }
    local_itr=tmp_vec.begin();
    while(local_itr!=tmp_vec.end())
    {
        //update
//         local_itr->sum_t=sum_t;
        local_itr->ut=sum_t; //acc->accumulator(sum_t,ut);
        key=string(local_itr->_key);
		
		flip_around(local_itr->_key,STR_SEP);
        {
            res_vec->push_back(*local_itr);//cooc_type(key,src_lex_score,tr_lex_score,&al));
        }

        do
        {
            ++local_itr;
            if(local_itr==tmp_vec.end())
                break;
        }
        while(!strcmp(key.c_str(),local_itr->_key)); //skip all similar pairs
    }
    if(res_vec->size()*sizeof(value_type)>=MAX_THREAD_MEM)
        throw VectorOverloaded();
}
/*************************************************/


/*************************************************/
// void Score_calculator::accumulate_mass_cooc(local_vector * tmp_vec,element_vector * orig,element_vector::const_iterator * merger,
//                       const Discounter * d)
// {
// //    std::vector<pair_t> & tmp_vec=*cooc_vec;
//     unsigned long size0=tmp_vec->size();
//     //Crono timer;
//     pair_t::key_t new_src,cur_src;
// //    pair<string,string> new_key,cur_key;
// #pragma omp critical(merger_access)
//     {
//         if(*merger!=orig->end())
//         {
//             strcpy(new_src,(*merger)->_key);
//             //strcpy(cur_src,(*merger)->key());
//             break_pair(new_src);//,STR_SEP)='\0';
//             strcpy(cur_src,new_src);
//             //memcpy(cur_src,new_src,strlen((*merger)->key())+1);
//             while(!strcmp(new_src,cur_src))
//             {
//                 tmp_vec->push_back((**merger));
//                 ++(*merger);
//                 if(*merger!=orig->end())
//                 {
//                     strcpy(cur_src,(*merger)->_key);
//                     break_pair(cur_src);//cur_key=split_phrase((*merger)->first);
//                 }
//                 else
//                     break;
//             }
// 
//         }
//     }
// 
//     if(tmp_vec->size()==size0  )
//         return;
// 
//     score_t gained_mass=0;
// 
// 
//     local_vector::iterator local_itr;
// 
//     //string key;
// 
//     for(local_itr=tmp_vec->begin()+size0; local_itr!=tmp_vec->end(); ++local_itr) //get all pairs with same source
//     {
// 
// // 				cout<<"Phrase pair: "<<new_pair<<endl;
// //        if(tids.insert(local_itr2->table_id).second)
//         local_itr->for_score=local_itr->back_score=d->discount(local_itr->cooc);
// 
// 
//         gained_mass+=local_itr->back_score;
// 
//     }
//     //measure number of bytes in the output
//     
// 
//     local_itr=tmp_vec->begin()+size0;
//     
//     
//     gained_mass=1-gained_mass/local_itr->sum_t;
//   
// 
// 
//     for(; local_itr!=tmp_vec->end(); ++local_itr)
//     {
//         local_itr->back_score/=local_itr->sum_t;
//         local_itr->sum_t=gained_mass;
//         
//         flip_around(local_itr->_key,STR_SEP);
// 
//     }
//     /*
//     score_t s=0;
//     for(local_itr=tmp_vec->begin()+size0; local_itr!=tmp_vec->end(); ++local_itr)
//     {
//         s+=local_itr->back_score;
//         
//     }
//     string out=to_string(s)+" "+to_string(gained_mass)+" "+to_string(s+gained_mass)+"\n";
// #pragma omp critical(output)
//     {
//         cout<<out;
//     }
//     */
//     if(tmp_vec->size()*sizeof(value_type)>=MAX_THREAD_MEM)
//         throw VectorOverloaded();
// }
/*********************************************************************/

void accumulate_mass_assoc(local_vector * tmp_vec,element_vector * orig,element_vector::const_iterator * merger
// ,count_t * ut_ptr
,const Smoothing_att_calculator *acc
// , const Discounter * d
						  )
{
//    std::vector<pair_t> & tmp_vec=*cooc_vec;
    unsigned long size0=tmp_vec->size();
    //Crono timer;
    pair_t::key_t new_src,cur_src;
//    pair<string,string> new_key,cur_key;
#pragma omp critical(merger_access)
    {
        if(*merger!=orig->end())
        {
            strcpy(new_src,(*merger)->_key);
            //strcpy(cur_src,(*merger)->key());
            break_pair(new_src);//,STR_SEP)='\0';
            strcpy(cur_src,new_src);
            //memcpy(cur_src,new_src,strlen((*merger)->key())+1);
            while(!strcmp(new_src,cur_src))
            {
                tmp_vec->push_back((**merger));
                ++(*merger);
                if(*merger!=orig->end())
                {
                    strcpy(cur_src,(*merger)->_key);
                    break_pair(cur_src);//cur_key=split_phrase((*merger)->first);
                }
                else
                    break;
            }

        }
    }

    if(tmp_vec->size()==size0  )
        return;
// 	++(*ut_ptr);
    score_t gained_mass=0;

	score_t sum_t=0;
    count_t ut=tmp_vec->size()-size0;

    local_vector::iterator local_itr;

    //string key;

    for(local_itr=tmp_vec->begin()+size0; local_itr!=tmp_vec->end(); ++local_itr) //get all pairs with same source
    {

// 				cout<<"Phrase pair: "<<new_pair<<endl;
//        if(tids.insert(local_itr2->table_id).second)
//         local_itr->for_score=local_itr->back_score=d->discount(local_itr->cooc);
		sum_t+=local_itr->sum_t;


        gained_mass+=local_itr->back_score;

    }
    //measure number of bytes in the output
    
    
    gained_mass=1-gained_mass/sum_t;
  


    for(local_itr=tmp_vec->begin()+size0; local_itr!=tmp_vec->end(); ++local_itr)
    {
        local_itr->back_score/=sum_t;
        local_itr->sum_t=gained_mass;
        
        local_itr->ut=acc->accumulator(local_itr->ut,ut,sum_t);
        flip_around(local_itr->_key,STR_SEP);

    }
    /*
    score_t s=0;
    for(local_itr=tmp_vec->begin()+size0; local_itr!=tmp_vec->end(); ++local_itr)
    {
        s+=local_itr->back_score;
        
    }
    string out=to_string(s)+" "+to_string(gained_mass)+" "+to_string(s+gained_mass)+"\n";
#pragma omp critical(output)
    {
        cout<<out;
    }
    */
    if(tmp_vec->size()*sizeof(value_type)>=MAX_THREAD_MEM)
        throw VectorOverloaded();
}


/*********************************************************************/

// void Score_calculator::score_one_source_cooc(local_vector * tmp_vec,element_vector * orig,element_vector::const_iterator * merger,
//                       count_t * us_ptr,score_t * sum_assocs,const Discounter * d,const Smoothing_att_calculator *acc, 
// 					  const Assoc_calculator * assoc, count_t n, unsigned long * text_bytes)//,unsigned long * klen,unsigned long * salen,unsigned long * talen)
// {
// //    std::vector<pair_t> & tmp_vec=*cooc_vec;
//     unsigned long size0=tmp_vec->size();
//     //Crono timer;
//     pair_t::key_t new_src,cur_src;
// //    pair<string,string> new_key,cur_key;
// #pragma omp critical(merger_access)
//     {
//         if(*merger!=orig->end())
//         {
//             strcpy(new_src,(*merger)->_key);
//             //strcpy(cur_src,(*merger)->key());
//             break_pair(new_src);//,STR_SEP)='\0';
//             strcpy(cur_src,new_src);
//             //memcpy(cur_src,new_src,strlen((*merger)->key())+1);
//             while(!strcmp(new_src,cur_src))
//             {
//                 tmp_vec->push_back((**merger));
//                 ++(*merger);
//                 if(*merger!=orig->end())
//                 {
//                     strcpy(cur_src,(*merger)->_key);
//                     break_pair(cur_src);//cur_key=split_phrase((*merger)->first);
//                 }
//                 else
//                     break;
//             }
// 
//         }
//     }
// 
//     if(tmp_vec->size()==size0  )
//         return;
// 
//     ++(*us_ptr);
// 
//     score_t gained_mass=0;
// 
//     local_vector::iterator local_itr;
//     count_t us=tmp_vec->size()-size0;
// 
//     score_t sum_s=0;
// 
//     //string key;
// 
//     for(local_itr=tmp_vec->begin()+size0; local_itr!=tmp_vec->end(); ++local_itr) //get all pairs with same source
//     {
// 
// // 				cout<<"Phrase pair: "<<new_pair<<endl;
// //        if(tids.insert(local_itr2->table_id).second)
// //         local_itr->for_score=d->discount(local_itr->cooc);
//         sum_s+=local_itr->sum_s;
// 
//         gained_mass+=local_itr->for_score;
// 
//     }
//     //measure number of bytes in the output
//     gained_mass=1-gained_mass/sum_s;
// 
// 
//     for(local_itr=tmp_vec->begin()+size0; local_itr!=tmp_vec->end(); ++local_itr)
//     {
//         //                     cout<<"Values to be updated: "<<start_itr->tocc<<"\t"<<start_itr->invn1<<"\t"<<start_itr->invn2<<"\t"<<start_itr->invn3p<<endl;
//         local_itr->sum_s=gained_mass;
//         local_itr->us=acc->accumulator(sum_s,us);
//         local_itr->for_score/=sum_s;
// 
//         *text_bytes+=strlen(local_itr->_key)-1 + local_itr->align.source_align().length()+
// 							 local_itr->align.target_align().length();
//                              
// //           *klen+=strlen(local_itr->_key)-1;
// //           *salen=local_itr->align.source_align().length();
// //           *talen=local_itr->align.target_align().length();
// 
// //        #pragma omp critical(insert)
// //         {
//             //cooc_vec->push_back(*local_itr2);
// //         }
//     }
//     if(tmp_vec->size()*sizeof(value_type)>=MAX_THREAD_MEM)
//         throw VectorOverloaded();
// }

/*************************************************/

void score_one_source_assoc(local_vector * tmp_vec,element_vector * orig,element_vector::const_iterator * merger,
                      count_t * us_ptr,score_t * sum_assocs, const Discounter * d,const Smoothing_att_calculator *acc, 
					  Assoc_calculator * assoc, count_t n, unsigned long * text_bytes)//,unsigned long * klen,unsigned long * salen,unsigned long * talen)
{
//    std::vector<pair_t> & tmp_vec=*cooc_vec;
    unsigned long size0=tmp_vec->size();
    //Crono timer;
    pair_t::key_t new_src,cur_src;
//    pair<string,string> new_key,cur_key;
#pragma omp critical(merger_access)
    {
        if(*merger!=orig->end())
        {
            strcpy(new_src,(*merger)->_key);
            //strcpy(cur_src,(*merger)->key());
            break_pair(new_src);//,STR_SEP)='\0';
            strcpy(cur_src,new_src);
            //memcpy(cur_src,new_src,strlen((*merger)->key())+1);
            while(!strcmp(new_src,cur_src))
            {
                tmp_vec->push_back((**merger));
                ++(*merger);
                if(*merger!=orig->end())
                {
                    strcpy(cur_src,(*merger)->_key);
                    break_pair(cur_src);//cur_key=split_phrase((*merger)->first);
                }
                else
                    break;
            }

        }
    }

    if(tmp_vec->size()==size0  )
        return;

    ++(*us_ptr);

    score_t gained_mass=0;

    local_vector::iterator local_itr;
    count_t us=tmp_vec->size()-size0;

    score_t sum_s=0;
	score_t sum_mass=0;

    //string key;

    for(local_itr=tmp_vec->begin()+size0; local_itr!=tmp_vec->end(); ++local_itr) //get all pairs with same source
    {

// 				cout<<"Phrase pair: "<<new_pair<<endl;
//        if(tids.insert(local_itr2->table_id).second)
//         local_itr->for_score=d->discount(local_itr->cooc);
        sum_s+=local_itr->sum_s;

//         gained_mass+=local_itr->for_score;

    }
    
    for(local_itr=tmp_vec->begin()+size0; local_itr!=tmp_vec->end(); ++local_itr) //get all pairs with same source
    {

// 				cout<<"Phrase pair: "<<new_pair<<endl;
//        if(tids.insert(local_itr2->table_id).second)
		local_itr->sum_t=(assoc->*(*assoc).assoc)(local_itr->cooc,sum_s,local_itr->ut,n);
        local_itr->back_score=local_itr->for_score=d->discount(local_itr->sum_t,local_itr->cooc);
		sum_mass+=local_itr->sum_t;
		
// 		local_itr->for_score;
//         sum_s+=local_itr->sum_s;

        gained_mass+=local_itr->for_score;

    }
    //measure number of bytes in the output
    gained_mass=1-gained_mass/sum_mass;


    for(local_itr=tmp_vec->begin()+size0; local_itr!=tmp_vec->end(); ++local_itr)
    {
        //                     cout<<"Values to be updated: "<<start_itr->tocc<<"\t"<<start_itr->invn1<<"\t"<<start_itr->invn2<<"\t"<<start_itr->invn3p<<endl;
        local_itr->sum_s=gained_mass;
        local_itr->us=acc->accumulator(sum_s,us,sum_mass);
        local_itr->for_score/=sum_mass;

        *text_bytes+=strlen(local_itr->_key)-1 + local_itr->align.source_align().length()+
							 local_itr->align.target_align().length();

        
        flip_around(local_itr->_key,STR_SEP);
    }
    (*sum_assocs)+=sum_mass;
    if(tmp_vec->size()*sizeof(value_type)>=MAX_THREAD_MEM)
        throw VectorOverloaded();
}

/*************************************************/

unsigned long score_assoc(element_vector ** cooc_vec_ptr,global_t* g_scores,const Options * opts)
{
//    double phase1span,phase2span;
    global_t gs;

    element_vector * cooc_vec=*cooc_vec_ptr;
    element_vector * res_vec=new element_vector;
//    cooc_vec->print_sizes();




	ROOT_MSG cout<<"Scoring using association"<<endl;
	
    gs.n=cooc_vec->size();
    MPI::COMM_WORLD.Allreduce(MPI::IN_PLACE,&(gs.n),1,COUNT_MPI_TYPE,MPI::SUM);
    ROOT_MSG cout<<"Total number of pairs: "<<gs.n<<endl;

    count_map_t meta_counts;


    Crono timer;
    ROOT_MSG cout<<endl;
    ROOT_MSG cout<<"-------------------------\n"<<MPI::COMM_WORLD.Get_rank()<<": Sorting (first run)\n-------------------------"<<endl;
    timer.start();
    do_sort (cooc_vec,res_vec,opts->mem);
    timer.stop();

//   phase1span=timer.span();
    ROOT_MSG cout<<"\n\t"<<MPI::COMM_WORLD.Get_rank()<<": Done sorting\nPartial sorting took "<<timer.formatted_span()<<endl;
    //print_cooc_vec(cooc_vec);
    //cout<<*cooc_vec<<endl;
    ROOT_MSG cout<<"\n-------------------------\n"<<MPI::COMM_WORLD.Get_rank()<<": Uniqifying phrase pairs\n-------------------------"<<endl;
	ROOT_MSG cout<<"Lexical aggregation using: "<<opts->lex_aggergator<<endl;
//    timer.reset();
//    timer.start();

    //typedef pair<pcooc_vector_t::merge_iterator,pcooc_vector_t::merge_iterator> task_type;
    //TQueue<task_type> tq;
    timer.reset();
    timer.start();
    element_vector::const_iterator  /*pcooc_vector_t::merge_iterator*/ g_merger;//=cooc_vec->begin();//->get_merge_iterator();

    local_vector myvec;
    omp_lock_t lock;
    omp_init_lock(&lock);
    unsigned n_threads;
    count_t u=0;
	Lexical_aggregator agg(opts->lex_aggergator);
	Smoothing_att_calculator acc(opts->smoothing_dist);
	Assoc_calculator assoc_c(opts->assoc_method);

#pragma omp parallel shared(cooc_vec,res_vec, g_merger,lock,n_threads,agg,acc) private(myvec) reduction(+:u)
    {
//        g_merger->init(omp_get_thread_num());
        #pragma omp single nowait
        {
            n_threads=omp_get_num_threads();
        }
//        #pragma omp barrier
//          #pragma omp critical(output)
//         {
//            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread: "<<omp_get_thread_num()<<" attempts to fix"<<endl;
//         }
        myvec.reserve(MAX_THREAD_MEM/sizeof(value_type)+100);
        //Let one of the threads exchange the borders of the vector with its neighbours
#pragma omp single
//         nowait
        {
//
            g_merger = send_receive_borders(cooc_vec);
        }

        //Aggregate the rest
        while(g_merger!=cooc_vec->end())
        {
            try
            {

                uniqify_assoc(&myvec,cooc_vec,&g_merger, &u, &agg, &acc);

            }
            catch(VectorOverloaded & o)
            {

                copy_to_global_and_count(&myvec,res_vec , &meta_counts, &lock);

            }

        }
//         }
//           #pragma omp critical(output)
//         {
//            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread: "<<omp_get_thread_num()<<" done, exiting threaded section"<<endl;
//         }
        while(!myvec.empty())//copy left overs
            copy_to_global_and_count(&myvec, res_vec, &meta_counts, &lock);



    }

    timer.stop();

    ROOT_MSG cout<<endl<<MPI::COMM_WORLD.Get_rank()<<": Done uniqifying in: "<<timer.formatted_span()<<endl;

    MPI::COMM_WORLD.Allreduce(&u,&(gs.ut),1,COUNT_MPI_TYPE,MPI::SUM);
    ROOT_MSG cout<<"Number of unique target phrases: "<<gs.ut<<endl;

    gs.un=res_vec->size();
    ROOT_MSG cout<<endl<<"Process " << MPI::COMM_WORLD.Get_rank() << ": Number of non-identical pairs: "<<gs.un<<endl;

    MPI::COMM_WORLD.Allreduce(MPI::IN_PLACE,&(gs.un),1,COUNT_MPI_TYPE,MPI::SUM);
    ROOT_MSG cout<<"Number of unique pairs: "<<gs.un<<endl;
    
    



    ROOT_MSG cout<<endl<<"Tuning XI functions parameter(s) ..."<<endl<<"XI function: "<<opts->xi_func<<endl;

	Discounter d(opts->xi_func);
	timer.reset();
//	Crono timer;
	timer.start();

	d.ptune(meta_counts, n_threads,Unseen_estimator(opts->unseen_estimator).estimate(meta_counts,gs.n,gs.un));

	ROOT_MSG cout<<"Done tuning in: "<<timer.formatted_span()<<endl;
	ROOT_MSG cout<<"Best XI parameter(s): "<<d<<endl;
//	timer.stop();
//	cerr<<MPI::COMM_WORLD.Get_rank()<<": "<<timer.formatted_span()<<endl;
    meta_counts.clear();

//    cout<<//"\n-------------------------------------------------------\n"<<
//            "\nFirst run of scoring done \nFirst run, including partial sorting, took "<<
//            timer.formatted_span(phase1span)<<endl;

//    delete g_merger;
    delete *cooc_vec_ptr;
//    cooc_vec->clear();
    //second sort by target sentences IDs
    cooc_vec=new element_vector ;
	
//     g_merger=res_vec->begin();
//     myvec.clear();
// //
// #pragma omp parallel shared(cooc_vec,res_vec, g_merger,lock,d) private(myvec)
//     {
//         myvec.reserve(MAX_THREAD_MEM/sizeof(value_type)+100);
// 
//         //Aggregate the rest
//         while(g_merger!=res_vec->end())
//         {
//             try
//             {
// 
//                 accumulate_mass(&myvec,res_vec,&g_merger,&d);
// 
//             }
//             catch(VectorOverloaded & o)
//             {
// 
//                 copy_to_global(&myvec,cooc_vec, &lock);
// 
//             }
// 
//         }
// //         }
// //           #pragma omp critical(output)
// //         {
// //            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread: "<<omp_get_thread_num()<<" done, exiting threaded section"<<endl;
// //         }
//         while(!myvec.empty())//copy left overs
//             copy_to_global(&myvec, cooc_vec, &lock);
//     }
// 
//     delete res_vec;
//     res_vec=new element_vector;

//    phase1span+=timer.span();
//    timer.reset();
//    timer.start();
//    res_vec->print_sizes();
    ROOT_MSG cout<<endl;
//    MPI::COMM_WORLD.Barrier();
    ROOT_MSG cout<<"---------------------------\n"<<MPI::COMM_WORLD.Get_rank()<<": Resorting (second run)\n---------------------------"<<endl;
    timer.reset();
    timer.start();

    do_sort (res_vec,cooc_vec,opts->mem);

    timer.stop();
//    phase2span=timer.span();
    ROOT_MSG cout<<endl<<"Process " << MPI::COMM_WORLD.Get_rank() << ": Done sorting partial sets\nPartial sorting took "<<timer.formatted_span()<<endl;
    ROOT_MSG cout<<"\n-------------------------\n"<<MPI::COMM_WORLD.Get_rank()<<": Merging partial sorts\n-------------------------"<<endl;
    timer.reset();
    timer.start();

    u=0;
    unsigned long bytes=0;
	score_t sum_assocs=0;
	Assoc_calculator scorer(opts->assoc_method);
//     unsigned long kbytes=0,sabytes=0,tabytes=0;
    myvec.clear();
#pragma omp parallel private(myvec) shared(lock,g_merger,cooc_vec,res_vec,d,acc,scorer),reduction(+:u), reduction(+:bytes),reduction(+:sum_assocs)
    //, reduction(+:kbytes),reduction(+:sabytes),reduction(+:tabytes)
    {
//         g_merger->init(omp_get_thread_num());
//        #pragma omp barrier

        myvec.reserve(MAX_THREAD_MEM/sizeof(value_type)+100);

//        g_merger->init(omp_get_thread_num());
//        #pragma omp barrier
        //Let one of the threads exchange the borders of the vector with its neighbours
#pragma omp single
//                      nowait
        {

            g_merger=send_receive_borders(res_vec);

        }

        while(g_merger!=res_vec->end())
        {
            try
            {
				
                score_one_source_assoc(&myvec,res_vec,&g_merger,&u,&sum_assocs,&d,&acc,&scorer,gs.n,&bytes); //,&kbytes,&sabytes,&tabytes);//score_one_source(&lock,&lq,res_vec,g_merger);
            }
            catch(VectorOverloaded & o)
            {
                copy_to_global(&myvec,cooc_vec , &lock);
            }

//                if(g_merger==res_vec->end())
//                    break;
//            cout<<omp_get_thread_num()<<": "<</*g_merger.count()<<*/"\t";

        }
        while(!myvec.empty())//copy left overs
            copy_to_global(&myvec, cooc_vec, &lock);

        //Balancing
    }


    delete res_vec;
    res_vec=new element_vector;

	MPI::COMM_WORLD.Allreduce(&u,&(gs.us),1,COUNT_MPI_TYPE,MPI::SUM);
    ROOT_MSG cout<<"Number of unique source phrases: "<<gs.us<<endl;
	
	MPI::COMM_WORLD.Allreduce(&sum_assocs,&(gs.sum),1,SCORE_MPI_TYPE,MPI::SUM);
    ROOT_MSG cout<<"Sum of association scores: "<<gs.sum<<endl;
    timer.stop();
    
//     cout<<endl<<MPI::COMM_WORLD.Get_rank()<<": NUMBER OF BYTES: "<<kbytes<<" "<<sabytes<<" "<<tabytes<<endl;

    ROOT_MSG cout<<endl<<MPI::COMM_WORLD.Get_rank()<<": Done merging in: "<<timer.formatted_span()<<endl;

//    cout<<"Disposing unneeded resources\n";
//    timer.start();
//    delete g_merger;
	
/********************************/

    ROOT_MSG cout<<endl;
//    MPI::COMM_WORLD.Barrier();
    ROOT_MSG cout<<"---------------------------\n"<<MPI::COMM_WORLD.Get_rank()<<": Resorting (third run)\n---------------------------"<<endl;
    timer.reset();
    timer.start();

    do_sort (cooc_vec,res_vec,opts->mem);

    timer.stop();
	ROOT_MSG cout<<"\n\t"<<MPI::COMM_WORLD.Get_rank()<<": Done sorting\nPartial sorting took "<<timer.formatted_span()<<endl;
	ROOT_MSG cout<<"\n-------------------------\n"<<MPI::COMM_WORLD.Get_rank()<<": Merging partial sorts\n-------------------------"<<endl;
    timer.reset();
    timer.start();
	
	
    myvec.clear();
#pragma omp parallel private(myvec) shared(lock,g_merger,cooc_vec,res_vec,acc)
    //, reduction(+:kbytes),reduction(+:sabytes),reduction(+:tabytes)
    {
//         g_merger->init(omp_get_thread_num());
//        #pragma omp barrier

        myvec.reserve(MAX_THREAD_MEM/sizeof(value_type)+100);

//        g_merger->init(omp_get_thread_num());
//        #pragma omp barrier
        //Let one of the threads exchange the borders of the vector with its neighbours
#pragma omp single
//                      nowait
        {

            g_merger=send_receive_borders(cooc_vec);

        }

        while(g_merger!=cooc_vec->end())
        {
            try
            {
				
                accumulate_mass_assoc(&myvec,cooc_vec,&g_merger,&acc); //,&kbytes,&sabytes,&tabytes);//score_one_source(&lock,&lq,res_vec,g_merger);
            }
            catch(VectorOverloaded & o)
            {
                copy_to_global(&myvec,res_vec , &lock);
            }

//                if(g_merger==res_vec->end())
//                    break;
//            cout<<omp_get_thread_num()<<": "<</*g_merger.count()<<*/"\t";

        }
        while(!myvec.empty())//copy left overs
            copy_to_global(&myvec, res_vec, &lock);

        //Balancing
    }
    
    delete cooc_vec;
	
	timer.stop();
    
//     cout<<endl<<MPI::COMM_WORLD.Get_rank()<<": NUMBER OF BYTES: "<<kbytes<<" "<<sabytes<<" "<<tabytes<<endl;

    ROOT_MSG cout<<endl<<MPI::COMM_WORLD.Get_rank()<<": Done merging in: "<<timer.formatted_span()<<endl;

	
/*********************************/

	
    omp_destroy_lock(&lock);

//    cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<"all value="<<gs.all<<endl;
    if(g_scores)
    {

        memcpy(g_scores,&gs,sizeof(global_t));
    }

//    timer.stop();
//    cout<<"\nSecond run of scoring done\nSecond run, including partial sorting, took "<<
//            timer.formatted_span(phase2span)<<endl;

//    cout<<"\nTwo phase phrase scoring took "<<timer.formatted_span(phase1span+phase2span)<<endl<<endl;
    //calc corresponding scores
    *cooc_vec_ptr=res_vec;
    return bytes;
}
