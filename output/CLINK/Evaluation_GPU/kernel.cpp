#include "kernel.h"

// --- from example.cu ---
void lstm_n5_o1(int input[SAMPLE_TEST_LEN], short output[SAMPLE_TEST_LEN])
{
    int i, j, t;

    float inW[4][5] = {
        -0.00902497, 0.0130347, -0.305604, 0.0103134, -0.00143173,
        -0.00892103, -0.00877193, -0.0158959, -0.00261989, 0.00238156,
        -1.17159, -1.05888, -0.0252563, 1.32337, 0.896013,
        0.112793, 0.107382, 0.459561, 0.112837, -0.0858938};

    float intW[4][5][5] = {
        {{0.01465, 0.017885, 0.00462623, -0.00366126, 0.00414583},
        {0.00709106, 0.00612325, 0.00509018, 0.00629193, -0.00820282},
        {0.0594903, 0.0594652, 0.0879106, -0.202968, 0.146663},
        {0.0173266, -0.00258213, -0.00156304, -0.0161799, 0.0206139},
        {0.00378391, 0.0190192, 0.0140174, 0.0183843, -0.00042357}},
        {{-0.007224, -2.52633e-05, -0.00375626, 0.0171819, -0.0146835},
        {0.0095475, 0.0111485, 0.00723207, -0.00279432, -0.00130744},
        {-0.00358937, -0.0211212, -0.0445563, -0.0203464, 0.0123881},
        {-0.00648264, -0.00841806, 0.00112013, 0.00435087, -0.0138258},
        {0.00533612, -0.00909088, 0.00789575, 0.00117046, 0.00834566}},
        {{0.74772, 0.635634, 0.730541, -1.11435, 0.814002},
        {0.623608, 0.53032, 0.652992, -1.01461, 0.768323},
        {0.120079, 0.113368, 0.0824013, -0.000308211, -0.0182162},
        {-1.28265, -1.18123, -0.480213, 0.984297, -0.576107},
        {-1.00799, -0.944089, -0.355751, 0.536079, -0.27723}},
        {{0.0134795, 0.0447042, 0.015088, 0.0920375, -0.0777375},
        {0.0384587, 0.0330071, 0.0205698, 0.0858556, -0.0671409},
        {-0.63912, -0.570696, -0.0891825, 0.706698, -0.5},
        {0.0172945, 0.0240723, 0.00149645, 0.0341813, -0.0418003},
        {-0.0122831, -0.0280598, -0.00341253, -0.0265756, 0.0246845}}
    };

    float intB[4][5] = {
        0.016, 0.0139732, -0.183891, 0.0139634, 0.00864378,
        5.00094, 5.00059, 4.97023, 5.0002, 5.00032,
        0.0676543, -0.0445895, 0.248995, -0.978814, -1.0258,
        0.204404, 0.190113, -0.156202, 0.219446, -0.179526};

    float outW[5] = {-0.4272, -0.33769, 0.167592, 0.50495, -0.502329};

    float outB = -0.0394433;

    short inWF[4][5] = {0};
    short intWF[4][5][5] = {0};
    short intBF[4][5] = {0};
    short outWF[5] = {0};
    short outBF = 0;

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 5; ++j) {
            inWF[i][j] = (short) (inW[i][j] * SCALER);
        }
    }

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 5; ++j) {
            for (t = 0; t < 5; ++t) {
                intWF[i][j][t] = (short) (intW[i][j][t] * SCALER);
            }
        }
    }

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 5; ++j) {
            intBF[i][j] = (short) (intB[i][j] * SCALER);
        }
    }

    for (i = 0; i < 5; ++i) {
        outWF[i] = (short) (outW[i] * SCALER);
    }

    outBF = (short) (outB * SCALER);

    short h_stateF[5] = {0};
    short c_stateF[5] = {0};
    short i_stateF[5] = {0};
    short f_stateF[5] = {0};
    short o_stateF[5] = {0};
    short g_stateF[5] = {0};

    short sampleinput_16b;

    for (t = 0; t < SAMPLE_TEST_LEN; ++t) {

        sampleinput_16b = (short) (input[t] + 120000) * 256 / 1875;

        for (j = 0; j < 5; ++j) {
            i_stateF[j] = (inWF[0][j] * sampleinput_16b) >> 15;
            for (i = 0; i < 5; ++i)
                i_stateF[j] += ((h_stateF[i] * intWF[0][j][i]) >> 12);
            i_stateF[j] += intBF[0][j];
            i_stateF[j] = i_stateF[j] >> 5;
            if (i_stateF[j] >= LUT_SIZE)
                i_stateF[j] = 4095;
            else if (i_stateF[j] >= 0)
                i_stateF[j] = lut_sigmoid[i_stateF[j]];
            else if (i_stateF[j] > -LUT_SIZE)
                i_stateF[j] = 4096 - lut_sigmoid[-i_stateF[j]];
            else
                i_stateF[j] = 1;
        }

        for (j = 0; j < 5; ++j) {
            f_stateF[j] = (inWF[1][j] * sampleinput_16b) >> 15;
            for (i = 0; i < 5; ++i)
                f_stateF[j] += ((h_stateF[i] * intWF[1][j][i]) >> 12);
            f_stateF[j] += intBF[1][j];
            f_stateF[j] = f_stateF[j] >> 5;
            if (f_stateF[j] >= LUT_SIZE)
                f_stateF[j] = 4095;
            else if (f_stateF[j] >= 0)
                f_stateF[j] = lut_sigmoid[f_stateF[j]];
            else if (f_stateF[j] > -LUT_SIZE)
                f_stateF[j] = 4096 - lut_sigmoid[-f_stateF[j]];
            else
                f_stateF[j] = 1;
        }

        for (j = 0; j < 5; ++j) {
            o_stateF[j] = (inWF[2][j] * sampleinput_16b) >> 15;
            for (i = 0; i < 5; ++i)
                o_stateF[j] += ((h_stateF[i] * intWF[2][j][i]) >> 12);
            o_stateF[j] += intBF[2][j];
            o_stateF[j] = o_stateF[j] >> 5;
            if (o_stateF[j] >= LUT_SIZE)
                o_stateF[j] = 4095;
            else if (o_stateF[j] >= 0)
                o_stateF[j] = lut_sigmoid[o_stateF[j]];
            else if (o_stateF[j] > -LUT_SIZE)
                o_stateF[j] = 4096 - lut_sigmoid[-o_stateF[j]];
            else
                o_stateF[j] = 1;
        }

        for (j = 0; j < 5; ++j) {
            g_stateF[j] = (inWF[3][j] * sampleinput_16b) >> 15;
            for (i = 0; i < 5; ++i)
                g_stateF[j] += ((h_stateF[i] * intWF[3][j][i]) >> 12);
            g_stateF[j] += intBF[3][j];
            g_stateF[j] = g_stateF[j] >> 5;
            if (g_stateF[j] >= LUT_SIZE)
                g_stateF[j] = 4096;
            else if (g_stateF[j] >= 0)
                g_stateF[j] = lut_tanh[g_stateF[j]];
            else if (g_stateF[j] > -LUT_SIZE)
                g_stateF[j] = -lut_tanh[-g_stateF[j]];
            else
                g_stateF[j] = -4096;
        }

        for (j = 0; j < 5; ++j) {
            c_stateF[j] = (((c_stateF[j] * f_stateF[j]) >> 8) + ((g_stateF[j] * i_stateF[j]) >> 12)) >> 4;
            h_stateF[j] = c_stateF[j] >> 1;
            if (h_stateF[j] >= LUT_SIZE)
                h_stateF[j] = 4096;
            else if (h_stateF[j] >= 0)
                h_stateF[j] = lut_tanh[h_stateF[j]];
            else if (h_stateF[j] > -LUT_SIZE)
                h_stateF[j] = -lut_tanh[-h_stateF[j]];
            else
                h_stateF[j] = -4096;
            h_stateF[j] = (h_stateF[j] * o_stateF[j]) >> 12;
        }

        output[t] = outBF;
        for (j = 0; j < 5; ++j)
            output[t] += ((h_stateF[j] * outWF[j]) >> 12);
    }
}

