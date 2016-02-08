/*
 * File:   pscore.cpp
 * Author: mmediani
 *
 * Created on February 23, 2011, 5:25 PM
 */


#include <stdlib.h>

#include "anyoption.h"
#include "scorer_types.h"
#include "load.h"
#include "prepare_space.h"
#include "fix_borders.h"
#include "lex-map.h"
#include "score.h"
#include "score.assoc.h"
#include "out.mpi.h"
#include "crono.h"


/*
 *
 */



#if defined(ROOT_MSG)
    #undef ROOT_MSG
    #define ROOT_MSG if(rank==0)
#endif


bool check_vec(element_vector * cooc_vec)
{
    cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" checking vector"<<endl;

    for(element_vector::const_iterator it=cooc_vec->begin();it!=cooc_vec->end();++it)
        if(strchr(it->_key,STR_SEP)==NULL){
            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" corrupted pair:'"<<*it<<endl;
        }
    cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" vector checked"<<endl;
}

void print_cooc_vec(element_vector * cooc_vec)
{
	element_vector::iterator itr;
	cout<<"src_id\ttrg_id\tcooc\ts_occ\tt_occ\ts_prob\tt_prob\ts_lex\tt_lex"<<endl;

	for (itr = cooc_vec->begin();
		 itr != cooc_vec->end(); ++itr) {
			 cout<<*itr<<endl;
		 }
}

bool operator < (const pair_t & a,const pair_t& b)
{
//    if((a!=cooc_type::max_value() && a!=cooc_type::min_value())|| (b!=cooc_type::max_value() &&  b!=cooc_type::min_value()))
//        cout<<"Comparing: '"<<a._key<<"' and '"<<b._key<<"'\n";
//	if(a.key()==b.key())
//		return a.val() < b.val();
    return strcmp(a._key,b._key)<0;//a.key()<b.key();
}

/*
bool operator < (const count_counts_t& a,const count_counts_t& b)
{
    return a._c.first < b._c.first;
}*/


std::ostream & operator << (std::ostream & o, const pair_t& obj)
{
    pair_t::key_t key;
    strcpy(key,obj._key);
//    cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" Position load 0, key: "<<key<<endl;
    char * pos=strchr(key,STR_SEP);
    if(pos)
        *pos='\t';
    o << key<<"\t"<<obj.cooc<<
		"\t"<<obj.sum_s<<"\t"<<obj.sum_t<<"\t"<<
		obj.for_score<<"\t"<<obj.back_score<<"\t"<<obj.for_lex_score<<"\t"<<
		obj.back_lex_score;
	return o;
}


std::ostream & operator << (std::ostream & o, const LexMap & lmap)
{
    char tmp_key[MAX_STR]={'\x0'};
    cerr<<"Writing"<<endl;
    for(auto const & p:lmap){
        strcpy(tmp_key,p.first.c_str());
        break_pair(tmp_key);
        o<<first_part(tmp_key)<<" "<<last_part(tmp_key)<<" "<<p.second<<endl;
    }
    return o;
}



