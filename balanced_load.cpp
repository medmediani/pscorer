#include <fcntl.h>
#include <algorithm>

#include "pack_comm.h"
#include "load.h"
//#include "processscore.h"
#include "lex-score.h"
#include "strspec.h"
#include "crono.h"



bool insert_line( local_vector * cooc,string tline,const LexMap *lsrcmap,const LexMap * ltrmap,const Lex_scorer * lscorer)
{
//    cout<<"Insert called for line: '"<<tline<<"'\n";
    char line[MAXL];
    const char *sep= FIELD_SEP;
//	unsigned long 	id0=0,
//					id1=0,
//					sid,
//					tid;
	char * 	first,
			*instr,
			*second,
			*third;
    string key;
    pair_t c;
    score_t src_lex_score,tr_lex_score;
//------------------------
    align_info al;
//------------------------
//    pcooc_vector_t::base_container_type * map=cooc->map_at(omp_get_thread_num());
//    cout<<"Treating: "<<tline<<endl;

    strcpy(line,tline.c_str());
    first=line;
    instr=strstr(line,sep);
    if(!instr){
        #pragma omp critical(warning)
        {
            cerr<<"Process "<< MPI::COMM_WORLD.Get_rank()<<": Warning: this line is not in adequate format (caught in first token): '"<<tline<<"'"<<endl;
        }
        return false;
    }

    *instr='\0';
    second=instr+strlen(sep);
    instr=strstr(second,sep);

    if(!instr){
        #pragma omp critical(warning)
        {
            cerr<<"Process "<< MPI::COMM_WORLD.Get_rank()<<": Warning: this line is not in adequate format (caught in second token): '"<<tline<<"'"<<endl;
        }
        return false;
    }
    *instr='\0';
    third=instr+strlen(sep);
    instr=strstr(third,sep);
    if(instr)
        *instr='\0';

//    cout<<"First= "<<first<<endl<<"Second= "<<second<<endl<<"Third= "<<third<<endl;
//    cout<<"Master process "<<MPI::COMM_WORLD.Get_rank()<<" tokenized "<<endl;

    key=string(second);
    key.append(1,STR_SEP).append(first);
    if(key.length()+1>MAX_STR){
        #pragma omp critical(warning)
        {
            cerr<<"Process "<< MPI::COMM_WORLD.Get_rank()<<": Warning: a pair is longer than the maximum string size. Change and recompile the program"<<endl;
        }
        return false;
    }
    //                                 #pragma omp critical(lex_count)
    //                                 {
//    cout<<"First= "<<first<<endl<<"Second= "<<second<<endl<<"Third= "<<third<<endl;
//-------------------------------------------------------------
    tr_lex_score=lscorer->lex_score(first,second,third,&al,lsrcmap);
//-------------------------------------------------------------
    //cout<<hex<<al.alignment<<dec<<al.nb_s_words;
    strcpy(line,tline.c_str());
    first=line;
    instr=strstr(line,sep);
    *instr='\0';
    second=instr+strlen(sep);
    instr=strstr(second,sep);
    *instr='\0';
    third=instr+strlen(sep);
    instr=strstr(third,sep);
    if(instr)
        *instr='\0';
//-----------------------------------------------
    invert_alignment(third);
//------------------------------------------------
    //cout<<"inverted alignment: "<<third<<endl;
//----------------------------------------------------
    src_lex_score=lscorer->lex_score(second,first,third,NULL,ltrmap);
//------------------------------------------------------
//     #pragma omp critical(warning)
//        {
//            cerr<<"Process "<< MPI::COMM_WORLD.Get_rank()<<" inserting '"<<key<<"'"<<endl;
//        }
    /*------------------------------------------------------------*/
    if(src_lex_score<=REAL_ZERO || tr_lex_score<= REAL_ZERO){
        #pragma omp critical(warning)
        {

            cerr<<"Process "<< MPI::COMM_WORLD.Get_rank()<<": Warning: Zero lexical scores were found for: '"<<key<<"\t"<<pair_t(key.c_str(),src_lex_score,tr_lex_score,&al)<<"'"<<endl;
        }
    }
/*-------------------------------------------------------------------*/
//    #pragma omp critical(warning)
//        {
//            cerr<<"Process "<< MPI::COMM_WORLD.Get_rank()<<" Thread "<<omp_get_thread_num()<<" inserting"<<endl;
//        }
//    if(key.length()==0){
//                                 #pragma omp critical(output)
//         {
//            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread "<<omp_get_thread_num()<<" is adding an empty phrase\n";
//         }
//                    }
//    #pragma omp critical(insert)
//    {
    cooc->push_back(pair_t(key.c_str(),src_lex_score,tr_lex_score,&al));//,src_lex_score,tr_lex_score,&al));
    if(cooc->size()*sizeof(value_type)>=MAX_THREAD_MEM)
        throw VectorOverloaded();
//        cout<<"Thread: "<<omp_get_thread_num()<<" Inserting: "<<cooc_type(key.c_str(),src_lex_score,tr_lex_score,&al)<<endl;
//    }
//         #pragma omp critical(warning)
//        {
//            cerr<<"Process "<< MPI::COMM_WORLD.Get_rank()<<" Thread "<<omp_get_thread_num()<<" inserted"<<endl;
//        }
    return true;
				//cout<<" added"<<endl;
}

