#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
using std::min;
using std::max;

// CUDA thread/block dimension constants (adjust for your workload)
#ifndef BLOCK_DIM_X
#define BLOCK_DIM_X 256
#endif
#ifndef GRID_DIM_X
#define GRID_DIM_X 1
#endif
#ifndef BLOCK_DIM_Y
#define BLOCK_DIM_Y 1
#endif
#ifndef GRID_DIM_Y
#define GRID_DIM_Y 1
#endif

// --- from QTC.cu ---
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <chrono>
#include <sstream>
#include <iostream>
#include <fstream>

#define _USE_MATH_DEFINES
#include <float.h>

using namespace std;


// ****************************************************************************
// Function: addBenchmarkSpecOptions
//
// Purpose:
//   Add benchmark specific options parsing.  The user is allowed to specify
//   the size of the input data in megabytes if they are not using a
//   predefined size (i.e. the -s option).
//
// Arguments:
//   op: the options parser / parameter database
//
// Programmer: Anthony Danalis
// Creation: February 04, 2011
// Returns:  nothing
//
// ****************************************************************************

// ****************************************************************************
// Function: RunBenchmark
//
// Purpose:
//   Calls single precision and, if viable, double precision QT-Clustering
//   benchmark.
//
// Arguments:
//  resultDB: the benchmark stores its results in this ResultDatabase
//  op: the options parser / parameter database
//
// Returns:  nothing
//
// Programmer: Anthony Danalis
// Creation: February 04, 2011
//
// ****************************************************************************

// ****************************************************************************
// Function: calculate_participants
//
// Purpose:
//   This function decides how many GPUs (up to the maximum requested by the user)
//   and threadblocks per GPU will be used. It also returns the total number of
//   thread-blocks across all GPUs and the number of thread-blocks that are in nodes
//   before the current one.
//   In the future, the behavior of this function should be decided based on
//   auto-tuning instead of arbitrary decisions.
//
// Arguments:
//   The number of nodes requested by the user and the four
//   variables that the function computes (passed by reference)
//
//
// Returns:  nothing
//
// Programmer: Anthony Danalis
// Creation: May 25, 2011
//
// ****************************************************************************

// ****************************************************************************
// Function: runTest
//
// Purpose:
//   This benchmark measures the performance of applying QT-clustering on
//   single precision data.
//
// Arguments:
//  resultDB: the benchmark stores its results in this ResultDatabase
//  op: the options parser / parameter database
//
// Returns:  nothing
//
// Programmer: Anthony Danalis
// Creation: February 04, 2011
//
// ****************************************************************************



////////////////////////////////////////////////////////////////////////////////






// --- from Option.h ---
#ifndef OPTION_H
#define OPTION_H

#include <string>

using namespace std;

enum OptionType {OPT_FLOAT, OPT_INT, OPT_STRING, OPT_BOOL,
                 OPT_VECFLOAT, OPT_VECINT, OPT_VECSTRING};

// ****************************************************************************
// Class:  Option
//
// Purpose:
//   Encapsulation of a single option, to be used by an option parser.
//
// Programmer:  Kyle Spafford
// Creation:    August 4, 2009
//
// ****************************************************************************
class Option {

  public:

   string longName;
   char   shortLetter;
   string defaultValue;
   string value;
   OptionType type;
   string helpText;

   void print();
};

#endif


// --- from OptionParser.h ---
#ifndef OPTION_PARSER_H
#define OPTION_PARSER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>


using namespace std;

// ****************************************************************************
// Class:  OptionParser
//
// Purpose:
//   Class used to specify and parse command-line options to programs.
//
// Programmer:  Kyle Spafford
// Creation:    August 4, 2009
//
// ****************************************************************************
class OptionParser
{
  private:
    typedef std::map<std::string, Option> OptionMap;

    OptionMap optionMap;
    map<char, string>   shortLetterMap;

    bool helpRequested;

  public:

    OptionParser();
    void addOption(const string &longName,
                   OptionType type,
                   const string &defaultValue,
                   const string &helpText = "No help specified",
                   char shortLetter = '\0');

    void print() const;

    //Returns false on failure, true on success
    bool parse(int argc, const char *const argv[]);
    bool parse(const vector<string> &args);
    bool parseFile(const string &fileName);

