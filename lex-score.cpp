#include <set>
#include <string>
#include <cmath>
#include "scorer_types.h"
#include "strspec.h"
#include "lex-score.h"

#define EMPTY "YTPME"

score_t Lex_scorer::lex_score_avg(char *src,char * tr,char * al,align_info * al_info,const LexMap * lmap)
{
    score_t score=1;
    str_vec src_vec,
    tr_vec;
    aligns alignments;

// 	cout<<"Lexical score for ('"<<src<<"','"<<tr<<"')"<<endl;

    tokenize_in_vec(src,&src_vec);

    tokenize_in_vec(tr,&tr_vec);

//     cout<<"Alignment: "<<al<<endl;
    get_alignments(al,&alignments);
//     cout<<"Extracted alignments:\n";
    /*for(aligns::iterator i=alignments.begin();i!=alignments.end();++i)
        cout<<i->first<<":"<<i->second	<<endl;
   */
    if (al_info)
        *al_info= align_info(src_vec.size() ,tr_vec.size() ,&alignments);

    ostringstream key;
    short i=0;
    for(str_vec::const_iterator src_itr=src_vec.begin();
        src_itr !=src_vec.end();
        ++src_itr,++i){


            if(alignments.count(i)==0){
// 				cout<<"the word '"<<*src_itr<<"' was unaligned"<<endl;
				//cout<<(*lmap)[*src_itr]<<"<---from lex map"<<endl;
                key.str(std::string());
                key<<*src_itr<<STR_SEP<<UNALIGNED;
//                 cout<<"Looking for: "<<key.str()<<endl;
                LexMap::const_iterator found=lmap->find(key.str());//src_itr->append(STR_SEP).append(UNALIGNED));
												  //lex_trans_type(*src_itr,UNALIGNED));
                if(found!=lmap->end()){
//                     cout<<"Map('"<<*src_itr<<"','"<<UNALIGNED<<"')="<<found->second<<endl;
                    score *=found->second;//(*lmap)[lex_trans_type(*src_itr,UNALIGNED)];
                }
					/*
                else
                cout<<"the couple ('"<<*src_itr<<"',NULL) was not found in the lexical map"<<endl;
                                        */
            }
            else{
                double sum=0;

				//cout<<"the word('"<<*src_itr<<"') was aligned at least once\n";
                for(aligns::const_iterator al_itr=alignments.equal_range(i).first;
                    al_itr!=alignments.equal_range(i).second;
                    ++al_itr){
                        key.str(std::string());
                        key<<*src_itr<<STR_SEP<<tr_vec[al_itr->second];
//                         cout<<"Looking for: "<<key.str()<<endl;
                        LexMap::const_iterator found=lmap->find(key.str());

                        if(found!=lmap->end()){
//                             cout<<"Map('"<<*src_itr<<"','"<<tr_vec[al_itr->second]<<"')="<<found->second<<endl;
                            sum+=found->second;//(*lmap)[lex_trans_type(*src_itr,tr_vec[(*al_itr).second])];
                        }else{
//                             cout<<"Couldn't find: '"<<*src_itr<<"','"<<tr_vec[al_itr->second]<<"' in the map\n";
                        }
                    }
                    score *= sum/alignments.count(i);
            }
        }
//         cout<<"Score= "<<score<<endl;
        return score;
}


