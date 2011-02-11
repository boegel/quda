#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <typeinfo>
#include <quda.h>
#include <gauge_quda.h>
#include <quda_internal.h>
#include <face_quda.h>
#include "misc_helpers.h"


#define SHORT_LENGTH 65536
#define SCALE_FLOAT ((SHORT_LENGTH-1) / 2.f)
#define SHIFT_FLOAT (-1.f / (SHORT_LENGTH-1))

static double Anisotropy;
static QudaTboundary tBoundary;
static int X[4];
extern float fat_link_max;

template <typename Float>
inline short FloatToShort(const Float &a) {
  return (short)((a+SHIFT_FLOAT)*SCALE_FLOAT);
}

template <typename Float>
inline void ShortToFloat(Float &a, const short &b) {
  a = ((Float)b/SCALE_FLOAT-SHIFT_FLOAT);
}

// Routines used to pack the gauge field matrices

template <typename Float>
inline void pack8(double2 *res, Float *g, int dir, int V) {
  double2 *r = res + dir*4*V;
  r[0].x = atan2(g[1], g[0]);
  r[0].y = atan2(g[13], g[12]);
  for (int j=1; j<4; j++) {
    r[j*V].x = g[2*j+0];
    r[j*V].y = g[2*j+1];
  }
}

template <typename Float>
inline void pack8(float4 *res, Float *g, int dir, int V) {
  float4 *r = res + dir*2*V;
  r[0].x = atan2(g[1], g[0]);
  r[0].y = atan2(g[13], g[12]);
  r[0].z = g[2];
  r[0].w = g[3];
  r[V].x = g[4];
  r[V].y = g[5];
  r[V].z = g[6];
  r[V].w = g[7];
}

template <typename Float>
inline void pack8(float2 *res, Float *g, int dir, int V) {
  float2 *r = res + dir*4*V;
  r[0].x = atan2(g[1], g[0]);
  r[0].y = atan2(g[13], g[12]);
  for (int j=1; j<4; j++) {
    r[j*V].x = g[2*j+0];
    r[j*V].y = g[2*j+1];
  }
}

template <typename Float>
inline void pack8(short4 *res, Float *g, int dir, int V) {
  short4 *r = res + dir*2*V;
  r[0].x = FloatToShort(atan2(g[1], g[0]) / M_PI);
  r[0].y = FloatToShort(atan2(g[13], g[12]) / M_PI);
  r[0].z = FloatToShort(g[2]);
  r[0].w = FloatToShort(g[3]);
  r[V].x = FloatToShort(g[4]);
  r[V].y = FloatToShort(g[5]);
  r[V].z = FloatToShort(g[6]);
  r[V].w = FloatToShort(g[7]);
}

template <typename Float>
inline void pack8(short2 *res, Float *g, int dir, int V) {
  short2 *r = res + dir*4*V;
  r[0].x = FloatToShort(atan2(g[1], g[0]) / M_PI);
  r[0].y = FloatToShort(atan2(g[13], g[12]) / M_PI);
  for (int j=1; j<4; j++) {
    r[j*V].x = FloatToShort(g[2*j+0]);
    r[j*V].y = FloatToShort(g[2*j+1]);
  }
}

template <typename Float>
inline void pack12(double2 *res, Float *g, int dir, int V) {
  double2 *r = res + dir*6*V;
  for (int j=0; j<6; j++) {
    r[j*V].x = g[j*2+0];
    r[j*V].y = g[j*2+1];
  }
}

template <typename Float>
inline void pack12(float4 *res, Float *g, int dir, int V) {
  float4 *r = res + dir*3*V;
  for (int j=0; j<3; j++) {
    r[j*V].x = g[j*4+0]; 
    r[j*V].y = g[j*4+1];
    r[j*V].z = g[j*4+2]; 
    r[j*V].w = g[j*4+3];
  }
}

template <typename Float>
inline void pack12(float2 *res, Float *g, int dir, int V) {
  float2 *r = res + dir*6*V;
  for (int j=0; j<6; j++) {
    r[j*V].x = g[j*2+0];
    r[j*V].y = g[j*2+1];
  }
}

template <typename Float>
inline void pack12(short4 *res, Float *g, int dir, int V) {
  short4 *r = res + dir*3*V;
  for (int j=0; j<3; j++) {
    r[j*V].x = FloatToShort(g[j*4+0]); 
    r[j*V].y = FloatToShort(g[j*4+1]);
    r[j*V].z = FloatToShort(g[j*4+2]);
    r[j*V].w = FloatToShort(g[j*4+3]);
  }
}

template <typename Float>
inline void pack12(short2 *res, Float *g, int dir, int V) {
  short2 *r = res + dir*6*V;
  for (int j=0; j<6; j++) {
    r[j*V].x = FloatToShort(g[j*2+0]);
    r[j*V].y = FloatToShort(g[j*2+1]);
  }
}

template <typename Float>
inline void pack18(double2 *res, Float *g, int dir, int V) {
  double2 *r = res + dir*9*V;
  for (int j=0; j<9; j++) {
    r[j*V].x = g[j*2+0]; 
    r[j*V].y = g[j*2+1]; 
  }
}

template <typename Float>
inline void pack18(float4 *res, Float *g, int dir, int V) {
  float4 *r = res + dir*5*V;
  for (int j=0; j<4; j++) {
    r[j*V].x = g[j*4+0]; 
    r[j*V].y = g[j*4+1]; 
    r[j*V].z = g[j*4+2]; 
    r[j*V].w = g[j*4+3]; 
  }
  r[4*V].x = g[16]; 
  r[4*V].y = g[17]; 
  r[4*V].z = 0.0;
  r[4*V].w = 0.0;
}

template <typename Float>
inline void pack18(float2 *res, Float *g, int dir, int V) {
  float2 *r = res + dir*9*V;
  for (int j=0; j<9; j++) {
    r[j*V].x = g[j*2+0]; 
    r[j*V].y = g[j*2+1]; 
  }
}

template <typename Float>
inline void pack18(short4 *res, Float *g, int dir, int V) {
  short4 *r = res + dir*5*V;
  for (int j=0; j<4; j++) {
    r[j*V].x = FloatToShort(g[j*4+0]); 
    r[j*V].y = FloatToShort(g[j*4+1]); 
    r[j*V].z = FloatToShort(g[j*4+2]); 
    r[j*V].w = FloatToShort(g[j*4+3]); 
  }
  r[4*V].x = FloatToShort(g[16]); 
  r[4*V].y = FloatToShort(g[17]); 
  r[4*V].z = (short)0;
  r[4*V].w = (short)0;
}

template <typename Float>
inline void pack18(short2 *res, Float *g, int dir, int V) 
{
  short2 *r = res + dir*9*V;
  for (int j=0; j<9; j++) {
    r[j*V].x = FloatToShort(g[j*2+0]); 
    r[j*V].y = FloatToShort(g[j*2+1]); 
  }
}

template<typename Float>
inline void fatlink_short_pack18(short2 *d_gauge, Float *h_gauge, int dir, int V) 
{
  short2 *dg = d_gauge + dir*9*V;
  for (int j=0; j<9; j++) {
    dg[j*V].x = FloatToShort((h_gauge[j*2+0]/fat_link_max)); 
    dg[j*V].y = FloatToShort((h_gauge[j*2+1]/fat_link_max)); 
  }
}



// a += b*c
template <typename Float>
inline void accumulateComplexProduct(Float *a, Float *b, Float *c, Float sign) {
  a[0] += sign*(b[0]*c[0] - b[1]*c[1]);
  a[1] += sign*(b[0]*c[1] + b[1]*c[0]);
}

// a = conj(b)*c
template <typename Float>
inline void complexDotProduct(Float *a, Float *b, Float *c) {
    a[0] = b[0]*c[0] + b[1]*c[1];
    a[1] = b[0]*c[1] - b[1]*c[0];
}

// a += conj(b) * conj(c)
template <typename Float>
inline void accumulateConjugateProduct(Float *a, Float *b, Float *c, int sign) {
  a[0] += sign * (b[0]*c[0] - b[1]*c[1]);
  a[1] -= sign * (b[0]*c[1] + b[1]*c[0]);
}

// a = conj(b)*conj(c)
template <typename Float>
inline void complexConjugateProduct(Float *a, Float *b, Float *c) {
    a[0] = b[0]*c[0] - b[1]*c[1];
    a[1] = -b[0]*c[1] - b[1]*c[0];
}