    //Accessors for options
    long long   getOptionInt(const string &name) const;
    float       getOptionFloat(const string &name) const;
    bool        getOptionBool(const string &name) const;
    string      getOptionString(const string &name) const;

    vector<long long>     getOptionVecInt(const string &name) const;
    vector<float>         getOptionVecFloat(const string &name) const;
    vector<string>        getOptionVecString(const string &name) const;

    void printHelp(const string &optionName) const;
    void usage() const;

    bool HelpRequested( void ) const    { return helpRequested; }
};

#endif


// --- from Utility.h ---
#ifndef UTILITY_H
#define UTILITY_H

#include <sstream>
#include <math.h>

// ****************************************************************************
// File:  Utility.h
//
// Purpose:
//   Various generic utility routines having to do with string and number
//   manipulation.
//
// Programmer:  Jeremy Meredith
// Creation:    September 18, 2009
// Modified:    Jan 2010, rothpc
//    Jeremy Meredith, Tue Oct  9 17:25:25 EDT 2012
//    Round is c99, not Windows-friendly.  Assuming we are using
//    positive values, replaced it with an equivalent of int(x+.5).
//
// ****************************************************************************

inline std::string HumanReadable(long long value, long long *rounding=0)
{
    std::ostringstream vstr;
    long long pVal;
    if (value>10ll*1024*1024*1024)
    {
        pVal = (long long)(0.5 + value/(1024.0*1024*1024));
        if (rounding)
            *rounding = pVal*1024*1024*1024 - value;
        vstr << pVal << 'G';
    }
    else if (value>10ll*1024*1024)
    {
        pVal = (long long)(0.5 + value/(1024.0*1024));
        if (rounding)
            *rounding = pVal*1024*1024 - value;
        vstr << pVal << 'M';
    }
    else if (value>10ll*1024)
    {
        pVal = (long long)(0.5 + value/(1024.0));
        if (rounding)
            *rounding = pVal*1024 - value;
        vstr << pVal << 'k';
    }
    else
    {
        if (rounding)
            *rounding = 0;
        vstr << value;
    }
    return vstr.str();
}

inline vector<string> SplitValues(const std::string &buff, char delim)
{
    vector<std::string> output;
    std::string tmp="";
    for (size_t i=0; i<buff.length(); i++)
    {
       if (buff[i] == delim)
       {
          if (!tmp.empty())
             output.push_back(tmp);
          tmp = "";
       }
       else
       {
          tmp += buff[i];
       }
    }
    if (!tmp.empty())
       output.push_back(tmp);

    return output;
}

#ifdef _WIN32

// On Windows, srand48 and drand48 don't exist.
// Create convenience routines that use srand/rand
// and let developers continue to use the -48 versions.

inline void srand48(unsigned int seed)
{
    srand(seed);
}

inline double drand48()
{
    return double(rand()) / RAND_MAX;
}

#endif // _WIN32

#endif


// --- from comm.h ---
#ifndef _COMM_H_
#define _COMM_H_

#if defined(PARALLEL)
#  include "mpi.h"
#endif

#define COMM_TYPE_INT   0
#define COMM_TYPE_FLOAT 1

void comm_update_communicator(int cwrank, int active_node_count);
void comm_find_winner(int *max_card, int *winner_node, int *winner_index, int cwrank, int max_index);
void comm_broadcast( void *ptr, int cnt, int type, int source);
void comm_barrier(void);
int comm_get_size(void);
int comm_get_rank(void);

#endif


// --- from cudacommon.h ---
#ifndef CUDACOMMON_H
#define CUDACOMMON_H

// workaround for OS X Snow Leopard w/ gcc 4.2.1 and CUDA 2.3a
// (undefined __sync_fetch_and_add)
#if defined(__APPLE__)
# if _GLIBCXX_ATOMIC_BUILTINS == 1
#undef _GLIBCXX_ATOMIC_BUILTINS
#endif // _GLIBC_ATOMIC_BUILTINS
#endif // __APPLE__

#include <stdio.h>

