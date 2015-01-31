#include <math.h>

#define __USE_C99_MATH
#include <stdbool.h>
#include "blas_like.h"
#include "blas_util.h"
#include <stdio.h>
#include <assert.h>

#define SCALAR double

void _dgzero(int n, double *a) {
  int i;
  for (i=0; i<n; ++i) assert(a[i] > 1E-10);
}

// a(:,*) = a(:,*) .+ b
void _dgcmv(int m, int n, double *a, double *b) {
  int i,j;
  double *pa, *pb;
  for (i=0,pa=a; i<n; ++i)
    for (j=0,pb=b; j<m; ++j, ++pa, ++pb)
      *pa += *pb;
}

// a(*,:) = a(*,:) .+ b
void _dgrmv(int m, int n, double *a, double *b) {
  int i,j;
  double *pa =a, *pb =b;
  for (i=0; i<n; ++i,++pb)
    for (j=0; j<m; ++j, ++pa)
      *pa += *pb;
}

// a = diag(b) * a
void _dgcms(int m, int n, double *a, double *b) {
  int i,j;
  double *pa = a, *pb;
  for (i=0; i<n; ++i)
    for (j=0, pb=b; j<m; ++j, ++pa, ++pb)
      *pa *= *pb;
}

// a = a * diag(b) 
void _dgrms(int m, int n, double *a, double *b) {
  int i,j;
  double *pa = a, *pb = b;
  for (i=0; i<n; ++i,++pb)
    for (j=0; j<m; ++j, ++pa)
      *pa *= *pb;
}

// a = diag(1./b) * a
void _dicms(int m, int n, double *a, double *b) {
  int i,j;
  double *pa, *pb;
  for (j=0; j<m; ++j) assert(b[j] > 0);
  for (i=0,pa=a; i<n; ++i)
    for (j=0,pb=b; j<m; ++j, ++pa, ++pb)
      *pa /= *pb;
}

// a = a * diag(1./b) 
void _dirms(int m, int n, double *a, double *b) {
  int i,j;
  double *pa, *pb;
  for (i=0; i<n; ++i) assert(b[i] > 0);
  for (i=0,pa=a,pb=b; i<n; ++i,++pb) {
    for (j=0; j<m; ++j, ++pa)
      *pa /= *pb;
  }
}

// b(*) = sum(a(:,*))
void _dcsum(int m, int n, double *a, double *b) {
  int i,j;
  double *pa, *pb;
  for (i=0,pa=a,pb=b; i<n; ++i, ++pb) {
    *pb = 0;
    for (j=0; j<m; ++j, ++pa)
      *pb += *pa;
  }
}

// b(*) += sum(a(:,*))
void _dcsum2(int m, int n, double *a, double *b) {
  int i,j;
  double *pa, *pb;
  for (i=0,pa=a,pb=b; i<n; ++i, ++pb) {
    for (j=0; j<m; ++j, ++pa)
      *pb += *pa;
  }
}


// b(*) = sum(a(*,:))
void _drsum(int m, int n, double *a, double *b) {
  int i,j;
  double *pa, *pb;
  for (j=0,pb=b; j<m; ++j, ++pb) 
    *pb = 0;
  for (i=0,pa=a; i<n; ++i)
    for (j=0,pb=b; j<m; ++j, ++pa, ++pb)
      *pb += *pa;  
}

// b(*) += sum(a(*,:))
void _drsum2(int m, int n, double *a, double *b) {
  int i,j;
  double *pa, *pb;
  for (i=0,pa=a; i<n; ++i)
    for (j=0,pb=b; j<m; ++j, ++pa, ++pb)
      *pb += *pa;  
}

// normalize by column
void _dcnorm(int m, int n, double *a, double *sa) {
  int i, j;
  double *pa;
  bool isAllocated = true;
  if (!sa) {
    isAllocated = false;
    sa = _D2_MALLOC_SCALAR(n);
  }    
  _dcsum(m, n, a, sa);
  for (i=0, pa=sa; i<n; ++i, ++pa) {
    for (j=0; j<m; ++j, ++a) (*a) /= *pa;
  }
  if (!isAllocated) _D2_FREE(sa);
}

// normalize by row
void _drnorm(int m, int n, double *a, double *sa) {
  int i, j;
  double *pa;
  bool isAllocated = true;
  if (!sa) {
    isAllocated = false;
    sa = _D2_MALLOC_SCALAR(m);
  }    
  _drsum(m, n, a, sa);
  for (i=0; i<n; ++i) {
    pa = sa;
    for (j=0; j<m; ++j, ++a, ++pa) (*a) /= *pa;
  }
  if (!isAllocated) _D2_FREE(sa);
}

// center by column
void _dccenter(int m, int n, double *a, double *sa) {
  int i, j;
  double *pa;
  bool isAllocated = true;
  if (!sa) {
    isAllocated = false;
    sa = _D2_MALLOC_SCALAR(n);
  }    
  _dcsum(m, n, a, sa);
  cblas_dscal(n, 1./m, sa, 1);
  for (i=0, pa=sa; i<n; ++i, ++pa) {
    for (j=0; j<m; ++j, ++a) (*a) -= *pa;
  }
  if (!isAllocated) _D2_FREE(sa);
}

// center by row
void _drcenter(int m, int n, double *a, double *sa) {
  int i, j;
  double *pa;
  bool isAllocated = true;
  if (!sa) {
    isAllocated = false;
    sa = _D2_MALLOC_SCALAR(m);
  }    
  _drsum(m, n, a, sa);
  cblas_dscal(m, 1./n, sa, 1);
  for (i=0; i<n; ++i) {
    pa = sa;
    for (j=0; j<m; ++j, ++a, ++pa) (*a) -= *pa;
  }
  if (!isAllocated) _D2_FREE(sa);
}

// c = a.*b
void _dvmul(int n, double *a, double *b, double *c) {
  int i;
  for (i=0; i<n; ++i, ++c, ++a, ++b)
    *c = (*a) * (*b);
}

void _dpdist2(int d, int n, int m, double * A, double * B, double *C) {
  int i, j, k;
  assert(d>0 && n>0 && m>0);
  cblas_dgemm(CblasColMajor, CblasTrans, CblasNoTrans, n, m, d, -2, 
	      A, d, B, d, 0, C, n);

  for (i=0; i<m; ++i)
    for (j=0; j<n; ++j)
      for (k=0; k<d; ++k)
	C[i*n + j] += A[j*d + k] * A[j*d + k] + B[i*d + k] * B[i*d + k];
}

void _dpdist_symbolic(int d, int n, int m, int * A, int * B, double *C, 
		      const int vocab_size, const double* dist_mat) {
  int i,j,k;
  assert(d>0 && n>0 && m>0);
 
  for (i=0; i<m*n; ++i) C[i] = 0;
  for (i=0; i<m; ++i)
    for (j=0; j<n; ++j) 
      for (k=0; k<d; ++k) {
	C[i*n+j] += dist_mat[A[j*d + k]*vocab_size + B[i*d + k]];
      }
}

// inplace a -> exp(a)
void _dexp(int n, double *a) {
  int i;
  for (i=0; i<n; ++i, ++a) *a = exp(*a);
}