// Routines used to unpack the gauge field matrices
template <typename Float>
inline void reconstruct8(Float *mat, int dir, int idx) {
  // First reconstruct first row
  Float row_sum = 0.0;
  row_sum += mat[2]*mat[2];
  row_sum += mat[3]*mat[3];
  row_sum += mat[4]*mat[4];
  row_sum += mat[5]*mat[5];
  Float u0 = (dir < 3 ? Anisotropy : (idx >= (X[3]-1)*X[0]*X[1]*X[2]/2 ? tBoundary : 1));
  Float diff = 1.f/(u0*u0) - row_sum;
  Float U00_mag = sqrt(diff >= 0 ? diff : 0.0);

  mat[14] = mat[0];
  mat[15] = mat[1];

  mat[0] = U00_mag * cos(mat[14]);
  mat[1] = U00_mag * sin(mat[14]);

  Float column_sum = 0.0;
  for (int i=0; i<2; i++) column_sum += mat[i]*mat[i];
  for (int i=6; i<8; i++) column_sum += mat[i]*mat[i];
  diff = 1.f/(u0*u0) - column_sum;
  Float U20_mag = sqrt(diff >= 0 ? diff : 0.0);

  mat[12] = U20_mag * cos(mat[15]);
  mat[13] = U20_mag * sin(mat[15]);

  // First column now restored

  // finally reconstruct last elements from SU(2) rotation
  Float r_inv2 = 1.0/(u0*row_sum);

  // U11
  Float A[2];
  complexDotProduct(A, mat+0, mat+6);
  complexConjugateProduct(mat+8, mat+12, mat+4);
  accumulateComplexProduct(mat+8, A, mat+2, u0);
  mat[8] *= -r_inv2;
  mat[9] *= -r_inv2;

  // U12
  complexConjugateProduct(mat+10, mat+12, mat+2);
  accumulateComplexProduct(mat+10, A, mat+4, -u0);
  mat[10] *= r_inv2;
  mat[11] *= r_inv2;

  // U21
  complexDotProduct(A, mat+0, mat+12);
  complexConjugateProduct(mat+14, mat+6, mat+4);
  accumulateComplexProduct(mat+14, A, mat+2, -u0);
  mat[14] *= r_inv2;
  mat[15] *= r_inv2;

  // U12
  complexConjugateProduct(mat+16, mat+6, mat+2);
  accumulateComplexProduct(mat+16, A, mat+4, u0);
  mat[16] *= -r_inv2;
  mat[17] *= -r_inv2;
}

template <typename Float>
inline void unpack8(Float *h_gauge, double2 *d_gauge, int dir, int V, int idx) {
  double2 *dg = d_gauge + dir*4*V;
  for (int j=0; j<4; j++) {
    h_gauge[2*j+0] = dg[j*V].x;
    h_gauge[2*j+1] = dg[j*V].y;
  }
  reconstruct8(h_gauge, dir, idx);
}

template <typename Float>
inline void unpack8(Float *h_gauge, float4 *d_gauge, int dir, int V, int idx) {
  float4 *dg = d_gauge + dir*2*V;
  h_gauge[0] = dg[0].x;
  h_gauge[1] = dg[0].y;
  h_gauge[2] = dg[0].z;
  h_gauge[3] = dg[0].w;
  h_gauge[4] = dg[V].x;
  h_gauge[5] = dg[V].y;
  h_gauge[6] = dg[V].z;
  h_gauge[7] = dg[V].w;
  reconstruct8(h_gauge, dir, idx);
}

template <typename Float>
inline void unpack8(Float *h_gauge, float2 *d_gauge, int dir, int V, int idx) {
  float2 *dg = d_gauge + dir*4*V;
  for (int j=0; j<4; j++) {
    h_gauge[2*j+0] = dg[j*V].x;
    h_gauge[2*j+1] = dg[j*V].y;
  }
  reconstruct8(h_gauge, dir, idx);
}

template <typename Float>
inline void unpack8(Float *h_gauge, short4 *d_gauge, int dir, int V, int idx) {
  short4 *dg = d_gauge + dir*2*V;
  ShortToFloat(h_gauge[0], dg[0].x);
  ShortToFloat(h_gauge[1], dg[0].y);
  ShortToFloat(h_gauge[2], dg[0].z);
  ShortToFloat(h_gauge[3], dg[0].w);
  ShortToFloat(h_gauge[4], dg[V].x);
  ShortToFloat(h_gauge[5], dg[V].y);
  ShortToFloat(h_gauge[6], dg[V].z);
  ShortToFloat(h_gauge[7], dg[V].w);
  h_gauge[0] *= M_PI;
  h_gauge[1] *= M_PI;
  reconstruct8(h_gauge, dir, idx);
}

template <typename Float>
inline void unpack8(Float *h_gauge, short2 *d_gauge, int dir, int V, int idx) {
  short2 *dg = d_gauge + dir*4*V;
  for (int j=0; j<4; j++) {
    ShortToFloat(h_gauge[2*j+0], dg[j*V].x);
    ShortToFloat(h_gauge[2*j+1], dg[j*V].y);
  }
  h_gauge[0] *= M_PI;
  h_gauge[1] *= M_PI;
  reconstruct8(h_gauge, dir, idx);
}

// do this using complex numbers (simplifies)
template <typename Float>
inline void reconstruct12(Float *mat, int dir, int idx) {
  Float *u = &mat[0*(3*2)];
  Float *v = &mat[1*(3*2)];
  Float *w = &mat[2*(3*2)];
  w[0] = 0.0; w[1] = 0.0; w[2] = 0.0; w[3] = 0.0; w[4] = 0.0; w[5] = 0.0;
  accumulateConjugateProduct(w+0*(2), u+1*(2), v+2*(2), +1);
  accumulateConjugateProduct(w+0*(2), u+2*(2), v+1*(2), -1);
  accumulateConjugateProduct(w+1*(2), u+2*(2), v+0*(2), +1);
  accumulateConjugateProduct(w+1*(2), u+0*(2), v+2*(2), -1);
  accumulateConjugateProduct(w+2*(2), u+0*(2), v+1*(2), +1);
  accumulateConjugateProduct(w+2*(2), u+1*(2), v+0*(2), -1);
  Float u0 = (dir < 3 ? Anisotropy :
	      (idx >= (X[3]-1)*X[0]*X[1]*X[2]/2 ? tBoundary : 1));
  w[0]*=u0; w[1]*=u0; w[2]*=u0; w[3]*=u0; w[4]*=u0; w[5]*=u0;
}

template <typename Float>
inline void unpack12(Float *h_gauge, double2 *d_gauge, int dir, int V, int idx) {
  double2 *dg = d_gauge + dir*6*V;
  for (int j=0; j<6; j++) {
    h_gauge[j*2+0] = dg[j*V].x;
    h_gauge[j*2+1] = dg[j*V].y; 
  }
  reconstruct12(h_gauge, dir, idx);
}

template <typename Float>
inline void unpack12(Float *h_gauge, float4 *d_gauge, int dir, int V, int idx) {
  float4 *dg = d_gauge + dir*3*V;
  for (int j=0; j<3; j++) {
    h_gauge[j*4+0] = dg[j*V].x;
    h_gauge[j*4+1] = dg[j*V].y; 
    h_gauge[j*4+2] = dg[j*V].z;
    h_gauge[j*4+3] = dg[j*V].w; 
  }
  reconstruct12(h_gauge, dir, idx);
}

template <typename Float>
inline void unpack12(Float *h_gauge, float2 *d_gauge, int dir, int V, int idx) {
  float2 *dg = d_gauge + dir*6*V;
  for (int j=0; j<6; j++) {
    h_gauge[j*2+0] = dg[j*V].x;
    h_gauge[j*2+1] = dg[j*V].y; 
  }
  reconstruct12(h_gauge, dir, idx);
}

template <typename Float>
inline void unpack12(Float *h_gauge, short4 *d_gauge, int dir, int V, int idx) {
  short4 *dg = d_gauge + dir*3*V;
  for (int j=0; j<3; j++) {
    ShortToFloat(h_gauge[j*4+0], dg[j*V].x);
    ShortToFloat(h_gauge[j*4+1], dg[j*V].y);
    ShortToFloat(h_gauge[j*4+2], dg[j*V].z);
    ShortToFloat(h_gauge[j*4+3], dg[j*V].w);
  }
  reconstruct12(h_gauge, dir, idx);
}

template <typename Float>
inline void unpack12(Float *h_gauge, short2 *d_gauge, int dir, int V, int idx) {
  short2 *dg = d_gauge + dir*6*V;
  for (int j=0; j<6; j++) {
    ShortToFloat(h_gauge[j*2+0], dg[j*V].x);
    ShortToFloat(h_gauge[j*2+1], dg[j*V].y); 
  }
  reconstruct12(h_gauge, dir, idx);
}

template <typename Float>
inline void unpack18(Float *h_gauge, double2 *d_gauge, int dir, int V) {
  double2 *dg = d_gauge + dir*9*V;
  for (int j=0; j<9; j++) {
    h_gauge[j*2+0] = dg[j*V].x; 
    h_gauge[j*2+1] = dg[j*V].y;
  }
}

template <typename Float>
inline void unpack18(Float *h_gauge, float4 *d_gauge, int dir, int V) {
  float4 *dg = d_gauge + dir*5*V;
  for (int j=0; j<4; j++) {
    h_gauge[j*4+0] = dg[j*V].x; 
    h_gauge[j*4+1] = dg[j*V].y;
    h_gauge[j*4+2] = dg[j*V].z; 
    h_gauge[j*4+3] = dg[j*V].w;
  }
  h_gauge[16] = dg[4*V].x; 
  h_gauge[17] = dg[4*V].y;
}