// On Windows, if we call exit, our console may disappear,
// taking the error message with it, so prompt before exiting.
#if defined(_WIN32)
#define safe_exit(val)                          \
{                                               \
    cout << "Press return to exit\n";           \
    cin.get();                                  \
    exit(val);                                  \
}
#else
#define safe_exit(val) exit(val)
#endif

#define CHECK_CUDA_ERROR()                                                    \
{                                                                             \

    if (err != cudaSuccess)                                                   \
    {                                                                         \
        printf("error=%d name=%s at "                                         \
               "ln: %d\n  ",err,cudaGetErrorString(err),__LINE__);            \
        safe_exit(-1);                                                        \
    }                                                                         \
}

// Alternative macro to catch CUDA errors
#define CUDA_SAFE_CALL( call) do {                                            \
   cudaError_t err = call;                                                      \
   if (cudaSuccess != err) {                                                  \
       fprintf(stderr, "Cuda error in file '%s' in line %i : %s.\n",          \
           __FILE__, __LINE__, cudaGetErrorString( err) );                    \
       safe_exit(EXIT_FAILURE);                                               \
   }                                                                          \
} while (0)

// Alleviate aliasing issues
#define RESTRICT 

#endif // CUDACOMMON_H


// --- from kernels_common.h ---
#ifndef _KERNELS_COMMON_H_
#define _KERNELS_COMMON_H_

#pragma once
#pragma warning(disable:4996)

#define _USE_MATH_DEFINES
#include <math.h>

// Forward declarations
void QTC_device( float *dist_matrix, char *Ai_mask, char *clustered_pnts_mask, int *indr_mtrx, int *cluster_cardinalities, int *ungrpd_pnts_indr, float *dist_to_clust, int *degrees, int point_count, int N0, int max_degree, float threshold, int cwrank, int node_rank, int node_count, int total_thread_block_count);

int generate_candidate_cluster_compact_storage(int seed_point, int degree, char *Ai_mask, float *compact_storage_dist_matrix, char *clustered_pnts_mask, int *indr_mtrx, float *dist_to_clust, int point_count, int N0, int max_degree, int *candidate_cluster, float threshold);

int find_closest_point_to_cluster(int seed_point, int latest_point, char *Ai_mask, char *clustered_pnts_mask, float *work, int *indr_mtrx, float *dist_to_clust, int pointCount, int N0, int max_degree, float threshold);

void QTC(const string& name, OptionParser& op, int matrix_type);

inline int closest_point_reduction(float min_dist, float threshold, int closest_point){
    float dist_array[THREADSPERBLOCK];
    int point_index_array[THREADSPERBLOCK];

    int tid = _tid_x;
    int curThreadCount = BLOCK_DIM_X*BLOCK_DIM_Y*BLOCK_DIM_Z;

    dist_array[tid] = min_dist;
    point_index_array[tid] = closest_point;

    if(tid == 0 ){
        for(int j=1; j<curThreadCount; j++){
            float dist = dist_array[j];
            // look for a point that is closer, or equally far, but with a smaller index.
            if( (dist < min_dist) || (dist == min_dist && point_index_array[j] < point_index_array[0]) ){
                min_dist = dist;
                point_index_array[0] = point_index_array[j];
            }
        }
        if( min_dist > threshold )
            point_index_array[0] = -1;
    }

    return point_index_array[0];
}

void reduce_card_device(int *cardinalities, int TB_count){
    int i, max_card = -1, winner_index;

    for(i=0; i<TB_count*2; i+=2){
        if( cardinalities[i] > max_card ){
            max_card = cardinalities[i];
            winner_index = cardinalities[i+1];
        }
    }

    cardinalities[0] = max_card;
    cardinalities[1] = winner_index;

}

void
compute_degrees(int *indr_mtrx, int *degrees, int N0, int max_degree){
    int tid, tblock_id, TB_count, offset;
    int local_point_count, curThreadCount;
    int starting_point;

    curThreadCount = BLOCK_DIM_X;
    tid = _tid_x;
    tblock_id = _bid_x;
    TB_count = GRID_DIM_X;
    local_point_count = (N0+TB_count-1)/TB_count;
    starting_point = tblock_id * local_point_count;
    offset =  starting_point*max_degree;
    indr_mtrx = &indr_mtrx[offset];
    degrees = &degrees[starting_point];

    // The last threadblock might end up with less points.
    if( (tblock_id+1)*local_point_count > N0 )
        local_point_count = MAX(0,N0-starting_point);

    for(int i=0; i+tid < local_point_count; i+=curThreadCount){
        int cnt = 0;
        for(int j=0; j < max_degree; j++){
            if( indr_mtrx[(i+tid)*max_degree+j] >= 0 ){
                ++cnt;
            }
        }
        degrees[i+tid] = cnt;
    }
}

