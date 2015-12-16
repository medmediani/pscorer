#include <math.h>
#include <iomanip>
#include <sstream>
#include <vector>
#include "out.posix.h"


//string format_sttk(cooc_type cooc,global_score_type * gs)
//{
//    ostringstream ret;
//
//    cooc_type::key_t key;
//    strcpy(key,cooc._key);
//    break_pair(key);
//    ret<<setprecision(PRECISION)<<scientific<<
//            "@PT # "<<first_part(key)<<" # "<<last_part(key)<<
//                     " # "<<cooc.smoothed_target_score(gs)<<" "<<cooc.tr_lex_score<<" "<<
//                     cooc.cooc<<" "<<cooc.tocc<<" "<<cooc.invn1<<" "<<cooc.invn2<<" "<<cooc.invn3p<<" "<<cooc.tr_lex_score<<" "<<
//                     cooc.smoothed_source_score(gs)<<" "<<cooc.src_lex_score<<" "<<
//                     cooc.cooc<<" "<<cooc.socc<<" "<<cooc.n1<<" "<<cooc.n2<<" "<<cooc.n3p<<" "<<cooc.src_lex_score
//                     <<" # "<<cooc.align.source_align().c_str() <<"# "<<cooc.align.target_align().c_str()
//
//            <<endl;
//
//
////    ret.str(string());
// return ret.str();
//
//}
//
//string format_moses(pair_t * cooc,const global_t * gs)
//{
//    ostringstream ret;
//
//    pair_t::key_t key;
////    strcpy(key,cooc->_key);
//    break_pair(cooc->_key);
//    ret<<setprecision(PRECISION)<<scientific<<
//    first_part(cooc->_key)<<MOSES_SEP<<last_part(cooc->_key)<<
//                     MOSES_SEP<<cooc->align.source_align()<<MOSES_SEP<<cooc->align.target_align()<<
//                     MOSES_SEP<< double(cooc.cooc)/cooc.tocc<<" "<<cooc.tr_lex_score<<
//                     " "<<double(cooc.cooc)/cooc.socc<<" "<<cooc.src_lex_score<<" "<<
//                     penalty<<
//                     " ||| "<<cooc.smoothed_target_score(gs)<<" "<<cooc.tr_lex_score<<" "<<
//                     cooc.smoothed_source_score(gs)<<" "<<cooc.src_lex_score<<endl;
//
//    return ret.str();
//
//}
//
//string format_moses_restricted(cooc_type cooc,global_score_type * gs)
//{
//    ostringstream ret;
//    double penalty=exp(1);
//
//    cooc_type::key_t key;
//    strcpy(key,cooc._key);
//    break_pair(key);
//    ret<<setprecision(PRECISION)<<scientific<<
//            first_part(key)<<" # "<<last_part(key)<<
//                     " ||| "<<cooc.align.source_align()<<"||| "<<cooc.align.target_align()<<
//                     "||| "<<double(cooc.cooc)/cooc.tocc<<" "<<cooc.tr_lex_score<<
//                     " "<<double(cooc.cooc)/cooc.socc<<" "<<cooc.src_lex_score<<" "<<
//                     penalty
//            <<endl;
//
//    return ret.str();
//
//}
//
//string format_entry(pair_t * cooc,const global_t * gs, string fmt)
//{
//    if(fmt=="sttk")
//        return format_sttk(*cooc,gs);
//    if(fmt=="moses")
//        return format_moses(*cooc,gs);
//
//    return format_sttk(*cooc,gs);
//}
//
//void save_to_file(File fh,unsigned long text_bytes,element_vector * cooc,global_t * gs, string fmt)
//{
//    //TODO: the following three lines should be removed
//    //DEBUG
////     ostringstream msg;
////     msg<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" ends here"<<endl;
////     text_bytes+=msg.str().length();
//	//
//
//    //determine my offset
//    unsigned long off;
//    MPI::COMM_WORLD.Scan(&text_bytes,&off,1,MPI::UNSIGNED_LONG,MPI::SUM);
//    off-=text_bytes;
//    if(text_bytes==0)
//        return;
//
////    int se=posix_fallocate(fh,off,text_bytes);
//    //Now output at this offset
////    fseek(fh,off,SEEK_SET);
////    cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" fallocate:"<<se<<endl;
//    string entry;
//    for(element_vector::const_iterator i=cooc->begin()
//            ;i!=cooc->end()
//            ;++i){
//        entry=format_entry(*i,gs,fmt);
//        fwrite(entry.c_str(),1,entry.length() ,fh);
////        fh.Write_at(off,entry.c_str(),entry.length() , MPI::CHAR);
////        off+=entry.length();
//    }
//
//    //just for testing
//
////   fwrite(msg.str().c_str(),1,msg.str().length() ,fh);
////    fh.Write_at(off,msg.str().c_str(),msg.str().length(),MPI::CHAR);
//
//}

