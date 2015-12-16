/* 
 * File:   pack_comm.h
 * Author: mmediani
 *
 * Created on February 10, 2011, 6:28 PM
 */
#include "scorer_types.h"
#include <mpi.h>

#ifndef _PACK_COMM_H
#define	_PACK_COMM_H

struct Req_t{
//    char req;
//    unsigned info_struct_length;
    unsigned long nb_bytes;//struct_nb;
//    unsigned * info_struct;
    char * info;
//    Req_t():
//    info_struct_length(0),
//    struct_nb(1),
//    info_struct(NULL),
//    info(NULL)
//    {
//
//    }
};


class PackOverflowException:public std::exception
{
	virtual const char* what() const throw()
	{
		return "A line was too long to pack (more than 10M characters)";
	}

};

int pack_req(Req_t * ,char *,int=0);

int unpack_req(char * ,Req_t * ,MPI::Status,int = 0 );

int unpack_req_primitive(char * ,Req_t * ,MPI::Status,int=0 );

int append_pack(Req_t * ,char * ,int =0);


#endif	/* _PACK_COMM_H */