/*
void
compute_degrees(int *indr_mtrx, int *degrees, int N0, int max_degree){
    int tid, tblock_id, TB_count, offset;
    int local_point_count, curThreadCount;
    int starting_point;

    curThreadCount = BLOCK_DIM_X*BLOCK_DIM_Y*BLOCK_DIM_Z;
    tid = _tid_x;
    tblock_id = (_bid_y * GRID_DIM_X + _bid_x);
    TB_count = GRID_DIM_Y * GRID_DIM_X;
    local_point_count = (N0+TB_count-1)/TB_count;
    starting_point = tblock_id * local_point_count;
    offset =  starting_point*max_degree;
    indr_mtrx = &indr_mtrx[offset];
    degrees = &degrees[starting_point];

    // The last threadblock might end up with less points.
    if( (tblock_id+1)*local_point_count > N0 )
        local_point_count = MAX(0,N0-starting_point);

    for(int i=0; i+tid < local_point_count; i+=curThreadCount){
        int cnt = 0;
        for(int j=0; j < max_degree; j++){
            if( indr_mtrx[(i+tid)*max_degree+j] >= 0 ){
                ++cnt;
            }
        }
        degrees[i+tid] = cnt;
    }
}
*/

void
update_clustered_pnts_mask(char *clustered_pnts_mask, char *Ai_mask, int N0 ) {
    int tid = _tid_x;
    int curThreadCount = BLOCK_DIM_X*BLOCK_DIM_Y*BLOCK_DIM_Z;

    // If a point is part of the latest winner cluster, then it should be marked as
    // clustered for the future iterations. Otherwise it should be left as it is.
    for(int i = 0; i+tid < N0; i+=curThreadCount){
        clustered_pnts_mask[i+tid] |= Ai_mask[i+tid];
    }
}

void
trim_ungrouped_pnts_indr_array(int seed_index, int *ungrpd_pnts_indr, float *dist_matrix, int *result_cluster, char *Ai_mask, char *clustered_pnts_mask, int *indr_mtrx, int *cluster_cardinalities, float *dist_to_clust, int *degrees, int point_count, int N0, int max_degree, float threshold) {
    int cnt;
    int tid = _tid_x;
    int curThreadCount = BLOCK_DIM_X*BLOCK_DIM_Y*BLOCK_DIM_Z;
    int tmp_pnts[THREADSPERBLOCK];

    int degree = degrees[seed_index];
        (void)generate_candidate_cluster_compact_storage(seed_index, degree, Ai_mask, dist_matrix, clustered_pnts_mask, indr_mtrx,
                                               dist_to_clust, point_count, N0, max_degree, result_cluster, threshold);

    int cnt_sh;
    bool flag_sh;

    if( 0 == tid ){
        cnt_sh = 0;
        flag_sh = false;
    }

    for(int i = 0; i+tid < point_count; i+=curThreadCount){
        // Have all threads make a coalesced read of contiguous global memory and copy the points assuming they are all good.
        tmp_pnts[tid] = ungrpd_pnts_indr[i+tid];
        int pnt = tmp_pnts[tid];
        // If a point is bad (which should not happen very often), raise a global flag so that thread zero fixes the problem.
        if( 1 == Ai_mask[pnt] ){
            flag_sh = true;
            tmp_pnts[tid] = INVALID_POINT_MARKER;
        }else{
            ungrpd_pnts_indr[cnt_sh+tid] = pnt;
        }

        if( 0 == tid ){
            if( flag_sh ){
                cnt = cnt_sh;
                for(int j = 0; (j < curThreadCount) && (i+j < point_count); j++ ){
                    if( INVALID_POINT_MARKER != tmp_pnts[j] ){
                        ungrpd_pnts_indr[cnt] = tmp_pnts[j];
                        cnt++;
                    }
                }
                cnt_sh = cnt;
            }else{
                cnt_sh += curThreadCount;
            }
            flag_sh  = false;
        }

    }
}

