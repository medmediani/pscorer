#include "strspec.h"

using namespace std;
void reverse_chars(char * str,int start,int end)
{
    if(start<0)
        start=0;
    if (end>=strlen(str)||end==-1)
        end=strlen(str)-1;
    if(end<=start)
        return;
    char * b=str+start;
    char * e=str+end;
    for(;b<e;++b,--e){
        *b ^=  *e;
        *e ^= *b ;
        *b ^= *e;
    }
	
}

void flip_around1(char * str,char c,int start,int end)
{
	
	if(start<0)
		start=0;
	if (end>=strlen(str)||end==-1)
		end=strlen(str)-1;
        if(end<=start)
		return;
//                  #pragma omp critical(output)
//         {
//            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread "<<omp_get_thread_num()<<" reversing '"<<str<<"'\n";
//         }
	//cout<<"reversing\n"	;
	reverse_chars(str,start,end);
//                  #pragma omp critical(output)
//         {
//            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread "<<omp_get_thread_num()<<" result '"<<str<<"'\n";
//         }
	for(int i=start;i<=end;++i)
		if(str[i]==c){
			reverse_chars(str,start,i-1);
			reverse_chars(str,i+1,end);
			break;
		}			
}

void tokenize_in_vec(char * str,str_vec * vec)
{
    const char *whitespace =" \t\n\f\r\0";
    char* word,
    * savepos;
	
    word = strtok_r(str,whitespace,&savepos);
    while (word) {
        vec->push_back(word);
        word = strtok_r(NULL,whitespace,&savepos);			
    }
}


void str_swap(char * str,unsigned b1,unsigned e1,unsigned b2,unsigned e2)
{
    string cp(str);
//     cout<<"STR0="<<str<<endl;
    //assume all the indices are correct
    //0<=b1<e1<b2<e2<len(str)
    memcpy(str+b1,&cp.front()+b2,e2-b2+1);
    
//     cout<<"STR1="<<str<<endl;
    memcpy(str+b1+e2-b2+1,&cp.front()+e1+1,b2-e1-1);
//     cout<<"STR2="<<str<<endl;
    memcpy(str+b1+e2-e1,&cp.front()+b1,e1-b1+1);
    
}

void flip_around(char * str,char c,unsigned max_chars)
{    
    char * cstr=str;
    if(!max_chars)
        max_chars=strlen(str);
    while(*cstr && *cstr !=c)
        cstr++;
    if(*cstr != c)
        return;
    
    str_swap(str,0,cstr-str-1,cstr-str+1,max_chars-1);
}


void invert_alignment(char * al)
{
    char whitespace[10]=" \t\n\0";

    unsigned nchr;
    while(*al){
        while(*al && strchr(whitespace,*al))
            ++al;
        if(!(*al))
            break;
        nchr=0;
        while(!strchr(whitespace,al[nchr]))
            ++nchr;
        if(nchr){
            flip_around(al,'-',nchr);              
            al+=nchr;
        }
    }    
}


void invert_alignment1(char * al)
{
    char whitespace[10]=" \t\n\0";
    for(int start=0,end=0;end<=strlen(al);end++)
        if(strchr(whitespace,al[end])){
        //  cout<<"flipping\n";
            flip_around1(al,'-',start,end-1);
            start=end+1;            
        }
}



void get_alignments(char * str,aligns * amap)
{
//    cout<<"Process "<< MPI::COMM_WORLD.Get_rank()<<" tokenizing the alignment: '"<<str<<"'\n";
    char *whitespace =(char *) " -\t\n\f\r\0";
    char* word,
    * savepos;
    
    word = strtok_r(str,whitespace,&savepos);
    while (word) {
        unsigned short first=atoi(word);            
        
        word = strtok_r(NULL,whitespace,&savepos);
        unsigned short second=atoi(word);
        amap->insert(make_pair(first,second));
        word = strtok_r(NULL,whitespace,&savepos);          
    }
}

/*
int main(int argc,char ** argv)
{
	char orig[100]="1-2 3-4 45-56 ";
	char whitespace[10]=" \t\n\0";
	char c='-';
	int start=0,end=0;
	cout<<"Orig before:"<<orig<<endl;
	for(;end<=strlen(orig);end++)
		if(strchr(whitespace,orig[end])){
		//	cout<<"flipping\n";
			flip_around(orig,c,start,end-1);
			start=end+1;			
		}
	cout<<"Orig after:"<<orig<<endl;
	
		return 0;
}
*/