score_t Lex_scorer::lex_score_max(char *src,char * tr,char * al,align_info * al_info,const LexMap * lmap)
{
    score_t score=1;
    str_vec src_vec,
    tr_vec;
    aligns alignments;

// 	cout<<"Lexical score for ('"<<src<<"','"<<tr<<"')"<<endl;

    tokenize_in_vec(src,&src_vec);

    tokenize_in_vec(tr,&tr_vec);

//     cout<<"Alignment: "<<al<<endl;
    get_alignments(al,&alignments);
//     cout<<"Extracted alignments:\n";
    /*for(aligns::iterator i=alignments.begin();i!=alignments.end();++i)
        cout<<i->first<<":"<<i->second	<<endl;
   */
    if (al_info)
        *al_info= align_info(src_vec.size() ,tr_vec.size() ,&alignments);

    ostringstream key;
    short i=0;
    for(str_vec::const_iterator src_itr=src_vec.begin();
        src_itr !=src_vec.end();
        ++src_itr,++i){


            if(alignments.count(i)==0){
// 				cout<<"the word '"<<*src_itr<<"' was unaligned"<<endl;
				//cout<<(*lmap)[*src_itr]<<"<---from lex map"<<endl;
                key.str(std::string());
                key<<*src_itr<<STR_SEP<<UNALIGNED;
//                 cout<<"Looking for: "<<key.str()<<endl;
                LexMap::const_iterator found=lmap->find(key.str());//src_itr->append(STR_SEP).append(UNALIGNED));
												  //lex_trans_type(*src_itr,UNALIGNED));
                if(found!=lmap->end()){
//                     cout<<"Map('"<<*src_itr<<"','"<<UNALIGNED<<"')="<<found->second<<endl;
                    score *=found->second;//(*lmap)[lex_trans_type(*src_itr,UNALIGNED)];
                }
					/*
                else
                cout<<"the couple ('"<<*src_itr<<"',NULL) was not found in the lexical map"<<endl;
                                        */
            }
            else{
                double mx=0;

				//cout<<"the word('"<<*src_itr<<"') was aligned at least once\n";
                for(aligns::const_iterator al_itr=alignments.equal_range(i).first;
                    al_itr!=alignments.equal_range(i).second;
                    ++al_itr){
                        key.str(std::string());
                        key<<*src_itr<<STR_SEP<<tr_vec[al_itr->second];
//                         cout<<"Looking for: "<<key.str()<<endl;
                        LexMap::const_iterator found=lmap->find(key.str());

                        if(found!=lmap->end()){
//                             cout<<"Map('"<<*src_itr<<"','"<<tr_vec[al_itr->second]<<"')="<<found->second<<endl;
                            mx=MAX(found->second,mx);//(*lmap)[lex_trans_type(*src_itr,tr_vec[(*al_itr).second])];
                        }else{
//                             cout<<"Couldn't find: '"<<*src_itr<<"','"<<tr_vec[al_itr->second]<<"' in the map\n";
                        }
                    }
                    score *= mx; //sum/alignments.count(i);
            }
        }
//         cout<<"Score= "<<score<<endl;
        return score;
}

// double lex_score(char *src,char * tr,char * al,LexMap * lmap)
// {
//     lex_score(src, tr, al,NULL, lmap);
// }

//Noisy-or lexical scores

score_t Lex_scorer::lex_score_nor(char *src,char * tr,char * al,align_info * al_info,const LexMap * lmap)
{
    score_t score=1;
    str_vec src_vec,
    tr_vec;
    aligns alignments;

// 	cout<<"Lexical score for ('"<<src<<"','"<<tr<<"')"<<endl;

    tokenize_in_vec(src,&src_vec);

    tokenize_in_vec(tr,&tr_vec);

//     cout<<"Alignment: "<<al<<endl;
    get_alignments(al,&alignments);
//     cout<<"Extracted alignments:\n";
    /*for(aligns::iterator i=alignments.begin();i!=alignments.end();++i)
        cout<<i->first<<":"<<i->second	<<endl;
   */
    if (al_info)
        *al_info= align_info(src_vec.size() ,tr_vec.size() ,&alignments);

    ostringstream key;
    short i=0;
    for(str_vec::const_iterator src_itr=src_vec.begin();
        src_itr !=src_vec.end();
        ++src_itr,++i){


            if(alignments.count(i)==0){
// 				cout<<"the word '"<<*src_itr<<"' was unaligned"<<endl;
				//cout<<(*lmap)[*src_itr]<<"<---from lex map"<<endl;
                key.str(std::string());
                key<<*src_itr<<STR_SEP<<UNALIGNED;
//                 cout<<"Looking for: "<<key.str()<<endl;
                LexMap::const_iterator found=lmap->find(key.str());//src_itr->append(STR_SEP).append(UNALIGNED));
												  //lex_trans_type(*src_itr,UNALIGNED));
                if(found!=lmap->end()){
//                     cout<<"Map('"<<*src_itr<<"','"<<UNALIGNED<<"')="<<found->second<<endl;
                    score *=found->second;//(*lmap)[lex_trans_type(*src_itr,UNALIGNED)];
                }
					/*
                else
                cout<<"the couple ('"<<*src_itr<<"',NULL) was not found in the lexical map"<<endl;
                                        */
            }
            else{
                score_t prod=1;

				//cout<<"the word('"<<*src_itr<<"') was aligned at least once\n";
                for(aligns::const_iterator al_itr=alignments.equal_range(i).first;
                    al_itr!=alignments.equal_range(i).second;
                    ++al_itr){
                        key.str(std::string());
                        key<<*src_itr<<STR_SEP<<tr_vec[al_itr->second];
//                         cout<<"Looking for: "<<key.str()<<endl;
                        LexMap::const_iterator found=lmap->find(key.str());

                        if(found!=lmap->end()){
//                             cout<<"Map('"<<*src_itr<<"','"<<tr_vec[al_itr->second]<<"')="<<found->second<<endl;
                            prod*=1-found->second;//(*lmap)[lex_trans_type(*src_itr,tr_vec[(*al_itr).second])];
                        }else{
//                             cout<<"Couldn't find: '"<<*src_itr<<"','"<<tr_vec[al_itr->second]<<"' in the map\n";
                        }
                    }
                    score *= 1-prod;
            }
        }
//         cout<<"Score= "<<score<<endl;
        return score;
}


