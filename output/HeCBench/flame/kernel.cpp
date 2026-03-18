#include "kernel.h"

// --- from iteration.cu ---
void affine_transform(float2 &p, const float *params)
{
  float2 tmp;
  tmp.x = params[0] * p.x + params[1] * p.y + params[2];
  tmp.y = params[3] * p.x + params[4] * p.y + params[5];

  p.x = tmp.x;
  p.y = tmp.y;
}

  void sierpinski(float2 &p, int idx)
{
  p.x = p.x / 2.0f;
  p.y = p.y / 2.0f;
  switch(idx % 3) {
    case 0:
      break;
    case 1:
      p.x += 0.5f;
      break;
    case 2:
      p.y += 0.5f;
      break;
  }
}

void iteration_fractal_flame(float2 &p, 
    float &color,
    int idx,
    int threadidx,
    const float *random_numbers,
    const ConstMemParams &params)
{

#define A params.pre_transform_params[idx][0]
#define B params.pre_transform_params[idx][1]
#define C params.pre_transform_params[idx][2]
#define D params.pre_transform_params[idx][3]
#define E params.pre_transform_params[idx][4]
#define F params.pre_transform_params[idx][5]

#define RANDOM(i) random_numbers[\
  (threadidx + (params.frame_counter << 7) * i)\
  & (NUM_RANDOMS - 1)]

  switch(params.enable_sierpinski) {
    default:
    case 0:
      break;
    case 1:
      sierpinski(p, threadidx);
      break;
    case 2:
      sierpinski(p, idx);
      break;
  }

  affine_transform(p, &params.pre_transform_params[idx][0]);

  float rad_square = p.x * p.x + p.y * p.y,
        radius     = sqrtf(rad_square),
        inv_radius = 1.0f / radius,
        theta      = atan2f(p.x, p.y),
        phi        = atan2f(p.y, p.x);

  float2 point_out;
  point_out.x = 0.0f;
  point_out.y = 0.0f;

  const VariationParameter *vp = &params.variation_parameters[idx][0];

  for(int i = 0; i < VARIATIONS_PER_FUNCTION; i++) {
    if(fabsf(vp[i].factor) < 0.01f)
      continue;
    float2 point;
    switch(vp[i].idx) {
      /* {{{ Variations */
      case 0:
        point.x = p.x;
        point.y = p.y;
        break;
      case 1:
        point.x = sinf(p.x);
        point.y = sinf(p.y);
        break;
      case 2:
        point.x = p.x / rad_square;
        point.y = p.y / rad_square;
        break;
      case 3:
        point.x = p.x * sinf(rad_square) - p.y * cosf(rad_square);
        point.y = p.x * cosf(rad_square) + p.y * sinf(rad_square);
        break;
      case 4:
        point.x = (p.x - p.y) * (p.x + p.y) * inv_radius;
        point.y = 2.0f * p.x * p.y * inv_radius;
        break;
      case 5:
        point.x = theta / M_PI;
        point.y = radius - 1.0f;
        break;
      case 6:
        point.x = radius * sinf(theta + radius);
        point.y = radius * cosf(theta - radius);
        break;
      case 7:
        point.x = radius * sinf(theta * radius);
        point.y = -radius * cosf(theta * radius);
        break;
      case 8: {
          float pi_r = M_PI * radius,
                theta_pi = theta / M_PI;
          point.x = theta_pi * sinf(pi_r);
          point.y = theta_pi * cosf(pi_r);
          break;
        }
      case 9:
        point.x = radius * (cosf(theta) + sinf(radius));
        point.y = radius * (sinf(theta) - cosf(radius));
        break;
      case 10:
        point.x = sinf(theta) / radius;
        point.y = radius * cosf(theta);
        break;
      case 11:
        point.x = sinf(theta) * cosf(radius);
        point.y = cosf(theta) * sinf(radius);
        break;
      case 12: {
           float p0 = powf(sinf(theta + radius), 3);
           float p1 = powf(sinf(theta - radius), 3);
           point.x = radius * (p0 + p1);
           point.y = radius * (p0 - p1);
           break;
         }
      case 13: {
           float sqrt_r = sqrtf(radius);
           float omega = RANDOM(0) > 0.5f ? 0.0f : M_PI;
           point.x = sqrt_r * cosf(theta / 2.0f + omega);
           point.y = sqrt_r * sinf(theta / 2.0f + omega);
           break;
         }
      case 14: {
           switch(((p.x >= 0.0f) << 1) | (p.y >= 0.0f)) {
             case 3: /* x >= 0, y >= 0 */
               point.x = p.x;
               point.y = p.y;
               break;
             case 1: /* x < 0, y >= 0 */
               point.x = 2.0f * p.x;
               point.y = p.y;
               break;
             case 2: /* x >= 0, y < 0 */
               point.x = p.x;
               point.y = p.y / 2.0f;
               break;
             case 0:
               point.x = 2.0f * p.x;
               point.y = p.y / 2.0f;
               break;
           }
           break;
         }
      case 15:
         point.x = p.x + B * sinf(p.y / (C * C));
         point.y = p.y + E * sinf(p.x / (F * F));
         break;
      case 16:
         point.x = point.y = 2.0f / (radius + 1.0f);
         point.x *= p.y;
         point.y *= p.x;
         break;
      case 17:
         point.x = p.x + C * sinf(tanf(3.0f * p.y));
         point.y = p.y + F * sinf(tanf(3.0f * p.x));
         break;
      case 18:
         point.x = point.y = expf(p.x - 1.0f);
         point.x *= cosf(M_PI * p.y);
         point.y *= sinf(M_PI * p.y);
         break;
      case 19:
         point.x = point.y = powf(radius, sinf(theta));
         point.x *= cosf(theta);
         point.y *= sinf(theta);
         break;
      case 20:
         point.x = cosf(M_PI * p.x) * coshf(p.y);
         point.y = -sinf(M_PI * p.x) * sinhf(p.y);
         break;
      case 21:
         point.x = point.y = fmodf(radius + C * C, 2.0f * C * C) - C * C
           + radius * (1.0f - C * C);
         point.x *= cosf(theta);
         point.y *= sinf(theta);
         break;
      case 22: {
           float t = M_PI * C * C;
           if(fmodf(theta + F, t) > t / 2.0f) {
             point.x = radius * cosf(theta - t / 2.0f);
             point.y = radius * sinf(theta - t / 2.0f);
           }
           else {
             point.x = radius * cosf(theta + t / 2.0f);
             point.y = radius * sinf(theta + t / 2.0f);
           }
           break;
         }
      case 23:
      case 24:
      case 25:
      case 26:
         /* TODO */
         break;

      case 27:
         point.x = point.y = 2.0f / (radius + 1.0f);
         point.x *= p.x;
         point.y *= p.y;
         break;
      case 28:
         point.x = point.y = 4.0f / (radius * radius + 4.0f);
         point.x *= p.x;
         point.y *= p.y;
         break;
      case 29:
         point.x = sinf(p.x);
         point.y = p.y;
         break;
      case 30:
         /* TODO */
         break;
      case 31: {
           float t = RANDOM(0) * 2.0f * M_PI;
           point.x = RANDOM(1) * point.x * cosf(t);
           point.y = RANDOM(1) * point.y * sinf(t);
           break;
         }
      case 32:
      case 33:
         break;
      case 34: {
           float t = RANDOM(0) * 2.0f * M_PI;
           point.x = RANDOM(1) * cosf(t);
           point.y = RANDOM(1) * sinf(t);
           break;
         }
      case 35:
      case 36:
      case 37:
      case 38:
      case 39:
      case 40:
         break;
      case 41: {
           float t = RANDOM(0) * M_PI * vp[i].factor;
           point.x = sinf(t);
           point.y = point.x * point.x / cosf(t);
           break;
         }
      case 42:
         point.x = sinf(p.x) / cosf(p.y);
         point.y = tanf(p.y);
         break;
      case 43:
         point.x = RANDOM(0) - 0.5f;
         point.y = RANDOM(1) - 0.5f;
         break;
      case 44:
         point.x = point.y = vp[i].factor * tanf(RANDOM(0) * M_PI
             * vp[i].factor) / (radius * radius);
         point.x *= cosf(p.x);
         point.y *= sinf(p.y);
         break;
      case 45: {
           float t = RANDOM(0) * radius * vp[i].factor;
           point.x = p.x * (cosf(t) + sinf(t));
           point.y = p.x * (cosf(t) - sinf(t));
           break;
         }
      case 46:
         point.x = p.x;
         point.y = 1.0f / (vp[i].factor * cosf(radius * vp[i].factor));
         break;
      case 47: {
           float f = RANDOM(0) * radius * vp[i].factor;
           float t = log10f(f * f) + cosf(f);
           point.x = p.x * t;
           point.y = t - M_PI * sinf(f);
           break;
         }
      case 48:
         point.x = point.y = fabsf(1.0f / (p.x * p.x - p.y * p.y));
         point.x *= p.x;
         point.y *= p.y;
         break;
         /* }}} end variations */
    } /* switch */

    point_out.x += vp[i].factor * point.x;
    point_out.y += vp[i].factor * point.y;

  }
  const float &col = params.function_colors[idx];

  color = (color + col) / 2.0f;

  p.x = point_out.x;
  p.y = point_out.y;

  affine_transform(p, &params.post_transform_params[idx][0]);

