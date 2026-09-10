#include "FFT.h"

int revIdxTab[FFT_NUM];     
complex<float> twiddles[FFT_NUM<2? 1 : FFT_NUM/2];

#pragma hls_design inline
void RADIX2_BFLY_double_buffer_quarter_CY(complex<float>* data_ld, complex<float>* data_st, int i0, int i1, bool inv_i1_enable, bool tw_enable, complex<float> tw) 
{
    complex<float> d0 = data_ld[i0];                     
    complex<float> d1 = data_ld[i1];
    float d0_real, d0_imag, d1_real, d1_imag, d2_real, d2_imag, d3_real, d3_imag;
    if(inv_i1_enable){
        d1_real = d1.imag();
        d1_imag = - d1.real();
    }
    else if(tw_enable){
        float a, b, c, d;
        a = d1.real(); b = d1.imag(); c = tw.real(); d = tw.imag();
        float ac, bd, ad, bc; 
        ac = a*c;
        bd = b*d;
        ad = a*d;
        bc = b*c;
        d1_real = ac - bd; 
        d1_imag = ad + bc;         
    }else{
        d1_real = d1.real();
        d1_imag = d1.imag();
    }

    d0_real = d0.real();
    d0_imag = d0.imag();
    d2_real = d0_real + d1_real;
    d2_imag = d0_imag + d1_imag;
    d3_real = d0_real - d1_real;
    d3_imag = d0_imag - d1_imag;
    data_st[i0] = complex<float>(d2_real, d2_imag);
    data_st[i1] = complex<float>(d3_real, d3_imag);
}

#pragma hls_design block
void output_result_array_to_stream (complex<float> data_6[FFT_NUM], ac_channel<ac_vector<complex<float>, UF*2>> & dataOut
){
    #pragma hls_pipeline_init_interval 1
    PostP_Fwd_loop: for (int i = 0; i < FFT_NUM/(UF*2); i++)  { 
        ac_vector<complex<float>, UF*2> temp;
        for (int u = 0; u < UF*2; u++) {
            temp[u] = data_6[i*UF*2+u];
        }
        dataOut.write(temp);
    }
}

template<int N>  ac_int<N, false> bit_reverse ( ac_int<N, false> input ){
    ac_int<N, false> reversed;
    #pragma hls_unroll yes
    Loop_Reverse: for (int bit_i = 0; bit_i < N; bit_i++) {
        reversed[bit_i] = input[N-1-bit_i];
    }
    return reversed;
}