score_t Lex_scorer::lex_score_gm(char *src,char * tr,char * al,align_info * al_info,const LexMap * lmap)
{
    score_t score=1;
    str_vec src_vec,
    tr_vec;
    aligns alignments;

// 	cout<<"Lexical score for ('"<<src<<"','"<<tr<<"')"<<endl;

    tokenize_in_vec(src,&src_vec);

    tokenize_in_vec(tr,&tr_vec);

//     cout<<"Alignment: "<<al<<endl;
    get_alignments(al,&alignments);
//     cout<<"Extracted alignments:\n";
    /*for(aligns::iterator i=alignments.begin();i!=alignments.end();++i)
        cout<<i->first<<":"<<i->second	<<endl;
   */
    if (al_info)
        *al_info= align_info(src_vec.size() ,tr_vec.size() ,&alignments);

    ostringstream key;
    short i=0;
    for(str_vec::const_iterator src_itr=src_vec.begin();
        src_itr !=src_vec.end();
        ++src_itr,++i){


            if(alignments.count(i)==0){
// 				cout<<"the word '"<<*src_itr<<"' was unaligned"<<endl;
				//cout<<(*lmap)[*src_itr]<<"<---from lex map"<<endl;
                key.str(std::string());
                key<<*src_itr<<STR_SEP<<UNALIGNED;
//                 cout<<"Looking for: "<<key.str()<<endl;
                LexMap::const_iterator found=lmap->find(key.str());//src_itr->append(STR_SEP).append(UNALIGNED));
												  //lex_trans_type(*src_itr,UNALIGNED));
                if(found!=lmap->end()){
//                     cout<<"Map('"<<*src_itr<<"','"<<UNALIGNED<<"')="<<found->second<<endl;
                    score *=found->second;//(*lmap)[lex_trans_type(*src_itr,UNALIGNED)];
                }
					/*
                else
                cout<<"the couple ('"<<*src_itr<<"',NULL) was not found in the lexical map"<<endl;
                                        */
            }
            else{
                score_t sum_logs=0;

				//cout<<"the word('"<<*src_itr<<"') was aligned at least once\n";
                for(aligns::const_iterator al_itr=alignments.equal_range(i).first;
                    al_itr!=alignments.equal_range(i).second;
                    ++al_itr){
                        key.str(std::string());
                        key<<*src_itr<<STR_SEP<<tr_vec[al_itr->second];
//                         cout<<"Looking for: "<<key.str()<<endl;
                        LexMap::const_iterator found=lmap->find(key.str());

                        if(found!=lmap->end()){
//                             cout<<"Map('"<<*src_itr<<"','"<<tr_vec[al_itr->second]<<"')="<<found->second<<endl;
                            sum_logs+=log(found->second);//(*lmap)[lex_trans_type(*src_itr,tr_vec[(*al_itr).second])];
                        }else{
//                             cout<<"Couldn't find: '"<<*src_itr<<"','"<<tr_vec[al_itr->second]<<"' in the map\n";
                        }
                    }
                    score *= exp(sum_logs/alignments.count(i));
            }
        }
//         cout<<"Score= "<<score<<endl;
        return score;
}

//Noisy-or smoothed lexcal table

