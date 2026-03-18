#include "kernel.h"

// --- from lulesh.cu ---
void SumElemFaceNormal(Real_t *normalX0, Real_t *normalY0, Real_t *normalZ0,
    Real_t *normalX1, Real_t *normalY1, Real_t *normalZ1,
    Real_t *normalX2, Real_t *normalY2, Real_t *normalZ2,
    Real_t *normalX3, Real_t *normalY3, Real_t *normalZ3,
    const Real_t x0, const Real_t y0, const Real_t z0,
    const Real_t x1, const Real_t y1, const Real_t z1,
    const Real_t x2, const Real_t y2, const Real_t z2,
    const Real_t x3, const Real_t y3, const Real_t z3)
{
  Real_t bisectX0 = Real_t(0.5) * (x3 + x2 - x1 - x0);
  Real_t bisectY0 = Real_t(0.5) * (y3 + y2 - y1 - y0);
  Real_t bisectZ0 = Real_t(0.5) * (z3 + z2 - z1 - z0);
  Real_t bisectX1 = Real_t(0.5) * (x2 + x1 - x3 - x0);
  Real_t bisectY1 = Real_t(0.5) * (y2 + y1 - y3 - y0);
  Real_t bisectZ1 = Real_t(0.5) * (z2 + z1 - z3 - z0);
  Real_t areaX = Real_t(0.25) * (bisectY0 * bisectZ1 - bisectZ0 * bisectY1);
  Real_t areaY = Real_t(0.25) * (bisectZ0 * bisectX1 - bisectX0 * bisectZ1);
  Real_t areaZ = Real_t(0.25) * (bisectX0 * bisectY1 - bisectY0 * bisectX1);

  *normalX0 += areaX;
  *normalX1 += areaX;
  *normalX2 += areaX;
  *normalX3 += areaX;

  *normalY0 += areaY;
  *normalY1 += areaY;
  *normalY2 += areaY;
  *normalY3 += areaY;

  *normalZ0 += areaZ;
  *normalZ1 += areaZ;
  *normalZ2 += areaZ;
  *normalZ3 += areaZ;
}

void CalcElemShapeFunctionDerivatives( Real_t const x[],
    Real_t const y[],
    Real_t const z[],
    Real_t b[][8],
    Real_t* const volume )
{
  const Real_t x0 = x[0] ;   const Real_t x1 = x[1] ;
  const Real_t x2 = x[2] ;   const Real_t x3 = x[3] ;
  const Real_t x4 = x[4] ;   const Real_t x5 = x[5] ;
  const Real_t x6 = x[6] ;   const Real_t x7 = x[7] ;

  const Real_t y0 = y[0] ;   const Real_t y1 = y[1] ;
  const Real_t y2 = y[2] ;   const Real_t y3 = y[3] ;
  const Real_t y4 = y[4] ;   const Real_t y5 = y[5] ;
  const Real_t y6 = y[6] ;   const Real_t y7 = y[7] ;

  const Real_t z0 = z[0] ;   const Real_t z1 = z[1] ;
  const Real_t z2 = z[2] ;   const Real_t z3 = z[3] ;
  const Real_t z4 = z[4] ;   const Real_t z5 = z[5] ;
  const Real_t z6 = z[6] ;   const Real_t z7 = z[7] ;

  Real_t fjxxi, fjxet, fjxze;
  Real_t fjyxi, fjyet, fjyze;
  Real_t fjzxi, fjzet, fjzze;
  Real_t cjxxi, cjxet, cjxze;
  Real_t cjyxi, cjyet, cjyze;
  Real_t cjzxi, cjzet, cjzze;

  fjxxi = Real_t(.125) * ( (x6-x0) + (x5-x3) - (x7-x1) - (x4-x2) );
  fjxet = Real_t(.125) * ( (x6-x0) - (x5-x3) + (x7-x1) - (x4-x2) );
  fjxze = Real_t(.125) * ( (x6-x0) + (x5-x3) + (x7-x1) + (x4-x2) );

  fjyxi = Real_t(.125) * ( (y6-y0) + (y5-y3) - (y7-y1) - (y4-y2) );
  fjyet = Real_t(.125) * ( (y6-y0) - (y5-y3) + (y7-y1) - (y4-y2) );
  fjyze = Real_t(.125) * ( (y6-y0) + (y5-y3) + (y7-y1) + (y4-y2) );

  fjzxi = Real_t(.125) * ( (z6-z0) + (z5-z3) - (z7-z1) - (z4-z2) );
  fjzet = Real_t(.125) * ( (z6-z0) - (z5-z3) + (z7-z1) - (z4-z2) );
  fjzze = Real_t(.125) * ( (z6-z0) + (z5-z3) + (z7-z1) + (z4-z2) );

  /* compute cofactors */
  cjxxi =    (fjyet * fjzze) - (fjzet * fjyze);
  cjxet =  - (fjyxi * fjzze) + (fjzxi * fjyze);
  cjxze =    (fjyxi * fjzet) - (fjzxi * fjyet);

  cjyxi =  - (fjxet * fjzze) + (fjzet * fjxze);
  cjyet =    (fjxxi * fjzze) - (fjzxi * fjxze);
  cjyze =  - (fjxxi * fjzet) + (fjzxi * fjxet);

  cjzxi =    (fjxet * fjyze) - (fjyet * fjxze);
  cjzet =  - (fjxxi * fjyze) + (fjyxi * fjxze);
  cjzze =    (fjxxi * fjyet) - (fjyxi * fjxet);

  /* calculate partials :
     this need only be done for l = 0,1,2,3   since , by symmetry ,
     (6,7,4,5) = - (0,1,2,3) .
   */
  b[0][0] =   -  cjxxi  -  cjxet  -  cjxze;
  b[0][1] =      cjxxi  -  cjxet  -  cjxze;
  b[0][2] =      cjxxi  +  cjxet  -  cjxze;
  b[0][3] =   -  cjxxi  +  cjxet  -  cjxze;
  b[0][4] = -b[0][2];
  b[0][5] = -b[0][3];
  b[0][6] = -b[0][0];
  b[0][7] = -b[0][1];

  b[1][0] =   -  cjyxi  -  cjyet  -  cjyze;
  b[1][1] =      cjyxi  -  cjyet  -  cjyze;
  b[1][2] =      cjyxi  +  cjyet  -  cjyze;
  b[1][3] =   -  cjyxi  +  cjyet  -  cjyze;
  b[1][4] = -b[1][2];
  b[1][5] = -b[1][3];
  b[1][6] = -b[1][0];
  b[1][7] = -b[1][1];

  b[2][0] =   -  cjzxi  -  cjzet  -  cjzze;
  b[2][1] =      cjzxi  -  cjzet  -  cjzze;
  b[2][2] =      cjzxi  +  cjzet  -  cjzze;
  b[2][3] =   -  cjzxi  +  cjzet  -  cjzze;
  b[2][4] = -b[2][2];
  b[2][5] = -b[2][3];
  b[2][6] = -b[2][0];
  b[2][7] = -b[2][1];

  /* calculate jacobian determinant (volume) */
  *volume = Real_t(8.) * ( fjxet * cjxet + fjyet * cjyet + fjzet * cjzet);
}