void lstm_n5_o2(int input[SAMPLE_TEST_LEN], short output[SAMPLE_TEST_LEN])
{
    int i, j, t;

    float inW[4][5] = {
        -0.133907, 0.0967799, -0.0249856, -0.0482016, 0.000138663,
        -0.0025821, -0.0107074, -0.0135626, -0.0265616, -0.00990482,
        0.0279149, 0.29944, 0.00367669, -0.0406378, -0.122106,
        0.305937, -1.54966, 0.108542, -0.086096, -0.278674};

    float intW[4][5][5] = {
        {{0.0465599, -0.0784586, 0.0703757, -0.0961503, 0.103885},
        {0.137839, 0.0785531, 0.172321, 0.00198996, 0.115174},
        {0.0896546, -0.00207286, 0.0280649, 0.0300854, 0.0549556},
        {0.0952124, 0.011873, 0.0253059, -0.00619738, 0.10025},
        {-0.0796523, -0.0310471, 0.0336561, -0.0999846, -0.00944991}},
        {{-0.00558139, -0.0249531, -0.0196812, -0.0283953, -0.00538974},
        {0.0124158, 0.00739093, 0.00918819, -0.00951965, 0.00634635},
        {-0.008908, 0.0113348, -0.00387874, 0.00339979, -0.000628876},
        {-0.00832763, 0.0040069, 0.00346749, -0.0256792, 0.00539768},
        {0.00337389, -0.0148225, -0.0283464, 0.00277652, 0.000571859}},
        {{-0.00736941, 0.0578041, 0.141176, 0.00565979, -0.079775},
        {-0.140356, 0.0521767, 0.0813636, -0.0342324, -0.0847605},
        {0.0534741, 0.0335436, 0.0464466, 0.0670157, 0.0266309},
        {0.0142565, -0.0397183, -0.0116136, -0.0507669, 0.0575363},
        {-0.0518841, 0.0358612, 0.0333015, -0.119254, 0.0368938}},
        {{-0.40111, 1.17447, 0.172804, 0.197255, 0.0786499},
        {-0.307048, -0.923395, -0.362905, 0.194527, -0.438387},
        {-0.671133, 0.728081, -0.520196, 0.0108215, -0.139992},
        {-0.600645, 0.151967, 0.0101909, -0.235608, -0.367466},
        {0.262652, 0.84919, -0.131239, 0.0756875, -0.261777}}
    };

    float intB[4][5] = {
        -0.0421559, 0.246112, 0.0348797, -0.0619016, 0.0988568,
        4.98184, 4.97131, 4.98673, 4.97446, 4.96925,
        0.255813, 0.527195, 0.120779, -0.0979445, 0.02733,
        0.0091722, 0.551458, -0.0521645, 0.0113755, 0.2287};

    float outW[5] = {-0.592906, 0.576557, -0.38704, 0.0146919, -0.35076};

    float outB = -0.0191289;

    short inWF[4][5] = {0};
    short intWF[4][5][5] = {0};
    short intBF[4][5] = {0};
    short outWF[5] = {0};
    short outBF = 0;

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 5; ++j) {
            inWF[i][j] = (short) (inW[i][j] * SCALER);
        }
    }

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 5; ++j) {
            for (t = 0; t < 5; ++t) {
                intWF[i][j][t] = (short) (intW[i][j][t] * SCALER);
            }
        }
    }

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 5; ++j) {
            intBF[i][j] = (short) (intB[i][j] * SCALER);
        }
    }

    for (i = 0; i < 5; ++i) {
        outWF[i] = (short) (outW[i] * SCALER);
    }

    outBF = (short) (outB * SCALER);

    short h_stateF[5] = {0};
    short c_stateF[5] = {0};
    short i_stateF[5] = {0};
    short f_stateF[5] = {0};
    short o_stateF[5] = {0};
    short g_stateF[5] = {0};

    short sampleinput_16b;

    for (t = 0; t < SAMPLE_TEST_LEN; ++t) {

        sampleinput_16b = (short) (input[t] + 120000) * 256 / 1875;

        for (j = 0; j < 5; ++j) {
            i_stateF[j] = (inWF[0][j] * sampleinput_16b) >> 15;
            for (i = 0; i < 5; ++i)
                i_stateF[j] += ((h_stateF[i] * intWF[0][j][i]) >> 12);
            i_stateF[j] += intBF[0][j];
            i_stateF[j] = i_stateF[j] >> 5;
            if (i_stateF[j] >= LUT_SIZE)
                i_stateF[j] = 4095;
            else if (i_stateF[j] >= 0)
                i_stateF[j] = lut_sigmoid[i_stateF[j]];
            else if (i_stateF[j] > -LUT_SIZE)
                i_stateF[j] = 4096 - lut_sigmoid[-i_stateF[j]];
            else
                i_stateF[j] = 1;
        }

        for (j = 0; j < 5; ++j) {
            f_stateF[j] = (inWF[1][j] * sampleinput_16b) >> 15;
            for (i = 0; i < 5; ++i)
                f_stateF[j] += ((h_stateF[i] * intWF[1][j][i]) >> 12);
            f_stateF[j] += intBF[1][j];
            f_stateF[j] = f_stateF[j] >> 5;
            if (f_stateF[j] >= LUT_SIZE)
                f_stateF[j] = 4095;
            else if (f_stateF[j] >= 0)
                f_stateF[j] = lut_sigmoid[f_stateF[j]];
            else if (f_stateF[j] > -LUT_SIZE)
                f_stateF[j] = 4096 - lut_sigmoid[-f_stateF[j]];
            else
                f_stateF[j] = 1;
        }

        for (j = 0; j < 5; ++j) {
            o_stateF[j] = (inWF[2][j] * sampleinput_16b) >> 15;
            for (i = 0; i < 5; ++i)
                o_stateF[j] += ((h_stateF[i] * intWF[2][j][i]) >> 12);
            o_stateF[j] += intBF[2][j];
            o_stateF[j] = o_stateF[j] >> 5;
            if (o_stateF[j] >= LUT_SIZE)
                o_stateF[j] = 4095;
            else if (o_stateF[j] >= 0)
                o_stateF[j] = lut_sigmoid[o_stateF[j]];
            else if (o_stateF[j] > -LUT_SIZE)
                o_stateF[j] = 4096 - lut_sigmoid[-o_stateF[j]];
            else
                o_stateF[j] = 1;
        }

        for (j = 0; j < 5; ++j) {
            g_stateF[j] = (inWF[3][j] * sampleinput_16b) >> 15;
            for (i = 0; i < 5; ++i)
                g_stateF[j] += ((h_stateF[i] * intWF[3][j][i]) >> 12);
            g_stateF[j] += intBF[3][j];
            g_stateF[j] = g_stateF[j] >> 5;
            if (g_stateF[j] >= LUT_SIZE)
                g_stateF[j] = 4096;
            else if (g_stateF[j] >= 0)
                g_stateF[j] = lut_tanh[g_stateF[j]];
            else if (g_stateF[j] > -LUT_SIZE)
                g_stateF[j] = -lut_tanh[-g_stateF[j]];
            else
                g_stateF[j] = -4096;
        }

        for (j = 0; j < 5; ++j) {
            c_stateF[j] = (((c_stateF[j] * f_stateF[j]) >> 8) + ((g_stateF[j] * i_stateF[j]) >> 12)) >> 4;
            h_stateF[j] = c_stateF[j] >> 1;
            if (h_stateF[j] >= LUT_SIZE)
                h_stateF[j] = 4096;
            else if (h_stateF[j] >= 0)
                h_stateF[j] = lut_tanh[h_stateF[j]];
            else if (h_stateF[j] > -LUT_SIZE)
                h_stateF[j] = -lut_tanh[-h_stateF[j]];
            else
                h_stateF[j] = -4096;
            h_stateF[j] = (h_stateF[j] * o_stateF[j]) >> 12;
        }

        output[t] = outBF;
        for (j = 0; j < 5; ++j)
            output[t] += ((h_stateF[j] * outWF[j]) >> 12);
    }
}
extern "C"

void lstm_task(int n, int *x, short *y1, short *y2)
{
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=y1 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=y2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int idx = _bid_x*BLOCK_DIM_X + _tid_x;
            //if (i < n) y[i] = a*x[i] + y[i];

            int i;
            int sampleinput[SAMPLE_TEST_LEN];
            short test_out1[SAMPLE_TEST_LEN];
            short test_out2[SAMPLE_TEST_LEN];

            if (idx < n)
            {
            for (i = 0; i < SAMPLE_TEST_LEN; ++i)
            {
            sampleinput[i] = x[idx * SAMPLE_TEST_LEN + i];
            }
            lstm_n5_o1(sampleinput, test_out1);
            lstm_n5_o2(sampleinput, test_out2);
            for (i = 0; i < SAMPLE_TEST_LEN; ++i)
            {
            y1[idx * SAMPLE_TEST_LEN + i] = test_out1[i];
            y2[idx * SAMPLE_TEST_LEN + i] = test_out2[i];
            }
            }

        }
    }
}
