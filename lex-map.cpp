#include <string>
#include "lex-map.h"

inline string join_pair(string first, string second)
{
    return first.append(1,STR_SEP).append(second);

}

void create_lex_map(File lex_file,LexMap * lmap)
{

	char line[MAXL];
	char *whitespace =(char *) " \t\n\f\r\0";
	char * s_word, *t_word,*score_str,
			* savepos;



	while(fgets(line,MAXL,lex_file)!=NULL){
		//cout<<"Line to be treated: "<<line<<endl;
			s_word = strtok_r(line,whitespace,&savepos);
			t_word = strtok_r(NULL,whitespace,&savepos);
			score_str=strtok_r(NULL,whitespace,&savepos);
			lmap->insert(
						 make_pair(
								   //lex_trans_type(s_word,t_word),
								   //string(s_word).append(1,STR_SEP).append(string(t_word)),
                                                                    join_pair(s_word,t_word),
								   strtod(score_str,NULL)
								   )
						 );
	}
//cout<<"Map created successfully"<<endl;
}