void QTC_device( float *dist_matrix, char *Ai_mask, char *clustered_pnts_mask, int *indr_mtrx, int *cluster_cardinalities, int *ungrpd_pnts_indr, float *dist_to_clust, int *degrees, int point_count, int N0, int max_degree, float threshold, int node_rank, int node_count, int total_thread_block_count) {
    int max_cardinality = -1;
    int max_cardinality_index;
    int i, tblock_id, tid, base_offset;

    tid = _tid_x;
    //tblock_id = (_bid_y * GRID_DIM_X + _bid_x);
    tblock_id = _bid_x;
    Ai_mask = &Ai_mask[tblock_id * N0];
    dist_to_clust = &dist_to_clust[tblock_id * max_degree];
    //base_offset = node_offset+tblock_id;
    base_offset = tblock_id*node_count + node_rank;

    // for i loop of the algorithm.
    // Each thread iterates over all points that the whole thread-block owns
    for(i = base_offset; i < point_count; i+= total_thread_block_count ){
        int cnt;
        int seed_index = ungrpd_pnts_indr[i];
        int degree = degrees[seed_index];
        if( degree <= max_cardinality ) continue;
            cnt = generate_candidate_cluster_compact_storage( seed_index, degree, Ai_mask, dist_matrix,
                                                    clustered_pnts_mask, indr_mtrx, dist_to_clust,
                                                    point_count, N0, max_degree, NULL, threshold);
        if( cnt > max_cardinality ){
            max_cardinality = cnt;
            max_cardinality_index = seed_index;
        }
    } // for (i

    // since only three elements per block go to the global memory, the offset is:
    //int card_offset = (_bid_y * GRID_DIM_X + _bid_x)*2;
    int card_offset = _bid_x*2;
    // only one thread needs to write into the global memory since they all have the same information.
    if( 0 == tid ){
        cluster_cardinalities[card_offset] = max_cardinality;
        cluster_cardinalities[card_offset+1] = max_cardinality_index;
    }
}

#endif


// --- from kernels_compact_storage.h ---
#ifndef _KERNELS_COMPACT_STORAGE_H_
#define _KERNELS_COMPACT_STORAGE_H_

#define COMPUTE_DIAMETER_WITH_POINT( _CAND_PNT_, _CURR_DIST_TO_CLUST_, _I_ ) \
    if( (_CAND_PNT_) < 0 ){\
        break;\
    }\
do{\
    int tmp_index = (_I_)*curThreadCount+tid;\
    if( (_CAND_PNT_) == seed_point ){\
        break;\
    }\
    _CURR_DIST_TO_CLUST_ = dist_to_clust[ tmp_index ];\
    /* if "_CAND_PNT_" is too far away, or already in Ai_mask, or in clustered_points, ignore it. */\
    if( (_CURR_DIST_TO_CLUST_ > threshold) || (0 != Ai_mask[(_CAND_PNT_)]) || (0 != clustered_pnts_mask[(_CAND_PNT_)]) ){ \
        _CAND_PNT_ = seed_point; /* This is so we don't do the lookup again. */\
        break;\
    }\
    dist_to_new_point = threshold+1;\
    /* Find _CAND_PNT_ in the neighborhood of the latest_point.*/\
    for(int j=last_index_checked; j<max_degree; j++){\
        int tmp_pnt = indr_mtrx[ latest_p_off + j ];\
        if( (tmp_pnt > (_CAND_PNT_)) || (tmp_pnt < 0) ){\
            last_index_checked = j;\
            break;\
        }\
        if( tmp_pnt == (_CAND_PNT_) ){\
                dist_to_new_point = compact_storage_dist_matrix[ latest_p_off + j ];\
            break;\
        }\
    }\
\
    /* See if the distance of "_CAND_PNT_" to the "latest_point" is larger */\
    /* than the previous, cached distance of "_CAND_PNT_" to the cluster.  */\
    if(dist_to_new_point > _CURR_DIST_TO_CLUST_){\
        diameter = dist_to_new_point;\
        dist_to_clust[ tmp_index ] = diameter;\
    }else{\
        diameter = _CURR_DIST_TO_CLUST_;\
    }\
\
    /* The point that leads to the cluster with the smallest diameter is the closest point */\
    if( diameter < min_dist ){\
        min_dist = diameter;\
        point_index = (_CAND_PNT_);\
    }\
}while(0)

