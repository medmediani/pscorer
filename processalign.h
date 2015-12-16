#include <map>

#ifndef _PROCESSALIGN_H
#define _PROCESSALIGN_H
using namespace std;
typedef multimap<unsigned short, unsigned short> aligns;
typedef long long packed_align_t;
struct align_info
{
	//private:
		packed_align_t alignment;
		unsigned	nb_s_words,
    				nb_t_words,
	  			nb_points;
	//public:
		 align_info(unsigned =0 ,unsigned =0 ,aligns * =NULL );
//          align_info(const align_info & other):alignment(other.alignment),nb_s_words(other.nb_s_words),nb_t_words(other.nb_t_words),nb_points(other.nb_points)
//          {
//          }
		 string align2str();
		 string source_align();
		 string target_align(); 
};
#endif