#include "kernel.h"

// --- from main.cu ---
extern "C"
void gaussian_kernel(const unsigned char * data,
                unsigned char * out,
                const int rows, const int cols) 
{
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=rows
    #pragma HLS INTERFACE s_axilite port=cols
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=l_mem complete dim=1
    #pragma HLS ARRAY_PARTITION variable=l_mem complete dim=1
    #pragma HLS ARRAY_PARTITION variable=l_mem complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int l_mem[4096];
                    int* l_data = l_mem;

                    const int L_SIZE = BLOCK_DIM_X;
                    int sum         = 0;
                    const int l_row = _tid_y + 1;
                    const int l_col = _tid_x + 1;
                    const int g_row = _bid_y * BLOCK_DIM_Y + l_row;
                    const int g_col = _bid_x * BLOCK_DIM_X + l_col;

                    const int pos = g_row * cols + g_col;

                    // copy to local
                    l_data[l_row * (L_SIZE + 2) + l_col] = data[pos];

                    // top most row
                    if(l_row == 1) {
                    l_data[0 * (L_SIZE + 2) + l_col] = data[pos - cols];
                    // top left
                    if(l_col == 1)
                    l_data[0 * (L_SIZE + 2) + 0] = data[pos - cols - 1];

                    // top right
                    else if(l_col == L_SIZE)
                    l_data[0 * (L_SIZE + 2) + L_SIZE + 1] = data[pos - cols + 1];
                    }
                    // bottom most row
                    else if(l_row == L_SIZE) {
                    l_data[(L_SIZE + 1) * (L_SIZE + 2) + l_col] = data[pos + cols];
                    // bottom left
                    if(l_col == 1)
                    l_data[(L_SIZE + 1) * (L_SIZE + 2) + 0] = data[pos + cols - 1];

                    // bottom right
                    else if(l_col == L_SIZE)
                    l_data[(L_SIZE + 1) * (L_SIZE + 2) + L_SIZE + 1] = data[pos + cols + 1];
                    }

                    if(l_col == 1)
                    l_data[l_row * (L_SIZE + 2) + 0] = data[pos - 1];
                    else if(l_col == L_SIZE)
                    l_data[l_row * (L_SIZE + 2) + L_SIZE + 1] = data[pos + 1];

                    for(int i = 0; i < 3; i++) {
                    for(int j = 0; j < 3; j++) {
                    sum += c_gaus[i*3+j] * l_data[(i + l_row - 1) * (L_SIZE + 2) + j + l_col - 1];
                    }
                    }

                    out[pos] = min(255, max(0, sum));

                }
            }
        }
    }
}
extern "C"

