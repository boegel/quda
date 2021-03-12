#include <quda_define.h>
#include <gauge_field_order.h>
#include <color_spinor_field_order.h>
#include <index_helper.cuh>
#include <float_vector.h>
#include <shared_memory_cache_helper.cuh>
#include <kernel.h>
#include <warp_collective.h>

namespace quda {

  enum DslashType {
    DSLASH_INTERIOR,
    DSLASH_EXTERIOR,
    DSLASH_FULL
  };

  constexpr int colors_per_thread() { return 1; }

  template <bool dslash_, bool clover_, bool dagger_, DslashType type_, int color_stride_, int dim_stride_, typename Float,
            typename yFloat, typename ghostFloat, int nSpin_, int nColor_, QudaFieldOrder csOrder, QudaGaugeFieldOrder gOrder>
  struct DslashCoarseArg {
    static constexpr bool dslash = dslash_;
    static constexpr bool clover = clover_;
    static constexpr bool dagger = dagger_;
    static constexpr DslashType type = type_;
    static constexpr int color_stride = color_stride_;
    static constexpr int dim_stride = dim_stride_;

    using real = typename mapper<Float>::type;
    static constexpr int nSpin = nSpin_;
    static constexpr int nColor = nColor_;
    static constexpr int nDim = 4;

    typedef typename colorspinor::FieldOrderCB<real, nSpin, nColor, 1, csOrder, Float, ghostFloat> F;
    typedef typename gauge::FieldOrder<real, nColor * nSpin, nSpin, gOrder, true, yFloat> G;
    typedef typename gauge::FieldOrder<real, nColor * nSpin, nSpin, gOrder, true, yFloat> GY;

    F out;
    const F inA;
    const F inB;
    const GY Y;
    const GY X;
    const real kappa;
    const int parity; // only use this for single parity fields
    const int nParity; // number of parities we're working on
    const int nFace;  // hard code to 1 for now
    const int_fastdiv X0h; // X[0]/2
    const int_fastdiv dim[5];   // full lattice dimensions
    const int commDim[4]; // whether a given dimension is partitioned or not
    const int volumeCB;
    dim3 threads;

    inline DslashCoarseArg(ColorSpinorField &out, const ColorSpinorField &inA, const ColorSpinorField &inB,
                           const GaugeField &Y, const GaugeField &X, real kappa, int parity) :
      out(const_cast<ColorSpinorField &>(out)),
      inA(const_cast<ColorSpinorField &>(inA)),
      inB(const_cast<ColorSpinorField &>(inB)),
      Y(const_cast<GaugeField &>(Y)),
      X(const_cast<GaugeField &>(X)),
      kappa(kappa),
      parity(parity),
      nParity(out.SiteSubset()),
      nFace(1),
      X0h(((3 - nParity) * out.X(0)) / 2),
      dim {(3 - nParity) * out.X(0), out.X(1), out.X(2), out.X(3), out.Ndim() == 5 ? out.X(4) : 1},
      commDim {comm_dim_partitioned(0), comm_dim_partitioned(1), comm_dim_partitioned(2), comm_dim_partitioned(3)},
      volumeCB((unsigned int)out.VolumeCB() / dim[4]),
      threads(color_stride * X.VolumeCB(), nParity, 2 * dim_stride * 2 * (nColor / colors_per_thread()))
    {  }
  };

  /**
     @brief Helper function to determine if should halo computation
  */
  template <DslashType type>
  static __host__ __device__ bool doHalo() {
    switch(type) {
    case DSLASH_EXTERIOR:
    case DSLASH_FULL:
      return true;
    default:
      return false;
    }
  }

  /**
     @brief Helper function to determine if should interior computation
  */
  template <DslashType type>
  static __host__ __device__ bool doBulk() {
    switch(type) {
    case DSLASH_INTERIOR:
    case DSLASH_FULL:
      return true;
    default:
      return false;
    }
  }