#undef A
#undef B
#undef C
#undef D
#undef E
#undef F
#undef RANDOM

}


// --- from kernels.cu ---
int get_function_idx(int idx, const ConstMemParams &params)
{
  int func;
  int start = 0, end = NUM_FUNCTIONS;

  /* use warp size granularity, decreases branch divergence */
  idx &= ~31;

  for(int i = 0; i < 6; i++) {
    func = (start + end) / 2;
    if(params.thread_function_mapping[func] <= idx)
      start = func + 1;
    else
      end = func;
  }

  return func;
}
extern "C"

void kernel_initialize(short2 *short_points, 
                  short *colors, 
                  const unsigned short *perms, 
                  const int perm_num,
                  float2 *start_pos,
                  const float *random_numbers,
                  const ConstMemParams params)
{
    #pragma HLS INTERFACE m_axi port=short_points offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=colors offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=perms offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=perm_num
    #pragma HLS INTERFACE m_axi port=start_pos offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=random_numbers offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=params
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int idx = BLOCK_DIM_X * _bid_x + _tid_x;

            int function = get_function_idx(idx, params);

            int perm_idx = perms[NUM_THREADS * perm_num + idx];
            float2 point = start_pos[perm_idx];

            float color = 0.5f;

            iteration_fractal_flame(point, color, function, idx, random_numbers, params);

            colors[idx] = __float2half_rn(color);
            short_points[idx].x = __float2half_rn(point.x);
            short_points[idx].y = __float2half_rn(point.y);

        }
    }
}
extern "C"