void CalcElemNodeNormals(Real_t pfx[8],
    Real_t pfy[8],
    Real_t pfz[8],
    const Real_t x[8],
    const Real_t y[8],
    const Real_t z[8])
{
  for (Index_t i = 0 ; i < 8 ; ++i) {
    pfx[i] = Real_t(0.0);
    pfy[i] = Real_t(0.0);
    pfz[i] = Real_t(0.0);
  }
  /* evaluate face one: nodes 0, 1, 2, 3 */
  SumElemFaceNormal(&pfx[0], &pfy[0], &pfz[0],
      &pfx[1], &pfy[1], &pfz[1],
      &pfx[2], &pfy[2], &pfz[2],
      &pfx[3], &pfy[3], &pfz[3],
      x[0], y[0], z[0], x[1], y[1], z[1],
      x[2], y[2], z[2], x[3], y[3], z[3]);
  /* evaluate face two: nodes 0, 4, 5, 1 */
  SumElemFaceNormal(&pfx[0], &pfy[0], &pfz[0],
      &pfx[4], &pfy[4], &pfz[4],
      &pfx[5], &pfy[5], &pfz[5],
      &pfx[1], &pfy[1], &pfz[1],
      x[0], y[0], z[0], x[4], y[4], z[4],
      x[5], y[5], z[5], x[1], y[1], z[1]);
  /* evaluate face three: nodes 1, 5, 6, 2 */
  SumElemFaceNormal(&pfx[1], &pfy[1], &pfz[1],
      &pfx[5], &pfy[5], &pfz[5],
      &pfx[6], &pfy[6], &pfz[6],
      &pfx[2], &pfy[2], &pfz[2],
      x[1], y[1], z[1], x[5], y[5], z[5],
      x[6], y[6], z[6], x[2], y[2], z[2]);
  /* evaluate face four: nodes 2, 6, 7, 3 */
  SumElemFaceNormal(&pfx[2], &pfy[2], &pfz[2],
      &pfx[6], &pfy[6], &pfz[6],
      &pfx[7], &pfy[7], &pfz[7],
      &pfx[3], &pfy[3], &pfz[3],
      x[2], y[2], z[2], x[6], y[6], z[6],
      x[7], y[7], z[7], x[3], y[3], z[3]);
  /* evaluate face five: nodes 3, 7, 4, 0 */
  SumElemFaceNormal(&pfx[3], &pfy[3], &pfz[3],
      &pfx[7], &pfy[7], &pfz[7],
      &pfx[4], &pfy[4], &pfz[4],
      &pfx[0], &pfy[0], &pfz[0],
      x[3], y[3], z[3], x[7], y[7], z[7],
      x[4], y[4], z[4], x[0], y[0], z[0]);
  /* evaluate face six: nodes 4, 7, 6, 5 */
  SumElemFaceNormal(&pfx[4], &pfy[4], &pfz[4],
      &pfx[7], &pfy[7], &pfz[7],
      &pfx[6], &pfy[6], &pfz[6],
      &pfx[5], &pfy[5], &pfz[5],
      x[4], y[4], z[4], x[7], y[7], z[7],
      x[6], y[6], z[6], x[5], y[5], z[5]);
}

void SumElemStressesToNodeForces( const Real_t B[][8],
    const Real_t stress_xx,
    const Real_t stress_yy,
    const Real_t stress_zz,
    Real_t fx[], Real_t fy[], Real_t fz[] )
{
  for(Index_t i = 0; i < 8; i++) {
    fx[i] = -( stress_xx * B[0][i] );
    fy[i] = -( stress_yy * B[1][i] );
    fz[i] = -( stress_zz * B[2][i] );
  }
}

void VoluDer(const Real_t x0, const Real_t x1, const Real_t x2,
    const Real_t x3, const Real_t x4, const Real_t x5,
    const Real_t y0, const Real_t y1, const Real_t y2,
    const Real_t y3, const Real_t y4, const Real_t y5,
    const Real_t z0, const Real_t z1, const Real_t z2,
    const Real_t z3, const Real_t z4, const Real_t z5,
    Real_t* dvdx, Real_t* dvdy, Real_t* dvdz)
{
  const Real_t twelfth = Real_t(1.0) / Real_t(12.0) ;

  *dvdx =
    (y1 + y2) * (z0 + z1) - (y0 + y1) * (z1 + z2) +
    (y0 + y4) * (z3 + z4) - (y3 + y4) * (z0 + z4) -
    (y2 + y5) * (z3 + z5) + (y3 + y5) * (z2 + z5);
  *dvdy =
    - (x1 + x2) * (z0 + z1) + (x0 + x1) * (z1 + z2) -
    (x0 + x4) * (z3 + z4) + (x3 + x4) * (z0 + z4) +
    (x2 + x5) * (z3 + z5) - (x3 + x5) * (z2 + z5);

  *dvdz =
    - (y1 + y2) * (x0 + x1) + (y0 + y1) * (x1 + x2) -
    (y0 + y4) * (x3 + x4) + (y3 + y4) * (x0 + x4) +
    (y2 + y5) * (x3 + x5) - (y3 + y5) * (x2 + x5);

  *dvdx *= twelfth;
  *dvdy *= twelfth;
  *dvdz *= twelfth;
}