#pragma hls_design block
void reverse_input_stream_UF2 (
    ac_channel<ac_vector<complex<float>, UF*2>> & dataIn,
    ac_channel<ac_vector<complex<float>, UF*2>> & reverse_in_stream_vector
){
    static complex<float> data_in_cyclic[UF*2][FFT_NUM/(UF*2)];
    static complex<float> data_rev_stream[UF*2][FFT_NUM/(UF*2)];

    const int TIME_STEP =  FFT_NUM/(UF*2);
    const int PAR =  UF*2;

    #pragma hls_pipeline_init_interval 1
    READ_STREAM_INPUT: for (int i = 0; i < TIME_STEP; i++){ 
        ac_vector<complex<float>, UF*2> temp;
        temp = dataIn.read();
        ac_int<EXP2_FFT, false> original[UF*2];
        for (int u = 0; u < UF*2; u++) {
            original[u] = i*UF*2+u;
        }
        ac_int<EXP2_FFT, false> reversed[UF*2];
        for (int u = 0; u < UF*2; u++) {
            reversed[u] = bit_reverse<EXP2_FFT>(original[u]);
        }
        data_rev_stream[0][reversed[0]%TIME_STEP] = temp[0];
        data_rev_stream[2][reversed[1]%TIME_STEP] = temp[1];
        data_rev_stream[1][reversed[2]%TIME_STEP] = temp[2];
        data_rev_stream[3][reversed[3]%TIME_STEP] = temp[3];
    }

    #pragma hls_pipeline_init_interval 1
    FROM_BLOCK_TO_CYCLIC: for (int i = 0; i < TIME_STEP; i= i + 1){ 
        int offset[UF*2];
        for (int u = 0; u < UF*2; u++) {
            offset[u] = (i+u)%TIME_STEP;
        }
        int cyclic_offset[UF*2];
        complex<float> block_data[UF*2];
        for (int u = 0; u < UF*2; u++) {
            block_data[u] = data_rev_stream[u][offset[u]];
        }
    
        complex<float> cyclic_data[UF*2];

        if (i%PAR ==0 ){
            cyclic_data[0] = block_data[0];
            cyclic_data[1] = block_data[1];
            cyclic_data[2] = block_data[2];
            cyclic_data[3] = block_data[3];

            cyclic_offset[0] = i/PAR ;
            cyclic_offset[1] = ((i+1)%TIME_STEP+TIME_STEP)/PAR;
            cyclic_offset[2] = ((i+2)%TIME_STEP+TIME_STEP*2)/PAR;
            cyclic_offset[3] = ((i+3)%TIME_STEP+TIME_STEP*3)/PAR;

        }else if (i%PAR ==1 ){
            cyclic_data[0] = block_data[3];
            cyclic_data[1] = block_data[0];
            cyclic_data[2] = block_data[1];
            cyclic_data[3] = block_data[2];

            cyclic_offset[0] = ((i+3)%TIME_STEP+TIME_STEP*3)/PAR;
            cyclic_offset[1] = i/PAR; 
            cyclic_offset[2] = ((i+1)%TIME_STEP+TIME_STEP)/PAR;
            cyclic_offset[3] = ((i+2)%TIME_STEP+TIME_STEP*2)/PAR; 

        }else if (i%PAR ==2 ){
            cyclic_data[0] = block_data[2];
            cyclic_data[1] = block_data[3];
            cyclic_data[2] = block_data[0];
            cyclic_data[3] = block_data[1];

            cyclic_offset[0] = ((i+2)%TIME_STEP+TIME_STEP*2)/PAR; 
            cyclic_offset[1] = ((i+3)%TIME_STEP+TIME_STEP*3)/PAR;
            cyclic_offset[2] = i/PAR;  
            cyclic_offset[3] = ((i+1)%TIME_STEP+TIME_STEP)/PAR;
            
        }else if (i%PAR ==3 ){
            cyclic_data[0] = block_data[1];
            cyclic_data[1] = block_data[2];
            cyclic_data[2] = block_data[3];
            cyclic_data[3] = block_data[0];

            cyclic_offset[0] = ((i+1)%TIME_STEP+TIME_STEP*1)/PAR;
            cyclic_offset[1] = ((i+2)%TIME_STEP+TIME_STEP*2)/PAR;
            cyclic_offset[2] = ((i+3)%TIME_STEP+TIME_STEP*3)/PAR; 
            cyclic_offset[3] = i/PAR; 
        }

        for (int u = 0; u < UF*2; u++) {
            data_in_cyclic[u][cyclic_offset[u]] = cyclic_data[u];
        }
    }

    #pragma hls_pipeline_init_interval 1
    STREAM_OUT_REVERSE: for (int i = 0; i < TIME_STEP; i= i + 1){ 
        ac_vector<complex<float>, UF*2> temp;
        for (int u = 0; u < UF*2; u++) {
            temp[u] = data_in_cyclic[u][i];
        }
        reverse_in_stream_vector.write(temp);
    }
}