#define FETCH_POINT( _CAND_PNT_ , _I_ )\
{\
    int tmp_index = (_I_)*curThreadCount+tid;\
    if( tmp_index >= max_degree ){\
        break;\
    }\
    _CAND_PNT_ = indr_mtrx[ seed_p_off + tmp_index ];\
    if( (_CAND_PNT_) < 0 ){\
        break;\
    }\
}

inline int generate_candidate_cluster_compact_storage(int seed_point, int degree, char *Ai_mask, float *compact_storage_dist_matrix, char *clustered_pnts_mask, int *indr_mtrx, float *dist_to_clust, int point_count, int N0, int max_degree, int *candidate_cluster, float threshold)
{

    bool flag;
    int cnt, latest_point;

    int curThreadCount = BLOCK_DIM_X*BLOCK_DIM_Y*BLOCK_DIM_Z;
    int tid = _tid_x;
    int seed_p_off;

    float curr_dist_to_clust_i;
    float curr_dist_to_clust_0, curr_dist_to_clust_1, curr_dist_to_clust_2, curr_dist_to_clust_3;
    float curr_dist_to_clust_4, curr_dist_to_clust_5, curr_dist_to_clust_6, curr_dist_to_clust_7;
    float curr_dist_to_clust_8, curr_dist_to_clust_9, curr_dist_to_clust_10, curr_dist_to_clust_11;
    int cand_pnt_i=-1;
    int cand_pnt_0=-1, cand_pnt_1=-1, cand_pnt_2=-1, cand_pnt_3=-1;
    int cand_pnt_4=-1, cand_pnt_5=-1, cand_pnt_6=-1, cand_pnt_7=-1;
    int cand_pnt_8=-1, cand_pnt_9=-1, cand_pnt_10=-1, cand_pnt_11=-1;

    // Cleanup the candidate-cluster-mask, Ai_mask
    for(int i=0; i+tid < N0; i+=curThreadCount){
        Ai_mask[i+tid] = 0;
    }

    // Cleanup the "distance cache"
    for(int i=0; i+tid < max_degree; i+=curThreadCount){
        dist_to_clust[i+tid] = 0;
    }

    // Put the seed point in the candidate cluster and mark it as taken in the candidate cluster mask Ai_mask.
    flag = true;
    cnt = 1;
    if( 0 == tid ){
        if( NULL != candidate_cluster )
            candidate_cluster[0] = seed_point;
        Ai_mask[seed_point] = 1;
    }
    seed_p_off = seed_point*max_degree;
    latest_point = seed_point;

    // Prefetch 12 points per thread, into registers, to reduce the memory pressure (and delay) of
    // constantly going to memory to fetch these points inside the while() loop that follows.
    do{
        FETCH_POINT(  cand_pnt_0,  0 );
        FETCH_POINT(  cand_pnt_1,  1 );
        FETCH_POINT(  cand_pnt_2,  2 );
        FETCH_POINT(  cand_pnt_3,  3 );
        FETCH_POINT(  cand_pnt_4,  4 );
        FETCH_POINT(  cand_pnt_5,  5 );
        FETCH_POINT(  cand_pnt_6,  6 );
        FETCH_POINT(  cand_pnt_7,  7 );
        FETCH_POINT(  cand_pnt_8,  8 );
        FETCH_POINT(  cand_pnt_9,  9 );
        FETCH_POINT( cand_pnt_10, 10 );
        FETCH_POINT( cand_pnt_11, 11 );
    }while(0);

    // different threads might exit this loop at different times, so let them catch up.

    while( (cnt < point_count) && flag ){
        int min_G_index;
        int point_index = -1;
        float min_dist=3*threshold;
        int last_index_checked = 0;
        float diameter;
        float dist_to_new_point;

        int latest_p_off = latest_point*max_degree;

        do{
            COMPUTE_DIAMETER_WITH_POINT(  cand_pnt_0,  curr_dist_to_clust_0,  0 );
            COMPUTE_DIAMETER_WITH_POINT(  cand_pnt_1,  curr_dist_to_clust_1,  1 );
            COMPUTE_DIAMETER_WITH_POINT(  cand_pnt_2,  curr_dist_to_clust_2,  2 );
            COMPUTE_DIAMETER_WITH_POINT(  cand_pnt_3,  curr_dist_to_clust_3,  3 );
            COMPUTE_DIAMETER_WITH_POINT(  cand_pnt_4,  curr_dist_to_clust_4,  4 );
            COMPUTE_DIAMETER_WITH_POINT(  cand_pnt_5,  curr_dist_to_clust_5,  5 );
            COMPUTE_DIAMETER_WITH_POINT(  cand_pnt_6,  curr_dist_to_clust_6,  6 );
            COMPUTE_DIAMETER_WITH_POINT(  cand_pnt_7,  curr_dist_to_clust_7,  7 );
            COMPUTE_DIAMETER_WITH_POINT(  cand_pnt_8,  curr_dist_to_clust_8,  8 );
            COMPUTE_DIAMETER_WITH_POINT(  cand_pnt_9,  curr_dist_to_clust_9,  9 );
            COMPUTE_DIAMETER_WITH_POINT( cand_pnt_10, curr_dist_to_clust_10, 10 );
            COMPUTE_DIAMETER_WITH_POINT( cand_pnt_11, curr_dist_to_clust_11, 11 );
        }while(0);

        // different threads might exit this loop at different times, so let them catch up.

        // The following loop implements the "find point pj s.t. diameter(Ai && pj) is minimum"
        for(int i=12; i*curThreadCount+tid < max_degree; i++){
            FETCH_POINT( cand_pnt_i, i );
            COMPUTE_DIAMETER_WITH_POINT( cand_pnt_i, curr_dist_to_clust_i, i );
        }

        min_G_index = closest_point_reduction(min_dist, threshold, point_index);

        if(min_G_index >= 0 ){
            if( 0 == tid ){
                Ai_mask[min_G_index] = 1;
                if( NULL != candidate_cluster ){
                    candidate_cluster[cnt] = min_G_index;
                }
            }
            latest_point = min_G_index;
            cnt++;
        }else{
            flag = false;
        }
    }

    return cnt;
}

