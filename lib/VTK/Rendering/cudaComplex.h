// Complex math functions
typedef float2 Complex;

__device__ Complex complexMul(Complex a, Complex b) {
    Complex t = {a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x};
    return t;
}


__device__ Complex complexScale(Complex a, float c) {
    Complex t = {a.x * c, a.y * c};
    return t;
}


__device__ Complex complexMulAndScale(Complex a, Complex b, float c) {
    return complexScale(complexMul(a, b), c);
}