inline unsigned long get_file_size(File f)
{
    unsigned long cur=ftell(f);
    fseek (f, 0L, SEEK_END);
    unsigned long size=ftell (f);
//    cout<<"Size="<<size<<endl;
    fseek(f,cur,SEEK_SET);
    return size;

}

void copy_to_global(local_vector * myvec, element_vector * cooc,omp_lock_t * global_lock)
{
    if(myvec->empty())
        return;

    //try getting the writing lock
    if(omp_test_lock(global_lock)){
//        cout<<"process "<<MPI::COMM_WORLD.Get_rank()<<" thread "<<
//                omp_get_thread_num()<<" saving to main vect "<<cooc<<"\n";

        for(local_vector::const_iterator it=myvec->begin();it != myvec->end();++it)
            cooc->push_back(*it);//(cooc->end(),myvec->begin(),myvec->end());
        //release
        omp_unset_lock(global_lock);
        myvec->clear();
    }
}

bool alldone(bool * flags, int size)
{
    for(int i=0;i<size;++i)
        if(flags[i])
            return false;
    return true;
}

void load_data(File f,element_vector * cooc,LexMap *lsrcmap,LexMap * ltrmap,const Options * opts)
{
	char line[MAXL];

//	const char *sep= FIELD_SEP;
//	unsigned long 	id0=0,
//					id1=0,
//					sid,
//					tid;
//	char * 	first,
//			*instr,
//			*second,
//			*third;
    int rank = MPI::COMM_WORLD.Get_rank(),
        size = MPI::COMM_WORLD.Get_size();
//         fcntl(fileno(f), F_SETFL, O_NONBLOCK);
#ifdef _TIME_IO
    Crono timer;
    double io_span=0,
        * ins_spans;
#endif
    unsigned nth;


    unsigned long filesize = get_file_size(f)//fh.Get_size(),
                ,quotasize=filesize/size
//                        ,myquotasize=filesize/size
                        ;
    unsigned long curoff=rank*quotasize;
//    cout<<"Process "<<rank<<" file size:"<<filesize<<" quota size:"<<quotasize<<endl;
//    timer.start();
#ifdef _TIME_IO
    timer.start();
#endif
    fseek(f,rank*quotasize,SEEK_SET);
#ifdef _TIME_IO
    timer.stop();
    io_span+=timer.span();
    timer.reset();
#endif
    if(rank==size-1)
        quotasize+=filesize%size;

//    cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" quotasize: "<<quotasize<<endl;

    unsigned long bytes_read=0;
    //Ignore first line except the first process
#ifdef _TIME_IO
    timer.start();
#endif    
    if(rank>0){
        fgets(line,MAXL,f);
        bytes_read=strlen(line);
    }

#ifdef _TIME_IO
    timer.stop();
    io_span+=timer.span();
    timer.reset();
#endif
    TQueue<string> lq;//Fifo thread-safe queue
//	global_score_type gscores;//global scores
	string tline;
//	double src_lex_score,tr_lex_score;
//        pair<PhraseMap::iterator,bool> ins;
    unsigned long nb_lines;
//	align_info al;
//        bool loop=true;
    local_vector myvec;

//        timer.stop();
//        cout<< "Process "<<MPI::COMM_WORLD.Get_rank()<<" startup time:"<<timer.formatted_span()<<endl;
//        timer.reset();
//        timer.start();
    omp_lock_t lock;
    omp_init_lock(&lock);
    Lex_scorer lscorer(opts->lex_scoring,opts->smoothed_lex);

#ifdef _TIME_IO    
    #pragma omp parallel  	private(myvec,/*loop,ins,first,src_lex_score,tr_lex_score,instr,second,third,*/line,tline,timer)\
                        shared(lock,cooc,f,lq,lsrcmap,ltrmap,bytes_read,io_span,ins_spans,nth,lscorer),reduction(+:nb_lines)
#else          
     #pragma omp parallel   private(myvec,/*loop,ins,first,src_lex_score,tr_lex_score,instr,second,third,*/line,tline)\
                        shared(lock,cooc,f,lq,lsrcmap,ltrmap,bytes_read,nth,lscorer),reduction(+:nb_lines)
#endif                        
	{
//            ,lsrcmap,ltrmap
//		RegisterMap(map0);
//		RegisterMap(map1);

//		RegisterMap(lsrcmap);
//		RegisterMap(ltrmap);
		//master will provide input lines
        myvec.reserve(MAX_THREAD_MEM/sizeof(value_type)+100);
		#pragma omp master
		{
                ///////////////////////////MASTER////////////////////////////
                    lq.realloc(FACTOR*omp_get_num_threads());

                    /***********load balancer****************/
                    MPI::Request * s_reqs=new  MPI::Request[size],
                                    *r_reqs=new  MPI::Request[size];
                    MPI::Status * statuses=new MPI::Status[size];

                    unsigned long actual_processed_bytes=0;
                    pos_t pos,
                    * r_pos=new pos_t[size];

                    bool active[size];
                    memset(active,1,sizeof(bool)*size);
                    pos_t dummy;
                    memset(dummy,0,sizeof ( pos_t));

                    /*****************************/

#ifdef _TIME_IO    
                    ins_spans=new double[nth=omp_get_num_threads()];
                    memset(ins_spans,0,omp_get_num_threads()*sizeof(double));
#endif
                    char * fr;
                    do{
                        if(bytes_read<=quotasize){
//                             cout<<long(MPI::Wtime())<<" :Process "<<rank<<" starting from: "<<curoff<<" to process: "<<quotasize<<" bytes\n";
                            r_reqs[0]=MPI::COMM_WORLD.Irecv(r_pos,2, MPI::UNSIGNED_LONG,MPI::ANY_SOURCE,MPI::ANY_TAG);


#ifdef _TIME_IO    
                            timer.start();
#endif
                            fr=fgets(line,MAXL,f);
#ifdef _TIME_IO    
                            timer.stop();
                            io_span+=timer.span();
                            timer.reset();
#endif

                            while(fr!=NULL && bytes_read<=quotasize){
                                try{
                                    lq.push(line);
                                    //					if((fr=fgets(line,MAXL,f))==NULL) break;
                                }catch(FullException & e){

                                    try{
#ifdef _TIME_IO    
                                        timer.start();
#endif
                                        insert_line(&myvec,line,lsrcmap,ltrmap,&lscorer);

#ifdef _TIME_IO    
                                        timer.stop();
                                        ins_spans[omp_get_thread_num()]+=timer.span();
                                        timer.reset();
#endif
                                    }catch(VectorOverloaded & o){
                                        copy_to_global(&myvec, cooc, &lock);
                                    }
                                }
                                actual_processed_bytes+=strlen(line);
                                bytes_read+=strlen(line);
                                if(bytes_read>quotasize)
                                    break;
                                //////listening for finished processes

                                if(r_reqs[0].Test(statuses[0])){

                                    if(statuses[0].Get_tag()==EXITING){
//                                         cout<<long(MPI::Wtime())<<" :Process "<<rank<<" notified that process: "<<statuses[0].Get_source()<<" exited"<<endl;
                                        active[statuses[0].Get_source()]=false;
                                    }else if(statuses[0].Get_tag()!=POS_SEND){

//                                         cout<<long(MPI::Wtime())<<" :Process "<<rank<<" sending current to process "<<statuses[0].Get_source();
                                        //Exchange infos
                                        s_reqs[0].Wait();
                        //                if(s_reqs[0].Test()){

                                            pos[0]=ftell(f);//mycurpos;//current position
                                            pos[1]=quotasize-bytes_read;//myshare-done;//rest
//                                             cout<<"(pos="<<pos[0]<<",share="<<pos[1]<<")\n";
                                            s_reqs[0]=MPI::COMM_WORLD.Isend(&pos,2,MPI::UNSIGNED_LONG,statuses[0].Get_source(),POS_SEND);
                        //                }
                                        if(statuses[0].Get_tag()==SHARE){
//                                             cout<<long(MPI::Wtime())<<" :Process "<<rank<<" sharing with process: "<<statuses[0].Get_source()<<endl;
                                            s_reqs[0].Wait();
                                            quotasize=(quotasize+bytes_read)/2+(quotasize+bytes_read)%2;
//                                             cout<<long(MPI::Wtime())<<" :Process "<<rank<<" new share: "<<quotasize<<endl;
                                        } //else
                                    }
                                    //reinitialize receiving
                                    r_reqs[0]=MPI::COMM_WORLD.Irecv(r_pos,2, MPI::UNSIGNED_LONG,MPI::ANY_SOURCE,MPI::ANY_TAG);
                                }

#ifdef _TIME_IO    
                                timer.start();
#endif
                                fr=fgets(line,MAXL,f);
#ifdef _TIME_IO    
                                timer.stop();
                                io_span+=timer.span();
                                timer.reset();
#endif

                            }
                            //      Cancel receiving
//                             cout<<long(MPI::Wtime())<<" :Process "<<rank<<" canceling receive request\n";
                            r_reqs[0].Cancel();
//                             cout<<long(MPI::Wtime())<<" :Process "<<rank<<" waiting send request\n";
                    //        s_reqs[0].Wait();
                            r_reqs[0].Wait();
                            s_reqs[0].Wait();

                        }else{
//                             cout<<long(MPI::Wtime())<<" :Process "<<rank<<" Had nothing to process\n";
                        }

//                         cout<<long(MPI::Wtime())<<" :Process "<<rank<<" finished its share (bytes="<<bytes_read<<"), looking to share with others\n";

                       for(int i=1,j=0;j<size;++j)
                           if(j!= rank && active[j]){
//                                cout<<long(MPI::Wtime())<<" :Process "<<rank<<" sending request to process "<<j<<endl;
                               r_reqs[i]=MPI::COMM_WORLD.Irecv(r_pos+i,2,MPI::UNSIGNED_LONG, j,MPI::ANY_TAG);
                               s_reqs[i]=MPI::COMM_WORLD.Isend(dummy,2,MPI::UNSIGNED_LONG,j,IAM_FINISHED);
                               ++i;
                           }
                       MPI::Request::Waitall(size-1,r_reqs+1,statuses+1);
                       int myindex=0;
                       share_vec shares;
                       for(int i=1,j=0;j<size;++j)
                           if(j!= rank && active[j]){
                               if(statuses[i].Get_tag()==IAM_FINISHED){
//                                    cout<<long(MPI::Wtime())<<" :Process "<<rank<<" it seems that process "<<statuses[i].Get_source()<<" has also finished\n";
                                   if(j<rank)
                                       myindex++;
                               }else if(statuses[i].Get_tag()==SHARE){
                //                       pos[0]=0;//current position
                //                       pos[1]=0;
//                                    cout<<long(MPI::Wtime())<<" :Process "<<rank<<": process "<<statuses[i].Get_source()<<" wanted to share with me, but I am also finished\n";
                                   s_reqs[0]=MPI::COMM_WORLD.Isend(dummy,2,MPI::UNSIGNED_LONG,statuses[i].Get_source(),IAM_FINISHED);
                               }else if(statuses[i].Get_tag()==EXITING){
//                                    cout<<long(MPI::Wtime())<<" :Process "<<rank<<" notified that process: "<<statuses[0].Get_source()<<" exited"<<endl;

//                                   s_reqs[statuses[i].Get_source()].Cancel();

                                   active[statuses[i].Get_source()]=false;
                               }else if(r_pos[i][1]>0){//tag=pos_send && process has still something remaining
//                                    cout<<long(MPI::Wtime())<<" :Process "<<rank<<" adding feedback from process "<<j<< " to list (Tag="<<statuses[i].Get_tag()<<",Cur="<<r_pos[i][0] <<",Rest="<<r_pos[i][1]<<")\n";
                                   shares.push_back(make_pair(r_pos[i][1],j));
                               }
                               ++i;

                           }
                       if(shares.empty()){
//                            cout<<long(MPI::Wtime())<<" :Process "<<rank<<" it seems that everybody has finished\n";
                           break;
                       }
                       if(myindex>=shares.size()){
//                            cout<<long(MPI::Wtime())<<" :Process "<<rank<<" Processes with lower indexes have already shared all the available\n";
                           continue;
                       }
//                        cout<<long(MPI::Wtime())<<" :Process "<<rank<<" sorting shares\n";
                       sort(shares.begin(),shares.end());

                       int myrevindex=shares.size()-1-myindex;
//                        cout<<long(MPI::Wtime())<<" :Process "<<rank<<" I will likely share with process "<<shares[myrevindex].second<<endl;
                       if(shares[myrevindex].first<1){
//                            cout<<long(MPI::Wtime())<<" :Process "<<rank<<" process "<<shares[myrevindex].second<<" had  nothing left"<<endl;

                           continue;
                       }

                       r_reqs[0]=MPI::COMM_WORLD.Irecv(&pos,2,MPI::UNSIGNED_LONG,shares[myrevindex].second ,MPI::ANY_TAG);
                       s_reqs[0]=MPI::COMM_WORLD.Isend(dummy,2,MPI::UNSIGNED_LONG,shares[myrevindex].second,SHARE);
                       r_reqs[0].Wait(statuses[0]);
                       if(statuses[0].Get_tag()==EXITING){
//                            cout<<long(MPI::Wtime())<<" :Process "<<rank<<" was notified that process: "<<statuses[0].Get_source()<<" exited"<<endl;

//                           s_reqs[0].Cancel();

                           active[statuses[0].Get_source()]=false;
                           continue;
                       }else if(statuses[0].Get_tag()!=POS_SEND){
//                            cout<<long(MPI::Wtime())<<" :Process "<<rank<<" process "<<shares[myrevindex].second<<" did't give position (TAG='"<<
//                                    statuses[0].Get_tag()<<"')\n";
//                           s_reqs[0].Cancel();
                           continue;
                       }
//                        cout<<long(MPI::Wtime())<<" :Process "<<rank<<" looping back ";
//                       bytes_read=0;
                       quotasize=pos[1]/2;

#ifdef _TIME_IO    
                       timer.start();
#endif
                       if(
                               fseek(f,pos[0]+quotasize+pos[1]%2,SEEK_SET)
                               )
                           cerr<<long(MPI::Wtime())<<"fseek to position failed (";
                           ;
#ifdef _TIME_IO    
                       timer.stop();
                       io_span+=timer.span();
                       timer.reset();
#endif
                       curoff=pos[0]+quotasize+pos[1]%2;
//                        cout<<"new share="<<quotasize<<",expected pos="<<curoff<<",new pos="<<ftell(f)<<")\n";
#ifdef _TIME_IO    
                       timer.start();
#endif
                       fgets(line,MAXL,f);
                       bytes_read=strlen(line);

#ifdef _TIME_IO    
                       timer.stop();
                       io_span+=timer.span();
                       timer.reset();
#endif
//                       if(bytes_read>quotasize)
//                           break;

                //        }

                    }while(true);

                    active[rank]=false;
//                     cout<<long(MPI::Wtime())<<" :Process "<<rank<<" finished, waiting for any pending communication\n";
                    MPI::Request::Waitall(size,r_reqs);
                    MPI::Request::Waitall(size,s_reqs);

                    for(int j=0;j<size;++j)
                           if(active[j]){
//                                cout<<long(MPI::Wtime())<<" :Process "<<rank<<" informing process "<<j<<" that it is exiting"<<endl;
                //               r_reqs[i]=MPI::COMM_WORLD.Irecv(r_pos+i,2,MPI::UNSIGNED_LONG, j,MPI::ANY_TAG);
                               s_reqs[j]=MPI::COMM_WORLD.Isend(dummy,2,MPI::UNSIGNED_LONG,j,EXITING);

                           }

//                    r_reqs[0]=MPI::COMM_WORLD.Irecv(r_pos,2, MPI::UNSIGNED_LONG,MPI::ANY_SOURCE,MPI::ANY_TAG);
//                    while(!alldone(active,size)){
//                        if(r_reqs[0].Test(statuses[0])){
//                            if(statuses[0].Get_tag()==EXITING){
//                                cout<<long(MPI::Wtime())<<" :Process "<<rank<<" notified that process: "<<statuses[0].Get_source()<<" exited"<<endl;
//                                active[statuses[0].Get_source()]=false;
//                            }
//                            cout<<long(MPI::Wtime())<<" :Process "<<rank<<" active: ";
//                            for(int i=0;i<size;++i)
//                                cout<<active[i]<<"\t";
//                            cout<<endl;
//                //            MPI::COMM_WORLD.Send(dummy,2, MPI::UNSIGNED_LONG,statuses[0].Get_source(),EXITING);
//                            r_reqs[0]=MPI::COMM_WORLD.Irecv(r_pos,2, MPI::UNSIGNED_LONG,MPI::ANY_SOURCE,MPI::ANY_TAG);
//                        }
//                    }
//
//                    r_reqs[0].Cancel();
//                    cout<<long(MPI::Wtime())<<" :Process "<<rank<<" waiting send request\n";
//                //        s_reqs[0].Wait();

//                    r_reqs[0].Wait();
//                     cout<<long(MPI::Wtime())<<" :Process "<<rank<<" waiting for any pending communication before exiting\n";
                    MPI::Request::Waitall(size,r_reqs);
                    MPI::Request::Waitall(size,s_reqs);



                    delete [] r_reqs;
                    delete [] s_reqs;
                    delete [] r_pos;
                    delete [] statuses;

//                     cout<<long(MPI::Wtime())<<" :Process "<<rank<<" completely done\n";

                    //////////End balancing







//                     cout<<"Process "<<rank<<" effictively treated: "<<actual_processed_bytes<<endl;

//                    main_loading_loop(f,&lq,cooc,lsrcmap,ltrmap);

//			cout<<"Master process "<<MPI::COMM_WORLD.Get_rank()<<" finished... messaging all threads..."<<endl;
                    int i=omp_get_num_threads();
                    while(true){
                        try{
                            lq.push(TASK_DONE);
                            if(--i<=0) break;
                        }catch(FullException & e){
                            
                             try{
#ifdef _TIME_IO                                 
                                timer.start();
#endif                                
                                insert_line(&myvec,lq.pop(),lsrcmap,ltrmap,&lscorer);
#ifdef _TIME_IO 
                                timer.stop();
                                ins_spans[omp_get_thread_num()]+=timer.span();
                                timer.reset();
#endif                                
                            }catch(VectorOverloaded & o){
                                copy_to_global(&myvec, cooc, &lock);
                            }
//                            loop=pop_insert(&lq,cooc,lsrcmap,ltrmap);
                        }
                        //cout<<"Master done!"<<endl;
                    }

		}//END master loop
            /////////////////////////////////////END MASTER/////////////////////////
			//other threads will treat the lines read by the master
//                cout<<"process "<<MPI::COMM_WORLD.Get_rank()<<" loading from th q..."<<endl;


                try{
			tline=lq.pop();
		}catch(EmptyException & e){//else
				tline="";
		}
			//main loop
		while(tline!=TASK_DONE){
			if(tline!=""){
                            try{
#ifdef _TIME_IO    
                                timer.start();
#endif
                                insert_line(&myvec,tline,lsrcmap,ltrmap,&lscorer);

#ifdef _TIME_IO    
                                timer.stop();
                                ins_spans[omp_get_thread_num()]+=timer.span();
                                timer.reset();
#endif
                            }catch(VectorOverloaded & o){
                                copy_to_global(&myvec, cooc, &lock);
                            }

			}
			try{
				tline=lq.pop();
			}catch(EmptyException & e){
				tline="";
                                copy_to_global(&myvec, cooc, &lock);
			}
		}
//                if(loop)
//                    while(pop_insert(&lq,cooc,lsrcmap,ltrmap));
//                cout<<"process "<<MPI::COMM_WORLD.Get_rank()<<" thread "<<omp_get_thread_num()<<" finished loading..."<<endl;

//		try{
//			tline=lq.pop();
//		}catch(EmptyException & e){//else
//				tline="";
//		}
//			//main loop
//		while(tline!=TASK_DONE){
//			if(tline!=""){
//
//                            insert_line(cooc,tline);//,lsrcmap,ltrmap);
//			}
//			try{
//				tline=lq.pop();
//			}catch(EmptyException & e){
//				tline="";
//			}
//		}
                while(!myvec.empty())//copy left overs
                    copy_to_global(&myvec, cooc, &lock);
	}

        omp_destroy_lock(&lock);
//        cout<< "Process "<<MPI::COMM_WORLD.Get_rank()<<" all threads finished\n";
//        timer.stop();
#ifdef _TIME_IO    
        double max_sp=0
                ,min_sp=0;

        for (unsigned i=0;i<nth;++i){
            if(ins_spans[i]<min_sp)
                min_sp=ins_spans[i];
            if(ins_spans[i]>max_sp)
                max_sp=ins_spans[i];
        }

//         cout<< "Process "<<MPI::COMM_WORLD.Get_rank()<<" io time:"<<timer.formatted_span(io_span)<<
//                 "[insertion time: (max:"<<timer.formatted_span(max_sp)<<", min:"<<timer.formatted_span(min_sp)<<")]"<<endl;

        delete [] ins_spans;
#endif
        //---------------------------------------------------------------------
        //consume any pending messages
         MPI::Request r_reqs;
//         MPI::Status  status;

         pos_t pos;
         int i=0;
//          cout<< "Process "<<MPI::COMM_WORLD.Get_rank()<<" consuming any pending messages\n";
         r_reqs=MPI::COMM_WORLD.Irecv(&pos,2,MPI::UNSIGNED_LONG,MPI::ANY_SOURCE ,MPI::ANY_TAG);
         while(r_reqs.Test())
             r_reqs=MPI::COMM_WORLD.Irecv(&pos,2,MPI::UNSIGNED_LONG,MPI::ANY_SOURCE ,MPI::ANY_TAG);
         r_reqs.Cancel();
//          cout<< "Process "<<MPI::COMM_WORLD.Get_rank()<<" consumed pending messages\n";
//        MPI::Finalize();
//        exit(0);
}