template<int stage> void FFT_stage_spatial_unroll(complex<float> data_ld[FFT_NUM], complex<float> data_st[FFT_NUM]){
    int bflySize = 1 << stage; 
    int bflyStep = bflySize >> 1; 
    int indexMultiply = FFT_NUM >> stage;
    
    if(bflyStep < UF){
        #pragma hls_pipeline_init_interval 1
        #pragma hls_unroll UF>>(stage-1)
        L_Pair_loop: 
        for (uint16_t m = 0; m < FFT_NUM; m += bflySize) {
            L_Group_loop: 
            for (uint16_t k = 0; k < bflyStep; ++k) {
                uint16_t index = indexMultiply * k;
                auto tw = twiddles[index];   
                uint16_t i0 = m + k; 
                uint16_t i1 = m + k + bflyStep;
                RADIX2_BFLY_double_buffer_quarter_CY(data_ld, data_st, i0, i1, k > 0 && k == bflyStep>>1, k > 0, tw);  
            }
        }
    }
    else if (bflySize == FFT_NUM){
            #pragma hls_pipeline_init_interval 1
            #pragma hls_unroll UF
            R_Group_loop_bflySize_equal_FFT_NUM : 
            for (uint16_t k = 0; k < bflyStep; ++k) {
                uint16_t index = indexMultiply * k;
                auto tw = twiddles[index];   
                uint16_t i0 = 0 + k; 
                uint16_t i1 = 0 + k + bflyStep;
                RADIX2_BFLY_double_buffer_quarter_CY(data_ld, data_st, i0, i1, k > 0 && k == bflyStep>>1, k > 0, tw);  
            }
    }
    else{
        R_Pair_loop: 
        for (uint16_t m = 0; m < FFT_NUM; m += bflySize) {
            #pragma hls_pipeline_init_interval 1
            #pragma hls_unroll UF
            R_Group_loop: 
            for (uint16_t k = 0; k < bflyStep; ++k) {
                uint16_t index = indexMultiply * k;
                auto tw = twiddles[index];   
                uint16_t i0 = m + k; 
                uint16_t i1 = m + k + bflyStep;
                RADIX2_BFLY_double_buffer_quarter_CY(data_ld, data_st, i0, i1, k > 0 && k == bflyStep>>1, k > 0, tw);                  
            }
        }
    }
}

#pragma hls_design inline
void RADIX2_BFLY_double_buffer_quarter_onlycompute(complex<float> d0, complex<float> d1, complex<float>& data_out0, complex<float>& data_out1, bool inv_i1_enable, bool tw_enable, complex<float> tw) 
{
    float d0_real, d0_imag, d1_real, d1_imag, d2_real, d2_imag, d3_real, d3_imag;
    if(inv_i1_enable){
        d1_real = d1.imag();
        d1_imag = - d1.real();
    }
    else if(tw_enable){
        float a, b, c, d;
        a = d1.real(); b = d1.imag(); c = tw.real(); d = tw.imag();
        float ac, bd, ad, bc; 
        ac = a*c;
        bd = b*d;
        ad = a*d;
        bc = b*c;
        d1_real = ac - bd; 
        d1_imag = ad + bc;         
    }else{
        d1_real = d1.real();
        d1_imag = d1.imag();
    }

    d0_real = d0.real();
    d0_imag = d0.imag();
    d2_real = d0_real + d1_real;
    d2_imag = d0_imag + d1_imag;
    d3_real = d0_real - d1_real;
    d3_imag = d0_imag - d1_imag;

    data_out0 = complex<float>(d2_real, d2_imag);
    data_out1 = complex<float>(d3_real, d3_imag);
}

#pragma hls_design block
void FFT_Stage1_vectorstream_parameterize(
    ac_channel<ac_vector<complex<float>, UF*2>> & reverse_in_stream_vector,
    ac_channel<ac_vector<complex<float>, UF*2>> & data_s1_stream_vector
){
    #pragma hls_pipeline_init_interval 1
    FFT_Stage1: for (int m = 0; m < FFT_NUM/(2*UF); m += 1) {
        auto tw = complex<float>(0,0); 
        ac_vector<complex<float>, UF*2> data = reverse_in_stream_vector.read();
        ac_vector<complex<float>, UF*2> data_out;
        for (int i = 0; i < UF; i++){
            auto data0 = data[i*2];
            auto data1 = data[i*2+1];
            complex<float> data_out0, data_out1;
            RADIX2_BFLY_double_buffer_quarter_onlycompute(data0, data1, data_out0, data_out1, false, false, tw); 
            data_out[i*2] = data_out0;
            data_out[i*2+1] = data_out1;
        }
        data_s1_stream_vector.write(data_out);
    }
}