void sobel_kernel(const unsigned char * data,
             unsigned char * out,
             unsigned char * theta,
             const int rows, const int cols)
{
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=theta offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=rows
    #pragma HLS INTERFACE s_axilite port=cols
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=l_mem complete dim=1
    #pragma HLS ARRAY_PARTITION variable=l_mem complete dim=1
    #pragma HLS ARRAY_PARTITION variable=l_mem complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int l_mem[4096];
                    int* l_data = l_mem;

                    // collect sums separately. we're storing them into floats because that
                    // is what hypot and atan2 will expect.
                    const int L_SIZE = BLOCK_DIM_X;
                    const float PI    = 3.14159265f;
                    const int   l_row = _tid_y + 1;
                    const int   l_col = _tid_x + 1;
                    const int   g_row = _bid_y * BLOCK_DIM_Y + l_row;
                    const int   g_col = _bid_x * BLOCK_DIM_X + l_col;

                    const int pos = g_row * cols + g_col;

                    // copy to local
                    l_data[l_row * (L_SIZE + 2) + l_col] = data[pos];

                    // top most row
                    if(l_row == 1) {
                    l_data[0 * (L_SIZE + 2) + l_col] = data[pos - cols];
                    // top left
                    if(l_col == 1)
                    l_data[0 * (L_SIZE + 2) + 0] = data[pos - cols - 1];

                    // top right
                    else if(l_col == L_SIZE)
                    l_data[0 * (L_SIZE + 2) + (L_SIZE + 1)] = data[pos - cols + 1];
                    }
                    // bottom most row
                    else if(l_row == L_SIZE) {
                    l_data[(L_SIZE + 1) * (L_SIZE + 2) + l_col] = data[pos + cols];
                    // bottom left
                    if(l_col == 1)
                    l_data[(L_SIZE + 1) * (L_SIZE + 2) + 0] = data[pos + cols - 1];

                    // bottom right
                    else if(l_col == L_SIZE)
                    l_data[(L_SIZE + 1) * (L_SIZE + 2) + (L_SIZE + 1)] = data[pos + cols + 1];
                    }

                    // left
                    if(l_col == 1)
                    l_data[l_row * (L_SIZE + 2) + 0] = data[pos - 1];
                    // right
                    else if(l_col == L_SIZE)
                    l_data[l_row * (L_SIZE + 2) + (L_SIZE + 1)] = data[pos + 1];

                    float sumx = 0, sumy = 0, angle = 0;
                    // find x and y derivatives
                    for(int i = 0; i < 3; i++) {
                    for(int j = 0; j < 3; j++) {
                    sumx += c_sobx[i*3+j] * l_data[(i + l_row - 1) * (L_SIZE + 2) + j + l_col - 1];
                    sumy += c_soby[i*3+j] * l_data[(i + l_row - 1) * (L_SIZE + 2) + j + l_col - 1];
                    }
                    }

                    // The output is now the square root of their squares, but they are
                    // constrained to 0 <= value <= 255. Note that hypot is a built in function
                    // defined as: hypot(x,y) = sqrt(x*x, y*y).
                    out[pos] = min(255, max(0, (int)hypot(sumx, sumy)));

                    // Compute the direction angle theta in radians
                    // atan2 has a range of (-PI, PI) degrees
                    angle = atan2(sumy, sumx);

                    // If the angle is negative,
                    // shift the range to (0, 2PI) by adding 2PI to the angle,
                    // then perform modulo operation of 2PI
                    if(angle < 0) {
                    angle = fmod((angle + 2 * PI), (2 * PI));
                    }

                    // Round the angle to one of four possibilities: 0, 45, 90, 135 degrees
                    // then store it in the theta buffer at the proper position
                    //theta[pos] = ((int)(degrees(angle * (PI/8) + PI/8-0.0001) / 45) * 45) % 180;
                    if(angle <= PI / 8)
                    theta[pos] = 0;
                    else if(angle <= 3 * PI / 8)
                    theta[pos] = 45;
                    else if(angle <= 5 * PI / 8)
                    theta[pos] = 90;
                    else if(angle <= 7 * PI / 8)
                    theta[pos] = 135;
                    else if(angle <= 9 * PI / 8)
                    theta[pos] = 0;
                    else if(angle <= 11 * PI / 8)
                    theta[pos] = 45;
                    else if(angle <= 13 * PI / 8)
                    theta[pos] = 90;
                    else if(angle <= 15 * PI / 8)
                    theta[pos] = 135;
                    else
                    theta[pos] = 0; // (angle <= 16*PI/8)

                }
            }
        }
    }
}
extern "C"

