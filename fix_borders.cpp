//#include "global.h"
#include "scorer_types.h"
#include "pack_comm.h"
#include "strspec.h"


element_vector::iterator send_receive_borders(element_vector * cooc)
{

    int rank = MPI::COMM_WORLD.Get_rank(),
            size = MPI::COMM_WORLD.Get_size();

    if(size==1) //no fixing needed for one process
        return cooc->begin();

    element_vector::iterator it=cooc->begin();

    char *s_pack_buf=new char[BUF_SIZE],
        *r_pack_buf=new char [BUF_SIZE];

//    key_exchange k;

    pair_t::key_t src
                    ,cur_src
                    ;




    MPI::Request s_req,
                r_req,
                final_req;
    MPI::Status //s_stat,
                r_stat;
    MPI::Status status;
    Req_t req,
          nbbytes_req;

    int pos=0;
//    if(next>=size)
//        next=-1;
    bool something2send=rank>0,

            something2receive=rank<size-1;
    if(something2send && it!=cooc->end() && cooc->size()>0){
        strcpy(src,it->_key);
        break_pair(src);
//        cout << "Process " << rank <<" first src: "<<src<<"\n";
    }else if(something2send){
        something2send=false;
        final_req=MPI::COMM_WORLD.Isend(0,0,MPI::PACKED,rank-1,EMPTY_BORDER_KEYS);
    }



    do{

        //start receiving from the next process
        if(something2receive){
//            cout << "Process " << rank <<" initiating receive "<<"\n";
            r_req=MPI::COMM_WORLD.Irecv(r_pack_buf,BUF_SIZE, MPI::PACKED,MPI::ANY_SOURCE,MPI::ANY_TAG);
        }
        //start sending
        if(something2send){
//            cout << "Process " << rank <<" initiating send "<<"\n";

            req.nb_bytes= sizeof(pair_t);//key_exchange);
            pair_t k(*it);
            req.info=(char *)&k;

//            strcpy(cur_src,it->_key);
//            break_pair(cur_src);//cur_ke
//            if(strcmp(src,cur_src)) //done with the first source phrase
//                break;
//            cout << "Process " << rank <<" packing: "<<it->get_key()<<"\n";
            pos=pack_req(&req,s_pack_buf);
//            cout << "Process " << rank <<" packed: "<<it->get_key()<<"\n";
            int i=1;
//            element_vector::iterator prev_it=it;

            for(++it;it!=cooc->end() ;++i,++it){//fill whole buffer or consume all similar source phrases
                strcpy(cur_src,it->_key);
                break_pair(cur_src);
                if(strcmp(src,cur_src)){
                    //Different sources ==> Done sending
//                    cout << "Process " << rank <<" detected different source: "<<src<<"#"<<cur_src<<"\n";
                    something2send=false;
                    break;
                }

                try{
                    k=/*key_exchange*/(*it);
                    req.info=(char *)&k;
//                    cout << "Process " << rank <<" packing: "<<it->get_key()<<"\n";
                    pos=append_pack(&req,s_pack_buf,pos);
//                    cout << "Process " << rank <<" packed: "<<it->get_key()<<"\n";
                }catch(PackOverflowException & e){
//                    it=prev_it;
//                    cout << "Process " << rank <<" consumed buffer "<<"\n";
//                    cerr<<"A line was extremely long to be processed"<<endl;
                    //Buffer filled, should be sent and try another run
                    break;
                }
//                prev_it=it;

            }
            if(it==cooc->end()){
//                cout << "Process " << rank <<" end of vector reached\n";
                something2send=false;
                //Whole vector consumed... Done sending
//                break;
            }
            //update the number of bytes before sending at first position of the buffer

//            cout << "Process " << rank <<" setting the number of bytes to send: "<<req.nb_bytes<<
//                    "*"<<i<<"="<<req.nb_bytes*i<<"\n";
            req.nb_bytes*=i;
            nbbytes_req.nb_bytes=sizeof(req.nb_bytes);
            nbbytes_req.info=(char *)&req.nb_bytes;
            append_pack(&nbbytes_req,s_pack_buf);
            s_req=MPI::COMM_WORLD.Isend(s_pack_buf,pos,MPI::PACKED,rank-1,NON_EMPTY_BORDER_KEYS);
//            cout << "Process " << rank <<" sent non-empty message: "<<"\n";
            if(!something2send){
                s_req.Wait();
                final_req=MPI::COMM_WORLD.Isend(0,0,MPI::PACKED,rank-1,EMPTY_BORDER_KEYS);
//                cout << "Process " << rank <<" sent empty message: "<<"\n";
            }
        }

        //wait for any submitted receiving request
        if(something2receive){
            r_req.Wait(r_stat);
//            cout << "Process " << rank <<" received: "<<"\n";
            if(r_stat.Get_tag()!=EMPTY_BORDER_KEYS){
//                cout << "Process " << rank <<" unpacking with tag= "<<r_stat.Get_tag()<<"\n";
                unpack_req(r_pack_buf,& req,r_stat);

                unsigned long nb_elems=req.nb_bytes/sizeof(pair_t);
                pair_t * elem_ptr=(pair_t *)req.info;
//                cout << "Process " << rank <<" unpacked: "<<nb_elems<<" elements\n";
                while(nb_elems--){
                    //TODO: make it critical
//                    if(strlen(elem_ptr->make_cooc_type()._key)==0){
//                                 #pragma omp critical(output)
//         {
//            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Thread "<<omp_get_thread_num()<<" is adding an empty phrase\n";
//         }
//                    }
//                    cout << "Process " << rank <<" adding to vector: "<<elem_ptr->make_cooc_type()<<"\n";
                    cooc->push_back(*elem_ptr);//->make_cooc_type());
                    ++elem_ptr;
                }
                //release the unpacking buffer
                delete [] req.info;
            }else{ //the neighbour is done sending
//                cout << "Process " << rank <<" nothing left to receive: "<<"\n";
                something2receive=false;
            }
        }
        //wait for submitted sending requests
        if(something2send)
            s_req.Wait();

    }while(something2receive || something2send);
//    cout << "Process " << rank <<" waiting for the final send: "<<"\n";

    if(rank>0)
        final_req.Wait();
//    cout << "Process " << rank <<" final send finished: "<<"\n";
    delete [] s_pack_buf;
    delete [] r_pack_buf;

    return it;
}