void create_phrase_table(File lex_file,File revlex_file,File extract_file,
                        element_vector  ** cooc_vec_ptr,//PhraseIdMap * sidmap,PhraseIdMap * tidmap,
                        global_t & g,
						 const Options *opts,
                        string out_fn="pt.stxxl")
{
    int rank=MPI::COMM_WORLD.Get_rank();

     element_vector * cooc_vec=*cooc_vec_ptr;
     Crono timer;

     remove(out_fn.c_str());

     ROOT_MSG cout<<"\n=====================\nLoading lexical tables\n=====================\n"<<endl;
	 fflush ( stdout );

     LexMap * lex= new LexMap,
            *revlex= new LexMap;

     timer.start();
     #pragma omp parallel sections
    {
        #pragma omp section
        {
            create_lex_map(lex_file,lex);
        }
        #pragma omp section
        {
            create_lex_map(revlex_file,revlex);
        }
    }
//     if(rank==0){
//         cout<<*lex;
//         MPI::Finalize();
//         exit(0);
//     }
    timer.stop();
    ROOT_MSG cout<<"Done loading!\n";
    string time_str=Crono::formatted_min_max_span(timer);
    ROOT_MSG cout<<"Lexical tables loaded in "
            <<time_str
            <<"\n";
    //printmap (&lex);
    ROOT_MSG cout<<"Lexical table (source-target) has "<<lex->size()<<" entries.\nLexical table (target-source) has "
                        <<revlex->size()<<" entries."<<endl;

    ROOT_MSG cout<<"\n=====================\nLoading phrase pairs\n=====================\n"<<endl;
	fflush ( stdout );
    timer.reset();
    timer.start();

    load_data(extract_file,cooc_vec,lex,revlex,opts);

    timer.stop();

//     cout<<"Process "<<rank<<"finished loading in "<<timer.formatted_span()<<endl;

    delete lex;
    delete revlex;


    ROOT_MSG cout<<"Done loading!\n";
    time_str=Crono::formatted_min_max_span(timer);
    ROOT_MSG cout<<"Data loaded in "
            <<time_str
            <<endl;

    ROOT_MSG cout<<"\n=====================\nScoring phrases\n=====================\n"<<endl;
	fflush ( stdout );
    timer.reset();
    timer.start();
    unsigned long tb;
	if(opts->assoc_method=="cooc")
		tb=score(cooc_vec_ptr,&g,opts);
	else
		tb=score_assoc(cooc_vec_ptr,&g,opts);


    timer.stop();
    cooc_vec=*cooc_vec_ptr;

    ROOT_MSG cout<<"Done scoring!\n";
    time_str=Crono::formatted_min_max_span(timer);
    ROOT_MSG cout<<"Scoring took "
            <<time_str
            <<endl;
    timer.reset();
    timer.start();
//#ifdef File
//#undef File
//#endif

//    MPI::File::Delete(out_fn.c_str(), MPI::INFO_NULL);//


//    MPI::COMM_WORLD.Barrier();
    ROOT_MSG cout<<"\n=====================\nSaving to disk\n=====================\n"<<endl;
	fflush ( stdout );
//    ostringstream outfn;
//    outfn<<out_fn<<"."<<rank;
//    MPI::File fho=MPI::File::Open(MPI::COMM_WORLD,out_fn.c_str(),MPI::MODE_CREATE | MPI::MODE_WRONLY,MPI::INFO_NULL);
//    File fho=fopen(outfn.str().c_str(),"w");
    //Low-level IO,

//     int fho=open(out_fn.c_str(),O_WRONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );


//#define File FILE *
    psave_to_file(out_fn,tb,cooc_vec,&g,opts);
    timer.stop();

//     cout<<"Process "<<rank<<" done writing to disk in "<<timer.formatted_span()<<endl;
//    fho.Close();
//    MPI::COMM_WORLD.Barrier();
    time_str=Crono::formatted_min_max_span(timer);
    ROOT_MSG cout<<"Writing successfully done in "
           <<time_str
            <<endl;

	fflush ( stdout );
// 	for(int i=MPI::COMM_WORLD.Get_size()-1;i>=0;--i){
// 		if (MPI::COMM_WORLD.Get_rank()==i){
// 			close(fho);
// 		}
// 		MPI::COMM_WORLD.Barrier();
// 	}

//     close(fho);
}

char * get_lex_fname(AnyOption * opt)
{
    char * fname=opt->getValue( 'l' );
    if(fname==NULL)
        return opt->getValue( "lex" ) ;
    return fname;
}

char * get_revlex_fname(AnyOption * opt)
{
    char * fname=opt->getValue( 'L' );
    if(fname==NULL)
        return opt->getValue( "revlex" ) ;
    return fname;
}

char * get_extract_fname(AnyOption * opt)
{
    char * fname=opt->getValue( 'e' );
    if(fname==NULL)
        return opt->getValue( "extract" ) ;
    return fname;
}


char * get_output_dir(AnyOption * opt)
{
    char * dirname=opt->getValue( 'd' );
    if(dirname==NULL)
        return opt->getValue( "output-dir" ) ;

    return dirname;
}
/*
char * get_stat_fname(AnyOption * opt)
{
    char * fname=opt->getValue( 's' );
    if(fname==NULL)
        return opt->getValue( "stat" ) ;
    return fname;
}*/