void CalcElemVolumeDerivative(Real_t dvdx[8],
    Real_t dvdy[8],
    Real_t dvdz[8],
    const Real_t x[8],
    const Real_t y[8],
    const Real_t z[8])
{
  VoluDer(x[1], x[2], x[3], x[4], x[5], x[7],
      y[1], y[2], y[3], y[4], y[5], y[7],
      z[1], z[2], z[3], z[4], z[5], z[7],
      &dvdx[0], &dvdy[0], &dvdz[0]);
  VoluDer(x[0], x[1], x[2], x[7], x[4], x[6],
      y[0], y[1], y[2], y[7], y[4], y[6],
      z[0], z[1], z[2], z[7], z[4], z[6],
      &dvdx[3], &dvdy[3], &dvdz[3]);
  VoluDer(x[3], x[0], x[1], x[6], x[7], x[5],
      y[3], y[0], y[1], y[6], y[7], y[5],
      z[3], z[0], z[1], z[6], z[7], z[5],
      &dvdx[2], &dvdy[2], &dvdz[2]);
  VoluDer(x[2], x[3], x[0], x[5], x[6], x[4],
      y[2], y[3], y[0], y[5], y[6], y[4],
      z[2], z[3], z[0], z[5], z[6], z[4],
      &dvdx[1], &dvdy[1], &dvdz[1]);
  VoluDer(x[7], x[6], x[5], x[0], x[3], x[1],
      y[7], y[6], y[5], y[0], y[3], y[1],
      z[7], z[6], z[5], z[0], z[3], z[1],
      &dvdx[4], &dvdy[4], &dvdz[4]);
  VoluDer(x[4], x[7], x[6], x[1], x[0], x[2],
      y[4], y[7], y[6], y[1], y[0], y[2],
      z[4], z[7], z[6], z[1], z[0], z[2],
      &dvdx[5], &dvdy[5], &dvdz[5]);
  VoluDer(x[5], x[4], x[7], x[2], x[1], x[3],
      y[5], y[4], y[7], y[2], y[1], y[3],
      z[5], z[4], z[7], z[2], z[1], z[3],
      &dvdx[6], &dvdy[6], &dvdz[6]);
  VoluDer(x[6], x[5], x[4], x[3], x[2], x[0],
      y[6], y[5], y[4], y[3], y[2], y[0],
      z[6], z[5], z[4], z[3], z[2], z[0],
      &dvdx[7], &dvdy[7], &dvdz[7]);
}

Real_t calcElemVolume( const Real_t x0, const Real_t x1,
    const Real_t x2, const Real_t x3,
    const Real_t x4, const Real_t x5,
    const Real_t x6, const Real_t x7,
    const Real_t y0, const Real_t y1,
    const Real_t y2, const Real_t y3,
    const Real_t y4, const Real_t y5,
    const Real_t y6, const Real_t y7,
    const Real_t z0, const Real_t z1,
    const Real_t z2, const Real_t z3,
    const Real_t z4, const Real_t z5,
    const Real_t z6, const Real_t z7 )
{
  Real_t twelveth = Real_t(1.0)/Real_t(12.0);

  Real_t dx61 = x6 - x1;
  Real_t dy61 = y6 - y1;
  Real_t dz61 = z6 - z1;

  Real_t dx70 = x7 - x0;
  Real_t dy70 = y7 - y0;
  Real_t dz70 = z7 - z0;

  Real_t dx63 = x6 - x3;
  Real_t dy63 = y6 - y3;
  Real_t dz63 = z6 - z3;

  Real_t dx20 = x2 - x0;
  Real_t dy20 = y2 - y0;
  Real_t dz20 = z2 - z0;

  Real_t dx50 = x5 - x0;
  Real_t dy50 = y5 - y0;
  Real_t dz50 = z5 - z0;

  Real_t dx64 = x6 - x4;
  Real_t dy64 = y6 - y4;
  Real_t dz64 = z6 - z4;

  Real_t dx31 = x3 - x1;
  Real_t dy31 = y3 - y1;
  Real_t dz31 = z3 - z1;

  Real_t dx72 = x7 - x2;
  Real_t dy72 = y7 - y2;
  Real_t dz72 = z7 - z2;

  Real_t dx43 = x4 - x3;
  Real_t dy43 = y4 - y3;
  Real_t dz43 = z4 - z3;

  Real_t dx57 = x5 - x7;
  Real_t dy57 = y5 - y7;
  Real_t dz57 = z5 - z7;

  Real_t dx14 = x1 - x4;
  Real_t dy14 = y1 - y4;
  Real_t dz14 = z1 - z4;

  Real_t dx25 = x2 - x5;
  Real_t dy25 = y2 - y5;
  Real_t dz25 = z2 - z5;

#define TRIPLE_PRODUCT(x1, y1, z1, x2, y2, z2, x3, y3, z3) \
  ((x1)*((y2)*(z3) - (z2)*(y3)) + (x2)*((z1)*(y3) - (y1)*(z3)) + (x3)*((y1)*(z2) - (z1)*(y2)))

  Real_t volume =
    TRIPLE_PRODUCT(dx31 + dx72, dx63, dx20,
        dy31 + dy72, dy63, dy20,
        dz31 + dz72, dz63, dz20) +
    TRIPLE_PRODUCT(dx43 + dx57, dx64, dx70,
        dy43 + dy57, dy64, dy70,
        dz43 + dz57, dz64, dz70) +
    TRIPLE_PRODUCT(dx14 + dx25, dx61, dx50,
        dy14 + dy25, dy61, dy50,
        dz14 + dz25, dz61, dz50);

#undef TRIPLE_PRODUCT

  volume *= twelveth;

  return volume ;
}

Real_t CalcElemVolume( const Real_t x[8], const Real_t y[8], const Real_t z[8] )
{
  return calcElemVolume(x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7],
      y[0], y[1], y[2], y[3], y[4], y[5], y[6], y[7],
      z[0], z[1], z[2], z[3], z[4], z[5], z[6], z[7]);
}

Real_t AreaFace( const Real_t x0, const Real_t x1,
    const Real_t x2, const Real_t x3,
    const Real_t y0, const Real_t y1,
    const Real_t y2, const Real_t y3,
    const Real_t z0, const Real_t z1,
    const Real_t z2, const Real_t z3)
{
  Real_t fx = (x2 - x0) - (x3 - x1);
  Real_t fy = (y2 - y0) - (y3 - y1);
  Real_t fz = (z2 - z0) - (z3 - z1);
  Real_t gx = (x2 - x0) + (x3 - x1);
  Real_t gy = (y2 - y0) + (y3 - y1);
  Real_t gz = (z2 - z0) + (z3 - z1);
  Real_t area =
    (fx * fx + fy * fy + fz * fz) *
    (gx * gx + gy * gy + gz * gz) -
    (fx * gx + fy * gy + fz * gz) *
    (fx * gx + fy * gy + fz * gz);
  return area ;
}

