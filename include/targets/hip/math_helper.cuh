#pragma once

#include <math.h>

namespace quda {

  
  /**
   * @brief Combined sin and cos colculation in QUDA NAMESPACE  
   * @param a the angle 
   * @param s pointer to the storage for the result of the sin
   * @param c pointer to the storage for the result of the cos
   *
   */
  template<typename T>
  inline __host__ __device__ void sincos(const T& a, T* s, T* c)
    {
        // Hip Does not have ::sincos(a,s,c);
	// Just on device sincosf
	// Need to do this as 2 stepper?
	*s = sin(a);
	*c = cos(a);
    }

 
  /**
   * @brief Combined sin and cos colculation in QUDA NAMESPACE
   *
   * @param a the angle
   * @param s pointer to the storage for the result of the sin
   * @param c pointer to the storage for the result of the cos
   *
   * Specialization to float arguments. Device function calls CUDA intrinsic
   *
   */ 
  template<>
  inline  __host__ __device__ void sincos(const float& a, float * s, float *c)
  {
#ifdef __HIP_DEVICE_COMPILE__
    __sincosf(a,s,c);
#else
    *s=sinf(a);
    *c=cosf(a);
#endif
  }

  /**
   * @brief Reciprocal square root function (rsqrt)
   * @param a the argument  (In|out)
   *
   * some math libraries provide a fast inverse sqrt() function.
   * this implementation uses the CUDA builtins
   */
  template<typename T>
  inline __host__ __device__ T rsqrt(T a)
    {
      return ::rsqrt(a);
    }


    /**
     Generic wrapper for Trig functions -- used in gauge field order 
    */
  template <bool isFixed, typename T>
  struct Trig {
    __device__ __host__ static T Atan2( const T &a, const T &b) { return ::atan2(a,b); }
    __device__ __host__ static T Sin( const T &a ) { return ::sin(a); }
    __device__ __host__ static T Cos( const T &a ) { return ::cos(a); }
    __device__ __host__ static void SinCos(const T &a, T *s, T *c) { 
	*s = cos(a);
        *c = cos(a);	
    }
  };
  
  /**
     Specialization of Trig functions using floats
   */
  template <>
    struct Trig<false,float> {
    __device__ __host__ static float Atan2( const float &a, const float &b) { return ::atan2f(a,b); }
    __device__ __host__ static float Sin(const float &a)
    {
#ifdef __HIP_DEVICE_COMPILE__
      return __sinf(a); 
#else
      return ::sinf(a);
#endif
    }
    __device__ __host__ static float Cos(const float &a)
    {
#ifdef __HIP_DEVICE_COMPILE__
      return __cosf(a); 
#else
      return ::cosf(a); 
#endif
    }

    __device__ __host__ static void SinCos(const float &a, float *s, float *c)
    {
#ifdef __HIP_DEVICE_COMPILE__
       __sincosf(a, s, c);
#else
       *s = sinf(a); 
       *c = cosf(a);
#endif
    }
  };

  /**
     Specialization of Trig functions using fixed b/c gauge reconstructs are -1 -> 1 instead of -Pi -> Pi
   */
  template <>
    struct Trig<true,float> {
    __device__ __host__ static float Atan2( const float &a, const float &b) { return ::atan2f(a,b)/M_PI; }
    __device__ __host__ static float Sin(const float &a)
    {
#ifdef __HIP_DEVICE_COMPILE__
      return __sinf(a * static_cast<float>(M_PI));
#else
      return ::sinf(a * static_cast<float>(M_PI));
#endif
    }

    __device__ __host__ static float Cos(const float &a)
    {
#ifdef __HIP_DEVICE_COMPILE__
      return __cosf(a * static_cast<float>(M_PI));
#else
      return ::cosf(a * static_cast<float>(M_PI));
#endif
    }

    __device__ __host__ static void SinCos(const float &a, float *s, float *c)
    {
#ifdef __HIP_DEVICE_COMPILE__
      __sincosf(a * static_cast<float>(M_PI), s, c);
#else
      auto ampi = a * static_cast<float>(M_PI);
      *s = sinf(ampi);
      *c = cosf(ampi);
#endif
    }
  };

/*
    @brief Fast power function that works for negative "a" argument
    @param a argument we want to raise to some power
    @param b power that we want to raise a to
    @return pow(a,b)
  */
  template <typename real> __device__ __host__ inline real __fast_pow(real a, int b)
  {
#ifdef __HIP_DEVICE_COMPILE__
    if (sizeof(real) == sizeof(double)) {
      return ::pow(a, b);
    } else {
      float sign = signbit(a) ? -1.0f : 1.0f;
      float power = __powf(fabsf(a), b);
      return b & 1 ? sign * power : power;
    }
#else
    return std::pow(a, b);
#endif
  }


}