template <typename Float>
inline void unpack18(Float *h_gauge, float2 *d_gauge, int dir, int V) {
  float2 *dg = d_gauge + dir*9*V;
  for (int j=0; j<9; j++) {
    h_gauge[j*2+0] = dg[j*V].x; 
    h_gauge[j*2+1] = dg[j*V].y;
  }
}

template <typename Float>
inline void unpack18(Float *h_gauge, short4 *d_gauge, int dir, int V) {
  short4 *dg = d_gauge + dir*5*V;
  for (int j=0; j<4; j++) {
    ShortToFloat(h_gauge[j*4+0], dg[j*V].x);
    ShortToFloat(h_gauge[j*4+1], dg[j*V].y);
    ShortToFloat(h_gauge[j*4+2], dg[j*V].z);
    ShortToFloat(h_gauge[j*4+3], dg[j*V].w);
  }
  ShortToFloat(h_gauge[16],dg[4*V].x);
  ShortToFloat(h_gauge[17],dg[4*V].y);

}

template <typename Float>
inline void unpack18(Float *h_gauge, short2 *d_gauge, int dir, int V) {
  short2 *dg = d_gauge + dir*9*V;
  for (int j=0; j<9; j++) {
    ShortToFloat(h_gauge[j*2+0], dg[j*V].x); 
    ShortToFloat(h_gauge[j*2+1], dg[j*V].y);
  }
}

// Assume the gauge field is "QDP" ordered: directions outside of
// space-time, row-column ordering, even-odd space-time
template <typename Float, typename FloatN>
static void packQDPGaugeField(FloatN *d_gauge, Float **h_gauge, int oddBit, 
			      ReconstructType reconstruct, int V, int pad, QudaLinkType type) {
  if (reconstruct == QUDA_RECONSTRUCT_12) {
    for (int dir = 0; dir < 4; dir++) {
      Float *g = h_gauge[dir] + oddBit*V*18;
      for (int i = 0; i < V; i++) pack12(d_gauge+i, g+i*18, dir, V+pad);
    }
  } else if (reconstruct == QUDA_RECONSTRUCT_8) {
    for (int dir = 0; dir < 4; dir++) {
      Float *g = h_gauge[dir] + oddBit*V*18;
      for (int i = 0; i < V; i++) pack8(d_gauge+i, g+i*18, dir, V+pad);
    }
  } else {
    for (int dir = 0; dir < 4; dir++) {
      Float *g = h_gauge[dir] + oddBit*V*18;
      if(type == QUDA_ASQTAD_FAT_LINKS && typeid(FloatN) == typeid(short2) ){
	for (int i = 0; i < V; i++) {
	  //we know it is half precison with fatlink at this stage
	  fatlink_short_pack18((short2*)(d_gauge+i), g+i*18, dir, V+pad);
	}
      }else{
	for (int i = 0; i < V; i++) pack18(d_gauge+i, g+i*18, dir, V+pad);
      }
    }
  }
}

template <typename Float, typename FloatN>
static void packQDPGaugeField_mg(FloatN *d_gauge, Float **h_gauge, Float* ghost_gauge, int oddBit, 
				 ReconstructType reconstruct, int V, int pad, int Vsh, int num_faces, 
				 QudaLinkType type) 
{

    
  if (reconstruct == QUDA_RECONSTRUCT_12) {
    for (int dir = 0; dir < 4; dir++) {
      Float *g = h_gauge[dir] + oddBit*V*18;
      for (int i = 0; i < V; i++) pack12(d_gauge+i, g+i*18, dir, V+pad);
    }
    //ghost gauge
    //we only need one direction, let's put it in the dir=3 space
    int dir =3;
    Float* g = ghost_gauge + oddBit*num_faces*Vsh*18;
    for(int i=0; i < num_faces*Vsh; i++){
      pack12(d_gauge+V+i, g + i*18, dir, V+pad);
    } 

  } else if (reconstruct == QUDA_RECONSTRUCT_8) {
    for (int dir = 0; dir < 4; dir++) {
      Float *g = h_gauge[dir] + oddBit*V*18;
      for (int i = 0; i < V; i++) pack8(d_gauge+i, g+i*18, dir, V+pad);
    }
    //ghost gauge
    //we only need one direction, let's put it in the dir=3 space
    int dir =3;
    Float* g = ghost_gauge + oddBit*num_faces*Vsh*18;
    for(int i=0; i < num_faces*Vsh; i++){
      pack8(d_gauge+V+i, g + i*18, dir, V+pad);
    } 

  } else { //18 reconstruct

    for (int dir = 0; dir < 4; dir++) {
      Float *g = h_gauge[dir] + oddBit*V*18;
      if ((type ==  QUDA_ASQTAD_FAT_LINKS) && (typeid(FloatN) == typeid(short2))){
	for (int i = 0; i < V; i++) {
	  //we know it is half precison with fatlink at this stage
	  fatlink_short_pack18((short2*)(d_gauge+i), g+i*18, dir, V+pad);
	}
      }else{
	for (int i = 0; i < V; i++) pack18(d_gauge+i, g+i*18, dir, V+pad);
      }
    }
    //ghost gauge
    //we only need one direction, let's put it in the dir=3 space
    int dir =3;
    Float* g = ghost_gauge + oddBit*num_faces*Vsh*18;
    
    if ((type ==  QUDA_ASQTAD_FAT_LINKS) && (typeid(FloatN) == typeid(short2))){
      for(int i=0; i < num_faces*Vsh; i++){
	  //we know it is half precison with fatlink at this stage
	fatlink_short_pack18((short2*)(d_gauge+V+i), g + i*18, dir, V+pad);
      }           
    }else{
      for(int i=0; i < num_faces*Vsh; i++){
	pack18(d_gauge+V+i, g + i*18, dir, V+pad);
      }     
    }   
  }
}



// Assume the gauge field is "QDP" ordered: directions outside of
// space-time, row-column ordering, even-odd space-time
template <typename Float, typename FloatN>
static void unpackQDPGaugeField(Float **h_gauge, FloatN *d_gauge, int oddBit, 
				ReconstructType reconstruct, int V, int pad) {
  if (reconstruct == QUDA_RECONSTRUCT_12) {
    for (int dir = 0; dir < 4; dir++) {
      Float *g = h_gauge[dir] + oddBit*V*18;
      for (int i = 0; i < V; i++) unpack12(g+i*18, d_gauge+i, dir, V+pad, i);
    }
  } else if (reconstruct == QUDA_RECONSTRUCT_8) {
    for (int dir = 0; dir < 4; dir++) {
      Float *g = h_gauge[dir] + oddBit*V*18;
      for (int i = 0; i < V; i++) unpack8(g+i*18, d_gauge+i, dir, V+pad, i);
    }
  } else {
    for (int dir = 0; dir < 4; dir++) {
      Float *g = h_gauge[dir] + oddBit*V*18;
      for (int i = 0; i < V; i++) unpack18(g+i*18, d_gauge+i, dir, V+pad);
    }
  }
}

// transpose and scale the matrix
template <typename Float, typename Float2>
static void transposeScale(Float *gT, Float *g, const Float2 &a) {
  for (int ic=0; ic<3; ic++) for (int jc=0; jc<3; jc++) for (int r=0; r<2; r++)
    gT[(ic*3+jc)*2+r] = a*g[(jc*3+ic)*2+r];
}

// Assume the gauge field is "Wilson" ordered directions inside of
// space-time column-row ordering even-odd space-time
template <typename Float, typename FloatN>
static void packCPSGaugeField(FloatN *d_gauge, Float *h_gauge, int oddBit, 
			      ReconstructType reconstruct, int V, int pad) {
  Float gT[18];
  if (reconstruct == QUDA_RECONSTRUCT_12) {
    for (int dir = 0; dir < 4; dir++) {
      Float *g = h_gauge + (oddBit*V*4+dir)*18;
      for (int i = 0; i < V; i++) {
	transposeScale(gT, g, 1.0 / Anisotropy);
	pack12(d_gauge+i, gT, dir, V+pad);
      }
    } 
  } else if (reconstruct == QUDA_RECONSTRUCT_8) {
    for (int dir = 0; dir < 4; dir++) {
      Float *g = h_gauge + (oddBit*V*4+dir)*18;
      for (int i = 0; i < V; i++) {
	transposeScale(gT, g, 1.0 / Anisotropy);
	pack8(d_gauge+i, gT, dir, V+pad);
      }
    }
  } else {
    for (int dir = 0; dir < 4; dir++) {
      Float *g = h_gauge + (oddBit*V*4+dir)*18;
      for (int i = 0; i < V; i++) {
	transposeScale(gT, g, 1.0 / Anisotropy);
	pack18(d_gauge+i, gT, dir, V+pad);
      }
    }
  }

}

