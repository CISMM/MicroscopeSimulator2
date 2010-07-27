#ifndef _vtkBinningFluorescenceRendererCUDA_h_
#define _vtkBinningFluorescenceRendererCUDA_h_

#include <cuda_runtime.h>
#include <cufft.h>
#include "cudaComplex.h"

//////////////////////////////////////////////////////////////////////////////
// Error macros
//////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

#  define CUDA_SAFE_CALL_NO_SYNC( call) do {                                 \
    cudaError_t err = call;                                                  \
    if( cudaSuccess != err) {                                                \
        fprintf(stderr, "Cuda error in file '%s' in line %i : %s.\n",        \
                __FILE__, __LINE__, cudaGetErrorString( err) );              \
        exit(EXIT_FAILURE);                                                  \
    } } while (0)

#  define CUDA_SAFE_CALL( call) do {                                         \
    CUDA_SAFE_CALL_NO_SYNC(call);                                            \
    cudaError_t err = cudaThreadSynchronize();                               \
    if( cudaSuccess != err) {                                                \
        fprintf(stderr, "Cuda error in file '%s' in line %i : %s.\n",        \
                __FILE__, __LINE__, cudaGetErrorString( err) );              \
        exit(EXIT_FAILURE);                                                  \
    } } while (0)

#  define CUFFT_SAFE_CALL( call) do {                                        \
    cufftResult err = call;                                                  \
    if( CUFFT_SUCCESS != err) {                                              \
        fprintf(stderr, "CUFFT error in file '%s' in line %i.\n",            \
                __FILE__, __LINE__);                                         \
        exit(EXIT_FAILURE);                                                  \
    } } while (0)

#else

#define CUDA_SAFE_CALL_NO_SYNC(call) { \
  call;                                \
  }

#define CUDA_SAFE_CALL(call) {         \
  call;                                \
  }

#define CUFFT_SAFE_CALL(call) {        \
  call;                                \
  }

#endif // DEBUG

//////////////////////////////////////////////////////////////////////////////
// CUDA bridge functions.
//////////////////////////////////////////////////////////////////////////////

extern "C"
cudaError_t 
splitPixels(float4 *pixels, int width, int height, float *r, float *g, float *b);

extern "C"
cudaError_t 
mergePixels(float4 *pixels, int width, int height, float *r, float *g, float *b);

extern "C"
void
forwardFFT(float *in, Complex *out, int width, int height, cufftHandle plan);

extern "C"
void
inverseFFT(Complex *in, float *out, int width, int height, cufftHandle plan);

extern "C"
cudaError_t
complexMultiply(Complex *c1, Complex *c2, Complex *result, int n, float scale);

#endif // _vtkBinningFluorescenceRendererCUDA_h_