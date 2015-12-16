#ifndef _CRONO_H
#define _CRONO_H

#include <sys/time.h>
#include <string>
#include <iostream>
#include <sstream>

class Crono
{
private:
    /*clock_t*/
    double s_clocks;
    /*clock_t*/
    double e_clocks;
    //bool started;
    double _span;
public:
    Crono():s_clocks(0),e_clocks(0),/*started(false),*/_span(0.0) {}
    void start()
    {
        timeval current;

        gettimeofday(&current, NULL);
        s_clocks=current.tv_sec+(current.tv_usec/1000000.0);

        //if(!started){
        //	s_clocks=time(NULL);//clock();
        //started=true;
        //}
    }
    void stop()
    {
        timeval current;
        gettimeofday(&current, NULL);
        e_clocks=current.tv_sec+(current.tv_usec/1000000.0);//time(NULL);//clock();
        _span+=double(e_clocks-s_clocks);//CLOCKS_PER_SEC;
    }
    void reset()
    {
        //started=false;
        _span=0.0;
    }
    double span()
    {
        return _span;
    }
    double span_plus(double plus)
    {
        return _span+plus;
    }
    string formatted_span(double plus)
    {
        double old_span=_span;
        _span+=plus;
        string res=formatted_span();
        _span=old_span;
        return res;
    }
    string formatted_span()
    {
        int total=int(_span);
        int hr=total/3600;
        int mn=(total%3600)/60;
        float sc=(total%3600)%60 + _span-total;
        char str[100];
        if(hr)
            sprintf(str,"%dh%dm%0.4fs",hr,mn,sc);
        else if(mn)
            sprintf(str,"%dm%0.4fs",mn,sc);
        else
            sprintf(str,"%0.4fs",sc);
        return string(str);
    }

    static Crono max_span(const Crono & c)
    {
        Crono max_timer;

        MPI::COMM_WORLD.Reduce (&(c._span), &(max_timer._span), 1, MPI::DOUBLE, MPI::MAX, 0);
        return max_timer;
    }
    static Crono min_span(const Crono & c)
    {
        Crono min_timer;

        MPI::COMM_WORLD.Reduce (&(c._span), &(min_timer._span), 1, MPI::DOUBLE, MPI::MIN, 0);
        return min_timer;
    }
    static string formatted_min_max_span(const Crono & c)
    {
        double maxs,mins;
        ostringstream fspan;

        MPI::COMM_WORLD.Reduce (&(c._span), &(maxs), 1, MPI::DOUBLE, MPI::MAX, 0);
        MPI::COMM_WORLD.Reduce (&(c._span), &(mins), 1, MPI::DOUBLE, MPI::MIN, 0);

        fspan<<"(max: "<<Crono().formatted_span(maxs)<<", min: "<<
        Crono().formatted_span(mins)<<")";
        return fspan.str();
    }

};
#endif