char * get_output_fname(AnyOption * opt)
{
    char * fname=opt->getValue( 'o' );
    if(fname==NULL)
        return opt->getValue( "output" ) ;
    return fname;
}
void get_work_dirs(AnyOption * opt,str_vec_t & dirs)
{
    char * list_str=opt->getValue( 'w' );
    if(list_str==NULL)
        list_str=opt->getValue( "work-dir" );
    if(list_str==NULL){
        dirs.push_back(DEFAULT_TMP);
        return;
    }

    const char *name_sep =",;";
    char * name,*savepos;

    name=strtok_r(list_str,name_sep,&savepos);
    while(name){
        dirs.push_back(name);
        name=strtok_r(NULL,name_sep,&savepos);
    }

//    for( int argc = 0 ; argc < opt->getArgc() ; argc++, fname=strtok_r(NULL,fname_sep,&savepos)){
//        if(!fname){
//            stats.push_back(def_stat4pt(opt->getArgv( argc )));//name.substr(pos).append(DEFAULT_STAT));
//            continue;
//        }
//        stats.push_back(fname);
//
//    }

}
//const char * get_tmp_working_dir(AnyOption * opt)
//{
//    char * dname=opt->getValue( 'w' );
//    if(dname==NULL)
//        dname=opt->getValue( "work-dir" ) ;
//    if(dname==NULL)
//        return DEFAULT_TMP;
//
//    return dname;
//}
//unsigned short get_ncpus(AnyOption * opt,char short_opt='n',
//                        const char * long_opt="ncpus",unsigned short default_val=0)
//{
//    char * n_str=opt->getValue( short_opt );
//    if(n_str==NULL)
//        n_str=opt->getValue( long_opt ) ;
//
//    if(n_str==NULL)
//        return default_val;
//    return atoi(n_str);
//}
unsigned long get_size(AnyOption * opt,char short_opt='t',
                        const char * long_opt="tmp",unsigned long default_val=DEFAULT_TMP_SIZE)
{
    char * sizestr=opt->getValue( short_opt );
    if(sizestr==NULL)
        sizestr=opt->getValue( long_opt ) ;

    if(sizestr==NULL)
        return default_val;
    unsigned long size=strtoul(sizestr,NULL,10);

    if(size==0)
        return default_val;
    return size;
}
/*
string def_stat4pt(char * ptfname)
{
   string name(ptfname);
   int pos=name.rfind(PATH_SEP);
   if(pos==string::npos)
       pos=0;
   else
       ++pos;
   return name.substr(0,pos).append(DEFAULT_STAT);

}*/
//
//void get_stat_files(AnyOption * opt,file_names_t & stats)
//{
//    char * list_str=opt->getValue( 'g' );
//    if(list_str==NULL)
//        list_str=opt->getValue( "globals" );
//    if(list_str==NULL)
//        list_str=(char*)"";
//
//    const char *fname_sep =",;";
//    char * fname,*savepos;
//
//    fname=strtok_r(list_str,fname_sep,&savepos);
//
//    for( int argc = 0 ; argc < opt->getArgc() ; argc++, fname=strtok_r(NULL,fname_sep,&savepos)){
//        if(!fname){
//            stats.push_back(def_stat4pt(opt->getArgv( argc )));//name.substr(pos).append(DEFAULT_STAT));
//            continue;
//        }
//        stats.push_back(fname);
//    }
//
//}

string get_format(AnyOption * opt)
{
    char * fname=opt->getValue( 'f' );
    if(fname==NULL)
        fname=opt->getValue( "format" ) ;

    if(fname)
        return fname;
    return "";
//    if(fmt=="moses" || fmt=="rmoses")
//        return fmt;

//    return string("sttk");

}


string get_xi_func(AnyOption * opt)
{
    char * fname=opt->getValue( 'x' );
    if(fname==NULL)
        fname=opt->getValue( "xi-func" ) ;

    if(fname)
        return fname;
    return "";
//    if(fmt=="moses" || fmt=="rmoses")
//        return fmt;

//    return string("sttk");

}



string get_smoothing_dist(AnyOption * opt)
{
    char * fname=opt->getValue( 's' );
    if(fname==NULL)
        fname=opt->getValue( "smoothing-dist" ) ;

    if(fname)
        return fname;
    return "";
//    if(fmt=="moses" || fmt=="rmoses")
//        return fmt;

//    return string("sttk");

}