void non_max_supp_kernel(const unsigned char * data,
                          unsigned char * out, 
                    const unsigned char * theta,
                    const int rows, const int cols)
{
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=theta offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=rows
    #pragma HLS INTERFACE s_axilite port=cols
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=l_mem complete dim=1
    #pragma HLS ARRAY_PARTITION variable=l_mem complete dim=1
    #pragma HLS ARRAY_PARTITION variable=l_mem complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int l_mem[4096];
                    int* l_data = l_mem;

                    // These variables are offset by one to avoid seg. fault errors
                    // As such, this kernel ignores the outside ring of pixels
                    const int L_SIZE = BLOCK_DIM_X;
                    const int l_row = _tid_y + 1;
                    const int l_col = _tid_x + 1;
                    const int g_row = _bid_y * BLOCK_DIM_Y + l_row;
                    const int g_col = _bid_x * BLOCK_DIM_X + l_col;

                    const int pos = g_row * cols + g_col;

                    // copy to l_data
                    l_data[l_row * (L_SIZE + 2) + l_col] = data[pos];

                    // top most row
                    if(l_row == 1) {
                    l_data[0 * (L_SIZE + 2) + l_col] = data[pos - cols];
                    // top left
                    if(l_col == 1)
                    l_data[0 * (L_SIZE + 2) + 0] = data[pos - cols - 1];

                    // top right
                    else if(l_col == L_SIZE)
                    l_data[0 * (L_SIZE + 2) + (L_SIZE + 1)] = data[pos - cols + 1];
                    }
                    // bottom most row
                    else if(l_row == L_SIZE) {
                    l_data[(L_SIZE + 1) * (L_SIZE + 2) + l_col] = data[pos + cols];
                    // bottom left
                    if(l_col == 1)
                    l_data[(L_SIZE + 1) * (L_SIZE + 2) + 0] = data[pos + cols - 1];

                    // bottom right
                    else if(l_col == L_SIZE)
                    l_data[(L_SIZE + 1) * (L_SIZE + 2) + (L_SIZE + 1)] = data[pos + cols + 1];
                    }

                    if(l_col == 1)
                    l_data[l_row * (L_SIZE + 2) + 0] = data[pos - 1];
                    else if(l_col == L_SIZE)
                    l_data[l_row * (L_SIZE + 2) + (L_SIZE + 1)] = data[pos + 1];

                    unsigned char my_magnitude = l_data[l_row * (L_SIZE + 2) + l_col];

                    // The following variables are used to address the matrices more easily
                    switch(theta[pos]) {
                    // A gradient angle of 0 degrees = an edge that is North/South
                    // Check neighbors to the East and West
                    case 0:
                    // supress me if my neighbor has larger magnitude
                    if(my_magnitude <= l_data[l_row * (L_SIZE + 2) + l_col + 1] || // east
                    my_magnitude <= l_data[l_row * (L_SIZE + 2) + l_col - 1]) // west
                    {
                    out[pos] = 0;
                    }
                    // otherwise, copy my value to the output buffer
                    else {
                    out[pos] = my_magnitude;
                    }
                    break;

                    // A gradient angle of 45 degrees = an edge that is NW/SE
                    // Check neighbors to the NE and SW
                    case 45:
                    // supress me if my neighbor has larger magnitude
                    if(my_magnitude <= l_data[(l_row - 1) * (L_SIZE + 2) + l_col + 1] || // north east
                    my_magnitude <= l_data[(l_row + 1) * (L_SIZE + 2) + l_col - 1]) // south west
                    {
                    out[pos] = 0;
                    }
                    // otherwise, copy my value to the output buffer
                    else {
                    out[pos] = my_magnitude;
                    }
                    break;

                    // A gradient angle of 90 degrees = an edge that is E/W
                    // Check neighbors to the North and South
                    case 90:
                    // supress me if my neighbor has larger magnitude
                    if(my_magnitude <= l_data[(l_row - 1) * (L_SIZE + 2) + l_col] || // north
                    my_magnitude <= l_data[(l_row + 1) * (L_SIZE + 2) + l_col]) // south
                    {
                    out[pos] = 0;
                    }
                    // otherwise, copy my value to the output buffer
                    else {
                    out[pos] = my_magnitude;
                    }
                    break;

                    // A gradient angle of 135 degrees = an edge that is NE/SW
                    // Check neighbors to the NW and SE
                    case 135:
                    // supress me if my neighbor has larger magnitude
                    if(my_magnitude <= l_data[(l_row - 1) * (L_SIZE + 2) + l_col - 1] || // north west
                    my_magnitude <= l_data[(l_row + 1) * (L_SIZE + 2) + l_col + 1]) // south east
                    {
                    out[pos] = 0;
                    }
                    // otherwise, copy my value to the output buffer
                    else {
                    out[pos] = my_magnitude;
                    }
                    break;

                    default: out[pos] = my_magnitude; break;
                    }

                }
            }
        }
    }
}
extern "C"

void hyst_kernel(const unsigned char * data,
                  unsigned char * out,
            const int rows, const int cols)
{
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=rows
    #pragma HLS INTERFACE s_axilite port=cols
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    // Establish our high and low thresholds as floats
                    float lowThresh  = 10;
                    float highThresh = 70;

                    // These variables are offset by one to avoid seg. fault errors
                    // As such, this kernel ignores the outside ring of pixels
                    const int row = _bid_y * BLOCK_DIM_Y + _tid_y + 1;
                    const int col = _bid_x * BLOCK_DIM_X + _tid_x + 1;
                    const int pos = row * cols + col;

                    const unsigned char EDGE = 255;

                    unsigned char magnitude = data[pos];

                    if(magnitude >= highThresh)
                    out[pos] = EDGE;
                    else if(magnitude <= lowThresh)
                    out[pos] = 0;
                    else {
                    float med = (highThresh + lowThresh) / 2;

                    if(magnitude >= med)
                    out[pos] = EDGE;
                    else
                    out[pos] = 0;
                    }

                }
            }
        }
    }
}