// Assume the gauge field is "Wilson" ordered directions inside of
// space-time column-row ordering even-odd space-time
template <typename Float, typename FloatN>
static void unpackCPSGaugeField(Float *h_gauge, FloatN *d_gauge, int oddBit, 
				ReconstructType reconstruct, int V, int pad) {
  Float gT[18];
  if (reconstruct == QUDA_RECONSTRUCT_12) {
    for (int dir = 0; dir < 4; dir++) {
      Float *hg = h_gauge + (oddBit*V*4+dir)*18;
      for (int i = 0; i < V; i++) {
	unpack12(gT, d_gauge+i, dir, V+pad, i);
	transposeScale(hg, gT, Anisotropy);
      }
    } 
  } else if (reconstruct == QUDA_RECONSTRUCT_8) {
    for (int dir = 0; dir < 4; dir++) {
      Float *hg = h_gauge + (oddBit*V*4+dir)*18;
      for (int i = 0; i < V; i++) {
	unpack8(gT, d_gauge+i, dir, V+pad, i);
	transposeScale(hg, gT, Anisotropy);
      }
    }
  } else {
    for (int dir = 0; dir < 4; dir++) {
      Float *hg = h_gauge + (oddBit*V*4+dir)*18;
      for (int i = 0; i < V; i++) {
	unpack18(gT, d_gauge+i, dir, V+pad);
	transposeScale(hg, gT, Anisotropy);
      }
    }
  }

}


static void allocateGaugeField(FullGauge *cudaGauge, ReconstructType reconstruct, QudaPrecision precision) {

  cudaGauge->reconstruct = reconstruct;
  cudaGauge->precision = precision;

  cudaGauge->Nc = 3;

  int floatSize;
  if (precision == QUDA_DOUBLE_PRECISION) floatSize = sizeof(double);
  else if (precision == QUDA_SINGLE_PRECISION) floatSize = sizeof(float);
  else floatSize = sizeof(float)/2;

  int elements;
  switch(reconstruct){
  case QUDA_RECONSTRUCT_8:
      elements = 8;
      break;
  case QUDA_RECONSTRUCT_12:
      elements = 12;
      break;
  case QUDA_RECONSTRUCT_NO:
      elements = 18;
      break;
  default:
      errorQuda("Invalid reconstruct value\n");
  }
 
  if (cudaGauge->even || cudaGauge->odd){
    errorQuda("Error: even/odd field is not null, probably already allocated(even=%p, odd=%p)\n", cudaGauge->even, cudaGauge->odd);
  }
 
  cudaGauge->bytes = 4*cudaGauge->stride*elements*floatSize;
  if (!cudaGauge->even) {
    if (cudaMalloc((void **)&cudaGauge->even, cudaGauge->bytes) == cudaErrorMemoryAllocation) {
      errorQuda("Error allocating even gauge field");
    }
  }
   
  if (!cudaGauge->odd) {
    if (cudaMalloc((void **)&cudaGauge->odd, cudaGauge->bytes) == cudaErrorMemoryAllocation) {
      errorQuda("Error allocating even odd gauge field");
    }
  }

}

static void allocateGaugeField2(FullGauge *cudaGauge, ReconstructType reconstruct, QudaPrecision precision) {

  cudaGauge->reconstruct = reconstruct;
  cudaGauge->precision = precision;
  cudaGauge->Nc = 3;
  cudaGauge->bytes = 4*cudaGauge->stride*reconstruct*precision;

  if (!cudaGauge->even) {
    if (cudaMalloc((void **)&cudaGauge->even, cudaGauge->bytes) == cudaErrorMemoryAllocation) {
      errorQuda("Error allocating even gauge field");
    }
  }
  cudaMemset(cudaGauge->even, 0, cudaGauge->bytes);

  if (!cudaGauge->odd) {
    if (cudaMalloc((void **)&cudaGauge->odd, cudaGauge->bytes) == cudaErrorMemoryAllocation) {
      errorQuda("Error allocating even odd gauge field");
    }
  }
  cudaMemset(cudaGauge->odd, 0, cudaGauge->bytes);

  checkCudaError();
}

void freeGaugeField(FullGauge *cudaGauge) {
  if (cudaGauge->even) cudaFree(cudaGauge->even);
  if (cudaGauge->odd) cudaFree(cudaGauge->odd);
  cudaGauge->even = NULL;
  cudaGauge->odd = NULL;
}

template <typename Float, typename FloatN>
static void loadGaugeField(FloatN *even, FloatN *odd, Float *cpuGauge, GaugeFieldOrder gauge_order,
			   ReconstructType reconstruct, int bytes, int Vh, int pad, QudaLinkType type) {
  
  // Use pinned memory
  FloatN *packedEven, *packedOdd;
    
#ifndef __DEVICE_EMULATION__
  cudaMallocHost((void**)&packedEven, bytes);
  cudaMallocHost((void**)&packedOdd, bytes);
#else
  packedEven = (FloatN*)malloc(bytes);
  packedOdd = (FloatN*)malloc(bytes);
#endif
    
  if( ! packedEven ) errorQuda( "packedEven is borked\n");
  if( ! packedOdd ) errorQuda( "packedOdd is borked\n");
  if( ! even ) errorQuda( "even is borked\n");
  if( ! odd ) errorQuda( "odd is borked\n");
  if( ! cpuGauge ) errorQuda( "cpuGauge is borked\n");

  if (gauge_order == QUDA_QDP_GAUGE_ORDER) {
    packQDPGaugeField(packedEven, (Float**)cpuGauge, 0, reconstruct, Vh, pad, type);
    packQDPGaugeField(packedOdd,  (Float**)cpuGauge, 1, reconstruct, Vh, pad, type);
  } else if (gauge_order == QUDA_CPS_WILSON_GAUGE_ORDER) {
    packCPSGaugeField(packedEven, (Float*)cpuGauge, 0, reconstruct, Vh, pad);
    packCPSGaugeField(packedOdd,  (Float*)cpuGauge, 1, reconstruct, Vh, pad);    
  } else {
    errorQuda("Invalid gauge_order");
  }

#ifdef MULTI_GPU
  QudaPrecision precision = (QudaPrecision) sizeof(even->x);
  int veclength = sizeof(FloatN)/precision;

  // assume pad is Vs
  transferGaugeFaces((void *)packedEven, (void *)(packedEven + Vh), precision,
		     veclength, reconstruct, Vh, pad);
  transferGaugeFaces((void *)packedOdd, (void *)(packedOdd + Vh), precision,
		     veclength, reconstruct, Vh, pad);
#endif

  cudaMemcpy(even, packedEven, bytes, cudaMemcpyHostToDevice);
  checkCudaError();
    
  cudaMemcpy(odd,  packedOdd, bytes, cudaMemcpyHostToDevice);
  checkCudaError();
  
#ifndef __DEVICE_EMULATION__
  cudaFreeHost(packedEven);
  cudaFreeHost(packedOdd);
#else
  free(packedEven);
  free(packedOdd);
#endif

}

template <typename Float, typename FloatN>
static void loadGaugeField_mg(FloatN *even, FloatN *odd, Float *cpuGauge, Float* ghost_gauge, GaugeFieldOrder gauge_order,
			      ReconstructType reconstruct, int bytes, int Vh, int pad, int Vsh, int num_faces, QudaLinkType type) 
{

  // Use pinned memory
  FloatN *packedEven, *packedOdd;
    
  cudaMallocHost((void**)&packedEven, bytes);
  cudaMallocHost((void**)&packedOdd, bytes);
    
  if (gauge_order == QUDA_QDP_GAUGE_ORDER) {
    packQDPGaugeField_mg(packedEven, (Float**)cpuGauge, (Float*)ghost_gauge, 0, reconstruct, Vh, pad, Vsh, num_faces, type);
    packQDPGaugeField_mg(packedOdd,  (Float**)cpuGauge, (Float*)ghost_gauge, 1, reconstruct, Vh, pad, Vsh, num_faces, type);
  } else {
    errorQuda("Invalid gauge_order");
  }


  cudaMemcpy(even, packedEven, bytes, cudaMemcpyHostToDevice);
  checkCudaError();
    
  cudaMemcpy(odd,  packedOdd, bytes, cudaMemcpyHostToDevice);
  checkCudaError();
  

  cudaFreeHost(packedEven);
  cudaFreeHost(packedOdd);
}