Real_t CalcElemCharacteristicLength( const Real_t x[8],
    const Real_t y[8],
    const Real_t z[8],
    const Real_t volume)
{
  Real_t a, charLength = Real_t(0.0);

  a = AreaFace(x[0],x[1],x[2],x[3],
      y[0],y[1],y[2],y[3],
      z[0],z[1],z[2],z[3]) ;
  charLength = max(a,charLength) ;

  a = AreaFace(x[4],x[5],x[6],x[7],
      y[4],y[5],y[6],y[7],
      z[4],z[5],z[6],z[7]) ;
  charLength = max(a,charLength) ;

  a = AreaFace(x[0],x[1],x[5],x[4],
      y[0],y[1],y[5],y[4],
      z[0],z[1],z[5],z[4]) ;
  charLength = max(a,charLength) ;

  a = AreaFace(x[1],x[2],x[6],x[5],
      y[1],y[2],y[6],y[5],
      z[1],z[2],z[6],z[5]) ;
  charLength = max(a,charLength) ;

  a = AreaFace(x[2],x[3],x[7],x[6],
      y[2],y[3],y[7],y[6],
      z[2],z[3],z[7],z[6]) ;
  charLength = max(a,charLength) ;

  a = AreaFace(x[3],x[0],x[4],x[7],
      y[3],y[0],y[4],y[7],
      z[3],z[0],z[4],z[7]) ;
  charLength = max(a,charLength) ;

  charLength = Real_t(4.0) * volume / sqrt(charLength);

  return charLength;
}

void CalcElemVelocityGradient( const Real_t* const xvel,
    const Real_t* const yvel,
    const Real_t* const zvel,
    const Real_t b[][8],
    const Real_t detJ,
    Real_t* const d )
{
  const Real_t inv_detJ = Real_t(1.0) / detJ ;
  Real_t dyddx, dxddy, dzddx, dxddz, dzddy, dyddz;
  const Real_t* const pfx = b[0];
  const Real_t* const pfy = b[1];
  const Real_t* const pfz = b[2];

  d[0] = inv_detJ * ( pfx[0] * (xvel[0]-xvel[6])
      + pfx[1] * (xvel[1]-xvel[7])
      + pfx[2] * (xvel[2]-xvel[4])
      + pfx[3] * (xvel[3]-xvel[5]) );

  d[1] = inv_detJ * ( pfy[0] * (yvel[0]-yvel[6])
      + pfy[1] * (yvel[1]-yvel[7])
      + pfy[2] * (yvel[2]-yvel[4])
      + pfy[3] * (yvel[3]-yvel[5]) );

  d[2] = inv_detJ * ( pfz[0] * (zvel[0]-zvel[6])
      + pfz[1] * (zvel[1]-zvel[7])
      + pfz[2] * (zvel[2]-zvel[4])
      + pfz[3] * (zvel[3]-zvel[5]) );

  dyddx  = inv_detJ * ( pfx[0] * (yvel[0]-yvel[6])
      + pfx[1] * (yvel[1]-yvel[7])
      + pfx[2] * (yvel[2]-yvel[4])
      + pfx[3] * (yvel[3]-yvel[5]) );

  dxddy  = inv_detJ * ( pfy[0] * (xvel[0]-xvel[6])
      + pfy[1] * (xvel[1]-xvel[7])
      + pfy[2] * (xvel[2]-xvel[4])
      + pfy[3] * (xvel[3]-xvel[5]) );

  dzddx  = inv_detJ * ( pfx[0] * (zvel[0]-zvel[6])
      + pfx[1] * (zvel[1]-zvel[7])
      + pfx[2] * (zvel[2]-zvel[4])
      + pfx[3] * (zvel[3]-zvel[5]) );

  dxddz  = inv_detJ * ( pfz[0] * (xvel[0]-xvel[6])
      + pfz[1] * (xvel[1]-xvel[7])
      + pfz[2] * (xvel[2]-xvel[4])
      + pfz[3] * (xvel[3]-xvel[5]) );

  dzddy  = inv_detJ * ( pfy[0] * (zvel[0]-zvel[6])
      + pfy[1] * (zvel[1]-zvel[7])
      + pfy[2] * (zvel[2]-zvel[4])
      + pfy[3] * (zvel[3]-zvel[5]) );

  dyddz  = inv_detJ * ( pfz[0] * (yvel[0]-yvel[6])
      + pfz[1] * (yvel[1]-yvel[7])
      + pfz[2] * (yvel[2]-yvel[4])
      + pfz[3] * (yvel[3]-yvel[5]) );
  d[5]  = Real_t( .5) * ( dxddy + dyddx );
  d[4]  = Real_t( .5) * ( dxddz + dzddx );
  d[3]  = Real_t( .5) * ( dzddy + dyddz );
}
extern "C"

void fill_sig(
    Real_t * sigxx,
    Real_t * sigyy,
    Real_t * sigzz,
    const Real_t * p,
    const Real_t * q,
    const Index_t numElem )
{
    #pragma HLS INTERFACE m_axi port=sigxx offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=sigyy offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=sigzz offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=p offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=q offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=numElem
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            Index_t i = BLOCK_DIM_X*_bid_x+_tid_x;
            if (i >= numElem) return;
            sigxx[i] = sigyy[i] = sigzz[i] = - p[i] - q[i] ;

        }
    }
}
extern "C"