  /**
     Applies the coarse dslash on a given parity and checkerboard site index

     @param out The result - kappa * Dslash in
     @param Y The coarse gauge field
     @param kappa Kappa value
     @param in The input field
     @param parity The site parity
     @param x_cb The checkerboarded site index
   */
  template <int Mc, int thread_dir, int thread_dim, typename V, typename Arg>
  __device__ __host__ inline void applyDslash(V &out, Arg &arg, int x_cb, int src_idx, int parity, int s_row, int color_block, int color_offset)
  {
    const int their_spinor_parity = (arg.nParity == 2) ? 1-parity : 0;

    int coord[5];
    getCoordsCB(coord, x_cb, arg.dim, arg.X0h, parity);
    coord[4] = src_idx;

    SharedMemoryCache<V> cache(target::block_dim());

    if (!thread_dir || target::is_host()) {

      //Forward gather - compute fwd offset for spinor fetch
#pragma unroll
      for(int d = thread_dim; d < Arg::nDim; d += Arg::dim_stride) // loop over dimension
      {
	const int fwd_idx = linkIndexP1(coord, arg.dim, d);

	if ( arg.commDim[d] && (coord[d] + arg.nFace >= arg.dim[d]) ) {
	  if (doHalo<Arg::type>()) {
            int ghost_idx = ghostFaceIndex<1, 5>(coord, arg.dim, d, arg.nFace);

#pragma unroll
	    for(int color_local = 0; color_local < Mc; color_local++) { //Color row
	      int c_row = color_block + color_local; // global color index
	      int row = s_row * Arg::nColor + c_row;
#pragma unroll
	      for(int s_col = 0; s_col < Arg::nSpin; s_col++) { //Spin column
#pragma unroll
		for(int c_col = 0; c_col < Arg::nColor; c_col += Arg::color_stride) { //Color column
		  int col = s_col * Arg::nColor + c_col + color_offset;
		  if (!Arg::dagger)
		    out[color_local] += arg.Y(d+4, parity, x_cb, row, col)
		      * arg.inA.Ghost(d, 1, their_spinor_parity, ghost_idx + src_idx*arg.volumeCB, s_col, c_col+color_offset);
		  else
		    out[color_local] += arg.Y(d, parity, x_cb, row, col)
		      * arg.inA.Ghost(d, 1, their_spinor_parity, ghost_idx + src_idx*arg.volumeCB, s_col, c_col+color_offset);
		}
	      }
	    }
	  }
	} else if (doBulk<Arg::type>()) {
#pragma unroll
	  for(int color_local = 0; color_local < Mc; color_local++) { //Color row
	    int c_row = color_block + color_local; // global color index
	    int row = s_row * Arg::nColor + c_row;
#pragma unroll
	    for(int s_col = 0; s_col < Arg::nSpin; s_col++) { //Spin column
#pragma unroll
	      for(int c_col = 0; c_col < Arg::nColor; c_col += Arg::color_stride) { //Color column
		int col = s_col * Arg::nColor + c_col + color_offset;
		if (!Arg::dagger)
		  out[color_local] += arg.Y(d+4, parity, x_cb, row, col)
		    * arg.inA(their_spinor_parity, fwd_idx + src_idx*arg.volumeCB, s_col, c_col+color_offset);
		else
		  out[color_local] += arg.Y(d, parity, x_cb, row, col)
		    * arg.inA(their_spinor_parity, fwd_idx + src_idx*arg.volumeCB, s_col, c_col+color_offset);
	      }
	    }
	  }
	}

      } // nDim

      // only need to write to shared memory if not master thread
      if (target::is_device() && thread_dim > 0) cache.save(out);
    }

    if (thread_dir || target::is_host()) {

      //Backward gather - compute back offset for spinor and gauge fetch
#pragma unroll
      for(int d = thread_dim; d < Arg::nDim; d += Arg::dim_stride)
	{
	const int back_idx = linkIndexM1(coord, arg.dim, d);
	const int gauge_idx = back_idx;
	if ( arg.commDim[d] && (coord[d] - arg.nFace < 0) ) {
	  if (doHalo<Arg::type>()) {
            const int ghost_idx = ghostFaceIndex<0, 5>(coord, arg.dim, d, arg.nFace);
#pragma unroll
	    for (int color_local=0; color_local<Mc; color_local++) {
	      int c_row = color_block + color_local;
	      int row = s_row * Arg::nColor + c_row;
#pragma unroll
	      for (int s_col=0; s_col < Arg::nSpin; s_col++)
#pragma unroll
		for (int c_col=0; c_col < Arg::nColor; c_col += Arg::color_stride) {
		  int col = s_col * Arg::nColor + c_col + color_offset;
		  if (!Arg::dagger)
		    out[color_local] += conj(arg.Y.Ghost(d, 1-parity, ghost_idx, col, row))
		      * arg.inA.Ghost(d, 0, their_spinor_parity, ghost_idx + src_idx*arg.volumeCB, s_col, c_col+color_offset);
		  else
		    out[color_local] += conj(arg.Y.Ghost(d+4, 1-parity, ghost_idx, col, row))
		      * arg.inA.Ghost(d, 0, their_spinor_parity, ghost_idx + src_idx*arg.volumeCB, s_col, c_col+color_offset);
		}
	    }
	  }
	} else if (doBulk<Arg::type>()) {
#pragma unroll
	  for(int color_local = 0; color_local < Mc; color_local++) {
	    int c_row = color_block + color_local;
	    int row = s_row * Arg::nColor + c_row;
#pragma unroll
	    for(int s_col = 0; s_col < Arg::nSpin; s_col++)
#pragma unroll
	      for(int c_col = 0; c_col < Arg::nColor; c_col += Arg::color_stride) {
		int col = s_col * Arg::nColor + c_col + color_offset;
		if (!Arg::dagger)
		  out[color_local] += conj(arg.Y(d, 1-parity, gauge_idx, col, row))
		    * arg.inA(their_spinor_parity, back_idx + src_idx*arg.volumeCB, s_col, c_col+color_offset);
		else
		  out[color_local] += conj(arg.Y(d+4, 1-parity, gauge_idx, col, row))
		    * arg.inA(their_spinor_parity, back_idx + src_idx*arg.volumeCB, s_col, c_col+color_offset);
	      }
	  }
	}

      } //nDim

      if (target::is_device()) cache.save(out);
    } // forwards / backwards thread split

    if (target::is_device()) cache.sync(); // device path has to recombine the foward and backward results

    // (colorspin * dim_stride + dim * 2 + dir)
    if (target::is_device() && thread_dim == 0 && thread_dir == 0) {

      // full split over dimension and direction
#pragma unroll
      for (int d=1; d < Arg::dim_stride; d++) { // get remaining forward fathers (if any)
	// 4-way 1,2,3  (stride = 4)
	// 2-way 1      (stride = 2)
        out += cache.load_z((target::thread_idx().z / (2 * Arg::dim_stride)) * (2 * Arg::dim_stride) + d * 2 + 0);
      }

#pragma unroll
      for (int d=0; d < Arg::dim_stride; d++) { // get all backward gathers
        out += cache.load_z((target::thread_idx().z / (2 * Arg::dim_stride)) * (2 * Arg::dim_stride) + d * 2 + 1);
      }

      out *= -arg.kappa;

    } else if (target::is_host()) {

      out *= -arg.kappa;

    }
  }