template <typename Float, typename FloatN>
static void retrieveGaugeField(Float *cpuGauge, FloatN *even, FloatN *odd, GaugeFieldOrder gauge_order,
			       ReconstructType reconstruct, int bytes, int Vh, int pad) {

  // Use pinned memory
  FloatN *packedEven, *packedOdd;
    
#ifndef __DEVICE_EMULATION__
  cudaMallocHost((void**)&packedEven, bytes);
  cudaMallocHost((void**)&packedOdd, bytes);
#else
  packedEven = (FloatN*)malloc(bytes);
  packedOdd = (FloatN*)malloc(bytes);
#endif
    
  cudaMemcpy(packedEven, even, bytes, cudaMemcpyDeviceToHost);
  cudaMemcpy(packedOdd, odd, bytes, cudaMemcpyDeviceToHost);    
    
  if (gauge_order == QUDA_QDP_GAUGE_ORDER) {
    unpackQDPGaugeField((Float**)cpuGauge, packedEven, 0, reconstruct, Vh, pad);
    unpackQDPGaugeField((Float**)cpuGauge, packedOdd, 1, reconstruct, Vh, pad);
  } else if (gauge_order == QUDA_CPS_WILSON_GAUGE_ORDER) {
    unpackCPSGaugeField((Float*)cpuGauge, packedEven, 0, reconstruct, Vh, pad);
    unpackCPSGaugeField((Float*)cpuGauge, packedOdd, 1, reconstruct, Vh, pad);
  } else {
    errorQuda("Invalid gauge_order");
  }
    
#ifndef __DEVICE_EMULATION__
  cudaFreeHost(packedEven);
  cudaFreeHost(packedOdd);
#else
  free(packedEven);
  free(packedOdd);
#endif

}

void createGaugeField(FullGauge *cudaGauge, void *cpuGauge, QudaPrecision cuda_prec, QudaPrecision cpu_prec,
		      GaugeFieldOrder gauge_order, ReconstructType reconstruct, GaugeFixed gauge_fixed,
		      Tboundary t_boundary, int *XX, double anisotropy, double tadpole_coeff, int pad, QudaLinkType type)
{
  if (cpu_prec == QUDA_HALF_PRECISION) {
    errorQuda("Half precision not supported on CPU");
  }

  Anisotropy = anisotropy;
  tBoundary = t_boundary;

  cudaGauge->anisotropy = anisotropy;
  cudaGauge->tadpole_coeff = tadpole_coeff;
  cudaGauge->volume = 1;
  for (int d=0; d<4; d++) {
    cudaGauge->X[d] = XX[d];
    cudaGauge->volume *= XX[d];
    X[d] = XX[d];
  }
  cudaGauge->X[0] /= 2; // actually store the even-odd sublattice dimensions
  cudaGauge->volume /= 2;
  cudaGauge->pad = pad;

#ifdef MULTI_GPU
  if (pad != cudaGauge->X[0]*cudaGauge->X[1]*cudaGauge->X[2]) {
    errorQuda("Gauge padding must match spatial volume");
  }
#endif
  cudaGauge->stride = cudaGauge->volume + cudaGauge->pad;
  cudaGauge->gauge_fixed = gauge_fixed;
  cudaGauge->t_boundary = t_boundary;
  
  allocateGaugeField(cudaGauge, reconstruct, cuda_prec);

  if (cuda_prec == QUDA_DOUBLE_PRECISION) {

    if (cpu_prec == QUDA_DOUBLE_PRECISION) {
      loadGaugeField((double2*)(cudaGauge->even), (double2*)(cudaGauge->odd), (double*)cpuGauge, 
		     gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, type);
    } else if (cpu_prec == QUDA_SINGLE_PRECISION) {
      loadGaugeField((double2*)(cudaGauge->even), (double2*)(cudaGauge->odd), (float*)cpuGauge, 
		     gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, type);
    }

  } else if (cuda_prec == QUDA_SINGLE_PRECISION) {

    if (cpu_prec == QUDA_DOUBLE_PRECISION) {
      if (reconstruct == QUDA_RECONSTRUCT_NO) {
	loadGaugeField((float2*)(cudaGauge->even), (float2*)(cudaGauge->odd), (double*)cpuGauge, 
		       gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, type);	      
      } else {
	loadGaugeField((float4*)(cudaGauge->even), (float4*)(cudaGauge->odd), (double*)cpuGauge, 
		       gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, type);
      }
    } else if (cpu_prec == QUDA_SINGLE_PRECISION) {
      if (reconstruct == QUDA_RECONSTRUCT_NO) {
	loadGaugeField((float2*)(cudaGauge->even), (float2*)(cudaGauge->odd), (float*)cpuGauge, 
		       gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, type);
      } else {
	loadGaugeField((float4*)(cudaGauge->even), (float4*)(cudaGauge->odd), (float*)cpuGauge, 
		       gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, type);
      }
    }

  } else if (cuda_prec == QUDA_HALF_PRECISION) {

    if (cpu_prec == QUDA_DOUBLE_PRECISION){
      if (reconstruct == QUDA_RECONSTRUCT_NO) {
	loadGaugeField((short2*)(cudaGauge->even), (short2*)(cudaGauge->odd), (double*)cpuGauge, 
		       gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, type);
      } else {
	loadGaugeField((short4*)(cudaGauge->even), (short4*)(cudaGauge->odd), (double*)cpuGauge, 
		       gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, type);	      
      }
    } else if (cpu_prec == QUDA_SINGLE_PRECISION) {
      if (reconstruct == QUDA_RECONSTRUCT_NO) {
	loadGaugeField((short2*)(cudaGauge->even), (short2*)(cudaGauge->odd), (float*)cpuGauge, 
		       gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, type);
      } else {
	loadGaugeField((short4*)(cudaGauge->even), (short4*)(cudaGauge->odd), (float*)cpuGauge, 
		       gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, type);	      
      }
    }

  }
}

void createGaugeField_mg(FullGauge *cudaGauge, void *cpuGauge, void* ghost_gauge, QudaPrecision cuda_prec, QudaPrecision cpu_prec,
			 GaugeFieldOrder gauge_order, ReconstructType reconstruct, GaugeFixed gauge_fixed,
			 Tboundary t_boundary, int *XX, double anisotropy, double tadpole_coeff, int pad, int num_faces, QudaLinkType type)
{

  Anisotropy = anisotropy;
  tBoundary = t_boundary;
  
  cudaGauge->anisotropy = anisotropy;
  cudaGauge->tadpole_coeff = tadpole_coeff;
  cudaGauge->volume = 1;
  for (int d=0; d<4; d++) {
    cudaGauge->X[d] = XX[d];
    cudaGauge->volume *= XX[d];
    X[d] = XX[d];
  }
  cudaGauge->X[0] /= 2; // actually store the even-odd sublattice dimensions
  cudaGauge->volume /= 2;
  cudaGauge->pad = pad;
  cudaGauge->stride = cudaGauge->volume + cudaGauge->pad;
  cudaGauge->gauge_fixed = gauge_fixed;
  cudaGauge->t_boundary = t_boundary;

  allocateGaugeField(cudaGauge, reconstruct, cuda_prec);

  int Vsh = XX[0]*XX[1]*XX[2]/2;
  if (cuda_prec == QUDA_DOUBLE_PRECISION) {

    if (cpu_prec == QUDA_DOUBLE_PRECISION)
      loadGaugeField_mg((double2*)(cudaGauge->even), (double2*)(cudaGauge->odd), (double*)cpuGauge,(double*)ghost_gauge,  
			gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, Vsh, num_faces, type);
    else if (cpu_prec == QUDA_SINGLE_PRECISION)
      loadGaugeField_mg((double2*)(cudaGauge->even), (double2*)(cudaGauge->odd), (float*)cpuGauge, (float*)ghost_gauge, 
			gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, Vsh, num_faces, type);

  } else if (cuda_prec == QUDA_SINGLE_PRECISION) {

      if (cpu_prec == QUDA_DOUBLE_PRECISION){
	  if (reconstruct == QUDA_RECONSTRUCT_NO){
	    loadGaugeField_mg((float2*)(cudaGauge->even), (float2*)(cudaGauge->odd), (double*)cpuGauge, (double*)ghost_gauge,
			      gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, Vsh, num_faces, type);	      
	  }else{
	    loadGaugeField_mg((float4*)(cudaGauge->even), (float4*)(cudaGauge->odd), (double*)cpuGauge, (double*)ghost_gauge,
			      gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, Vsh, num_faces, type);
	  }
	  
      }
      else if (cpu_prec == QUDA_SINGLE_PRECISION){
	if (reconstruct == QUDA_RECONSTRUCT_NO){
	  loadGaugeField_mg((float2*)(cudaGauge->even), (float2*)(cudaGauge->odd), (float*)cpuGauge, (float*)ghost_gauge,
			    gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, Vsh, num_faces, type);
	}else{
	  loadGaugeField_mg((float4*)(cudaGauge->even), (float4*)(cudaGauge->odd), (float*)cpuGauge, (float*)ghost_gauge, 
			    gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, Vsh, num_faces, type);
	}
      }
      
  } else if (cuda_prec == QUDA_HALF_PRECISION) {
    if (cpu_prec == QUDA_DOUBLE_PRECISION){
      if (reconstruct == QUDA_RECONSTRUCT_NO){	  
	loadGaugeField_mg((short2*)(cudaGauge->even), (short2*)(cudaGauge->odd), (double*)cpuGauge, (double*)ghost_gauge, 
			  gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, Vsh, num_faces, type);
	
      }else{
	loadGaugeField_mg((short4*)(cudaGauge->even), (short4*)(cudaGauge->odd), (double*)cpuGauge, (double*)ghost_gauge,
			  gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, Vsh, num_faces, type);	      
      }
      }
    else if (cpu_prec == QUDA_SINGLE_PRECISION){
      if (reconstruct == QUDA_RECONSTRUCT_NO){	  
	loadGaugeField_mg((short2*)(cudaGauge->even), (short2*)(cudaGauge->odd), (float*)cpuGauge, (float*)ghost_gauge,
			  gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, Vsh, num_faces, type);
      }else{
	loadGaugeField_mg((short4*)(cudaGauge->even), (short4*)(cudaGauge->odd), (float*)cpuGauge, (float*)ghost_gauge,
			  gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, pad, Vsh, num_faces, type);
      }
    }
  }
}