////void printmap(PhraseMap * map)
////{
////    try {
////        PhraseMap::iterator itr;
////        for (itr = map->begin();
////             itr != map->end(); ++itr)
////            cout<<itr->first<<" : "<<itr->second<<endl;
////    }
////    catch (DbException ex) {
////        cerr<<"Error "<<ex.what()<<endl;
////    }
////}
//
//void printmap(LexMap* lmap)
//{
////    try {
//        LexMap::iterator itr;
//        for (itr = lmap->begin();
//             itr != lmap->end(); ++itr){
//                 size_t sep=itr->first.find(STR_SEP);
//                 cout<<"("<<itr->first.substr(0,sep)<<","
//                         <<itr->first.substr(sep+1)<<") : "<<itr->second<<endl;
//             }
////    }
////    catch (DbException ex) {
////        cerr<<"Error "<<ex.what()<<endl;
////    }
//}
//
////void phrase_map2id_map( PhraseMap * pmap,PhraseIdMap * idmap)
////{
////    try {
////        PhraseMap::iterator itr;
////        for (itr = pmap->begin();
////             itr != pmap->end(); ++itr)
////            if((idmap->insert(make_pair(itr->second,itr->first))).second==false)
////                cerr<<"Warning: some different phrases have the same Id's\n";
////			//cout<<itr->first<<" : "<<itr->second<<endl;
////    }
////    catch (DbException ex) {
////        cerr<<"Error "<<ex.what()<<endl;
////    }
////}
//