void kernel_iterate(short2 *short_points, 
               short *colors, 
               const unsigned short *perms,
               const int perm_num,
               const float *random_numbers,
               const ConstMemParams params)
{
    #pragma HLS INTERFACE m_axi port=short_points offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=colors offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=perms offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=perm_num
    #pragma HLS INTERFACE m_axi port=random_numbers offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=params
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int idx = BLOCK_DIM_X * _bid_x + _tid_x;

            int function = get_function_idx(idx, params);

            float2 point;
            float color;

            int perm_idx = perms[NUM_THREADS * perm_num + idx];

            color = __half2float(colors[perm_idx]);
            point.x = __half2float(short_points[perm_idx].x);
            point.y = __half2float(short_points[perm_idx].y);

            iteration_fractal_flame(point, color, function, idx, random_numbers, params);

            colors[perm_idx] = __float2half_rn(color);
            short_points[perm_idx].x = __float2half_rn(point.x);
            short_points[perm_idx].y = __float2half_rn(point.y);

        }
    }
}
extern "C"

void kernel_generate_points(float3 *vertices,
                       short2 *short_points, 
                       short *colors, 
                       const unsigned short *perms, 
                       const int perm_num,
                       const float *random_numbers,
                       const ConstMemParams params)
{
    #pragma HLS INTERFACE m_axi port=vertices offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=short_points offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=colors offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=perms offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=perm_num
    #pragma HLS INTERFACE m_axi port=random_numbers offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=params
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int idx = BLOCK_DIM_X * _bid_x + _tid_x;

            int function = get_function_idx(idx, params);

            float2 point;
            float color;

            for(int i = 0; i < NUM_POINTS_PER_THREAD; i++) {
            int perm_idx = perms[((perm_num + i) % NUM_PERMUTATIONS) *
            NUM_THREADS + idx];

            short2 _p = short_points[perm_idx];
            point.x = __half2float(_p.x);
            point.y = __half2float(_p.y);
            color = __half2float(colors[perm_idx]);

            iteration_fractal_flame(point, color, function, idx, random_numbers, params);

            vertices[idx + i * NUM_THREADS].x = point.x;
            vertices[idx + i * NUM_THREADS].y = point.y;
            vertices[idx + i * NUM_THREADS].z = color;
            }

        }
    }
}