void restoreGaugeField(void *cpuGauge, FullGauge *cudaGauge, QudaPrecision cpu_prec, GaugeFieldOrder gauge_order)
{
  if (cpu_prec == QUDA_HALF_PRECISION) {
    errorQuda("Half precision not supported on CPU");
  }

  if (cudaGauge->precision == QUDA_DOUBLE_PRECISION) {

    if (cpu_prec == QUDA_DOUBLE_PRECISION) {
      retrieveGaugeField((double*)cpuGauge, (double2*)(cudaGauge->even), (double2*)(cudaGauge->odd), 
			 gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, cudaGauge->pad);
    } else if (cpu_prec == QUDA_SINGLE_PRECISION) {
      retrieveGaugeField((float*)cpuGauge, (double2*)(cudaGauge->even), (double2*)(cudaGauge->odd), 
			 gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, cudaGauge->pad);
    }

  } else if (cudaGauge->precision == QUDA_SINGLE_PRECISION) {

    if (cpu_prec == QUDA_DOUBLE_PRECISION) {
      if (cudaGauge->reconstruct == QUDA_RECONSTRUCT_NO) {
	retrieveGaugeField((double*)cpuGauge, (float2*)(cudaGauge->even), (float2*)(cudaGauge->odd), 
			   gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, cudaGauge->pad);
      } else {
	retrieveGaugeField((double*)cpuGauge, (float4*)(cudaGauge->even), (float4*)(cudaGauge->odd), 
			   gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, cudaGauge->pad);
      }
    } else if (cpu_prec == QUDA_SINGLE_PRECISION) {
      if (cudaGauge->reconstruct == QUDA_RECONSTRUCT_NO) {
	retrieveGaugeField((float*)cpuGauge, (float2*)(cudaGauge->even), (float2*)(cudaGauge->odd), 
			   gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, cudaGauge->pad);
      } else {
	retrieveGaugeField((float*)cpuGauge, (float4*)(cudaGauge->even), (float4*)(cudaGauge->odd), 
			   gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, cudaGauge->pad);
      }
    }

  } else if (cudaGauge->precision == QUDA_HALF_PRECISION) {

    if (cpu_prec == QUDA_DOUBLE_PRECISION) {
      if (cudaGauge->reconstruct == QUDA_RECONSTRUCT_NO) {
	retrieveGaugeField((double*)cpuGauge, (short2*)(cudaGauge->even), (short2*)(cudaGauge->odd), 
			   gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, cudaGauge->pad);
      } else {
	retrieveGaugeField((double*)cpuGauge, (short4*)(cudaGauge->even), (short4*)(cudaGauge->odd), 
			   gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, cudaGauge->pad);
      }
    } else if (cpu_prec == QUDA_SINGLE_PRECISION) {
      if (cudaGauge->reconstruct == QUDA_RECONSTRUCT_NO) {
	retrieveGaugeField((float*)cpuGauge, (short2*)(cudaGauge->even), (short2*)(cudaGauge->odd), 
			   gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, cudaGauge->pad);
      } else {
	retrieveGaugeField((float*)cpuGauge, (short4*)(cudaGauge->even), (short4*)(cudaGauge->odd), 
			   gauge_order, cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume, cudaGauge->pad);
      }
    }

  }

}


/********************** Staple code, used by link fattening **************/

#ifdef GPU_FATLINK

static void 
allocateStapleQuda(FullStaple *cudaStaple, QudaPrecision precision) 
{
    cudaStaple->precision = precision;    
    cudaStaple->Nc = 3;
    
    int floatSize;
    if (precision == QUDA_DOUBLE_PRECISION) {
	floatSize = sizeof(double);
    }
    else if (precision == QUDA_SINGLE_PRECISION) {
	floatSize = sizeof(float);
    }else{
	printf("ERROR: stape does not support half precision\n");
	exit(1);
    }
    
    int elements = 18;
    
    cudaStaple->bytes = cudaStaple->stride*elements*floatSize;
    
    cudaMalloc((void **)&cudaStaple->even, cudaStaple->bytes);
    cudaMalloc((void **)&cudaStaple->odd, cudaStaple->bytes); 	    
}

void
createStapleQuda(FullStaple* cudaStaple, QudaGaugeParam* param)
{
    QudaPrecision cpu_prec = param->cpu_prec;
    QudaPrecision cuda_prec= param->cuda_prec;
    
    if (cpu_prec == QUDA_HALF_PRECISION) {
	printf("ERROR: %s:  half precision not supported on cpu\n", __FUNCTION__);
	exit(-1);
    }
    
    if (cuda_prec == QUDA_DOUBLE_PRECISION && param->cpu_prec != QUDA_DOUBLE_PRECISION) {
	printf("Error: can only create a double GPU gauge field from a double CPU gauge field\n");
	exit(-1);
    }
    
    cudaStaple->volume = 1;
    for (int d=0; d<4; d++) {
	cudaStaple->X[d] = param->X[d];
	cudaStaple->volume *= param->X[d];
    }
    cudaStaple->X[0] /= 2; // actually store the even-odd sublattice dimensions
    cudaStaple->volume /= 2;    
    cudaStaple->pad = param->staple_pad;
    cudaStaple->stride = cudaStaple->volume + cudaStaple->pad;
    allocateStapleQuda(cudaStaple,  param->cuda_prec);
    
    return;
}


void
freeStapleQuda(FullStaple *cudaStaple) 
{
    if (cudaStaple->even) {
	cudaFree(cudaStaple->even);
    }
    if (cudaStaple->odd) {
	cudaFree(cudaStaple->odd);
    }
    cudaStaple->even = NULL;
    cudaStaple->odd = NULL;
}
void
packGhostStaple(FullStaple* cudaStaple, void* fwd_nbr_buf, void* back_nbr_buf,
		void* f_norm_buf, void* b_norm_buf, cudaStream_t* stream)
{
  //FIXME: ignore half precision for now
  void* even = cudaStaple->even;
  void* odd = cudaStaple->odd;
  int Vh = cudaStaple->volume;
  int Vsh = cudaStaple->X[0]*cudaStaple->X[1]*cudaStaple->X[2];
  int prec= cudaStaple->precision;
  int sizeOfFloatN = 2*prec;
  int len = Vsh*sizeOfFloatN;
  int i;


  if(cudaStaple->X[3] %2 == 0){
  //back,even
  for(i=0;i < 9; i++){
    void* dst = ((char*)back_nbr_buf) + i*len ; 
    void* src = ((char*)even) + i*cudaStaple->stride*sizeOfFloatN;
    cudaMemcpyAsync(dst, src, len, cudaMemcpyDeviceToHost, *stream); CUERR;
  }
  //back, odd
  for(i=0;i < 9; i++){
    void* dst = ((char*)back_nbr_buf) + 9*len + i*len ; 
    void* src = ((char*)odd) + i*cudaStaple->stride*sizeOfFloatN;
    cudaMemcpyAsync(dst, src, len, cudaMemcpyDeviceToHost, *stream); CUERR;
  }

  //fwd,even
  for(i=0;i < 9; i++){
    void* dst = ((char*)fwd_nbr_buf) + i*len ; 
    void* src = ((char*)even) + (Vh-Vsh)*sizeOfFloatN + i*cudaStaple->stride*sizeOfFloatN;
    cudaMemcpyAsync(dst, src, len, cudaMemcpyDeviceToHost, *stream); CUERR;
  }
  //fwd, odd
  for(i=0;i < 9; i++){
    void* dst = ((char*)fwd_nbr_buf) + 9*len + i*len ; 
    void* src = ((char*)odd) + (Vh-Vsh)*sizeOfFloatN + i*cudaStaple->stride*sizeOfFloatN;
    cudaMemcpyAsync(dst, src, len, cudaMemcpyDeviceToHost, *stream); CUERR;
  }
 }else{
   //reverse even and odd position
  //back,odd
  for(i=0;i < 9; i++){
    void* dst = ((char*)back_nbr_buf) + i*len ; 
    void* src = ((char*)odd) + i*cudaStaple->stride*sizeOfFloatN;
    cudaMemcpyAsync(dst, src, len, cudaMemcpyDeviceToHost, *stream); CUERR;
  }
  //back, even
  for(i=0;i < 9; i++){
    void* dst = ((char*)back_nbr_buf) + 9*len + i*len ; 
    void* src = ((char*)even) + i*cudaStaple->stride*sizeOfFloatN;
    cudaMemcpyAsync(dst, src, len, cudaMemcpyDeviceToHost, *stream); CUERR;
  }

  //fwd,odd
  for(i=0;i < 9; i++){
    void* dst = ((char*)fwd_nbr_buf) + i*len ; 
    void* src = ((char*)odd) + (Vh-Vsh)*sizeOfFloatN + i*cudaStaple->stride*sizeOfFloatN;
    cudaMemcpyAsync(dst, src, len, cudaMemcpyDeviceToHost, *stream); CUERR;
  }
  //fwd, even
  for(i=0;i < 9; i++){
    void* dst = ((char*)fwd_nbr_buf) + 9*len + i*len ; 
    void* src = ((char*)even) + (Vh-Vsh)*sizeOfFloatN + i*cudaStaple->stride*sizeOfFloatN;
    cudaMemcpyAsync(dst, src, len, cudaMemcpyDeviceToHost, *stream); CUERR;
  }

 } 
  
  
}


