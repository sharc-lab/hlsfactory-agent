#include "kernel.h"

// --- from ao.cu ---
  static float vdot(vec v0, vec v1)
{
  return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

  static void vcross(vec *c, vec v0, vec v1)
{

  c->x = v0.y * v1.z - v0.z * v1.y;
  c->y = v0.z * v1.x - v0.x * v1.z;
  c->z = v0.x * v1.y - v0.y * v1.x;
}

  static void vnormalize(vec *c)
{
  float length = sqrtf(vdot((*c), (*c)));

  if (fabs(length) > 1.0e-17f) {
    c->x /= length;
    c->y /= length;
    c->z /= length;
  }
}

  void ray_sphere_intersect(Isect *isect, const Ray *ray, const Sphere *sphere)
{
  vec rs;

  rs.x = ray->org.x - sphere->center.x;
  rs.y = ray->org.y - sphere->center.y;
  rs.z = ray->org.z - sphere->center.z;

  float B = vdot(rs, ray->dir);
  float C = vdot(rs, rs) - sphere->radius * sphere->radius;
  float D = B * B - C;

  if (D > 0.f) {
    float t = -B - sqrtf(D);

    if ((t > 0.f) && (t < isect->t)) {
      isect->t = t;
      isect->hit = 1;

      isect->p.x = ray->org.x + ray->dir.x * t;
      isect->p.y = ray->org.y + ray->dir.y * t;
      isect->p.z = ray->org.z + ray->dir.z * t;

      isect->n.x = isect->p.x - sphere->center.x;
      isect->n.y = isect->p.y - sphere->center.y;
      isect->n.z = isect->p.z - sphere->center.z;

      vnormalize(&(isect->n));
    }
  }
}

  void ray_plane_intersect(Isect *isect, const Ray *ray, const Plane *plane)
{
  float d = -vdot(plane->p, plane->n);
  float v = vdot(ray->dir, plane->n);

  if (fabsf(v) < 1.0e-17f) return;

  float t = -(vdot(ray->org, plane->n) + d) / v;

  if ((t > 0.f) && (t < isect->t)) {
    isect->t = t;
    isect->hit = 1;

    isect->p.x = ray->org.x + ray->dir.x * t;
    isect->p.y = ray->org.y + ray->dir.y * t;
    isect->p.z = ray->org.z + ray->dir.z * t;

    isect->n = plane->n;
  }
}

  void orthoBasis(vec *basis, vec n)
{
  basis[2] = n;
  basis[1].x = 0.f; basis[1].y = 0.f; basis[1].z = 0.f;

  if ((n.x < 0.6f) && (n.x > -0.6f)) {
    basis[1].x = 1.0f;
  } else if ((n.y < 0.6f) && (n.y > -0.6f)) {
    basis[1].y = 1.0f;
  } else if ((n.z < 0.6f) && (n.z > -0.6f)) {
    basis[1].z = 1.0f;
  } else {
    basis[1].x = 1.0f;
  }

  vcross(&basis[0], basis[1], basis[2]);
  vnormalize(&basis[0]);

  vcross(&basis[1], basis[2], basis[0]);
  vnormalize(&basis[1]);
}

    RNG(const unsigned int seed) { x = seed; }   

    int next() {     
        x ^= x >> 6;
        x ^= x << 17;     
        x ^= x >> 9;
        return int(x);
      }

    float operator()(void) {
        union {
          float f;
          int i;
        } u;
        u.i = (next() & fmask) | 0x3f800000;
        return u.f - 1.f;
      }

  void ambient_occlusion(vec *col, const Isect *isect, const Sphere *spheres, const Plane *plane, RNG &rng)
{
  int    i, j;
  int    ntheta = NAO_SAMPLES;
  int    nphi   = NAO_SAMPLES;
  float eps = 0.0001f;

  vec p;

  p.x = isect->p.x + eps * isect->n.x;
  p.y = isect->p.y + eps * isect->n.y;
  p.z = isect->p.z + eps * isect->n.z;

  vec basis[3];
  orthoBasis(basis, isect->n);

  float occlusion = 0.f;

  for (j = 0; j < ntheta; j++) {
    for (i = 0; i < nphi; i++) {
      float theta = sqrtf(rng());
      float phi = 2.0f * (float)M_PI * rng();
      float x = cosf(phi) * theta;
      float y = sinf(phi) * theta;
      float z = sqrtf(1.0f - theta * theta);

      // local -> global
      float rx = x * basis[0].x + y * basis[1].x + z * basis[2].x;
      float ry = x * basis[0].y + y * basis[1].y + z * basis[2].y;
      float rz = x * basis[0].z + y * basis[1].z + z * basis[2].z;

      Ray ray;

      ray.org = p;
      ray.dir.x = rx;
      ray.dir.y = ry;
      ray.dir.z = rz;

      Isect occIsect;
      occIsect.t   = 1.0e+17f;
      occIsect.hit = 0;

      ray_sphere_intersect(&occIsect, &ray, spheres); 
      ray_sphere_intersect(&occIsect, &ray, spheres+1); 
      ray_sphere_intersect(&occIsect, &ray, spheres+2); 
      ray_plane_intersect (&occIsect, &ray, plane); 

      if (occIsect.hit) occlusion += 1.f;

    }
  }

  occlusion = (ntheta * nphi - occlusion) / (float)(ntheta * nphi);

  col->x = occlusion;
  col->y = occlusion;
  col->z = occlusion;
}

  unsigned char clamp(float f)
{
  int i = (int)(f * 255.5f);

  if (i < 0) i = 0;
  if (i > 255) i = 255;

  return (unsigned char)i;
}
extern "C"

  void render_kernel (unsigned char *fimg, const Sphere *spheres, const Plane plane, 
    const int h, const int w, const int nsubsamples)
{
    #pragma HLS INTERFACE m_axi port=fimg offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=spheres offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=plane
    #pragma HLS INTERFACE s_axilite port=h
    #pragma HLS INTERFACE s_axilite port=w
    #pragma HLS INTERFACE s_axilite port=nsubsamples
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int x = _bid_x * BLOCK_DIM_X + _tid_x;
                    int y = _bid_y * BLOCK_DIM_Y + _tid_y;
                    if (y < h && x < w) {

                    RNG rng(y * w + x);
                    float s0 = 0;
                    float s1 = 0;
                    float s2 = 0;

                    for(int  v = 0; v < nsubsamples; v++ ) {
                    for(int  u = 0; u < nsubsamples; u++ ) {
                    float px = ( x + ( u / ( float )nsubsamples ) - ( w / 2.0f ) ) / ( w / 2.0f );
                    float py = -( y + ( v / ( float )nsubsamples ) - ( h / 2.0f ) ) / ( h / 2.0f );

                    Ray ray;
                    ray.org.x = 0.f;
                    ray.org.y = 0.f;
                    ray.org.z = 0.f;
                    ray.dir.x = px;
                    ray.dir.y = py;
                    ray.dir.z = -1.f;
                    vnormalize( &( ray.dir ) );

                    Isect isect;
                    isect.t = 1.0e+17f;
                    isect.hit = 0;

                    ray_sphere_intersect( &isect, &ray, spheres   );
                    ray_sphere_intersect( &isect, &ray, spheres + 1  );
                    ray_sphere_intersect( &isect, &ray, spheres + 2  );
                    ray_plane_intersect ( &isect, &ray, &plane );

                    if( isect.hit ) {
                    vec col;
                    ambient_occlusion( &col, &isect, spheres, &plane, rng );
                    s0 += col.x;
                    s1 += col.y;
                    s2 += col.z;
                    }

                    }
                    }
                    fimg[ 3 * ( y * w + x ) + 0 ] = clamp ( s0 / ( float )( nsubsamples * nsubsamples ) );
                    fimg[ 3 * ( y * w + x ) + 1 ] = clamp ( s1 / ( float )( nsubsamples * nsubsamples ) );
                    fimg[ 3 * ( y * w + x ) + 2 ] = clamp ( s2 / ( float )( nsubsamples * nsubsamples ) );
                    }

                }
            }
        }
    }
}
