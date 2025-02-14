#include <gauge_field.h>
#include <gauge_tools.h>

namespace quda
{

  void gaugeObservables(GaugeField &u, QudaGaugeObservableParam &param, TimeProfile &profile)
  {
    profile.TPSTART(QUDA_PROFILE_COMPUTE);
    if (param.su_project) {
      int *num_failures_h = static_cast<int *>(pool_pinned_malloc(sizeof(int)));
      int *num_failures_d = static_cast<int *>(get_mapped_device_pointer(num_failures_h));
      *num_failures_h = 0;
      auto tol = u.Precision() == QUDA_DOUBLE_PRECISION ? 1e-14 : 1e-6;
      projectSU3(u, tol, num_failures_d);
      if (*num_failures_h > 0) errorQuda("Error in the SU(3) unitarization: %d failures\n", *num_failures_h);
      pool_pinned_free(num_failures_h);
    }

    if (param.compute_plaquette) {
      double3 plaq = plaquette(u);
      param.plaquette[0] = plaq.x;
      param.plaquette[1] = plaq.y;
      param.plaquette[2] = plaq.z;
    }
    profile.TPSTOP(QUDA_PROFILE_COMPUTE);

    // no point constructing Fmunu unless we are going to use it
    if (!param.compute_qcharge && !param.compute_qcharge_density) return;

    // create the Fmunu field
    profile.TPSTART(QUDA_PROFILE_INIT);
    // u is an extended field we need to shrink for the Fmunu field
    int x[4];
    for (int i = 0; i < 4; i++) x[i] = u.X()[i] - 2 * u.R()[i];
    GaugeFieldParam tensorParam(x, u.Precision(), QUDA_RECONSTRUCT_NO, 0, QUDA_TENSOR_GEOMETRY);
    tensorParam.siteSubset = QUDA_FULL_SITE_SUBSET;
    tensorParam.order = QUDA_FLOAT2_GAUGE_ORDER;
    tensorParam.ghostExchange = QUDA_GHOST_EXCHANGE_NO;
    cudaGaugeField gaugeFmunu(tensorParam);
    profile.TPSTOP(QUDA_PROFILE_INIT);

    profile.TPSTART(QUDA_PROFILE_COMPUTE);
    computeFmunu(gaugeFmunu, u);
    profile.TPSTOP(QUDA_PROFILE_COMPUTE);
    profile.TPSTOP(QUDA_PROFILE_TOTAL);

    if (param.compute_qcharge || param.compute_qcharge_density) {
      profile.TPSTART(QUDA_PROFILE_TOTAL);
      profile.TPSTART(QUDA_PROFILE_INIT);
      if (param.compute_qcharge_density && !param.qcharge_density)
        errorQuda("Charge density requested, but destination field not defined");
      size_t size = gaugeFmunu.Volume() * gaugeFmunu.Precision();
      void *d_qDensity = param.compute_qcharge_density ? pool_device_malloc(size) : nullptr;
      profile.TPSTOP(QUDA_PROFILE_INIT);

      profile.TPSTART(QUDA_PROFILE_COMPUTE);

      if (param.compute_qcharge_density)
        computeQChargeDensity(param.energy, param.qcharge, d_qDensity, gaugeFmunu);
      else
        computeQCharge(param.energy, param.qcharge, gaugeFmunu);

      profile.TPSTOP(QUDA_PROFILE_COMPUTE);

      if (param.compute_qcharge_density) {
        profile.TPSTART(QUDA_PROFILE_D2H);
        qudaMemcpy(param.qcharge_density, d_qDensity, size, qudaMemcpyDeviceToHost);
        profile.TPSTOP(QUDA_PROFILE_D2H);

        profile.TPSTART(QUDA_PROFILE_FREE);
        pool_device_free(d_qDensity);
        profile.TPSTOP(QUDA_PROFILE_FREE);
      }
    }
  }

} // namespace quda
