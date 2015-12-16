#include <mpi.h>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

void main(int argc, char **argv)
{

    int rank, size;

    MPI::Init_thread(argc, argv, MPI_THREAD_MULTIPLE);
//    MPI::Init(argc, argv);
    rank = MPI::COMM_WORLD.Get_rank();
    size = MPI::COMM_WORLD.Get_size();
    vector<pair<int,int>> v;
    for(unsigned i=0;i<10;++i){
        v.push_back(pair(i,2*rank+1));
    }

}
