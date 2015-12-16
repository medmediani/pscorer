/*
 * File:   score.h
 * Author: mmediani
 *
 * Created on February 21, 2011, 11:09 AM
 */
#include <queue>
#include <algorithm>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <map>
#include "scorer_types.h"
#include "processalign.h"
#include "assocs.h"
#include "score.h"
#include "tdiscounter.h"

#ifndef _SCORE_ASSOC_H
#define	_SCORE_ASSOC_H


// void  uniqify_assoc(local_vector * ,element_vector * ,element_vector::const_iterator * , count_t * , const Lexical_aggregator * ,
// 				const Smoothing_att_calculator *);
// void accumulate_mass_assoc(local_vector * ,element_vector * ,element_vector::const_iterator * ,
//                       const Discounter * );
// void score_one_source_assoc(local_vector * ,element_vector * ,element_vector::const_iterator * ,
//                       count_t * ,const Discounter * ,const Smoothing_att_calculator *, unsigned long * );
// 

unsigned long score_assoc(element_vector ** ,global_t* ,const Options * );

#endif	/* _SCORE_H */