void integrateStress (
    Real_t * fx_elem,
    Real_t * fy_elem,
    Real_t * fz_elem,
    const Real_t * x,
    const Real_t * y,
    const Real_t * z,
    const Index_t * nodelist,
    const Real_t * sigxx,
    const Real_t * sigyy,
    const Real_t * sigzz,
    Real_t * determ,
    const Index_t numElem) 
{
    #pragma HLS INTERFACE m_axi port=fx_elem offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=fy_elem offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=fz_elem offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=z offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=nodelist offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=sigxx offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=sigyy offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=sigzz offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=determ offset=slave bundle=gmem10
    #pragma HLS INTERFACE s_axilite port=numElem
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            Index_t k = BLOCK_DIM_X*_bid_x+_tid_x;
            if (k >= numElem) return;

            const Index_t* const elemToNode = nodelist + Index_t(8)*k;
            Real_t B[3][8] ;// shape function derivatives
            Real_t x_local[8] ;
            Real_t y_local[8] ;
            Real_t z_local[8] ;
            determ[k] = Real_t(10.0);

            // get nodal coordinates from global arrays and copy into local arrays.
            Index_t nd0i = elemToNode[0] ;
            Index_t nd1i = elemToNode[1] ;
            Index_t nd2i = elemToNode[2] ;
            Index_t nd3i = elemToNode[3] ;
            Index_t nd4i = elemToNode[4] ;
            Index_t nd5i = elemToNode[5] ;
            Index_t nd6i = elemToNode[6] ;
            Index_t nd7i = elemToNode[7] ;

            x_local[0] = x[nd0i];
            x_local[1] = x[nd1i];
            x_local[2] = x[nd2i];
            x_local[3] = x[nd3i];
            x_local[4] = x[nd4i];
            x_local[5] = x[nd5i];
            x_local[6] = x[nd6i];
            x_local[7] = x[nd7i];

            y_local[0] = y[nd0i];
            y_local[1] = y[nd1i];
            y_local[2] = y[nd2i];
            y_local[3] = y[nd3i];
            y_local[4] = y[nd4i];
            y_local[5] = y[nd5i];
            y_local[6] = y[nd6i];
            y_local[7] = y[nd7i];

            z_local[0] = z[nd0i];
            z_local[1] = z[nd1i];
            z_local[2] = z[nd2i];
            z_local[3] = z[nd3i];
            z_local[4] = z[nd4i];
            z_local[5] = z[nd5i];
            z_local[6] = z[nd6i];
            z_local[7] = z[nd7i];

            // Volume calculation involves extra work for numerical consistency
            CalcElemShapeFunctionDerivatives(x_local, y_local, z_local, B, &determ[k]);

            CalcElemNodeNormals( B[0], B[1], B[2], x_local, y_local, z_local );

            // Eliminate thread writing conflicts at the nodes by giving
            // each element its own copy to write to
            SumElemStressesToNodeForces( B, sigxx[k], sigyy[k], sigzz[k],
            &fx_elem[k*8],
            &fy_elem[k*8],
            &fz_elem[k*8] ) ;

        }
    }
}
extern "C"

void acc_final_force (
    const Real_t * fx_elem,
    const Real_t * fy_elem,
    const Real_t * fz_elem,
    Real_t * fx,
    Real_t * fy,
    Real_t * fz,
    const Index_t * nodeElemStart,
    const Index_t * nodeElemCornerList,
    const Index_t numNode) 
{
    #pragma HLS INTERFACE m_axi port=fx_elem offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=fy_elem offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=fz_elem offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fx offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=fy offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=fz offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=nodeElemStart offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=nodeElemCornerList offset=slave bundle=gmem7
    #pragma HLS INTERFACE s_axilite port=numNode
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            Index_t gnode = BLOCK_DIM_X*_bid_x+_tid_x;
            if (gnode >= numNode) return;
            // element count
            const Index_t count = nodeElemStart[gnode+1] - nodeElemStart[gnode];//domain.nodeElemCount(gnode) ;
            // list of all corners
            const Index_t *cornerList = nodeElemCornerList + nodeElemStart[gnode];//domain.nodeElemCornerList(gnode) ;
            Real_t fx_tmp = Real_t(0.0) ;
            Real_t fy_tmp = Real_t(0.0) ;
            Real_t fz_tmp = Real_t(0.0) ;
            for (Index_t i=0 ; i < count ; ++i) {
            Index_t elem = cornerList[i] ;
            fx_tmp += fx_elem[elem] ;
            fy_tmp += fy_elem[elem] ;
            fz_tmp += fz_elem[elem] ;
            }
            fx[gnode] = fx_tmp ;
            fy[gnode] = fy_tmp ;
            fz[gnode] = fz_tmp ;

        }
    }
}
extern "C"

void hgc (
    Real_t * dvdx,
    Real_t * dvdy,
    Real_t * dvdz,
    Real_t * x8n,
    Real_t * y8n,
    Real_t * z8n,
    Real_t * determ,

    const Real_t * x,
    const Real_t * y,
    const Real_t * z,
    const Index_t * nodelist,
    const Real_t * volo,
    const Real_t * v,
    int * vol_error,
    const Index_t numElem )
{
    #pragma HLS INTERFACE m_axi port=dvdx offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dvdy offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dvdz offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=x8n offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=y8n offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=z8n offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=determ offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=z offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=nodelist offset=slave bundle=gmem10
    #pragma HLS INTERFACE m_axi port=volo offset=slave bundle=gmem11
    #pragma HLS INTERFACE m_axi port=v offset=slave bundle=gmem12
    #pragma HLS INTERFACE m_axi port=vol_error offset=slave bundle=gmem13
    #pragma HLS INTERFACE s_axilite port=numElem
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            Index_t i = BLOCK_DIM_X*_bid_x+_tid_x;
            if (i >= numElem) return;

            Real_t  x1[8],  y1[8],  z1[8] ;
            Real_t pfx[8], pfy[8], pfz[8] ;

            const Index_t* elemToNode = nodelist + Index_t(8)*i;

            // CollectDomainNodesToElemNodes(domain, elemToNode, x1, y1, z1);

            // inline the function manually
            Index_t nd0i = elemToNode[0] ;
            Index_t nd1i = elemToNode[1] ;
            Index_t nd2i = elemToNode[2] ;
            Index_t nd3i = elemToNode[3] ;
            Index_t nd4i = elemToNode[4] ;
            Index_t nd5i = elemToNode[5] ;
            Index_t nd6i = elemToNode[6] ;
            Index_t nd7i = elemToNode[7] ;

            x1[0] = x[nd0i];
            x1[1] = x[nd1i];
            x1[2] = x[nd2i];
            x1[3] = x[nd3i];
            x1[4] = x[nd4i];
            x1[5] = x[nd5i];
            x1[6] = x[nd6i];
            x1[7] = x[nd7i];

            y1[0] = y[nd0i];
            y1[1] = y[nd1i];
            y1[2] = y[nd2i];
            y1[3] = y[nd3i];
            y1[4] = y[nd4i];
            y1[5] = y[nd5i];
            y1[6] = y[nd6i];
            y1[7] = y[nd7i];

            z1[0] = z[nd0i];
            z1[1] = z[nd1i];
            z1[2] = z[nd2i];
            z1[3] = z[nd3i];
            z1[4] = z[nd4i];
            z1[5] = z[nd5i];
            z1[6] = z[nd6i];
            z1[7] = z[nd7i];

            CalcElemVolumeDerivative(pfx, pfy, pfz, x1, y1, z1);

            /* load into temporary storage for FB Hour Glass control */
            for(Index_t ii=0;ii<8;++ii){
            Index_t jj=8*i+ii;

            dvdx[jj] = pfx[ii];
            dvdy[jj] = pfy[ii];
            dvdz[jj] = pfz[ii];
            x8n[jj]  = x1[ii];
            y8n[jj]  = y1[ii];
            z8n[jj]  = z1[ii];
            }

            determ[i] = volo[i] * v[i];

            /* Do a check for negative volumes */
            if ( v[i] <= Real_t(0.0) ) {
            vol_error[0] = i;
            }

        }
    }
}
extern "C"

