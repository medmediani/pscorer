#include "pack_comm.h"


int pack_req(Req_t * req_info,char * buf,int pos)
{
//    unsigned info_size=0;
//    for(int i=0;i<req_info->info_struct_length;++i)
//        info_size+=req_info->info_struct[i];
//    info_size *= req_info->struct_nb;
//    cout<<"Now packing the real info "<<info_size<<" charctares"<<endl;
    //the size to pack goes beyond the buffer size??
    int packed_size=MPI::UNSIGNED_LONG.Pack_size(1,MPI::COMM_WORLD)+
                    MPI::CHAR.Pack_size(req_info->nb_bytes,MPI::COMM_WORLD);
    
    if((pos+packed_size)>BUF_SIZE)

//    if((pos+info_size+(2+req_info->info_struct_length)*sizeof( MPI::UNSIGNED))>BUF_SIZE)
        throw PackOverflowException();

//    int pos=0;
    //MPI::CHAR.Pack(&(req_info->req),1,buf,BUF_SIZE,pos,MPI::COMM_WORLD);
//    cout<<"Packing: "<<req_info->info<<endl;
    MPI::UNSIGNED_LONG.Pack(&(req_info->nb_bytes),1,buf,BUF_SIZE,pos,MPI::COMM_WORLD);
//    MPI::UNSIGNED.Pack(&(req_info->info_struct_length),1,buf,BUF_SIZE,pos,MPI::COMM_WORLD);
//    MPI::UNSIGNED.Pack(req_info->info_struct,req_info->info_struct_length, buf,BUF_SIZE,pos,MPI::COMM_WORLD);
//    MPI::UNSIGNED.Pack(&(req_info->struct_nb),1, buf,BUF_SIZE,pos,MPI::COMM_WORLD);
//    cout<<"Internal info packed"<<endl;

    MPI::CHAR.Pack(req_info->info,req_info->nb_bytes, buf,BUF_SIZE,pos,MPI::COMM_WORLD);
//    MPI::CHAR.Pack(req_info->info,info_size, buf,BUF_SIZE,pos,MPI::COMM_WORLD);
    return pos;
}
//
int append_pack(Req_t * req_info,char * buf,int pos)
{

    int packed_size=MPI::CHAR.Pack_size(req_info->nb_bytes,MPI::COMM_WORLD);

    if((pos+packed_size)>BUF_SIZE)
        throw PackOverflowException();

    MPI::CHAR.Pack(req_info->info,req_info->nb_bytes, buf,BUF_SIZE,pos,MPI::COMM_WORLD);
    return pos;
}
int unpack_req(char * buf,Req_t * req_info,MPI::Status status,int pos)
{
//    int pos=0;
    int m_size=status.Get_count(MPI::PACKED);

    MPI::UNSIGNED_LONG.Unpack(buf,m_size,&(req_info->nb_bytes),1,pos,MPI::COMM_WORLD);
    //MPI::CHAR.Unpack(buf,m_size,&(req_info->req),1,pos,MPI::COMM_WORLD);
//    MPI::UNSIGNED.Unpack(buf,m_size,&(req_info->info_struct_length),1,pos,MPI::COMM_WORLD);

//    req_info->info_struct=new unsigned[req_info->info_struct_length];
//    MPI::UNSIGNED.Unpack(buf,m_size,req_info->info_struct,req_info->info_struct_length, pos,MPI::COMM_WORLD);
//
//    MPI::UNSIGNED.Unpack(buf,m_size,&(req_info->struct_nb),1,pos,MPI::COMM_WORLD);

//    unsigned info_size=0;
//    for(int i=0;i<req_info->info_struct_length;++i)
//        info_size+=req_info->info_struct[i];
//
//    info_size *= req_info->struct_nb;

//    cout<<"Unpacking the real info: "<<info_size<<" characters"<<endl;
    req_info->info=new char[req_info->nb_bytes];
    MPI::CHAR.Unpack(buf,m_size,req_info->info,req_info->nb_bytes, pos,MPI::COMM_WORLD);

//    delete [] req_info->info_struct;
    return pos;
}

int unpack_req_primitive(char * buf,Req_t * req_info,MPI::Status status,int pos)
{
//    int pos=0;
    int m_size=status.Get_count(MPI::PACKED);
    MPI::UNSIGNED_LONG.Unpack(buf,m_size,&(req_info->nb_bytes),1,pos,MPI::COMM_WORLD);
    //MPI::CHAR.Unpack(buf,m_size,&(req_info->req),1,pos,MPI::COMM_WORLD);
//    MPI::UNSIGNED.Unpack(buf,m_size,&(req_info->info_struct_length),1,pos,MPI::COMM_WORLD);
//    req_info->info_struct=new unsigned[req_info->info_struct_length];
//    MPI::UNSIGNED.Unpack(buf,m_size,req_info->info_struct,req_info->info_struct_length, pos,MPI::COMM_WORLD);
//
//    MPI::UNSIGNED.Unpack(buf,m_size,&(req_info->struct_nb),1,pos,MPI::COMM_WORLD);
//
//    unsigned info_size=0;
//    for(int i=0;i<req_info->info_struct_length;++i)
//        info_size+=req_info->info_struct[i];
//
//    info_size *= req_info->struct_nb;
//
    //req_info->info=new char[info_size];
    MPI::CHAR.Unpack(buf,m_size,req_info->info,req_info->nb_bytes, pos,MPI::COMM_WORLD);
//    delete [] req_info->info_struct;
    return pos;
}