score_t Lex_scorer::sm_lex_score_nor(char *src,char * tr,char * al,align_info * al_info,const LexMap * lmap)
{
    score_t score=1;
    str_vec src_vec,
    tr_vec;


// 	cout<<"Lexical score for ('"<<src<<"','"<<tr<<"')"<<endl;

    tokenize_in_vec(src,&src_vec);

    tokenize_in_vec(tr,&tr_vec);
    if (al_info){
		aligns alignments;
		get_alignments(al,&alignments);
        *al_info= align_info(src_vec.size() ,tr_vec.size() ,&alignments);
	}


//     cout<<"Alignment: "<<al<<endl;
//     get_alignments(al,&alignments);
//     cout<<"Extracted alignments:\n";

    ostringstream key,other_key;
	score_t prod;
	LexMap::const_iterator gained_mass_ptr, prob_ptr;
//     short i=0;
    for(str_vec::const_iterator src_itr=src_vec.begin();
        src_itr !=src_vec.end();
        ++src_itr){
		prod=1;
		for(str_vec::const_iterator tr_itr=tr_vec.begin();
        tr_itr !=tr_vec.end();
        ++tr_itr

		){
			key.str(std::string());
			key<<*src_itr<<STR_SEP<<*tr_itr;
//                         cout<<"Looking for: "<<key.str()<<endl;
			prob_ptr=lmap->find(key.str());
			if(prob_ptr!=lmap->end()){
//                             cout<<"Map('"<<*src_itr<<"','"<<tr_vec[al_itr->second]<<"')="<<found->second<<endl;
				prod*=1-prob_ptr->second;//(*lmap)[lex_trans_type(*src_itr,tr_vec[(*al_itr).second])];

			}else{
// 				cout<<"Couldn't find: '"<<*src_itr<<"','"<<*tr_itr<<"' in the map\n";
				key.str(std::string());
				other_key.str(std::string());

				key<<*src_itr<<STR_SEP<<EMPTY;
				prob_ptr=lmap->find(key.str());
				other_key<<EMPTY<<STR_SEP<<*tr_itr;
				gained_mass_ptr=lmap->find(other_key.str());
				if(prob_ptr!=lmap->end()){
					if(gained_mass_ptr!=lmap->end()){

						prod*=1-prob_ptr->second*gained_mass_ptr->second;
					}else{
						prod*=1-prob_ptr->second;
					}
				}else{
				    throw runtime_error((string("A lexical table has no smoothing entry for word '")+*src_itr)+"'");
				}



			}

		}
		score *= 1-prod;

	}

//         cout<<"Score= "<<score<<endl;
	return score;
}


//average smoothed lexcal table

score_t Lex_scorer::sm_lex_score_avg(char *src,char * tr,char * al,align_info * al_info,const LexMap * lmap)
{
    score_t score=1;
    str_vec src_vec,
    tr_vec;

// 	cout<<"Lexical score for ('"<<src<<"','"<<tr<<"')"<<endl;

    tokenize_in_vec(src,&src_vec);

    tokenize_in_vec(tr,&tr_vec);
    if (al_info){
		aligns alignments;
		get_alignments(al,&alignments);
        *al_info= align_info(src_vec.size() ,tr_vec.size() ,&alignments);
	}

//     cout<<"Alignment: "<<al<<endl;
//     get_alignments(al,&alignments);
//     cout<<"Extracted alignments:\n";

    ostringstream key,other_key;
	score_t sum;
	LexMap::const_iterator found,other_found;
//     short i=0;
    for(str_vec::const_iterator src_itr=src_vec.begin();
        src_itr !=src_vec.end();
        ++src_itr){
		sum=0;
		for(str_vec::const_iterator tr_itr=tr_vec.begin();
        tr_itr !=tr_vec.end();
        ++tr_itr

		){
			key.str(std::string());
			key<<*src_itr<<STR_SEP<<*tr_itr;
//                         cout<<"Looking for: "<<key.str()<<endl;
			found=lmap->find(key.str());
			if(found!=lmap->end()){
//                             cout<<"Map('"<<*src_itr<<"','"<<tr_vec[al_itr->second]<<"')="<<found->second<<endl;
				sum+=found->second;//(*lmap)[lex_trans_type(*src_itr,tr_vec[(*al_itr).second])];

			}else{
//                             cout<<"Couldn't find: '"<<*src_itr<<"','"<<tr_vec[al_itr->second]<<"' in the map\n";
				key.str(std::string());
				other_key.str(std::string());

				key<<*src_itr<<STR_SEP<<EMPTY;
				found=lmap->find(key.str());
				if(found!=lmap->end()){
					other_key<<EMPTY<<STR_SEP<<*tr_itr;
					other_found=lmap->find(other_key.str());
					if (other_found!=lmap->end()){
						sum+=found->second*other_found->second;
					}else{
					    sum+=found->second;
					}
				}



			}

		}
		score *= sum/tr_vec.size();

	}

//         cout<<"Score= "<<score<<endl;
	return score;
}