#endif


// --- from libdata.h ---
#ifndef _LIBDATA_H_
#define _LIBDATA_H_
#include <math.h>

float *generate_synthetic_data(float **rslt_mtrx, int **indr_mtrx, int *max_degree, float threshold, int N, int type);

#endif


// --- from qtc_common.h ---
#ifndef _QTC_COMMON_H_
#define _QTC_COMMON_H_

#define GLOBAL_MEMORY 0x0
#define TEXTUR_MEMORY 0x1
#define COMPACT_STORAGE_MATRIX 0x00
#define FULL_STORAGE_MATRIX    0x10

#ifdef MIN
# undef MIN
#endif
#define MIN(_X, _Y) ( ((_X) < (_Y)) ? (_X) : (_Y) )

#ifdef MAX
# undef MAX
#endif
#define MAX(_X, _Y) ( ((_X) > (_Y)) ? (_X) : (_Y) )

#define INVALID_POINT_MARKER -42

#endif


// --- from qtclib.h ---
#ifndef QTLIB_H
#define QTLIB_H


void reduce_card(void *card, int pointCount);
void allocDeviceBuffer(void** bufferp, unsigned long bytes);
void freeDeviceBuffer(void* buffer);
void copyToDevice(void* to_device, void* from_host, unsigned long bytes);
void copyFromDevice(void* to_host, void* from_device, unsigned long bytes);

#endif // QTLIB_H


// --- from tuningParameters.h ---
#ifndef _TUNINGPARAMETERS_H_
#define _TUNINGPARAMETERS_H_

#define THREADSPERBLOCK     64

#define SM_COUNT 24
#define OVR_SBSCR_FACTOR 24

#define GPU_MIN_SATURATION_FACTOR 32

#endif