void collect_final_force (
    const Real_t * fx_elem,
    const Real_t * fy_elem,
    const Real_t * fz_elem,
    Real_t * fx,
    Real_t * fy,
    Real_t * fz,
    const Index_t * nodeElemStart,
    const Index_t * nodeElemCornerList,
    const Index_t numNode )
{
    #pragma HLS INTERFACE m_axi port=fx_elem offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=fy_elem offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=fz_elem offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fx offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=fy offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=fz offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=nodeElemStart offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=nodeElemCornerList offset=slave bundle=gmem7
    #pragma HLS INTERFACE s_axilite port=numNode
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            Index_t gnode = BLOCK_DIM_X*_bid_x+_tid_x;
            if (gnode >= numNode) return;
            // element count
            const Index_t count = nodeElemStart[gnode+1] - nodeElemStart[gnode];//domain.nodeElemCount(gnode) ;
            // list of all corners
            const Index_t *cornerList = nodeElemCornerList + nodeElemStart[gnode];//domain.nodeElemCornerList(gnode) ;
            Real_t fx_tmp = Real_t(0.0) ;
            Real_t fy_tmp = Real_t(0.0) ;
            Real_t fz_tmp = Real_t(0.0) ;
            for (Index_t i=0 ; i < count ; ++i) {
            Index_t elem = cornerList[i] ;
            fx_tmp += fx_elem[elem] ;
            fy_tmp += fy_elem[elem] ;
            fz_tmp += fz_elem[elem] ;
            }
            fx[gnode] = fx_tmp ;
            fy[gnode] = fy_tmp ;
            fz[gnode] = fz_tmp ;

        }
    }
}
extern "C"

void accelerationForNode (
    const Real_t * fx,
    const Real_t * fy,
    const Real_t * fz,
    const Real_t * nodalMass,
    Real_t * xdd,
    Real_t * ydd,
    Real_t * zdd,
    const Index_t numNode)
{
    #pragma HLS INTERFACE m_axi port=fx offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=fy offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=fz offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=nodalMass offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=xdd offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=ydd offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=zdd offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=numNode
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            Index_t i = BLOCK_DIM_X*_bid_x+_tid_x;
            if (i >= numNode) return;
            Real_t one_over_nMass = Real_t(1.) / nodalMass[i];
            xdd[i] = fx[i] * one_over_nMass;
            ydd[i] = fy[i] * one_over_nMass;
            zdd[i] = fz[i] * one_over_nMass;

        }
    }
}
extern "C"

void applyAccelerationBoundaryConditionsForNodes (
    const Index_t * symmX,
    const Index_t * symmY,
    const Index_t * symmZ,
    Real_t * xdd,
    Real_t * ydd,
    Real_t * zdd,
    const Index_t s1,
    const Index_t s2,
    const Index_t s3,
    const Index_t numNodeBC ) 
{
    #pragma HLS INTERFACE m_axi port=symmX offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=symmY offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=symmZ offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=xdd offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=ydd offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=zdd offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=s1
    #pragma HLS INTERFACE s_axilite port=s2
    #pragma HLS INTERFACE s_axilite port=s3
    #pragma HLS INTERFACE s_axilite port=numNodeBC
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            Index_t i = BLOCK_DIM_X*_bid_x+_tid_x;
            if (i >= numNodeBC) return;
            if (s1 == 0)
            xdd[symmX[i]] = Real_t(0.0);
            if (s2 == 0) ydd[symmY[i]] = Real_t(0.0);
            if (s3 == 0) zdd[symmZ[i]] = Real_t(0.0);

        }
    }
}
extern "C"

void  calcVelocityForNodes (
    Real_t * xd,
    Real_t * yd,
    Real_t * zd,
    const Real_t * xdd,
    const Real_t * ydd,
    const Real_t * zdd,
    const Real_t deltaTime,
    const Real_t u_cut,
    const Index_t numNode )
{
    #pragma HLS INTERFACE m_axi port=xd offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=yd offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=zd offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=xdd offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=ydd offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=zdd offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=deltaTime
    #pragma HLS INTERFACE s_axilite port=u_cut
    #pragma HLS INTERFACE s_axilite port=numNode
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            Index_t i = BLOCK_DIM_X*_bid_x+_tid_x;
            if (i >= numNode) return;

            Real_t xdtmp = xd[i] + xdd[i] * deltaTime;
            // FABS is not compiled with target regions in mind
            // To get around this, compute the absolute value manually:
            // if( xdtmp > Real_t(0.0) && xdtmp < u_cut || Real_t(-1.0) * xdtmp < u_cut)
            if( fabs(xdtmp) < u_cut ) xdtmp = Real_t(0.0);
            xd[i] = xdtmp ;

            Real_t ydtmp = yd[i] + ydd[i] * deltaTime;
            if( fabs(ydtmp) < u_cut ) ydtmp = Real_t(0.0);
            yd[i] = ydtmp ;

            Real_t zdtmp = zd[i] + zdd[i] * deltaTime;
            if( fabs(zdtmp) < u_cut ) zdtmp = Real_t(0.0);
            zd[i] = zdtmp ;

        }
    }
}
extern "C"

void calcPositionForNodes (
    Real_t * x,
    Real_t * y,
    Real_t * z,
    const Real_t * xd,
    const Real_t * yd,
    const Real_t * zd,
    const Real_t deltaTime,
    const Index_t numNode) 
{
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=z offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=xd offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=yd offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=zd offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=deltaTime
    #pragma HLS INTERFACE s_axilite port=numNode
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            Index_t i = BLOCK_DIM_X*_bid_x+_tid_x;
            if (i >= numNode) return;
            x[i] += xd[i] * deltaTime;
            y[i] += yd[i] * deltaTime;
            z[i] += zd[i] * deltaTime;

        }
    }
}
extern "C"