#pragma hls_design block
void FFT_Stage2_vectorstreamIn_arrayOut_parametize(
    ac_channel<ac_vector<complex<float>, UF*2>> & data_s1_stream_vector,
    complex<float> data_2[FFT_NUM]
){
    #pragma hls_pipeline_init_interval 1
    FFT_Stage2:  for (int m = 0; m < FFT_NUM; m += 2*UF) {
        ac_vector<complex<float>, UF*2> data = data_s1_stream_vector.read();
        for (int i = 0; i < UF*2; i+= 4){
            auto index0 = 64*0;
            auto index1 = 64*1;
            auto tw0 = twiddles[index0];   
            auto tw1 = twiddles[index1];   
            auto data0 = data[i];
            auto data1 = data[i+2];
            auto data2 = data[i+1];
            auto data3 = data[i+3];
            complex<float> data_out0, data_out1, data_out2, data_out3;
            RADIX2_BFLY_double_buffer_quarter_onlycompute(data0, data1, data_out0, data_out1, false,  0 > 0, tw0); 
            RADIX2_BFLY_double_buffer_quarter_onlycompute(data2, data3, data_out2, data_out3, true,  1 > 0, tw1); 
            data_2[m+i] =  data_out0;
            data_2[m+i+2] =  data_out1;
            data_2[m+i+1] =  data_out2;
            data_2[m+i+3] =  data_out3;
        }
    }
}

#pragma hls_design block
void FFT_DIT_spatial_unroll_CY_stream_vector( 
    ac_channel<ac_vector<complex<float>, UF*2>> & dataIn,  
    ac_channel<ac_vector<complex<float>, UF*2>> & dataOut
){
    static complex<float> data_0[FFT_NUM];
    static complex<float> data_1[FFT_NUM];
    static complex<float> data_2[FFT_NUM];
    static complex<float> data_3[FFT_NUM];
    static complex<float> data_4[FFT_NUM];
    static complex<float> data_5[FFT_NUM];
    static complex<float> data_6[FFT_NUM];

    ac_channel<ac_vector<complex<float>, UF*2>> reverse_in_stream_vector; 
    ac_channel<ac_vector<complex<float>, UF*2>> data_s1_stream_vector; 

    reverse_input_stream_UF2(dataIn, reverse_in_stream_vector);
    FFT_Stage1_vectorstream_parameterize (reverse_in_stream_vector, data_s1_stream_vector);
    FFT_Stage2_vectorstreamIn_arrayOut_parametize (data_s1_stream_vector, data_0);
    FFT_stage_spatial_unroll<3>(data_0, data_1);
    FFT_stage_spatial_unroll<4>(data_1, data_2);
    FFT_stage_spatial_unroll<5>(data_2, data_3);
    FFT_stage_spatial_unroll<6>(data_3, data_4);
    FFT_stage_spatial_unroll<7>(data_4, data_5);
    FFT_stage_spatial_unroll<8>(data_5, data_6);
    output_result_array_to_stream (data_6, dataOut);
}

#pragma hls_design top
void FFT_TOP(ac_channel<ac_vector<complex<float>, UF*2>> & in, 
    ac_channel<ac_vector<complex<float>, UF*2>> & out
){
    LOOP_TWIDDLES: for (int i = 0; i < FFT_NUM/2; ++i) {
        double angle = -2 * PI * i / FFT_NUM;
        auto tw = complex<float>(cos(angle), sin(angle));
        twiddles[i] = (complex<float>)tw;
    }
    LOOP_REVIDTAB: for (int i = 0; i < FFT_NUM; ++i) {
        int reversed = 0;
        int number = i;
        for (int j = 0; j < EXP2_FFT; ++j) {
            reversed = reversed * 2 + (number & 1);
            number >>= 1;
        }
        revIdxTab[i] = reversed;
    }
    FFT_DIT_spatial_unroll_CY_stream_vector(in, out);
}