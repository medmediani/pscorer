/***********************************************************
 * This file is part of DEMSort, a high-performance        *
 * distributed-memory external-memory sorting program      *
 *                                                         *
 * Copyright (C) 2011 Mirko Rahn and Johannes Singler      *
 * Karlsruhe Institute of Technology (KIT), Germany        *
 *                                                         *
 * All rights reserved.  Free for academic use.            *
 ***********************************************************/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <stxxl/io>

#include "global_types.h"

struct DemsortSettings
{
    enum MwselVariant
    {
        MWSEL_SIMPLE,
        MWSEL_COLLECTIVE,
        MWSEL_VARMAN
    };

    local_byte_index memory_per_pe; //amount of internal memory to use per PE in bytes
    bool aio_file;                  //do kernel-based asynchronous I/O?
    bool randomize_runs;            //randomize runs?
    bool overlap_read_and_sort;     //sort each block immediately after reading?
    phase_type num_phases_make_runs;//number of overlapping phases for make runs
    MwselVariant mwsel_variant;     //variant of the mwsel algorithm to execute
    bool quicksort;                 //use parallel quicksort in order to save memory
    phase_type num_phases_alltoall; //number of overlapping phases for all-to-all
    bool shrink_input_file;         //shrink input file?
    bool file_per_run;              //one file per run?  temporary block-wise storage otherwise
    global_ext_byte_index truncate_block_size; //round to this block size when truncating
    global_ext_byte_index max_space_overhead;  //maximum tolerable space overhead at any point in time
    const char* fpr_stem;          //stem for file-per-run files
    const char* fpr_stem_spec_file;//specification file for stem for file-per-run files
    bool keep_intermediates;        //keep the intermediate file-per-run files?
    int prefill_percentage;         //prefill this percentage of the output, in order to speed up subsequent writes to the output
    bool range_io;                  //use range I/O?
    int verbose;                    //verboseness level
    bool use_standard_alltoallv;    //use MPI_Alltoallv or own implementation?
    bool show_io_stats;

    DemsortSettings() :
        memory_per_pe(0),
        aio_file(false),
        randomize_runs(false),
        overlap_read_and_sort(false),
        num_phases_make_runs(2),
        mwsel_variant(MWSEL_SIMPLE),
        quicksort(false),
        num_phases_alltoall(2),
        shrink_input_file(false),
        file_per_run(false),
        truncate_block_size(-1),
        max_space_overhead(-1), //at least one block, for padding
        fpr_stem("fpr_"),
        fpr_stem_spec_file(NULL),
        keep_intermediates(false),
        prefill_percentage(0),
        range_io(false),
        verbose(0),
        use_standard_alltoallv(false),
        show_io_stats(false)
    { }

    stxxl::file* create_file(const std::string& filename, int mode, int queue_id) const
    {
    #if STXXL_HAVE_AIO_FILE
        if (aio_file)
            return new stxxl::aio_file(filename, mode, queue_id);
        else
    #endif
            return new stxxl::syscall_file(filename, mode, queue_id);
    }

    const char* show_mwsel_variant()
    {
        switch (mwsel_variant)
        {
        case MWSEL_SIMPLE:          return "SIMPLE";
        case MWSEL_COLLECTIVE:      return "COLLECTIVE";
        case MWSEL_VARMAN:          return "VARMAN";
        default:                    return "<STRANGE: UKNOWN MWSEL ALGORITHM>";
        }
    }

    void read_mwsel_algorithm(const char* const descr)
    {
        switch (*descr)
        {
        case 's': case 'S': mwsel_variant = MWSEL_SIMPLE; break;
        case 'c': case 'C': mwsel_variant = MWSEL_COLLECTIVE; break;
        case 'v': case 'V': mwsel_variant = MWSEL_VARMAN; break;
        default:
            std::cerr << "NOTE: unknown mwsel algorithm '" << descr << "'"
                      << ", fall back to SIMPLE" << std::endl;
            mwsel_variant = MWSEL_SIMPLE;
            break;
        }
    }
};

#endif /*SETTINGS_H*/