  /**
     Applies the coarse clover matrix on a given parity and
     checkerboard site index

     @param out The result out += X * in
     @param X The coarse clover field
     @param in The input field
     @param parity The site parity
     @param x_cb The checkerboarded site index
   */
  template <int Mc, typename V, typename Arg>
  __device__ __host__ inline void applyClover(V &out, Arg &arg, int x_cb, int src_idx, int parity, int s, int color_block, int color_offset) {
    const int spinor_parity = (arg.nParity == 2) ? parity : 0;

    // M is number of colors per thread
#pragma unroll
    for(int color_local = 0; color_local < Mc; color_local++) {//Color out
      int c = color_block + color_local; // global color index
      int row = s * Arg::nColor + c;
#pragma unroll
      for (int s_col = 0; s_col < Arg::nSpin; s_col++) //Spin in
#pragma unroll
	for (int c_col = 0; c_col < Arg::nColor; c_col += Arg::color_stride) { //Color in
	  //Factor of kappa and diagonal addition now incorporated in X
	  int col = s_col * Arg::nColor + c_col + color_offset;
	  if (!Arg::dagger) {
	    out[color_local] += arg.X(0, parity, x_cb, row, col)
	      * arg.inB(spinor_parity, x_cb+src_idx*arg.volumeCB, s_col, c_col+color_offset);
	  } else {
	    out[color_local] += conj(arg.X(0, parity, x_cb, col, row))
	      * arg.inB(spinor_parity, x_cb+src_idx*arg.volumeCB, s_col, c_col+color_offset);
	  }
	}
    }

  }

