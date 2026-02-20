#pragma once

// This project calls a subset of LAPACK/BLAS Fortran entry points directly (e.g. dgetrf_).
// Some backends/headers do not declare these symbols for C++ builds, which can cause
// compile errors even when linking is correctly configured.
//
// Declaring them as varargs keeps this header backend-agnostic and also tolerates
// the optional hidden string-length arguments that some toolchains append.

#if defined(ACCELERATE) || defined(INTEL_MKL)
extern "C" {
void dgetrf_(...);
void dgetri_(...);
void dgetrs_(...);
void dgesv_(...);
void dgels_(...);
}
#endif
