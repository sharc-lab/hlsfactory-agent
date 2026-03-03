#include "knn.h"
static inline DATA_TYPE absval(DATA_TYPE input){
    return (input > 0 ? input : static_cast<DATA_TYPE>(-1*input));
}

#define SORT_TO_HIERMERGE_STREAM_ARGS(PE, SEG) \
    sort_to_hiermerge_dist_stream_##PE[D2L_FACTOR_W*SEG + 0],  \
    sort_to_hiermerge_id_stream_##PE[D2L_FACTOR_W*SEG + 0]


#define INVOKE_PPS_UNITS_FOR_PE(PE) \
    .invoke(para_partial_sort, PE, 0, compute_to_sort_stream_##PE[0], SORT_TO_HIERMERGE_STREAM_ARGS(PE, 0) ) \
    .invoke(para_partial_sort, PE, 1, compute_to_sort_stream_##PE[1], SORT_TO_HIERMERGE_STREAM_ARGS(PE, 1) ) \


// purposefully empty #define
#define HIERMERGE_STREAM_DECLS(PE) \


#define INVOKE_HIERMERGE_UNITS_FOR_PE(PE) \
    .invoke(merge_dual_streams, PE, 0, 1, sort_to_hiermerge_dist_stream_##PE[0], sort_to_hiermerge_id_stream_##PE  [0], sort_to_hiermerge_dist_stream_##PE[1], sort_to_hiermerge_id_stream_##PE  [1], L0_out_dist[PE],L0_out_id[PE])


/*************************************************/
/******************** LOADS: *********************/
/*************************************************/
void load_KNN(   int debug_PE_ID,
                 tapa::async_mmap<INTERFACE_WIDTH> & searchSpace,
                 tapa::ostream<INTERFACE_WIDTH>& load_to_compute_stream)
{
#pragma HLS INLINE OFF
    INTERFACE_WIDTH loaded_value = 0;


    LOAD_QUERY: 
    for (int i_req = 0, i_resp = 0; i_resp < (INPUT_DIM-1)/NUM_FEATURES_PER_READ + 1; ) {
        #pragma HLS loop_tripcount min=((INPUT_DIM-1)/NUM_FEATURES_PER_READ + 1) max=((INPUT_DIM-1)/NUM_FEATURES_PER_READ + 1)
        #pragma HLS pipeline II=1
        //Think of addr as an array index.
        int addr = i_req;
        if (i_req < (INPUT_DIM-1)/NUM_FEATURES_PER_READ + 1 && searchSpace.read_addr.try_write(addr)) {
            i_req++;
        }

        if (!searchSpace.read_data.empty()) {
            loaded_value = searchSpace.read_data.read(nullptr);
            i_resp++;

            // DEBUGGING:
            #ifndef __SYNTHESIS__
            for (int i = 0; i < NUM_FEATURES_PER_READ; ++i)
            {
                DATA_TYPE cur_value = 0;
                TRANSFER_TYPE tmp;
                tmp.range(DATA_TYPE_TOTAL_SZ - 1, 0)
                    = loaded_value(i*DATA_TYPE_TOTAL_SZ + (DATA_TYPE_TOTAL_SZ - 1),
                                    i*DATA_TYPE_TOTAL_SZ);

                cur_value = *((DATA_TYPE*) (&tmp));
                if (cur_value < MAX_DATA_TYPE_VAL && debug_PE_ID == 0)
                {
                    printf("LOAD QUERY: value = %f, i_resp = %d\n", cur_value, i_resp);
                }
            }
            #endif


            load_to_compute_stream.write(loaded_value);
        }
    }

    LOAD_SEARCHSPACE:
    for (int i_req = 0, i_resp = 0; i_resp < PARTITION_LEN_IN_I; ) {
        #pragma HLS loop_tripcount min=PARTITION_LEN_IN_I max=PARTITION_LEN_IN_I
        #pragma HLS pipeline II=1
        //Think of addr as an array index.
        int addr = QUERY_DATA_RESERVE + i_req;
        if (i_req < PARTITION_LEN_IN_I && searchSpace.read_addr.try_write(addr)) {
            i_req++;
        }
        if (!searchSpace.read_data.empty()) {
            loaded_value = searchSpace.read_data.read(nullptr);
            i_resp++;

            //// DEBUGGING: Printing the loaded data:
            //#ifndef __SYNTHESIS__
            //for (int i = 0; i < NUM_FEATURES_PER_READ; ++i)
            //{
            //    DATA_TYPE cur_value = 0;
            //    TRANSFER_TYPE tmp;
            //    tmp.range(DATA_TYPE_TOTAL_SZ - 1, 0)
            //        = loaded_value(i*DATA_TYPE_TOTAL_SZ + (DATA_TYPE_TOTAL_SZ - 1),
            //                        i*DATA_TYPE_TOTAL_SZ);
            //    cur_value = *((DATA_TYPE*) (&tmp));
            //    if (cur_value < MAX_DATA_TYPE_VAL && debug_PE_ID == 0)
            //    {
            //        printf("LOAD SEARCHSPACE: value = %f, i_resp = %d\n", cur_value, i_resp);
            //    }
            //}
            //#endif

            //// DEBUGGING: Printing how many times we write to each stream:
            //#ifndef __SYNTHESIS__
            //if (debug_PE_ID == 0)
            //{
            //    printf("LOAD: Writing to load_to_compute_stream for the %d-th time\n",
            //            i_resp-1);
            //}
            //#endif

            load_to_compute_stream.write(loaded_value);
        }
    }
}
/*************************************************/
/******************* COMPUTES: *******************/
/*************************************************/

void compute_KNN(   int debug_pe_idx,
                    int debug_start_idx,
                    tapa::istream<INTERFACE_WIDTH>&     load_to_compute_stream,
                    tapa::ostreams<LOCAL_DIST_DTYPE, NUM_SEGMENTS>&     compute_to_sort_stream)
{
#pragma HLS INLINE OFF

    #ifndef __SYNTHESIS__
    int DEBUG_load_ctr = 0;
    #endif

    INTERFACE_WIDTH cur_data = 0;
    DATA_TYPE local_Query[INPUT_DIM];
    #pragma HLS ARRAY_PARTITION variable=local_Query complete dim=1

    #ifndef __SYNTHESIS__
    int DEBUG_write_counters[NUM_SEGMENTS] = {};
    #endif

    /***********************************************/

    GET_QUERYDATA:
    for (int i = 0 ; i < (INPUT_DIM-1)/NUM_FEATURES_PER_READ + 1; ++i)
    {
        TRANSFER_TYPE tmp = 0;
        int input_dim_idx = 0;

        cur_data = load_to_compute_stream.read();

        for ( int j = 0; 
              j < NUM_FEATURES_PER_READ && input_dim_idx < INPUT_DIM; 
              ++j, ++input_dim_idx) 
        {
            tmp.range(DATA_TYPE_TOTAL_SZ-1, 0)
                = cur_data.range(j*DATA_TYPE_TOTAL_SZ + (DATA_TYPE_TOTAL_SZ-1),
                                 j*DATA_TYPE_TOTAL_SZ);

            local_Query[input_dim_idx] = *((DATA_TYPE*)(&tmp));
        }
    }

    COMPUTE_DATA:
    for (int jj = 0; jj < SEGMENT_SIZE_IN_I; ++jj){

        for (int ii = 0 ; ii < NUM_SEGMENTS; ++ii){
            #pragma HLS PIPELINE II=1
            LOCAL_DIST_DTYPE aggregated_local_dists = 0;

            //#ifndef __SYNTHESIS__
            //if (debug_pe_idx == 0)
            //{
            //    printf("COMPUTE: Reading from load_to_compute_stream for the %d-th time\n",
            //            DEBUG_load_ctr++);
            //}
            //#endif

            cur_data = load_to_compute_stream.read();

            for (int l2i = 0; l2i < L2I_FACTOR_W; ++l2i)
            {
            #pragma HLS UNROLL
                for (int d2l = 0; d2l < D2L_FACTOR_W; ++d2l){
                #pragma HLS UNROLL
                    int d2i = d2l + D2L_FACTOR_W*l2i;

                    DATA_TYPE delta_squared_sum = 0.0;
                    int start_idx = d2i * INPUT_DIM;

                    for (int ll = 0; ll < INPUT_DIM; ++ll){
                        unsigned int sp_range_idx = (start_idx + ll) * DATA_TYPE_TOTAL_SZ;
                        DATA_TYPE sp_dim_item_value;
                        TRANSFER_TYPE tmp = 0;

                        tmp.range(DATA_TYPE_TOTAL_SZ-1, 0) = 
                            cur_data.range(sp_range_idx + (DATA_TYPE_TOTAL_SZ-1), 
                                                   sp_range_idx);

                        sp_dim_item_value = *((DATA_TYPE*) (&tmp));

                        #if DISTANCE_METRIC == 0 // manhattan
                        DATA_TYPE delta = absval(sp_dim_item_value - local_Query[ll]);
                        delta_squared_sum += delta;
                        #elif DISTANCE_METRIC == 1 // L2
                        DATA_TYPE delta = absval(sp_dim_item_value - local_Query[ll]);
                        delta_squared_sum += delta * delta;
                        #endif
                    }
                    aggregated_local_dists = delta_squared_sum;

                    //#ifndef __SYNTHESIS__
                    //if (delta_squared_sum < MAX_DATA_TYPE_VAL)
                    //{
                    //    printf("COMPUTE: At index %d, delta_squared_sum = %f\n", 
                    //            debug_start_idx + ii*SEGMENT_SIZE_IN_D +
                    //            jj*D2I_FACTOR_W + d2i,
                    //            delta_squared_sum);
                    //}
                    //#endif
                }
                int stream_idx = (ii*L2I_FACTOR_W + l2i)%NUM_SEGMENTS;
                compute_to_sort_stream[stream_idx].write(aggregated_local_dists);

                //#ifndef __SYNTHESIS__
                //if (debug_pe_idx == 0)
                //{
                //    printf("COMPUTE: Writing the value %f to compute_to_sort_stream number %d, for the %d'th time\n", 
                //            aggregated_local_dists,
                //            stream_idx,
                //            DEBUG_write_counters[stream_idx]++);
                //}
                //#endif

                aggregated_local_dists = 0;
            }
        }
    }
}


void swap(DATA_TYPE* a, DATA_TYPE* b, 
               int* x, int* y)
{
#pragma HLS INLINE

    DATA_TYPE tmpdist_a;
    DATA_TYPE tmpdist_b;

    int tmpid_x;
    int tmpid_y;

    tmpdist_a = *a;
    tmpdist_b = *b;
    *b = tmpdist_a;
    *a = tmpdist_b;

    tmpid_x = *x;
    tmpid_y = *y;
    *x = tmpid_y;
    *y = tmpid_x;
}
void para_partial_sort(const int PE_idx,
                       int seg_idx,
                       tapa::istream<LOCAL_DIST_DTYPE>&     compute_to_sort_stream,
                       tapa::ostream<DATA_TYPE>&           sort_to_hiermerge_dist_stream_0,
                       tapa::ostream<int>&                 sort_to_hiermerge_id_stream_0)
{
#pragma HLS INLINE OFF

    #ifndef __SYNTHESIS__
    printf("SORT UNIT FOR PE #%d, SEGMENT #%d IS STARTING NOW.\n", PE_idx, seg_idx);
    #endif

    #ifdef __SYNTHESIS__
    static      // TAPA Known-issue: Static keyword fails CSIM because this is not thread-safe. 
                //  but when running the HW build, it will instantiate several copies of this function. So this is OK.
    #endif
    DATA_TYPE local_kNearstDist[D2L_FACTOR_W][(TOP+1)];
    #pragma HLS ARRAY_PARTITION variable=local_kNearstDist complete dim=0

    #ifdef __SYNTHESIS__
    static
    #endif
    int local_kNearstId[D2L_FACTOR_W][(TOP+1)];
    #pragma HLS ARRAY_PARTITION variable=local_kNearstId complete dim=0

    #ifndef __SYNTHESIS__
    int DEBUG_stream_counters = 0;
    #endif

    /* Our segments used to be large chunks of each partition.
     * Now, however, our segments are cylically split, so our ID 
     * logic has to change.
     */
    int start_id = PE_idx * NUM_SP_PTS_PER_KRNL_PADDED + seg_idx*D2L_FACTOR_W;

    /*******************************************/

    // Initialize all top-K distances to MAX, and their IDs to an invalid value.
    INIT_LOOP:
    for (int i = 0; i < D2L_FACTOR_W; ++i)
    {
        for (int j = 0; j < TOP+1; ++j)
        {
            local_kNearstId[i][j] = -1;
            local_kNearstDist[i][j] = MAX_DATA_TYPE_VAL;
        }
    }

    SORT_LOOP:
    for (int lvalue_idx = 0; lvalue_idx < (SEGMENT_SIZE_IN_L + TOP); ++lvalue_idx) {
    #pragma HLS PIPELINE II=2
        LOCAL_DIST_DTYPE cur_Lval = 0;
        int stream_idx = seg_idx;

        if (lvalue_idx >= SEGMENT_SIZE_IN_L) {
            cur_Lval = MAX_DATA_TYPE_VAL;
        } else {
            //#ifndef __SYNTHESIS__
            //if (PE_idx == 0)
            //{
            //    printf("PPS Unit %d: Reading from compute_to_sort_stream %d, for the %d-th time\n", 
            //            seg_idx,
            //            stream_idx,
            //            DEBUG_stream_counters++);
            //}
            //#endif

            cur_Lval = compute_to_sort_stream.read();
        }

        for (int D_idx = 0; D_idx < D2L_FACTOR_W; ++D_idx)
        {
        #pragma HLS UNROLL

            unsigned char range_idx = (D_idx)*DATA_TYPE_TOTAL_SZ;
            DATA_TYPE cur_Dval;

            cur_Dval = cur_Lval;

            local_kNearstDist[D_idx][0] = cur_Dval;

            local_kNearstId[D_idx][0] = start_id + lvalue_idx*D2L_FACTOR_W*NUM_SEGMENTS + D_idx;

            //#ifndef __SYNTHESIS__
            //printf("SORT: Current ID = %d, cur_Dval = %f\n", start_id + lvalue_idx, cur_Dval);
            //printf("SORT: Best ID = %d, Best Dval = %f\n", local_kNearstId[D_idx][TOP], local_kNearstDist[D_idx][TOP]);
            //#endif


            //compare and swap odd
            for(int ii=1; ii<TOP; ii+=2){
            #pragma HLS UNROLL
            #pragma HLS DEPENDENCE variable="local_kNearstDist" inter false
            #pragma HLS DEPENDENCE variable="local_kNearstId" inter false

                if (local_kNearstDist[D_idx][ii] < local_kNearstDist[D_idx][ii+1]){
                    swap(&local_kNearstDist[D_idx][ii], &local_kNearstDist[D_idx][ii+1], 
                              &local_kNearstId[D_idx][ii], &local_kNearstId[D_idx][ii+1]);
                }

            }


            //compare and swap even
            for(int ii=1; ii<TOP+1; ii+=2){
            #pragma HLS UNROLL
            #pragma HLS DEPENDENCE variable="local_kNearstDist" inter false
            #pragma HLS DEPENDENCE variable="local_kNearstId" inter false

                if (local_kNearstDist[D_idx][ii] > local_kNearstDist[D_idx][ii-1]){
                    swap(&local_kNearstDist[D_idx][ii], &local_kNearstDist[D_idx][ii-1], 
                              &local_kNearstId[D_idx][ii], &local_kNearstId[D_idx][ii-1]);
                }

            }
        }
    }

    // Write data out
    OUTPUT_LOOP:
    for (int j = TOP; j > 0; --j)
    {
    #pragma HLS PIPELINE II=1
        sort_to_hiermerge_dist_stream_0.write(local_kNearstDist[0][j]);
        sort_to_hiermerge_id_stream_0.write(local_kNearstId[0][j]);
    }

    //#ifndef __SYNTHESIS__
    //for (int i = 0; i < D2L_FACTOR_W; ++i)
    //{
    //    for (int j = 0; j < TOP+1; ++j)
    //    {
    //        printf("AFTER SORT: local_kNearst[%3d][%3d][%3d]:\n", seg_idx, i, j);
            //printf("AFTER SORT:     Dist = %5.10f\n",   local_kNearstDist[i][j]);
            //printf("AFTER SORT:     Id = %d\n",         local_kNearstId[i][j]);

    //    }
    //    printf("\n");
    //}
    //#endif

    REINITIALIZATION_LOOP:
    for (int i = 0; i < D2L_FACTOR_W; ++i){
        for (int j = 0; j < TOP+1; ++j){
        #pragma HLS UNROLL
            // Reset the kNearst values so we can run the next iteration.
            local_kNearstId[i][j] = -1;
            local_kNearstDist[i][j] = MAX_DATA_TYPE_VAL;
        }
    }
}





void write_out_mmap(
                    tapa::async_mmap<INT32>&    output_knn,
                    DATA_TYPE                   output_dist,
                    int                         output_id,
                    int&                        i_req_output, 
                    int&                        i_resp_output
) {
#pragma HLS INLINE
    INT32 outval = 0;

    if (i_req_output < 2*TOP && i_req_output >= 0 && 
        !output_knn.write_addr.full() && 
        !output_knn.write_data.full()
    ) {
        outval = *(INT32*) &output_dist;

        //#ifndef __SYNTHESIS__
        //printf("KDEBUG: i_req_output = %d. Outval = %f, output_dist = %f.\n",
        //        i_req_output, outval.to_float(), output_dist.to_float());
        //#endif

        output_knn.write_addr.try_write(i_req_output);
        output_knn.write_data.try_write(outval);
        --i_req_output;
    }

    if (!output_knn.write_resp.empty()) {
        i_resp_output += (unsigned int) (output_knn.write_resp.read(nullptr)) + 1;
    }

    if (i_req_output < 2*TOP && i_req_output >= 0 && 
        !output_knn.write_addr.full() && 
        !output_knn.write_data.full()
    ) {
        outval = * (INT32*) &output_id;

        //#ifndef __SYNTHESIS__
        //printf("KDEBUG: i_req_output = %d. Outval = %f, output_id = %d\n",
        //        i_req_output, outval.to_float(), output_id);
        //#endif

        output_knn.write_addr.try_write(i_req_output);
        output_knn.write_data.try_write(outval);
        --i_req_output;
    }

    if (!output_knn.write_resp.empty()) {
        i_resp_output += (unsigned int) (output_knn.write_resp.read(nullptr)) + 1;
    }
}


void merge_dual_streams(
                        int debug_PE_idx,
                        int debug_seg_d2l_idx,
                        int debug_stage_idx,
                        tapa::istream<DATA_TYPE>&   hiermerge_dist_istream_1,
                        tapa::istream<int>&         hiermerge_id_istream_1,
                        tapa::istream<DATA_TYPE>&   hiermerge_dist_istream_2,
                        tapa::istream<int>&         hiermerge_id_istream_2,
                        tapa::ostream<DATA_TYPE>&   hiermerge_dist_ostream,
                        tapa::ostream<int>&         hiermerge_id_ostream
)
{

    #ifndef __SYNTHESIS__
    printf("NOTE: USING MONOMERGE!\n");
    #endif

    DATA_TYPE dist_1 = hiermerge_dist_istream_1.read();
    DATA_TYPE dist_2 = hiermerge_dist_istream_2.read();
    int id_1 = hiermerge_id_istream_1.read();
    int id_2 = hiermerge_id_istream_2.read();
    int stream1_read_count = 1;
    int stream2_read_count = 1;

    for (int k = TOP-1; k > 0; --k)
    {
        #ifndef __SYNTHESIS__
        if (debug_PE_idx == 0)
        {
            printf("KDEBUG: Hiermerge for PE %d, STAGE %d, seg_d2l = %d, stream1_read_count = %d, stream2_read_count = %d\n", 
                    debug_PE_idx, debug_stage_idx, debug_seg_d2l_idx,
                    stream1_read_count, stream2_read_count);
        }
        #endif
        if (dist_1 <= dist_2)
        {
            hiermerge_dist_ostream.write(dist_1);
            hiermerge_id_ostream.write(id_1);

            if (stream1_read_count < TOP)
            {
                ++stream1_read_count;
                dist_1 = hiermerge_dist_istream_1.read();
                id_1 = hiermerge_id_istream_1.read();
            }
        }
        else
        {
            hiermerge_dist_ostream.write(dist_2);
            hiermerge_id_ostream.write(id_2);

            if (stream2_read_count < TOP)
            {
                ++stream2_read_count;
                dist_2 = hiermerge_dist_istream_2.read();
                id_2 = hiermerge_id_istream_2.read();
            }
        }
    }
    // Final write.
    if (dist_1 <= dist_2) {
        hiermerge_dist_ostream.write(dist_1);
        hiermerge_id_ostream.write(id_1);
    }
    else {
        hiermerge_dist_ostream.write(dist_2);
        hiermerge_id_ostream.write(id_2);
    }

    #ifndef __SYNTHESIS__
    if (debug_PE_idx == 0)
    {
        printf("KDEBUG: Hiermerge for PE %d, STAGE #%d, seg_d2l = %d, Emptying the input FIFOs now...\n",
                debug_PE_idx, debug_stage_idx, debug_seg_d2l_idx);
    }
    #endif
    // Empty the input streams.
    // NOTE: The total tripcount of these loops will be TOP.
    while (stream1_read_count < TOP)
    {
    #pragma HLS loop_tripcount min=TOP/2 max=TOP/2
        ++stream1_read_count;
        dist_1 = hiermerge_dist_istream_1.read();
        id_1 = hiermerge_id_istream_1.read();
    }
    while (stream2_read_count < TOP)
    {
    #pragma HLS loop_tripcount min=TOP/2 max=TOP/2
        ++stream2_read_count;
        dist_2 = hiermerge_dist_istream_2.read();
        id_2 = hiermerge_id_istream_2.read();
    }
}


void merge_trio_streams(
                        int debug_PE_idx,
                        int debug_seg_d2l_idx,
                        int debug_stage_idx,
                        tapa::istream<DATA_TYPE>&   hiermerge_dist_istream_1,
                        tapa::istream<int>&         hiermerge_id_istream_1,
                        tapa::istream<DATA_TYPE>&   hiermerge_dist_istream_2,
                        tapa::istream<int>&         hiermerge_id_istream_2,
                        tapa::istream<DATA_TYPE>&   hiermerge_dist_istream_3,
                        tapa::istream<int>&         hiermerge_id_istream_3,
                        tapa::ostream<DATA_TYPE>&   hiermerge_dist_ostream,
                        tapa::ostream<int>&         hiermerge_id_ostream
)
{
    DATA_TYPE dist_1 = hiermerge_dist_istream_1.read();
    DATA_TYPE dist_2 = hiermerge_dist_istream_2.read();
    DATA_TYPE dist_3 = hiermerge_dist_istream_3.read();
    int id_1 = hiermerge_id_istream_1.read();
    int id_2 = hiermerge_id_istream_2.read();
    int id_3 = hiermerge_id_istream_3.read();
    int stream1_read_count = 1;
    int stream2_read_count = 1;
    int stream3_read_count = 1;

    for (int k = TOP-1; k > 0; --k)
    {
        #ifndef __SYNTHESIS__
        if (debug_PE_idx == 0)
        {
            printf("KDEBUG: Hiermerge for PE %d, STAGE %d, seg_d2l = %d, stream1_read_count = %d, stream2_read_count = %d, stream3_read_count = %d\n", 
                    debug_PE_idx, debug_stage_idx, debug_seg_d2l_idx,
                    stream1_read_count, stream2_read_count, stream3_read_count);
        }
        #endif

        if ( (dist_1 <= dist_2) && (dist_1 <= dist_3) )
        {
            hiermerge_dist_ostream.write(dist_1);
            hiermerge_id_ostream.write(id_1);

            if (stream1_read_count < TOP)
            {
                ++stream1_read_count;
                dist_1 = hiermerge_dist_istream_1.read();
                id_1 = hiermerge_id_istream_1.read();
            }
        }

        else if ( (dist_2 <= dist_3) && (dist_2 <= dist_1) )
        {
            hiermerge_dist_ostream.write(dist_2);
            hiermerge_id_ostream.write(id_2);

            if (stream2_read_count < TOP)
            {
                ++stream2_read_count;
                dist_2 = hiermerge_dist_istream_2.read();
                id_2 = hiermerge_id_istream_2.read();
            }
        }
        else
        {
            hiermerge_dist_ostream.write(dist_3);
            hiermerge_id_ostream.write(id_3);

            if (stream3_read_count < TOP)
            {
                ++stream3_read_count;
                dist_3 = hiermerge_dist_istream_3.read();
                id_3 = hiermerge_id_istream_3.read();
            }
        }
    }
    // Final write.
    if ( (dist_1 <= dist_2) && (dist_1 <= dist_3) ){
        hiermerge_dist_ostream.write(dist_1);
        hiermerge_id_ostream.write(id_1);
    }
    else if ( (dist_2 <= dist_3) && (dist_2 <= dist_1) ){
        hiermerge_dist_ostream.write(dist_2);
        hiermerge_id_ostream.write(id_2);
    }
    else{
        hiermerge_dist_ostream.write(dist_3);
        hiermerge_id_ostream.write(id_3);
    }

    #ifndef __SYNTHESIS__
    if (debug_PE_idx == 0)
    {
        printf("KDEBUG: Hiermerge for PE %d, STAGE #%d, seg_d2l = %d, Emptying the input FIFOs now...\n",
                debug_PE_idx, debug_stage_idx, debug_seg_d2l_idx);
    }
    #endif
    // Empty the input streams.
    // NOTE: The total tripcount of these loops will be 2*TOP.
    while (stream1_read_count < TOP)
    {
    #pragma HLS loop_tripcount min=TOP/2 max=TOP/2
        ++stream1_read_count;
        dist_1 = hiermerge_dist_istream_1.read();
        id_1 = hiermerge_id_istream_1.read();
    }
    while (stream2_read_count < TOP)
    {
    #pragma HLS loop_tripcount min=TOP/2 max=TOP/2
        ++stream2_read_count;
        dist_2 = hiermerge_dist_istream_2.read();
        id_2 = hiermerge_id_istream_2.read();
    }
    while (stream3_read_count < TOP)
    {
    #pragma HLS loop_tripcount min=TOP max=TOP
        ++stream3_read_count;
        dist_3 = hiermerge_dist_istream_3.read();
        id_3 = hiermerge_id_istream_3.read();
    }
}


void merge_dual_streams_FINAL(
                        int debug_PE_idx,
                        int debug_seg_d2l_idx,
                        int debug_stage_idx,
                        tapa::istream<DATA_TYPE>&   hiermerge_dist_istream_1,
                        tapa::istream<int>&         hiermerge_id_istream_1,
                        tapa::istream<DATA_TYPE>&   hiermerge_dist_istream_2,
                        tapa::istream<int>&         hiermerge_id_istream_2,
                        tapa::async_mmap<INT32>&    hiermerge_output
)
{
    DATA_TYPE dist_1 = hiermerge_dist_istream_1.read();
    DATA_TYPE dist_2 = hiermerge_dist_istream_2.read();
    int id_1 = hiermerge_id_istream_1.read();
    int id_2 = hiermerge_id_istream_2.read();
    int stream1_read_count = 1;
    int stream2_read_count = 1;
    int i_req_output  = 2*TOP-1;
    int i_resp_output = 2*TOP-1;

    for (int k = TOP-1; k > 0; --k)
    {
        #ifndef __SYNTHESIS__
        if (debug_PE_idx == 0)
        {
            printf("KDEBUG: FINAL Hiermerge, stream1_read_count = %d, stream2_read_count = %d\n", 
                    stream1_read_count, stream2_read_count);
        }
        #endif
        if (dist_1 <= dist_2)
        {
            write_out_mmap( hiermerge_output,
                            dist_1,
                            id_1,
                            i_req_output, 
                            i_resp_output);

            if (stream1_read_count < TOP)
            {
                ++stream1_read_count;
                dist_1 = hiermerge_dist_istream_1.read();
                id_1 = hiermerge_id_istream_1.read();
            }
        }
        else
        {
            write_out_mmap( hiermerge_output,
                            dist_2,
                            id_2,
                            i_req_output, 
                            i_resp_output);

            if (stream2_read_count < TOP)
            {
                ++stream2_read_count;
                dist_2 = hiermerge_dist_istream_2.read();
                id_2 = hiermerge_id_istream_2.read();
            }
        }
    }
    // Final write.
    if (dist_1 <= dist_2) {
            write_out_mmap( hiermerge_output,
                            dist_1,
                            id_1,
                            i_req_output, 
                            i_resp_output);
    }
    else {
            write_out_mmap( hiermerge_output,
                            dist_2,
                            id_2,
                            i_req_output, 
                            i_resp_output);
    }

    #ifndef __SYNTHESIS__
    if (debug_PE_idx == 0)
    {
        printf("KDEBUG: FINAL Hiermerge, Emptying the input FIFOs now...\n");
    }
    #endif
    // Empty the input streams.
    while (stream1_read_count < TOP)
    {
    #pragma HLS loop_tripcount min=TOP/2 max=TOP/2
        ++stream1_read_count;
        dist_1 = hiermerge_dist_istream_1.read();
        id_1 = hiermerge_id_istream_1.read();
    }
    while (stream2_read_count < TOP)
    {
    #pragma HLS loop_tripcount min=TOP/2 max=TOP/2
        ++stream2_read_count;
        dist_2 = hiermerge_dist_istream_2.read();
        id_2 = hiermerge_id_istream_2.read();
    }
}


void merge_trio_streams_FINAL(
                        int debug_PE_idx,
                        int debug_seg_d2l_idx,
                        int debug_stage_idx,
                        tapa::istream<DATA_TYPE>&   hiermerge_dist_istream_1,
                        tapa::istream<int>&         hiermerge_id_istream_1,
                        tapa::istream<DATA_TYPE>&   hiermerge_dist_istream_2,
                        tapa::istream<int>&         hiermerge_id_istream_2,
                        tapa::istream<DATA_TYPE>&   hiermerge_dist_istream_3,
                        tapa::istream<int>&         hiermerge_id_istream_3,
                        tapa::async_mmap<INT32>&    hiermerge_output
)
{
    DATA_TYPE dist_1 = hiermerge_dist_istream_1.read();
    DATA_TYPE dist_2 = hiermerge_dist_istream_2.read();
    DATA_TYPE dist_3 = hiermerge_dist_istream_3.read();
    int id_1 = hiermerge_id_istream_1.read();
    int id_2 = hiermerge_id_istream_2.read();
    int id_3 = hiermerge_id_istream_3.read();
    int stream1_read_count = 1;
    int stream2_read_count = 1;
    int stream3_read_count = 1;
    int i_req_output  = 2*TOP-1;
    int i_resp_output = 2*TOP-1;


    for (int k = TOP-1; k > 0; --k)
    {
        #ifndef __SYNTHESIS__
        if (debug_PE_idx == 0)
        {
            printf("KDEBUG: FINAL Hiermerge, stream1_read_count = %d, stream2_read_count = %d, stream3_read_count = %d\n", 
                    stream1_read_count, stream2_read_count, stream3_read_count);
        }
        #endif

        if ( (dist_1 <= dist_2) && (dist_1 <= dist_3) )
        {
            write_out_mmap( hiermerge_output,
                            dist_1,
                            id_1,
                            i_req_output, 
                            i_resp_output);

            if (stream1_read_count < TOP)
            {
                ++stream1_read_count;
                dist_1 = hiermerge_dist_istream_1.read();
                id_1 = hiermerge_id_istream_1.read();
            }
        }

        else if ( (dist_2 <= dist_3) && (dist_2 <= dist_1) )
        {
            write_out_mmap( hiermerge_output,
                            dist_2,
                            id_2,
                            i_req_output, 
                            i_resp_output);

            if (stream2_read_count < TOP)
            {
                ++stream2_read_count;
                dist_2 = hiermerge_dist_istream_2.read();
                id_2 = hiermerge_id_istream_2.read();
            }
        }
        else
        {
            write_out_mmap( hiermerge_output,
                            dist_3,
                            id_3,
                            i_req_output, 
                            i_resp_output);

            if (stream3_read_count < TOP)
            {
                ++stream3_read_count;
                dist_3 = hiermerge_dist_istream_3.read();
                id_3 = hiermerge_id_istream_3.read();
            }
        }
    }
    // Final write.
    if ( (dist_1 <= dist_2) && (dist_1 <= dist_3) ){
        write_out_mmap( hiermerge_output,
                        dist_1,
                        id_1,
                        i_req_output, 
                        i_resp_output);
    }
    else if ( (dist_2 <= dist_3) && (dist_2 <= dist_1) ){
        write_out_mmap( hiermerge_output,
                        dist_2,
                        id_2,
                        i_req_output, 
                        i_resp_output);
    }
    else{
        write_out_mmap( hiermerge_output,
                        dist_3,
                        id_3,
                        i_req_output, 
                        i_resp_output);
    }

    #ifndef __SYNTHESIS__
    if (debug_PE_idx == 0)
    {
        printf("KDEBUG: FINAL Hiermerge, Emptying the input FIFOs now...\n");
    }
    #endif
    // Empty the input streams.
    while (stream1_read_count < TOP)
    {
    #pragma HLS loop_tripcount min=TOP/2 max=TOP/2
        ++stream1_read_count;
        dist_1 = hiermerge_dist_istream_1.read();
        id_1 = hiermerge_id_istream_1.read();
    }
    while (stream2_read_count < TOP)
    {
    #pragma HLS loop_tripcount min=TOP/2 max=TOP/2
        ++stream2_read_count;
        dist_2 = hiermerge_dist_istream_2.read();
        id_2 = hiermerge_id_istream_2.read();
    }
    while (stream3_read_count < TOP)
    {
    #pragma HLS loop_tripcount min=TOP max=TOP
        ++stream3_read_count;
        dist_3 = hiermerge_dist_istream_3.read();
        id_3 = hiermerge_id_istream_3.read();
    }
}


void krnl_singlePE_Write_Outputs(
    tapa::istream<DATA_TYPE> &in_dist0,
    tapa::istream<int> &in_id0,
    tapa::async_mmap<INT32>& output_knn
) {
    DATA_TYPE cur_dist;
    int cur_id;
    int i_req_output = 2*TOP-1; 
    int i_resp_output = 2*TOP-1;
    for (int i = 0; i < TOP; ++i ) {
        #pragma HLS pipeline II=1
        cur_dist = in_dist0.read();
        cur_id = in_id0.read();

        write_out_mmap(
            output_knn,
            cur_dist,
            cur_id,
            i_req_output,
            i_resp_output
        );

    }
}


void Knn(
    tapa::mmap<INTERFACE_WIDTH> in_0,
    tapa::mmap<INT32> final_out
) {

    // Streams, for the global merge:
    tapa::streams<DATA_TYPE, 1, TOP> L0_out_dist;
    tapa::streams<int,       1, TOP> L0_out_id;
    // Streams, for load->compute->sort:
    tapa::streams<INTERFACE_WIDTH, NUM_PE, 2>           load_to_compute_stream;
    tapa::streams<LOCAL_DIST_DTYPE, NUM_SEGMENTS, 2>    compute_to_sort_stream_0;
    tapa::streams<DATA_TYPE, NUM_SEGMENTS*D2L_FACTOR_W, TOP>                 sort_to_hiermerge_dist_stream_0;
    tapa::streams<int, NUM_SEGMENTS*D2L_FACTOR_W, TOP>                       sort_to_hiermerge_id_stream_0;

    HIERMERGE_STREAM_DECLS(0)


    tapa::task()
        .invoke( load_KNN, 0, in_0, load_to_compute_stream[0])

        .invoke( compute_KNN, 0, NUM_SP_PTS_PER_KRNL_PADDED*0, load_to_compute_stream[0 ], compute_to_sort_stream_0  )

        INVOKE_PPS_UNITS_FOR_PE(0)

        ////////// MERGING LOGIC

        INVOKE_HIERMERGE_UNITS_FOR_PE(0)


        // INTER-PE HIERMERGE:
        .invoke( krnl_singlePE_Write_Outputs,  L0_out_dist[0],  L0_out_id[0],
                                               final_out);
}