void calcKinematicsForElems ( 
    const Real_t * xd,
    const Real_t * yd,
    const Real_t * zd,
    const Real_t * x,
    const Real_t * y,
    const Real_t * z,
    const Index_t * nodeList,
    const Real_t * volo,
    const Real_t * v,
    Real_t * delv,
    Real_t * arealg,
    Real_t * dxx,
    Real_t * dyy,
    Real_t * dzz,
    Real_t * vnew,
    const Real_t deltaTime,
    const Index_t numElem )
{
    #pragma HLS INTERFACE m_axi port=xd offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=yd offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=zd offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=z offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=nodeList offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=volo offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=v offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=delv offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=arealg offset=slave bundle=gmem10
    #pragma HLS INTERFACE m_axi port=dxx offset=slave bundle=gmem11
    #pragma HLS INTERFACE m_axi port=dyy offset=slave bundle=gmem12
    #pragma HLS INTERFACE m_axi port=dzz offset=slave bundle=gmem13
    #pragma HLS INTERFACE m_axi port=vnew offset=slave bundle=gmem14
    #pragma HLS INTERFACE s_axilite port=deltaTime
    #pragma HLS INTERFACE s_axilite port=numElem
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            Index_t k = BLOCK_DIM_X*_bid_x+_tid_x;
            if (k >= numElem) return;

            Real_t B[3][8] ; // shape function derivatives
            Real_t D[6] ;
            Real_t x_local[8] ;
            Real_t y_local[8] ;
            Real_t z_local[8] ;
            Real_t xd_local[8] ;
            Real_t yd_local[8] ;
            Real_t zd_local[8] ;
            Real_t detJ = Real_t(0.0) ;

            Real_t volume ;
            Real_t relativeVolume ;
            const Index_t* elemToNode = nodeList + Index_t(8)*k;

            // get nodal coordinates from global arrays and copy into local arrays.

            Index_t nd0i = elemToNode[0] ;
            Index_t nd1i = elemToNode[1] ;
            Index_t nd2i = elemToNode[2] ;
            Index_t nd3i = elemToNode[3] ;
            Index_t nd4i = elemToNode[4] ;
            Index_t nd5i = elemToNode[5] ;
            Index_t nd6i = elemToNode[6] ;
            Index_t nd7i = elemToNode[7] ;

            x_local[0] = x[nd0i];
            x_local[1] = x[nd1i];
            x_local[2] = x[nd2i];
            x_local[3] = x[nd3i];
            x_local[4] = x[nd4i];
            x_local[5] = x[nd5i];
            x_local[6] = x[nd6i];
            x_local[7] = x[nd7i];

            y_local[0] = y[nd0i];
            y_local[1] = y[nd1i];
            y_local[2] = y[nd2i];
            y_local[3] = y[nd3i];
            y_local[4] = y[nd4i];
            y_local[5] = y[nd5i];
            y_local[6] = y[nd6i];
            y_local[7] = y[nd7i];

            z_local[0] = z[nd0i];
            z_local[1] = z[nd1i];
            z_local[2] = z[nd2i];
            z_local[3] = z[nd3i];
            z_local[4] = z[nd4i];
            z_local[5] = z[nd5i];
            z_local[6] = z[nd6i];
            z_local[7] = z[nd7i];

            // volume calculations
            volume = CalcElemVolume(x_local, y_local, z_local );
            relativeVolume = volume / volo[k] ;
            vnew[k] = relativeVolume ;
            delv[k] = relativeVolume - v[k] ;

            // set characteristic length
            arealg[k] = CalcElemCharacteristicLength(x_local, y_local, z_local,
            volume);

            // get nodal velocities from global array and copy into local arrays.
            for( Index_t lnode=0 ; lnode<8 ; ++lnode )
            {
            Index_t gnode = elemToNode[lnode];
            xd_local[lnode] = xd[gnode];
            yd_local[lnode] = yd[gnode];
            zd_local[lnode] = zd[gnode];
            }

            Real_t dt2 = Real_t(0.5) * deltaTime;
            for ( Index_t j=0 ; j<8 ; ++j )
            {
            x_local[j] -= dt2 * xd_local[j];
            y_local[j] -= dt2 * yd_local[j];
            z_local[j] -= dt2 * zd_local[j];
            }

            CalcElemShapeFunctionDerivatives( x_local, y_local, z_local,
            B, &detJ );

            CalcElemVelocityGradient( xd_local, yd_local, zd_local,
            B, detJ, D );

            // put velocity gradient quantities into their global arrays.
            dxx[k] = D[0];
            dyy[k] = D[1];
            dzz[k] = D[2];

        }
    }
}
extern "C"

void calcStrainRates(
    Real_t * dxx,
    Real_t * dyy,
    Real_t * dzz,
    const Real_t * vnew,
    Real_t * vdov,
    int * vol_error,
    const Index_t numElem )
{
    #pragma HLS INTERFACE m_axi port=dxx offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dyy offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dzz offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=vnew offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=vdov offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=vol_error offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=numElem
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            Index_t k = BLOCK_DIM_X*_bid_x+_tid_x;
            if (k >= numElem) return;

            // calc strain rate and apply as constraint (only done in FB element)
            Real_t vvdov = dxx[k] + dyy[k] + dzz[k] ;
            Real_t vdovthird = vvdov/Real_t(3.0) ;

            // make the rate of deformation tensor deviatoric
            vdov[k] = vvdov;
            dxx[k] -= vdovthird ;  //LG:   why to update dxx?  it is deallocated right after
            dyy[k] -= vdovthird ;
            dzz[k] -= vdovthird ;

            // See if any volumes are negative, and take appropriate action.
            if (vnew[k] <= Real_t(0.0))
            {
            vol_error[0] = k;
            }

        }
    }
}
extern "C"

