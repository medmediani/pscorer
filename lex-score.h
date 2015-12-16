/*
 * File:   lex-score.h
 * Author: mmediani
 *
 * Created on February 21, 2011, 1:51 PM
 */
#include "processalign.h"

#ifndef _LEX_SCORE_H
#define	_LEX_SCORE_H


struct Lex_scorer
{
    typedef score_t (*lex_score_func)(char *,char * ,char * ,align_info * ,const LexMap * );

    lex_score_func lex_score;
    Lex_scorer(string lex_scoring="",bool smoothed=false):lex_score(Lex_scorer::lex_score_avg)
    {
        if(lex_scoring=="noisy-or" || lex_scoring=="nor"){
			if (smoothed)
				lex_score=Lex_scorer::sm_lex_score_nor;
			else
				lex_score=Lex_scorer::lex_score_nor;
        }else if(lex_scoring=="m" || lex_scoring=="max"){
			lex_score=Lex_scorer::lex_score_max;
		}else if(lex_scoring=="gm" || lex_scoring=="geometric-mean"){
			lex_score=Lex_scorer::lex_score_gm;
		}else
			if(smoothed)
				lex_score=Lex_scorer::sm_lex_score_avg;


    }

    static score_t lex_score_avg(char *,char * ,char * ,align_info * ,const LexMap * );
    static score_t lex_score_max(char *,char * ,char * ,align_info * ,const LexMap * );
    static score_t lex_score_nor(char *,char * ,char * ,align_info * ,const LexMap * );
    static score_t lex_score_gm(char *,char * ,char * ,align_info * ,const LexMap * );
    static score_t sm_lex_score_avg(char *,char * ,char * ,align_info * ,const LexMap * );
    static score_t sm_lex_score_nor(char *,char * ,char * ,align_info * ,const LexMap * );
};





#endif	/* _LEX_SCORE_H */