void 
unpackGhostStaple(FullStaple* cudaStaple, void* fwd_nbr_buf, void* back_nbr_buf,
		  void* f_norm_buf, void* b_norm_buf, cudaStream_t* stream)
{
  //FIXME: ignore half precision for now  
  void* even = cudaStaple->even;
  void* odd = cudaStaple->odd;
  int Vh = cudaStaple->volume;
  int Vsh = cudaStaple->X[0]*cudaStaple->X[1]*cudaStaple->X[2];
  int prec= cudaStaple->precision;
  int sizeOfFloatN = 2*prec;
  int len = Vsh*sizeOfFloatN;
  int i;

  //back,even
  for(i=0;i < 9; i++){
    void* dst = ((char*)even) + Vh*sizeOfFloatN + i*cudaStaple->stride*sizeOfFloatN;
    void* src = ((char*)back_nbr_buf) + i*len ; 
    cudaMemcpyAsync(dst, src, len, cudaMemcpyHostToDevice, *stream); CUERR;
  }
  //back, odd
  for(i=0;i < 9; i++){
    void* dst = ((char*)odd) + Vh*sizeOfFloatN + i*cudaStaple->stride*sizeOfFloatN;
    void* src = ((char*)back_nbr_buf) + 9*len + i*len ; 
    cudaMemcpyAsync(dst, src, len, cudaMemcpyHostToDevice, *stream); CUERR;
  }
  
  //fwd,even
  for(i=0;i < 9; i++){
    void* dst = ((char*)even) + (Vh+Vsh)*sizeOfFloatN + i*cudaStaple->stride*sizeOfFloatN;
    void* src = ((char*)fwd_nbr_buf) + i*len ; 
    cudaMemcpyAsync(dst, src, len, cudaMemcpyHostToDevice, *stream); CUERR;
  }
  //fwd, odd
  for(i=0;i < 9; i++){
    void* dst = ((char*)odd) + (Vh+Vsh)*sizeOfFloatN + i*cudaStaple->stride*sizeOfFloatN;
    void* src = ((char*)fwd_nbr_buf) + 9*len + i*len ; 
    cudaMemcpyAsync(dst, src, len, cudaMemcpyHostToDevice, *stream); CUERR;
  }
  
}
#endif


/******************************** Mom code, used by Fermi force code ****************************/

#ifdef GPU_FERMION_FORCE
static void 
allocateMomQuda(FullMom *cudaMom, QudaPrecision precision) 
{
    cudaMom->precision = precision;    
    
    int floatSize;
    if (precision == QUDA_DOUBLE_PRECISION) {
	floatSize = sizeof(double);
    }
    else if (precision == QUDA_SINGLE_PRECISION) {
	floatSize = sizeof(float);
    }else{
	printf("ERROR: stape does not support half precision\n");
	exit(1);
    }
    
    int elements = 10;
     
    cudaMom->bytes = cudaMom->volume*elements*floatSize*4;
    
    cudaMalloc((void **)&cudaMom->even, cudaMom->bytes);
    cudaMalloc((void **)&cudaMom->odd, cudaMom->bytes); 	    
}

void
createMomQuda(FullMom* cudaMom, QudaGaugeParam* param)
{
    QudaPrecision cpu_prec = param->cpu_prec;
    QudaPrecision cuda_prec= param->cuda_prec;
    
    if (cpu_prec == QUDA_HALF_PRECISION) {
	printf("ERROR: %s:  half precision not supported on cpu\n", __FUNCTION__);
	exit(-1);
    }
    
    if (cuda_prec == QUDA_DOUBLE_PRECISION && param->cpu_prec != QUDA_DOUBLE_PRECISION) {
	printf("Error: can only create a double GPU gauge field from a double CPU gauge field\n");
	exit(-1);
    }
    
    cudaMom->volume = 1;
    for (int d=0; d<4; d++) {
	cudaMom->X[d] = param->X[d];
	cudaMom->volume *= param->X[d];
    }
    cudaMom->X[0] /= 2; // actually store the even-odd sublattice dimensions
    cudaMom->volume /= 2;    
    
    allocateMomQuda(cudaMom,  param->cuda_prec);
    
    return;
}


void
freeMomQuda(FullMom *cudaMom) 
{
    if (cudaMom->even) {
	cudaFree(cudaMom->even);
    }
    if (cudaMom->odd) {
	cudaFree(cudaMom->odd);
    }
    cudaMom->even = NULL;
    cudaMom->odd = NULL;
}

template <typename Float, typename Float2>
inline void pack10(Float2 *res, Float *m, int dir, int Vh) 
{
    Float2 *r = res + dir*5*Vh;
    for (int j=0; j<5; j++) {
	r[j*Vh].x = (float)m[j*2+0]; 
	r[j*Vh].y = (float)m[j*2+1]; 
    }
}

template <typename Float, typename Float2>
void packMomField(Float2 *res, Float *mom, int oddBit, int Vh) 
{    
    for (int dir = 0; dir < 4; dir++) {
	Float *g = mom + (oddBit*Vh*4 + dir)*momSiteSize;
	for (int i = 0; i < Vh; i++) {
	    pack10(res+i, g + 4*i*momSiteSize, dir, Vh);
	}
    }      
}

