#include <math.h>
#include <iomanip>
#include <sstream>
#include <vector>
#include "out.mpi.h"


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

void psave_to_file(string fname,unsigned long text_bytes,element_vector * cooc,global_t * gs,const Options * opts)
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
#ifdef File
	#undef File
#endif
	MPI::File mfho = MPI::File::Open(MPI::COMM_WORLD, fname.c_str(), //"testfile", 
                                        MPI::MODE_WRONLY | MPI::MODE_CREATE, 
                                        MPI::INFO_NULL); 
    if(text_bytes==0){
		mfho.Close();
        return;
	}

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
    #pragma omp parallel shared(cooc,local_off,mfho,gs,global_it,formatter,pc),private(output_buf,myoff,myvec)
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
		MPI::Request w_req;
        while(true){
            myvec.clear();
            get_thread_share(&myvec,cooc, &global_it);
            if(myvec.size()==0)
                break;
            
			
			w_req.Wait();
            output_buf.str(string());
            
            fill_buf(&output_buf,&myvec,gs, &formatter,&pc);
			const std::string& tmp=output_buf.str();
// 			tmp=output_buf.str();
            #pragma omp critical(offset_update)
            {
                myoff=local_off;
                local_off+=tmp.length();
            }
            w_req=mfho.Iwrite_at(myoff,&tmp.front(),tmp.length(),MPI::CHAR);
//             pwrite(fh,output_buf.str().c_str(),output_buf.str().length(),myoff);
//             written_bytes+=output_buf.str().length();
        }
//         delete [] buf;
    }

	mfho.Close();

}