string get_lex_aggregator(AnyOption * opt)
{
    char * fname=opt->getValue( 'a' );
    if(fname==NULL)
        fname=opt->getValue( "lex-aggregator" ) ;
//     string lex="";
    if(fname)
        return fname;
    return "";
//    if(fmt=="moses" || fmt=="rmoses")
//        return fmt;

//    return string("sttk");

}


string get_unseen_estimator(AnyOption * opt)
{
    char * fname=opt->getValue( 'u' );
    if(fname==NULL)
        fname=opt->getValue( "unseen-estimator" ) ;
//     string ue="";
    if(fname)
        return fname;
    return "";
//    if(fmt=="moses" || fmt=="rmoses")
//        return fmt;

//    return string("sttk");

}


string get_lex_scoring(AnyOption * opt)
{
    char * fname=opt->getValue( 'j' );
    if(fname==NULL)
        fname=opt->getValue( "join-lexical" ) ;

    if(fname)
        return fname;
    return "";
//    if(fmt=="moses" || fmt=="rmoses")
//        return fmt;

//    return string("sttk");

}

string get_assoc_method(AnyOption * opt)
{
    char * fname=opt->getValue( 'm' );
    if(fname==NULL)
        fname=opt->getValue( "association-method" ) ;

    if(fname)
        return fname;
    return "";

}