template <typename Float, typename Float2>
void loadMomField(Float2 *even, Float2 *odd, Float *mom,
		  int bytes, int Vh) 
{
    
    Float2 *packedEven, *packedOdd;
    cudaMallocHost((void**)&packedEven, bytes); 
    cudaMallocHost((void**)&packedOdd, bytes);
    
    packMomField(packedEven, (Float*)mom, 0, Vh);
    packMomField(packedOdd,  (Float*)mom, 1, Vh);
    
    cudaMemcpy(even, packedEven, bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(odd,  packedOdd, bytes, cudaMemcpyHostToDevice); 
    
    cudaFreeHost(packedEven);
    cudaFreeHost(packedOdd);
}




void
loadMomToGPU(FullMom cudaMom, void* mom, QudaGaugeParam* param)
{
    if (param->cuda_prec == QUDA_DOUBLE_PRECISION) {
	//loadGaugeField((double2*)(cudaGauge->even), (double2*)(cudaGauge->odd), (double*)cpuGauge, 
	//cudaGauge->reconstruct, cudaGauge->bytes, cudaGauge->volume);
    } else { //single precision
	loadMomField((float2*)(cudaMom.even), (float2*)(cudaMom.odd), (float*)mom, 
		     cudaMom.bytes, cudaMom.volume);
	
    }
}


template <typename Float, typename Float2>
inline void unpack10(Float* m, Float2 *res, int dir, int Vh) 
{
    Float2 *r = res + dir*5*Vh;
    for (int j=0; j<5; j++) {
	m[j*2+0] = r[j*Vh].x;
	m[j*2+1] = r[j*Vh].y;
    }
    
}

template <typename Float, typename Float2>
void 
unpackMomField(Float* mom, Float2 *res, int oddBit, int Vh) 
{
    int dir, i;
    Float *m = mom + oddBit*Vh*momSiteSize*4;
    
    for (i = 0; i < Vh; i++) {
	for (dir = 0; dir < 4; dir++) {	
	    Float* thismom = m + (4*i+dir)*momSiteSize;
	    unpack10(thismom, res+i, dir, Vh);
	}
    }
}

template <typename Float, typename Float2>
void 
storeMomToCPUArray(Float* mom, Float2 *even, Float2 *odd, 
		   int bytes, int Vh) 
{    
    Float2 *packedEven, *packedOdd;   
    cudaMallocHost((void**)&packedEven, bytes); 
    cudaMallocHost((void**)&packedOdd, bytes); 
    cudaMemcpy(packedEven, even, bytes, cudaMemcpyDeviceToHost); 
    cudaMemcpy(packedOdd, odd, bytes, cudaMemcpyDeviceToHost);  

    unpackMomField((Float*)mom, packedEven,0, Vh);
    unpackMomField((Float*)mom, packedOdd, 1, Vh);
        
    cudaFreeHost(packedEven); 
    cudaFreeHost(packedOdd); 
}

void 
storeMomToCPU(void* mom, FullMom cudaMom, QudaGaugeParam* param)
{
    QudaPrecision cpu_prec = param->cpu_prec;
    QudaPrecision cuda_prec= param->cuda_prec;
    
    if (cpu_prec != cuda_prec){
	printf("Error:%s: cpu and gpu precison has to be the same at this moment \n", __FUNCTION__);
	exit(1);	
    }
    
    if (cpu_prec == QUDA_HALF_PRECISION){
	printf("ERROR: %s:  half precision is not supported at this moment\n", __FUNCTION__);
	exit(1);
    }
    
    if (cpu_prec == QUDA_DOUBLE_PRECISION){
	
    }else { //SINGLE PRECISIONS
	storeMomToCPUArray( (float*)mom, (float2*) cudaMom.even, (float2*)cudaMom.odd, 
			    cudaMom.bytes, cudaMom.volume);	
    }
    
}
#endif

/********** link code, used by link fattening  **********************/

#ifdef GPU_FATLINK

void
createLinkQuda(FullGauge* cudaGauge, QudaGaugeParam* param)
{
    QudaPrecision cpu_prec = param->cpu_prec;
    QudaPrecision cuda_prec= param->cuda_prec;
    
    if (cpu_prec == QUDA_HALF_PRECISION) {
	printf("ERROR: %s:  half precision not supported on cpu\n", __FUNCTION__);
	exit(-1);
    }
    
    if (cuda_prec == QUDA_DOUBLE_PRECISION && param->cpu_prec != QUDA_DOUBLE_PRECISION) {
	printf("Error: can only create a double GPU gauge field from a double CPU gauge field\n");
	exit(-1);
    }
        
    cudaGauge->anisotropy = param->anisotropy;
    cudaGauge->volume = 1;
    for (int d=0; d<4; d++) {
	cudaGauge->X[d] = param->X[d];
	cudaGauge->volume *= param->X[d];
    }
    cudaGauge->X[0] /= 2; // actually store the even-odd sublattice dimensions
    cudaGauge->volume /= 2;    
    cudaGauge->pad = param->ga_pad;
    cudaGauge->stride = cudaGauge->volume + cudaGauge->pad;
    cudaGauge->reconstruct = param->reconstruct;

    allocateGaugeField(cudaGauge, param->reconstruct, param->cuda_prec);
    
    return;
}


template<typename FloatN, typename Float>
static void 
do_loadLinkToGPU(FloatN *even, FloatN *odd, Float **cpuGauge, Float* ghost_cpuGauge,
		 ReconstructType reconstruct, int bytes, int Vh, int pad, int Vsh,
		 QudaPrecision prec) 
{
  // Use pinned memory
  int i;
  char* tmp;
  int len = Vh*gaugeSiteSize*sizeof(Float);

#ifdef MULTI_GPU  
  int glen = Vsh*gaugeSiteSize*sizeof(Float);
#else
  int glen = 0;
#endif  
  cudaMalloc(&tmp, 4*(len+2*glen)); CUERR;

  //even links
  for(i=0;i < 4; i++){
      cudaMemcpy(tmp + i*(len+2*glen), cpuGauge[i], len, cudaMemcpyHostToDevice); 
#ifdef MULTI_GPU  
      cudaMemcpy(tmp + i*(len+2*glen)+len, ((char*)ghost_cpuGauge)+i*2*glen, glen, cudaMemcpyHostToDevice); 
      cudaMemcpy(tmp + i*(len+2*glen)+len + glen, ((char*)ghost_cpuGauge)+8*glen+i*2*glen, glen, cudaMemcpyHostToDevice); 
#endif
  }    
  
  link_format_cpu_to_gpu((void*)even, (void*)tmp,  reconstruct, bytes, Vh, pad, Vsh, prec); CUERR;
  
  //odd links
  for(i=0;i < 4; i++){
      cudaMemcpy(tmp + i*(len+2*glen), cpuGauge[i] + Vh*gaugeSiteSize, len, cudaMemcpyHostToDevice);CUERR;
#ifdef MULTI_GPU  
      cudaMemcpy(tmp + i*(len+2*glen)+len, ((char*)ghost_cpuGauge)+glen +i*2*glen, glen, cudaMemcpyHostToDevice); CUERR;
      cudaMemcpy(tmp + i*(len+2*glen)+len + glen, ((char*)ghost_cpuGauge)+8*glen+glen +i*2*glen, glen, cudaMemcpyHostToDevice); CUERR;
#endif
  }
  link_format_cpu_to_gpu((void*)odd, (void*)tmp, reconstruct, bytes, Vh, pad, Vsh, prec); CUERR;
  
  cudaFree(tmp);
  CUERR;
}



void 
loadLinkToGPU(FullGauge cudaGauge, void **cpuGauge, void* ghost_cpuGauge, QudaGaugeParam* param)
{
  QudaPrecision cpu_prec = param->cpu_prec;
  QudaPrecision cuda_prec= param->cuda_prec;
  int pad = param->ga_pad;
  int Vsh = param->X[0]*param->X[1]*param->X[2]/2;
  
  
  if (cpu_prec  != cuda_prec){
    printf("ERROR: cpu precision and cuda precision must be the same in this function %s\n", __FUNCTION__);
    exit(1);
  }

  if (cuda_prec == QUDA_DOUBLE_PRECISION) {
    do_loadLinkToGPU((double2*)(cudaGauge.even), (double2*)(cudaGauge.odd), (double**)cpuGauge, 
		     (double*)ghost_cpuGauge, cudaGauge.reconstruct, cudaGauge.bytes, cudaGauge.volume, pad, Vsh, 
		     cuda_prec);
  } else if (cuda_prec == QUDA_SINGLE_PRECISION) {
    do_loadLinkToGPU((float2*)(cudaGauge.even), (float2*)(cudaGauge.odd), (float**)cpuGauge, 
		     (float*)ghost_cpuGauge, cudaGauge.reconstruct, cudaGauge.bytes, cudaGauge.volume, pad, Vsh, 
		     cuda_prec);
  }else{
    printf("ERROR: half precision not supported in this funciton %s\n", __FUNCTION__);
    exit(1);
  }
}


template<typename FloatN, typename Float>
static void 
do_storeLinkToCPU(Float* cpuGauge, FloatN *even, FloatN *odd, 
		  int bytes, int Vh, int stride, QudaPrecision prec) 
{  
  double* unpackedData;
  int datalen = 4*Vh*gaugeSiteSize*sizeof(Float);
  cudaMalloc(&unpackedData, datalen); CUERR;
  
  //unpack even data kernel
  link_format_gpu_to_cpu((void*)unpackedData, (void*)even, bytes, Vh, stride, prec);
  cudaMemcpy(cpuGauge, unpackedData, datalen, cudaMemcpyDeviceToHost);
  
  //unpack odd data kernel
  link_format_gpu_to_cpu((void*)unpackedData, (void*)odd,  bytes, Vh, stride, prec);
  cudaMemcpy(cpuGauge + 4*Vh*gaugeSiteSize, unpackedData, datalen, cudaMemcpyDeviceToHost);  
  
  cudaFree(unpackedData);

  CUERR;
}



void 
storeLinkToCPU(void* cpuGauge, FullGauge *cudaGauge, QudaGaugeParam* param)
{
  
  QudaPrecision cpu_prec = param->cpu_prec;
  QudaPrecision cuda_prec= param->cuda_prec;

  if (cpu_prec  != cuda_prec){
    printf("ERROR: cpu precision and cuda precision must be the same in this function %s\n", __FUNCTION__);
    exit(1);
  }
    
  if (cudaGauge->reconstruct != QUDA_RECONSTRUCT_NO){
    printf("ERROR: it makes no sense to get data back to cpu for 8/12 reconstruct, function %s\n", __FUNCTION__);
    exit(1);
  }
  
  int stride = cudaGauge->volume + param->ga_pad;
  
  if (cuda_prec == QUDA_DOUBLE_PRECISION){
    do_storeLinkToCPU( (double*)cpuGauge, (double2*) cudaGauge->even, (double2*)cudaGauge->odd, 
		       cudaGauge->bytes, cudaGauge->volume, stride, cuda_prec);
  }else if (cuda_prec == QUDA_SINGLE_PRECISION){
    do_storeLinkToCPU( (float*)cpuGauge, (float2*) cudaGauge->even, (float2*)cudaGauge->odd, 
		       cudaGauge->bytes, cudaGauge->volume, stride, cuda_prec);
  }else{
    printf("ERROR: half precision not supported in this function %s\n", __FUNCTION__);
    exit(1);
  }
}


#endif

