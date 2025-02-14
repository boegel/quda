#include <gauge_field.h>
#include <instantiate.h>
#include <tunable_reduction.h>
#include <kernels/gauge_qcharge.cuh>

namespace quda
{

  template <typename Float, int nColor, QudaReconstructType recon>
  class QCharge : TunableReduction2D<> {
    const GaugeField &Fmunu;
    double *energy;
    double &qcharge;
    void *qdensity;
    bool density;

  public:
    QCharge(const GaugeField &Fmunu, double energy[3], double &qcharge, void *qdensity, bool density) :
      TunableReduction2D(Fmunu),
      Fmunu(Fmunu),
      energy(energy),
      qcharge(qcharge),
      qdensity(qdensity),
      density(density)
    {
      if (!Fmunu.isNative()) errorQuda("Topological charge only supported on native ordered fields");
      apply(device::get_default_stream());
    }

    void apply(const qudaStream_t &stream)
    {
      TuneParam tp = tuneLaunch(*this, getTuning(), getVerbosity());

      std::vector<double> result(3);
      if (density) {
        QChargeArg<Float, nColor, recon, true> arg(Fmunu, (Float*)qdensity);
        launch<qCharge>(result, tp, stream, arg);
      } else {
        QChargeArg<Float, nColor, recon, false> arg(Fmunu, (Float*)qdensity);
        launch<qCharge>(result, tp, stream, arg);
      }

      for (int i=0; i<2; i++) energy[i+1] = result[i] / (Fmunu.Volume() * comm_size());
      energy[0] = energy[1] + energy[2];
      qcharge = result[2];
    }

    long long flops() const
    {
      auto Nc = Fmunu.Ncolor();
      auto mm_flops = 8 * Nc * Nc * (Nc - 2);
      auto traceless_flops = (Nc * Nc + Nc + 1);
      auto energy_flops = 6 * (mm_flops + traceless_flops + Nc);
      auto q_flops = 3*mm_flops + 2 * Nc + 2;
      return Fmunu.Volume() * (energy_flops + q_flops);
    }

    long long bytes() const { return Fmunu.Bytes() + Fmunu.Volume() * (density * Fmunu.Precision()); }
  }; // QChargeCompute

#ifdef GPU_GAUGE_TOOLS
  void computeQCharge(double energy[3], double &qcharge, const GaugeField &Fmunu)
  {
    instantiate<QCharge,ReconstructNone>(Fmunu, energy, qcharge, nullptr, false);
  }
#else
  void computeQCharge(double [3], double &, const GaugeField &)
  {
    errorQuda("Gauge tools are not built");
  }
#endif // GPU_GAUGE_TOOLS

#ifdef GPU_GAUGE_TOOLS
  void computeQChargeDensity(double energy[3], double &qcharge, void *qdensity, const GaugeField &Fmunu)
  {
    instantiate<QCharge,ReconstructNone>(Fmunu, energy, qcharge, qdensity, true);
  }
#else
  void computeQChargeDensity(double [3], double &, void *, const GaugeField &)
  {
    errorQuda("Gauge tools are not built");
  }
#endif // GPU_GAUGE_TOOLS

} // namespace quda
