#pragma once

#include <quda_internal.h>
#include <quda.h>

namespace quda
{
  // called in interface_quda.cpp, defined in contract.cu
  void contractSummedQuda(const ColorSpinorField &x, const ColorSpinorField &y, std::vector<Complex> &result,
                          QudaContractType cType, const int *const source_position, const std::vector<int[4]> &mom_mode, const size_t s1, const size_t b1);
  
  void contractQuda(const ColorSpinorField &x, const ColorSpinorField &y, void *result, QudaContractType cType);
} // namespace quda
