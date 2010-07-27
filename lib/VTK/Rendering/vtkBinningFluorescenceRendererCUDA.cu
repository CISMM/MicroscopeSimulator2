#include <stdio.h>

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  include <windows.h>
#endif
#include "cuda_gl_interop.h"

#include "vtkBinningFluorescenceRendererCUDA.h"


#define BLOCKS 16
#define THREADS_PER_BLOCK 128

//////////////////////////////////////////////////////////////////////////////
// Splits an RGBA image into individual RGB channel buffers
//////////////////////////////////////////////////////////////////////////////
__global__ void splitPixelsKernel(float4 *pixels, int n,
                                  float *r, float *g, float *b) {
  const int tid     = __mul24(blockIdx.x, blockDim.x) + threadIdx.x;
  const int threadN = __mul24(blockDim.x, gridDim.x);

  // Naive version. Make use of shared memory in future
  // to exploit coalesced reads and writes.
  for (int i = tid; i < n; i += threadN) {
    float4 pixel = pixels[i];
    r[i] = pixel.x;
    g[i] = pixel.y;
    b[i] = pixel.z;
  }
}


extern "C"
cudaError_t
splitPixels(float4 *pixels, int width, int height,
            float *r, float *g, float *b) {
  int n = width*height;
  dim3 grid(BLOCKS);
  dim3 block(THREADS_PER_BLOCK);

  splitPixelsKernel<<<grid, block>>>(pixels, n, r, g, b);

  return cudaGetLastError();
}


//////////////////////////////////////////////////////////////////////////////
// Merges RGB channels into an RGBA image. Alpha value is 1.0.
//////////////////////////////////////////////////////////////////////////////
__global__ void mergePixelsKernel(float4 *pixels, int n,
                                  float *r, float *g, float *b) {

  const int tid     = __mul24(blockIdx.x, blockDim.x) + threadIdx.x;
  const int threadN = __mul24(blockDim.x, gridDim.x);

  // Naive version. Make use of shared memory in future
  // to exploit coalesced reads and writes.
  for (int i = tid; i < n; i += threadN) {
    pixels[i] = make_float4(r[i], g[i], b[i], 1.0f);

  }
}


extern "C"
cudaError_t
mergePixels(float4 *pixels, int width, int height, float *r, float *g, float *b) {
  int n = width*height;
  dim3 grid(BLOCKS);
  dim3 block(THREADS_PER_BLOCK);

  mergePixelsKernel<<<grid, block>>>(pixels, n, r, g, b);

  return cudaGetLastError();
}


//////////////////////////////////////////////////////////////////////////////
// Forward FFT
//////////////////////////////////////////////////////////////////////////////
extern "C"
void
forwardFFT(float *in, Complex *out, int width, int height, cufftHandle plan) {
  CUFFT_SAFE_CALL(cufftExecR2C(plan, (cufftReal*) in, (cufftComplex*) out));
}



//////////////////////////////////////////////////////////////////////////////
// Inverse FFT
//////////////////////////////////////////////////////////////////////////////
extern "C"
void
inverseFFT(Complex *in, float *out, int width, int height, cufftHandle plan) {
  CUFFT_SAFE_CALL(cufftExecC2R(plan, (cufftComplex*) in, (cufftReal*) out));
}


//////////////////////////////////////////////////////////////////////////////
// Does a component-wise multiply across two arrays of complex values.
//////////////////////////////////////////////////////////////////////////////
__global__ void complexMultiplyKernel(Complex *c1, Complex *c2, Complex *result, float scale, int n) {
  const int tid     = __mul24(blockIdx.x, blockDim.x) + threadIdx.x;
  const int threadN = __mul24(blockDim.x, gridDim.x);

  for (int i = tid; i < n; i += threadN) {
    result[i] = complexMulAndScale(c1[i], c2[i], scale);
  }
}


extern "C"
cudaError_t
complexMultiply(Complex *c1, Complex *c2, Complex *result, int n, float scale) {
  dim3 grid(BLOCKS);
  dim3 block(THREADS_PER_BLOCK);

  complexMultiplyKernel<<<grid, block>>>(c1, c2, result, scale, n);

  return cudaGetLastError();
}