void fill_buf(ostringstream * buf, local_vector * myvec,  const global_t * gs, const Entry_fomatter * formatter,const Prob_calculator * pc)
{
//    unsigned long written_bytes=0;
//     char * pos_ptr=buf;
//     string entry;
    for(local_vector::iterator it=myvec->begin();it!=myvec->end();++it){
        formatter->format(&(*it),gs,pc,buf);
//        #pragma omp critical(output)
//        {
//            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" thread "<<omp_get_thread_num()<<" ENTRY='"<<entry<<"'"<<endl;
//        }
//        if((pos_ptr+entry.length()-buf)>BUF_SIZE)
//            break;
//         memcpy(pos_ptr,entry.c_str(),entry.length());
//         pos_ptr+=entry.length();
    }
//     return pos_ptr-buf;
}
void get_thread_share(local_vector * myvec,element_vector * cooc,element_vector::const_iterator * current)
{
    
    #pragma omp critical(vector_access)
    {
        for(unsigned i=0;i<SHARE_SIZE && *current!=cooc->end();++(*current),++i){
//             if((myvec->size()+1)*sizeof(value_type)>BUF_SIZE)
//                 break;
            myvec->push_back(**current);
        }
    }
}

void psave_to_file(int fh,unsigned long text_bytes,element_vector * cooc,global_t * gs,const Options * opts)
{
    //TODO: the following three lines should be removed
//     char * buf;
    ostringstream output_buf;
    
    
    
//    ostringstream msg;
//    msg<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" ends here"<<endl;
//    text_bytes+=msg.str().length();

    //determine my offset
    unsigned long off;
    Entry_fomatter formatter(opts->output_fmt);
    Prob_calculator pc(opts->smoothing_dist);


    text_bytes += formatter.add_per_entry*cooc->size();

    MPI::COMM_WORLD.Scan(&text_bytes,&off,1,MPI::UNSIGNED_LONG,MPI::SUM);
    off-=text_bytes;
    if(text_bytes==0)
        return;
    

    cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" writing to disk: "<<text_bytes<<" bytes"<<endl; //"; starting at offset: "<<off<<endl;
//    int se=posix_fallocate(fh,off,text_bytes);
    //Now output at this offset
//    fseek(fh,off,SEEK_SET);
//    cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" fallocate:"<<se<<endl;
    unsigned long local_off=off,
            myoff;
    element_vector::const_iterator  global_it=cooc->begin()
//                                ,mybegin,
//                                    myend
                                ;
    local_vector myvec;
//    unsigned long myquota;
//     unsigned long written_bytes=0;

//    omp_lock_t lock;
//    omp_init_lock(&lock);
//
    #pragma omp parallel shared(cooc,local_off,fh,gs,global_it,formatter,pc),private(output_buf,myoff,myvec)
//     ,reduction(+:written_bytes)
    {
//         buf=new char[BUF_SIZE];
        
//        myquota=cooc->size()/omp_get_num_threads();

//        #pragma omp critical(output)
//        {
//            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" thread "<<omp_get_thread_num()<<" QUOTA="<<myquota<<endl;
//        }
//
//        mybegin=cooc->begin()+omp_get_thread_num()*myquota;
//        myend=mybegin+myquota;
//
//        if(omp_get_thread_num()==omp_get_num_threads()-1)
////            myquota+=cooc->size()%omp_get_num_threads();
//            myend=cooc->end();
        while(true){
            get_thread_share(&myvec,cooc, &global_it);
            if(myvec.size()==0)
                break;
            
            output_buf.str(string());
            
            fill_buf(&output_buf,&myvec,gs, &formatter,&pc);
            myvec.clear();
            #pragma omp critical(offset_update)
            {
                myoff=local_off;
                local_off+=output_buf.str().length();
            }
            if(pwrite(fh,output_buf.str().c_str(),output_buf.str().length(),myoff)==-1)
            {
                perror("'pwrite' error");
            }
//             written_bytes+=output_buf.str().length();
        }
//         delete [] buf;
    }
//    element_vector::const_iterator it=cooc->begin(),
//                                last=cooc->end();
//    while(unsigned long buffer_bytes=fill_buf(buf,&it, &last,gs,fmt))
//        write(fh,buf,buffer_bytes);
//    string entry;
//    for(element_vector::const_iterator i=cooc->begin()
//            ;i!=cooc->end()
//            ;++i){
//        entry=format_entry(i->get_key(),gs,fmt);
//        fwrite(entry.c_str(),1,entry.length() ,fh);
////        fh.Write_at(off,entry.c_str(),entry.length() , MPI::CHAR);
////        off+=entry.length();
//    }

    //just for testing

//   pwrite(fh,msg.str().c_str(),msg.str().length(),local_off );
//   written_bytes+=msg.str().length();
  
//     cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" wrote to disk: "<<written_bytes<<" bytes; Ending at offset: "<<local_off+msg.str().length()<<endl;

//    fh.Write_at(off,msg.str().c_str(),msg.str().length(),MPI::CHAR);

}