void calcMonotonicQGradientsForElems (
    const Real_t * xd,
    const Real_t * yd,
    const Real_t * zd,
    const Real_t * x,
    const Real_t * y,
    const Real_t * z,
    const Index_t * nodelist,
    const Real_t * volo,
    Real_t * delv_eta,
    Real_t * delx_eta,
    Real_t * delv_zeta,
    Real_t * delx_zeta,
    Real_t * delv_xi,
    Real_t * delx_xi,
    const Real_t * vnew,
    const Index_t numElem )
{
    #pragma HLS INTERFACE m_axi port=xd offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=yd offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=zd offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=z offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=nodelist offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=volo offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=delv_eta offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=delx_eta offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=delv_zeta offset=slave bundle=gmem10
    #pragma HLS INTERFACE m_axi port=delx_zeta offset=slave bundle=gmem11
    #pragma HLS INTERFACE m_axi port=delv_xi offset=slave bundle=gmem12
    #pragma HLS INTERFACE m_axi port=delx_xi offset=slave bundle=gmem13
    #pragma HLS INTERFACE m_axi port=vnew offset=slave bundle=gmem14
    #pragma HLS INTERFACE s_axilite port=numElem
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            Index_t i = BLOCK_DIM_X*_bid_x+_tid_x;
            if (i >= numElem) return;

            Real_t ax,ay,az ;
            Real_t dxv,dyv,dzv ;

            const Index_t *elemToNode = nodelist + Index_t(8) * i;
            Index_t n0 = elemToNode[0] ;
            Index_t n1 = elemToNode[1] ;
            Index_t n2 = elemToNode[2] ;
            Index_t n3 = elemToNode[3] ;
            Index_t n4 = elemToNode[4] ;
            Index_t n5 = elemToNode[5] ;
            Index_t n6 = elemToNode[6] ;
            Index_t n7 = elemToNode[7] ;

            Real_t x0 = x[n0] ;
            Real_t x1 = x[n1] ;
            Real_t x2 = x[n2] ;
            Real_t x3 = x[n3] ;
            Real_t x4 = x[n4] ;
            Real_t x5 = x[n5] ;
            Real_t x6 = x[n6] ;
            Real_t x7 = x[n7] ;

            Real_t y0 = y[n0] ;
            Real_t y1 = y[n1] ;
            Real_t y2 = y[n2] ;
            Real_t y3 = y[n3] ;
            Real_t y4 = y[n4] ;
            Real_t y5 = y[n5] ;
            Real_t y6 = y[n6] ;
            Real_t y7 = y[n7] ;

            Real_t z0 = z[n0] ;
            Real_t z1 = z[n1] ;
            Real_t z2 = z[n2] ;
            Real_t z3 = z[n3] ;
            Real_t z4 = z[n4] ;
            Real_t z5 = z[n5] ;
            Real_t z6 = z[n6] ;
            Real_t z7 = z[n7] ;

            Real_t xv0 = xd[n0] ;
            Real_t xv1 = xd[n1] ;
            Real_t xv2 = xd[n2] ;
            Real_t xv3 = xd[n3] ;
            Real_t xv4 = xd[n4] ;
            Real_t xv5 = xd[n5] ;
            Real_t xv6 = xd[n6] ;
            Real_t xv7 = xd[n7] ;

            Real_t yv0 = yd[n0] ;
            Real_t yv1 = yd[n1] ;
            Real_t yv2 = yd[n2] ;
            Real_t yv3 = yd[n3] ;
            Real_t yv4 = yd[n4] ;
            Real_t yv5 = yd[n5] ;
            Real_t yv6 = yd[n6] ;
            Real_t yv7 = yd[n7] ;

            Real_t zv0 = zd[n0] ;
            Real_t zv1 = zd[n1] ;
            Real_t zv2 = zd[n2] ;
            Real_t zv3 = zd[n3] ;
            Real_t zv4 = zd[n4] ;
            Real_t zv5 = zd[n5] ;
            Real_t zv6 = zd[n6] ;
            Real_t zv7 = zd[n7] ;

            Real_t vol = volo[i] * vnew[i] ;
            Real_t norm = Real_t(1.0) / ( vol + PTINY ) ;

            Real_t dxj = Real_t(-0.25)*((x0+x1+x5+x4) - (x3+x2+x6+x7)) ;
            Real_t dyj = Real_t(-0.25)*((y0+y1+y5+y4) - (y3+y2+y6+y7)) ;
            Real_t dzj = Real_t(-0.25)*((z0+z1+z5+z4) - (z3+z2+z6+z7)) ;

            Real_t dxi = Real_t( 0.25)*((x1+x2+x6+x5) - (x0+x3+x7+x4)) ;
            Real_t dyi = Real_t( 0.25)*((y1+y2+y6+y5) - (y0+y3+y7+y4)) ;
            Real_t dzi = Real_t( 0.25)*((z1+z2+z6+z5) - (z0+z3+z7+z4)) ;

            Real_t dxk = Real_t( 0.25)*((x4+x5+x6+x7) - (x0+x1+x2+x3)) ;
            Real_t dyk = Real_t( 0.25)*((y4+y5+y6+y7) - (y0+y1+y2+y3)) ;
            Real_t dzk = Real_t( 0.25)*((z4+z5+z6+z7) - (z0+z1+z2+z3)) ;

            /* find delvk and delxk ( i cross j ) */

            ax = dyi*dzj - dzi*dyj ;
            ay = dzi*dxj - dxi*dzj ;
            az = dxi*dyj - dyi*dxj ;

            delx_zeta[i] = vol / sqrt(ax*ax + ay*ay + az*az + PTINY) ;

            ax *= norm ;
            ay *= norm ;
            az *= norm ;

            dxv = Real_t(0.25)*((xv4+xv5+xv6+xv7) - (xv0+xv1+xv2+xv3)) ;
            dyv = Real_t(0.25)*((yv4+yv5+yv6+yv7) - (yv0+yv1+yv2+yv3)) ;
            dzv = Real_t(0.25)*((zv4+zv5+zv6+zv7) - (zv0+zv1+zv2+zv3)) ;

            delv_zeta[i] = ax*dxv + ay*dyv + az*dzv ;

            /* find delxi and delvi ( j cross k ) */

            ax = dyj*dzk - dzj*dyk ;
            ay = dzj*dxk - dxj*dzk ;
            az = dxj*dyk - dyj*dxk ;

            delx_xi[i] = vol / sqrt(ax*ax + ay*ay + az*az + PTINY) ;

            ax *= norm ;
            ay *= norm ;
            az *= norm ;

            dxv = Real_t(0.25)*((xv1+xv2+xv6+xv5) - (xv0+xv3+xv7+xv4)) ;
            dyv = Real_t(0.25)*((yv1+yv2+yv6+yv5) - (yv0+yv3+yv7+yv4)) ;
            dzv = Real_t(0.25)*((zv1+zv2+zv6+zv5) - (zv0+zv3+zv7+zv4)) ;

            delv_xi[i] = ax*dxv + ay*dyv + az*dzv ;

            /* find delxj and delvj ( k cross i ) */

            ax = dyk*dzi - dzk*dyi ;
            ay = dzk*dxi - dxk*dzi ;
            az = dxk*dyi - dyk*dxi ;

            delx_eta[i] = vol / sqrt(ax*ax + ay*ay + az*az + PTINY) ;

            ax *= norm ;
            ay *= norm ;
            az *= norm ;

            dxv = Real_t(-0.25)*((xv0+xv1+xv5+xv4) - (xv3+xv2+xv6+xv7)) ;
            dyv = Real_t(-0.25)*((yv0+yv1+yv5+yv4) - (yv3+yv2+yv6+yv7)) ;
            dzv = Real_t(-0.25)*((zv0+zv1+zv5+zv4) - (zv3+zv2+zv6+zv7)) ;

            delv_eta[i] = ax*dxv + ay*dyv + az*dzv ;

        }
    }
}