  //out(x) = M*in = \sum_mu Y_{-\mu}(x)in(x+mu) + Y^\dagger_mu(x-mu)in(x-mu)
  template <int Mc, int dir, int dim, typename Arg>
  __device__ __host__ inline void coarseDslash(Arg &arg, int x_cb, int parity, int s, int color_block, int color_offset)
  {
    constexpr int src_idx = 0;
    using Float = typename Arg::real;
    vector_type<complex <Float>, Mc> out;
    if (Arg::dslash) applyDslash<Mc,dir,dim>(out, arg, x_cb, src_idx, parity, s, color_block, color_offset);
    if (doBulk<Arg::type>() && Arg::clover && dir==0 && dim==0) applyClover<Mc>(out, arg, x_cb, src_idx, parity, s, color_block, color_offset);

    if (dir==0 && dim==0) {
      const int my_spinor_parity = (arg.nParity == 2) ? parity : 0;

      // reduce down to the first group of column-split threads
      out = warp_combine<Arg::color_stride>(out);

#pragma unroll
      for (int color_local=0; color_local<Mc; color_local++) {
	int c = color_block + color_local; // global color index
	if (color_offset == 0) {
	  // if not halo we just store, else we accumulate
	  if (doBulk<Arg::type>()) arg.out(my_spinor_parity, x_cb+src_idx*arg.volumeCB, s, c) = out[color_local];
	  else arg.out(my_spinor_parity, x_cb+src_idx*arg.volumeCB, s, c) += out[color_local];
	}
      }
    }

  }

  template <typename Arg> struct CoarseDslash {
    Arg &arg;
    constexpr CoarseDslash(Arg &arg) : arg(arg) {}
    static constexpr const char *filename() { return KERNEL_FILE; }

    __device__ __host__ inline void operator()(int x_cb_color_offset, int parity, int sMd)
    {
      int x_cb = x_cb_color_offset;
      int color_offset = 0;

      if (target::is_device() && Arg::color_stride > 1) { // on the device we support warp fission of the inner product
        const int lane_id = target::thread_idx().x % device::warp_size();
        const int warp_id = target::thread_idx().x / device::warp_size();
        const int vector_site_width = device::warp_size() / Arg::color_stride; // number of sites per warp

        x_cb = target::block_idx().x * (target::block_dim().x / Arg::color_stride) + warp_id * (device::warp_size() / Arg::color_stride) + lane_id % vector_site_width;
        color_offset = lane_id / vector_site_width;
      }

      parity = (arg.nParity == 2) ? parity : arg.parity;

      // z thread dimension is (( s*(Nc/Mc) + color_block )*dim_thread_split + dim)*2 + dir
      constexpr int Mc = colors_per_thread();
      int dir = sMd & 1;
      int sMdim = sMd >> 1;
      int dim = sMdim % Arg::dim_stride;
      int sM = sMdim / Arg::dim_stride;
      int s = sM / (Arg::nColor/Mc);
      int color_block = (sM % (Arg::nColor/Mc)) * Mc;

      if (dir == 0) {
        if (dim == 0)      coarseDslash<Mc,0,0>(arg, x_cb, parity, s, color_block, color_offset);
        else if (dim == 1) coarseDslash<Mc,0,1>(arg, x_cb, parity, s, color_block, color_offset);
        else if (dim == 2) coarseDslash<Mc,0,2>(arg, x_cb, parity, s, color_block, color_offset);
        else if (dim == 3) coarseDslash<Mc,0,3>(arg, x_cb, parity, s, color_block, color_offset);
      } else if (dir == 1) {
        if (dim == 0)      coarseDslash<Mc,1,0>(arg, x_cb, parity, s, color_block, color_offset);
        else if (dim == 1) coarseDslash<Mc,1,1>(arg, x_cb, parity, s, color_block, color_offset);
        else if (dim == 2) coarseDslash<Mc,1,2>(arg, x_cb, parity, s, color_block, color_offset);
        else if (dim == 3) coarseDslash<Mc,1,3>(arg, x_cb, parity, s, color_block, color_offset);
      }
    }
  };

} // namespace quda