int main(int argc, char** argv)
{
    int rank, size;

    MPI::Init_thread(argc, argv, MPI_THREAD_MULTIPLE);
    MPI::COMM_WORLD.Set_errhandler ( MPI::ERRORS_THROW_EXCEPTIONS );
//    MPI::Init(argc, argv);
    rank = MPI::COMM_WORLD.Get_rank();
    size = MPI::COMM_WORLD.Get_size();


    AnyOption *opt = new AnyOption();
//    opt->autoUsagePrint(true);

    string usage_str("Usage: ");
    usage_str.append(argv[0]).append(" [<options>]");

    opt->addUsage( "" );
    opt->addUsage( usage_str.c_str() );
    opt->addUsage( "" );
    opt->addUsage( "Options: " );
    opt->addUsage( " -h  --help\t\tPrints this help " );
    opt->addUsage( " -o  --output\t\tOutput file name (default: 'stdout')" );
//    opt->addUsage( " -g  --globals\t\tComma-separated list of stat files, one for each phrase table respectively (default: '/path/to/pt/"DEFAULT_STAT"')" );
    opt->addUsage( " -l  --lex\t\tSource-target lexical file name (default: '"DEFAULT_LEX"')" );
    opt->addUsage( " -L  --revlex\t\tTarget-source lexical file name (default: '"DEFAULT_REVLEX"')" );
    opt->addUsage( " -e  --extract\t\tExtract file name (default: '"DEFAULT_EXTRACT"')" );

//     opt->addUsage( " -n  --ngrams\t\tOptionally write only matching source phrases to the provided ngram list (default: '')" );
//     opt->addUsage( " -r  --match-raw-text\t\tOptionally write only matching source phrases to the provided test set (default: '')" );

    opt->addUsage( " -b  --mem-block\tThe memory block used by STXXL in MB (default: 512 MB)" );
    opt->addUsage( " -d  --output-dir\tOutput directory. This directory name will be prepended to all output file names, if given (default: '')" );
    opt->addUsage( " -f  --format\t\t{sttk|moses|rmoses} phrase table format (default: 'sttk')" );
    opt->addUsage( " -c  --compress\t\tWhether the output table will be zipped or not (default: false)" );
    opt->addUsage( " -p  --prepare\t\tWhether to prepare the working space (default: false)" );
    opt->addUsage( " -w  --work-dir\t\tIf {-p|--prepare} is true, this option specifies the temporary working directory(ies), in which STXXL files should be created (default: /tmp), comma-separated if many" );
    opt->addUsage( " -t  --tmp\t\tIf {-p|--prepare} is true, this option specifies the size of the temporary space in MB (default: 0 MB)" );
    opt->addUsage( " -k  --keep-tmp\t\tIf {-p|--prepare} is true, this flag states whether to remove the temporary allocated space or leave it for further usage (default: false)" );

	opt->addUsage( " -a  --lex-aggregator\t\t{max|max-occur|average|m|o|a} lexical score selection method: maximum, most occurring, or the average. (default: max)" );
	opt->addUsage( " -x  --xi-func\t\t{kn|hyp|hyp1|log|log1|exp|exp1|none} the discounting function (xi). (default: kn)" );
    opt->addUsage( " -s  --smoothing-dist\t\t{uniform|unigram|aunigram|lower-order|none|f|g|ag|l|n} the smoothing distribution. (default: unigram)" );
    opt->addUsage( " -u  --unseen-estimator\t\t{none|chao|bias-corrected-chao|nonparametric-lindley|geometric|good-turing|c|b|l|g|gt} the unseen event estimator. (default: none, meaning the unseen events is estimated using leave-one-out)" );

    opt->addUsage( " -m  --association-method\t\t{cooc|chi-square|chi2...} the association scoring method. (default: cooc)" );

    opt->addUsage( " -j  --join-lexical\t\t{average|noisy-or|geometric-mean|a|nor|gm} Method used to combine lexical scores. (default: average)" );
    opt->addUsage( " -N  --no-null-in-lexical-tables\t\tLexical tables contain no entries for NULL. In such case, the alignment is not used, and a matrix between target and source words is considered. (default: false)" );

    opt->addUsage( "" );

    opt->setFlag(  "help", 'h' );
    opt->setFlag(  "compress", 'c' );
    opt->setFlag(  "prepare", 'p' );
    opt->setFlag(  "keep-tmp", 'k' );
    opt->setFlag(  "no-null-in-lexical-tables", 'N' );

    opt->setOption(  "lex", 'l' );
    opt->setOption(  "revlex", 'L' );
    opt->setOption(  "extract", 'e' );
    opt->setOption(  "join-lexical", 'j' );

    opt->setOption(  "mem-block", 'b' );
    opt->setOption(  "output-dir", 'd' );
    opt->setOption(  "output", 'o' );
    opt->setOption(  "format", 'f' );
    opt->setOption(  "work-dir", 'w' );
    opt->setOption(  "tmp", 't' );
//     opt->setOption(  "stat", 's' );
    opt->setOption(  "globals", 'g' );

    opt->setOption(  "lex-aggregator", 'a' );
    opt->setOption(  "xi-func", 'x' );
    opt->setOption(  "smoothing-dist", 's' );

    opt->setOption(  "unseen-estimator", 'u' );
//    opt->setOption(  "ncpus", 'n' );

    opt->setOption(  "association-method", 'm' );

    opt->processCommandArgs( argc, argv );


    if( opt->getFlag( "help" ) || opt->getFlag( 'h' ) ){

          ROOT_MSG  opt->printUsage();
        MPI::Finalize();
        exit(0);
    }

    ostream * table_out_strm=NULL;
//    MPI::File output;

    string out_dir;
    try{

        out_dir=string(get_output_dir(opt));

    }catch(logic_error){

        out_dir=string("");
    }

    if(out_dir !=""){
        if(*out_dir.rend()!=PATH_SEP)
            out_dir.append(1,PATH_SEP);
        ROOT_MSG cout<<"Output directory: '"<<out_dir<<"'"<<endl;
        mkdir(out_dir.c_str(),0744);
    }

    /****************output file name***********/

    string output_fn;
    try{
        output_fn=string(get_output_fname(opt));
    }catch(logic_error){
        output_fn=string("");
    }

//    const char * output_fn=get_output_fname(opt);
    if( output_fn != ""  ){
        output_fn=string (out_dir).append(output_fn);

        ROOT_MSG  cout << "Output file name: '" << output_fn<<endl;
//        if(opt->getFlag( "compress" ) || opt->getFlag( 'c' )){
//            output=fopen(string(output_fn).append(".gz").c_str(),"w");
//
//            ROOT_MSG  cout<<".gz";
//        }else
//            table_out_strm=new ofstream(output_fn);
//        ROOT_MSG cout<<"'"<<endl;
//    }else{
//        if(opt->getFlag( "compress" ) || opt->getFlag( 'c' )){
//            output=fdopen(dup(1),"w");
//            dup2(2,1);
//        }else{
//            table_out_strm=new ostream(cout.rdbuf());
//            cout.rdbuf(cerr.rdbuf());
//        }
//        ROOT_MSG cout<<"No output file name was specified!\nThe output is redirected to 'stderr' and the phrase table will be written to 'stdout'\n";

    }else{
        //FIXME: this later
        output_fn=string (out_dir).append("pt.pstxxl");
    }

    if( output_fn=="" && (opt->getFlag( "compress" ) || opt->getFlag( 'c' ))) {
	
        ROOT_MSG cout<<"Compression is yet suppported for distributed version!\n";
	MPI::Finalize();
        exit(0);
    }
    /**************** EXTRACT and LEX FILES *************/

    char  def_lex[]=DEFAULT_LEX;
    char def_revlex[]=DEFAULT_REVLEX;
    char def_ext[]=DEFAULT_EXTRACT;

    char * lex_fn=get_lex_fname(opt);
    if( lex_fn != NULL  ){
        ROOT_MSG cout << "Source-target lexical file name: "  ;
    }else{
        lex_fn=def_lex;
        ROOT_MSG cout << "Source-target lexical file name not specified, taking the default: "  ;
        //cout<<"Please provide a source-target lexical file!\n";
    }
    ROOT_MSG cout<<"'"<< lex_fn<<"'"<< endl;
    char * revlex_fn=get_revlex_fname(opt);
    if( revlex_fn != NULL  ){
        ROOT_MSG cout << "Target-source lexical file name: "  ;
    }else{
        revlex_fn=def_revlex;
        ROOT_MSG cout << "Target-source lexical file name not specified, taking the default: "  ;
        //cout<<"Please provide a target-source lexical file!\n";
    }
    ROOT_MSG cout<< "'"<<revlex_fn<<"'"<< endl;

    char * extract_fn=get_extract_fname(opt);
    if( extract_fn != NULL  ){
        ROOT_MSG cout << "Extract file name: "  ;
    }else{
        extract_fn=def_ext;
        ROOT_MSG cout << "Extract file name not specified, taking the default: "  ;
        //cout<<"Please provide an extract file!\n";
    }
    ROOT_MSG cout<<"'"<< extract_fn<< "'"<<endl;
    /*************************STAT FILE***************/
/*
    char default_stat[]=DEFAULT_STAT;

    string  stat_fn;//=get_stat_fname(opt);

    try{
        stat_fn=string(get_stat_fname(opt));
    }catch(logic_error){
        stat_fn=string("");
    }

    if( stat_fn != ""  ){
        stat_fn=string (out_dir).append(stat_fn);

        ROOT_MSG cout << "Stat file name: "  ;
    }
    else{
        stat_fn=string (out_dir).append(default_stat);
        ROOT_MSG cout << "Stat file name not specified, saving to: "  ;
        //cout<<"Please provide an extract file!\n";
    }
    ROOT_MSG cout<<"'"<< stat_fn<< "'"<<endl;*/
	Options opts;
    /*****************TABLE FORMAT*******************/

     string tmp=get_format(opt);
	 if(tmp!= "")
		 opts.output_fmt=tmp;
    //if( table_format!= NULL  )
    ROOT_MSG cout << "Format: '"<<opts.output_fmt<<"'\n";

    //cout << "Format: " << (table_format==0? "'sttk'":table_format==1? "'moses'":"unknown")<< endl ;
//    ROOT_MSG cout<<endl;
    /***********************Test existence of necessary files***********************/
     File lex_f=fopen(lex_fn,"r");
	if(!lex_f)
		ROOT_MSG cerr<<"Could not open file: '"<<lex_fn<<"'\n";
    File revlex_f=fopen(revlex_fn,"r");
	if(!revlex_f)
		ROOT_MSG cerr<<"Could not open file: '"<<revlex_fn<<"'\n";

    File extract= fopen(extract_fn,"r");

    if(!extract)
		ROOT_MSG cerr<<"Could not open file: '"<<extract_fn<<"'\n";

    if(!lex_f || !revlex_f || !extract){
        MPI::COMM_WORLD.Abort( EXIT_FAILURE );
    }


    /*****************Number of cores*********************/
//    unsigned short ncpus=get_ncpus(opt);
    ostringstream nump;
//    if(ncpus){
//        nump<<ncpus;
//        setenv("OMP_NUM_THREADS",nump.str().c_str(),1);
//    }else{
        nump<<omp_get_num_procs();
        setenv("OMP_NUM_THREADS",nump.str().c_str(),0);
//    }
    ROOT_MSG cout<<"Number of Processes: "<<size<<endl;
    ROOT_MSG cout<<"Number of threads per process: "<<getenv("OMP_NUM_THREADS")<<endl;

    MPI::COMM_WORLD.Barrier();
    char host[MAXHOSTNAMELEN];
    gethostname(host,sizeof host);
    cout<<"Process "<<rank<<" is on host: "<<host<<endl;

    /***************** Sort Memory*********************/
    opts.mem=get_size(opt,'b',"mem-block",DEFAULT_MEM_BLOCK);
    ROOT_MSG cout<<"STXXL internatl memory size: "<<opts.mem<<" MB"<<endl;
    opts.mem *= 1024*1024; //from mega to bytes

	/***************** Lexical aggregation function *****************/
	tmp=get_lex_aggregator(opt);
	if(tmp !="")
		opts.lex_aggergator=tmp;
	ROOT_MSG cout<<"Lexical aggregation method: "<<opts.lex_aggergator<<endl;

	/***************** Discounting function *****************/
	tmp=get_xi_func(opt);
	if(tmp !="")
		opts.xi_func=tmp;
	ROOT_MSG cout<<"Discounting function: "<<opts.xi_func<<endl;

    /***************** Smoothing distribution *****************/
    tmp=get_smoothing_dist(opt);
    if(tmp !="")
        opts.smoothing_dist=tmp;

    ROOT_MSG cout<<"Smoothing distribution: "<<opts.smoothing_dist<<endl;

    /***************** Unseen estimator *****************/
    tmp=get_unseen_estimator(opt);
    if(tmp !="")
        opts.unseen_estimator=tmp;
    ROOT_MSG cout<<"Unseen estimator: "<<opts.unseen_estimator<<endl;


    /***************** Lexical table: is it smoothed?  *****************/

    if( opt->getFlag( "no-null-in-lexical-tables" ) || opt->getFlag( 'N' ) ){
		opts.smoothed_lex=true;
		ROOT_MSG cout<<"Assuming smoothed lexical tables"<<endl;
	}

    /***************** Unseen estimator *****************/
    tmp=get_lex_scoring(opt);
    if(tmp !="")
        opts.lex_scoring=tmp;
    ROOT_MSG cout<<"Lexical decomposition method: "<<opts.lex_scoring<<endl;

    /***************** Association scoring method *****************/
    tmp=get_assoc_method(opt);
    if(tmp !="")
        opts.assoc_method=tmp;
	if ((opts.assoc_method =="cooc") && (opts.smoothing_dist=="aunigram" || opts.smoothing_dist=="ag")){
		opts.smoothing_dist="unigram";
	}
    ROOT_MSG cout<<"Association scoring method: "<<opts.assoc_method<<endl;

    /*****************Temporary space management*********************/
    bool remove_tmp_at_exit=false;
    str_vec_t working_dirs;

    if( opt->getFlag( "prepare" ) || opt->getFlag( 'p' ) ){
        get_work_dirs(opt,working_dirs);
//        working_dir=get_tmp_working_dir(opt);
        ROOT_MSG cout<<"Working directory (ies): ";//<<working_dir<<"'\n";
        for(str_vec_t::const_iterator dname_ptr=working_dirs.begin();dname_ptr!=working_dirs.end();++dname_ptr)
            ROOT_MSG cout<<"'"<<*dname_ptr<<"', ";
        ROOT_MSG cout<<endl;

        unsigned long tmp_size=get_size(opt);
        ROOT_MSG cout<<"Working space size: "<<tmp_size<<" MB"<<(tmp_size>0? "\n":" (Auto-grow)\n");


        remove_tmp_at_exit=!(opt->getFlag( "keep-tmp" ) || opt->getFlag( 'k' ));
        ROOT_MSG cout<<"The disk space allocated will be "<<(remove_tmp_at_exit? "deallocated":"kept")<<" after finishing\n";

        ROOT_MSG cout<<"\n=============================================\n\n";
//        timer.start();
        prepare(tmp_size,working_dirs);
//        timer.stop();
        ROOT_MSG cout<<"\nPreparing the disk working space took "<<
//                timer.formatted_span()<<
                endl<<endl;
//        timer.reset();
    }

    /**************************************/
   //open extract file for parallel IO
//    File extract_f=fopen(extract_fn,"r");//MPI::COMM_WORLD, extract_fn,MPI::MODE_RDONLY,MPI::INFO_NULL);

    //Creating the phrase table
    element_vector * phrase_cooc=new element_vector;
    assert(phrase_cooc != NULL);
    global_t gs;
    Crono timer;
    timer.start();

    create_phrase_table(lex_f,revlex_f,extract,&phrase_cooc,gs,&opts,output_fn);
    fclose(lex_f);
    fclose(revlex_f);
    fclose(extract);
    timer.stop();

    string time_str=Crono::formatted_min_max_span(timer);

//    extract_f.Close();
    /**********************************************/
    //output some findings
//    ROOT_MSG cout<<"\n========================================\nGlobal scores \n========================================\n"<<endl;
//    cout.precision(2);
//    ROOT_MSG cout<<"N1:\t"<<setw(20)<<gs.big_n1<<" ("<<100*(double)gs.big_n1/gs.all<<"%)"<<endl;
//    ROOT_MSG cout<<"N2:\t"<<setw(20)<<gs.big_n2<<" ("<<100*(double)gs.big_n2/gs.all<<"%)"<<endl;
//    ROOT_MSG cout<<"N3:\t"<<setw(20)<<gs.big_n3<<" ("<<100*(double)gs.big_n3/gs.all<<"%)"<<endl;
//    ROOT_MSG cout<<"N4:\t"<<setw(20)<<gs.big_n4<<" ("<<100*(double)gs.big_n4/gs.all<<"%)"<<endl;
//    ROOT_MSG cout<<"#Pairs:\t"<<setw(20)<<gs.all<<endl;
    /******************************************/
    ROOT_MSG cout<<"\nDone creating the phrase table!\nThe whole process took "<<
            time_str<<
//            timer.formatted_span()<<
            endl;
//     ROOT_MSG
//     {
//         cout<<"\nSaving the global scores to '"<<stat_fn<<"'...\n";
//          string rename_stat=string(stat_fn).append(".").append(tmp_name("",false)).append(".old");
//
//
//     //     string sf(stat_fn);
//          if(rename(stat_fn.c_str(),rename_stat.c_str())==0)
//               cout<<"The file '"<<stat_fn.c_str()<<"' existed before and was renamed to '"<<rename_stat<<"'\n";
//
//         ofstream stat_f(stat_fn.c_str(),ios::out);
//         if(!stat_f){
//              cerr<<"Could not open file: '"<<stat_fn<<"' for writing\n";
//         }else{
// //            stat_f<<"N1 = "<< gs.big_n1<<"\nN2 = "<<gs.big_n2 <<"\nN3 = "<<gs.big_n3
// //                    <<"\nN4 = "<< gs.big_n4<<"\nall = "<< gs.all<<"\n";
//     //        fprintf(stat_f,"N1 = %ld\nN2 = %ld\nN3 = %ld\nN4 = %ld\nall = %ld\n",
//     //                gs.big_n1,gs.big_n2,gs.big_n3,gs.big_n4,gs.all);
//     //        fclose(stat_f);
//             stat_f.close();
//         }
//     }
    /***************************/

    ROOT_MSG cout<<"Releasing resources...\n";
    delete phrase_cooc;
    if(remove_tmp_at_exit && ! working_dirs.empty()){
        ROOT_MSG cout<<"Releasing temporary disk space...\n";
        remove_tmp(working_dirs);
    }

//    if( output_fn != ""  ){
//        //         cout<<"closing output file\n";
//        if(opt->getFlag( "compress" ) || opt->getFlag( 'c' ))
//            fclose(output);
//        else
//            ((ofstream *)table_out_strm)->close();
//    }else{
//        //         cout<<"restoring stdout\n";
//        if(opt->getFlag( "compress" ) || opt->getFlag( 'c' )){
//            fclose(output);
//            //dup2(fileno(output),1);
//        }else
//            cout.rdbuf(table_out_strm->rdbuf());
//    }

    //     cout<<"stdout closed\n";
//    for(file_list_t::iterator f=file_list.begin();f!=file_list.end();++f){
//        fclose(*f);
//    }

    //     cout<<"All files were closed\n";
    delete opt;
    //     cout<<"opt deleted\n";
    if(table_out_strm)
        delete table_out_strm;
    ROOT_MSG cout<<"Scoring successfully finishing!\n";
//    MPI::COMM_WORLD.Barrier();
    /*++++++++++++++++++++*/
    MPI::Finalize();

    return (EXIT_SUCCESS);
}

