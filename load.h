/* 
 * File:   load.h
 * Author: mmediani
 *
 * Created on February 10, 2011, 3:14 PM
 */
#include <vector>
#include "scorer_types.h"
#include "tqueue.h"
#ifndef _LOAD_H
#define	_LOAD_H

/******************* added for load balancer *****************/

#define IAM_FINISHED 10
#define POS_SEND 20
#define SHARE 30
#define EXITING 40

struct Options;
typedef unsigned long pos_t [2];
typedef std::pair<unsigned long,int> share_rank_pair;
typedef std::pair<share_rank_pair,unsigned long> share_rank_off_pair;
typedef vector<share_rank_pair> share_vec;

/*************************************************************/
//#include "crono.h"
//#include "tqueue.h"
//#include "strspec.h"
//
//#include "pvector.h"

//void create_lex_map(File ,LexMap * );

//void create_lex_map_par(File ,LexMap * );

//void screate_maps(File ,PhraseMap * ,PhraseMap* ,LexMap *,LexMap * ,cooc_vector_t * );
//
//void create_maps_runs(File ,PhraseMap * ,PhraseMap* ,LexMap *,LexMap * ,cooc_vector_t * );

void load_data(File ,element_vector  *,LexMap *,LexMap * , const Options *);

//void printmap(PhraseMap * );
//void printmap(LexMap* );
bool insert_line(element_vector *, string,LexMap *,LexMap * );
//void phrase_map2id_map( PhraseMap * ,PhraseIdMap * );


void copy_to_global(local_vector * , element_vector * ,omp_lock_t * );

#endif	/* _LOAD_H */

