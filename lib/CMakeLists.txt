# all files for quda -- needs some cleanup
# cmake-format: off

# QUDA_HASH for tunecache
set(HASH cpu_arch=${CPU_ARCH},gpu_arch=${QUDA_GPU_ARCH},cuda_version=${CMAKE_CUDA_COMPILER_VERSION})
set(GITVERSION "${PROJECT_VERSION}-${GITVERSION}-${QUDA_GPU_ARCH}")

# this allows simplified running of clang-tidy
if(${CMAKE_BUILD_TYPE} STREQUAL "DEVEL")
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# build up git version add -debug to GITVERSION if we build with debug options enabled
string(REGEX MATCH [Dd][Ee][Bb][Uu][Gg] DEBUG_BUILD ${CMAKE_BUILD_TYPE})
if(DEBUG_BUILD)
  if(GITVERSION)
    set(GITVERSION ${GITVERSION}-debug)
  else()
    set(GITVERSION debug)
  endif()
endif()

set (QUDA_OBJS
  # cmake-format: sortable
  dirac_coarse.cpp dslash_coarse.cu dslash_coarse_dagger.cu
  coarse_op.cu coarsecoarse_op.cu coarsecoarse_op_mma.cu
  coarse_op_preconditioned.cu staggered_coarse_op.cu
  eig_iram.cpp eig_trlm.cpp eig_block_trlm.cpp vector_io.cpp
  eigensolve_quda.cpp quda_arpack_interface.cpp
  multigrid.cpp transfer.cpp block_orthogonalize.cu inv_bicgstab_quda.cpp
  prolongator.cu restrictor.cu staggered_prolong_restrict.cu
  gauge_phase.cu timer.cpp
  solver.cpp inv_bicgstab_quda.cpp inv_cg_quda.cpp inv_bicgstabl_quda.cpp
  inv_multi_cg_quda.cpp inv_eigcg_quda.cpp gauge_ape.cu
  gauge_stout.cu gauge_wilson_flow.cu gauge_plaq.cu
  gauge_laplace.cpp gauge_observable.cpp
  inv_cg3_quda.cpp inv_ca_gcr.cpp inv_ca_cg.cpp
  inv_gcr_quda.cpp inv_mr_quda.cpp inv_sd_quda.cpp
  inv_pcg_quda.cpp inv_mre.cpp interface_quda.cpp util_quda.cpp
  color_spinor_field.cpp color_spinor_util.cu color_spinor_pack.cu
  gauge_covdev.cpp 
  cpu_color_spinor_field.cpp cuda_color_spinor_field.cpp dirac.cpp
  clover_field.cpp lattice_field.cpp gauge_field.cpp
  cpu_gauge_field.cpp cuda_gauge_field.cpp extract_gauge_ghost.cu
  extract_gauge_ghost_mg.cu max_gauge.cu gauge_update_quda.cu
  max_clover.cu dirac_clover.cpp dirac_wilson.cpp dirac_staggered.cpp
  dirac_staggered_kd.cpp dirac_clover_hasenbusch_twist.cpp
  dirac_improved_staggered.cpp dirac_improved_staggered_kd.cpp dirac_domain_wall.cpp
  dirac_domain_wall_4d.cpp dirac_mobius.cpp dirac_twisted_clover.cpp
  dirac_twisted_mass.cpp 
  llfat_quda.cu gauge_force.cu gauge_random.cu
  gauge_field_strength_tensor.cu clover_quda.cu dslash_quda.cu
  dslash_gamma_helper.cu dslash_clover_helper.cu
  staggered_kd_build_xinv.cu staggered_kd_apply_xinv.cu
  blas_quda.cu multi_blas_quda.cu reduce_quda.cu
  multi_reduce_quda.cu reduce_helper.cu
  contract.cu comm_common.cpp communicator_stack.cpp
  clover_deriv_quda.cu clover_invert.cu copy_gauge_extended.cu
  extract_gauge_ghost_extended.cu copy_color_spinor.cpp spinor_noise.cu
  copy_color_spinor_dd.cu copy_color_spinor_ds.cu
  copy_color_spinor_dh.cu copy_color_spinor_dq.cu
  copy_color_spinor_ss.cu copy_color_spinor_sd.cu
  copy_color_spinor_sh.cu copy_color_spinor_sq.cu
  copy_color_spinor_hd.cu copy_color_spinor_hs.cu
  copy_color_spinor_hh.cu copy_color_spinor_hq.cu
  copy_color_spinor_qd.cu copy_color_spinor_qs.cu
  copy_color_spinor_qh.cu copy_color_spinor_qq.cu
  copy_color_spinor_mg_dd.cu copy_color_spinor_mg_ds.cu
  copy_color_spinor_mg_sd.cu copy_color_spinor_mg_ss.cu
  copy_color_spinor_mg_sh.cu copy_color_spinor_mg_sq.cu
  copy_color_spinor_mg_hs.cu copy_color_spinor_mg_hh.cu
  copy_color_spinor_mg_hq.cu copy_color_spinor_mg_qs.cu
  copy_color_spinor_mg_qh.cu copy_color_spinor_mg_qq.cu
  copy_gauge_double.cu copy_gauge_single.cu
  copy_gauge_half.cu copy_gauge_quarter.cu
  copy_gauge.cpp copy_gauge_mg.cu copy_clover.cu
  copy_gauge_offset.cu copy_color_spinor_offset.cu copy_clover_offset.cu
  staggered_oprod.cu clover_trace_quda.cu
  hisq_paths_force_quda.cu
  unitarize_force_quda.cu unitarize_links_quda.cu milc_interface.cpp
  blas_magma.cu tune.cpp
  inv_mpcg_quda.cpp inv_mpbicgstab_quda.cpp inv_gmresdr_quda.cpp
  pgauge_exchange.cu pgauge_init.cu pgauge_heatbath.cu random.cu
  gauge_fix_fft.cu gauge_fix_ovr.cu
  pgauge_det_trace.cu clover_outer_product.cu
  clover_sigma_outer_product.cu momentum.cu gauge_qcharge.cu
  deflation.cpp checksum.cu transform_reduce.cu
  dslash_mdw_fused.cu dslash5_mobius_eofa.cu
  instantiate.cpp version.cpp )
# cmake-format: on

# current workaround for nvshmem
set(QUDA_DSLASH_OBJS 
  dslash_staggered.cu dslash_improved_staggered.cu
  dslash_wilson.cu dslash_wilson_clover.cu dslash5_domain_wall.cu
  dslash_wilson_clover_preconditioned.cu 
  dslash_twisted_mass.cu dslash_twisted_mass_preconditioned.cu
  dslash_ndeg_twisted_mass.cu dslash_ndeg_twisted_mass_preconditioned.cu
  dslash_twisted_clover.cu dslash_twisted_clover_preconditioned.cu
  dslash_wilson_clover_hasenbusch_twist.cu
  dslash_wilson_clover_hasenbusch_twist_preconditioned.cu
  dslash_domain_wall_4d.cu  dslash_domain_wall_5d.cu
  dslash_domain_wall_4d_m5inv.cu
  dslash_domain_wall_4d_m5pre.cu
  dslash_domain_wall_4d_m5pre_m5inv.cu
  dslash_domain_wall_4d_m5inv_m5pre.cu
  dslash_domain_wall_4d_m5inv_m5inv.cu
  dslash_domain_wall_4d_m5mob.cu
  dslash_domain_wall_4d_m5pre_m5mob.cu
  dslash_pack2.cu laplace.cu covDev.cu)

if(QUDA_NVSHMEM)
  list(APPEND QUDA_DSLASH_OBJS dslash_constant_arg.cu)
endif()

if(NOT QUDA_NVSHMEM)
  list(APPEND QUDA_OBJS ${QUDA_DSLASH_OBJS})
endif()

# split source into cu and cpp files
foreach(item ${QUDA_OBJS})
  string(REGEX MATCH ".+\\.cu$" item_match ${item})
  if(item_match)
    list(APPEND QUDA_CU_OBJS ${item})
  endif(item_match)
endforeach(item ${QUDA_OBJS})

list(REMOVE_ITEM QUDA_OBJS ${QUDA_CU_OBJS})

if(BUILD_FORTRAN_INTERFACE)
  list(APPEND QUDA_OBJS quda_fortran.F90)
  set_source_files_properties(quda_fortran.F90 PROPERTIES OBJECT_OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/quda_fortran.mod)
endif()

# QUDA_CU_OBJS should contain all cuda files now QUDA_OBJS all c, cpp, fortran sources

# if we have a git version make version.cpp depend on git head so that it is rebuild if the git sha changed
if(${CMAKE_BUILD_TYPE} STREQUAL "DEVEL")
  if(GITVERSION)
    find_path(
      QUDA_GITDIR NAME HEAD
      PATHS ${CMAKE_SOURCE_DIR}/.git/logs
      NO_DEFAULT_PATH)
    include(AddFileDependencies)
    if(QUDA_GITDIR)
      add_file_dependencies(version.cpp ${QUDA_GITDIR}/HEAD)
    endif()
  endif()
  mark_as_advanced(QUDA_GITDIR)
endif()

# generate a cmake object library for all cpp files first
add_library(quda_cpp OBJECT ${QUDA_OBJS})

# add comms and QIO
target_sources(quda_cpp PRIVATE $<IF:$<BOOL:${QUDA_MPI}>,communicator_mpi.cpp,$<IF:$<BOOL:${QUDA_QMP}>,communicator_qmp.cpp,communicator_single.cpp>>)

target_sources(quda_cpp PRIVATE $<$<BOOL:${QUDA_QIO}>:qio_field.cpp layout_hyper.cpp>)

# add some deifnitions that cause issues with cmake 3.7 and nvcc only to cpp files
target_compile_definitions(quda_cpp PUBLIC -DQUDA_HASH="${HASH}")
if(GITVERSION)
  target_compile_definitions(quda_cpp PUBLIC -DGITVERSION="${GITVERSION}")
endif()

# make one library
if(QUDA_BUILD_SHAREDLIB)
  set_target_properties(quda_cpp PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
  add_library(quda SHARED $<TARGET_OBJECTS:quda_cpp> $<$<TARGET_EXISTS:quda_pack>:$<TARGET_OBJECTS:quda_pack>> ${QUDA_CU_OBJS})
  if(CUDAToolkit_FOUND)
    target_link_libraries(quda INTERFACE CUDA::cudart_static)
  endif()
else()
  add_library(quda STATIC $<TARGET_OBJECTS:quda_cpp> $<$<TARGET_EXISTS:quda_pack>:$<TARGET_OBJECTS:quda_pack>> ${QUDA_CU_OBJS})
endif()

include(CheckLinkerFlag)
check_linker_flag(CXX "-Wl,--compress-debug-sections=zlib" QUDA_LINKER_COMPRESS)
target_link_options(quda PRIVATE $<$<CONFIG:DEVEL>:$<${QUDA_LINKER_COMPRESS}:-Wl,--compress-debug-sections=zlib>>)

add_library(QUDA::quda ALIAS quda)

# malloc.cpp uses both the driver and runtime api So we need to find the CUDA_CUDA_LIBRARY (driver api) or the stub
# version for cmake 3.8 and later this has been integrated into  FindCUDALibs.cmake
target_link_libraries(quda PUBLIC ${CUDA_cuda_driver_LIBRARY})

# set up QUDA compile options
target_compile_definitions(
  quda PRIVATE $<$<CONFIG:DEVEL>:DEVEL> $<$<CONFIG:HOSTDEBUG>:HOST_DEBUG>
               $<$<CONFIG:DEBUG>:HOST_DEBUG> $<$<CONFIG:DEBUG>:DEVICE_DEBUG> $<$<CONFIG:SANITIZE>:HOST_DEBUG>)

target_compile_options(
  quda
  PRIVATE $<$<COMPILE_LANG_AND_ID:CUDA,NVIDIA>:
          -ftz=true
          -prec-div=false
          -prec-sqrt=false>
          $<$<COMPILE_LANG_AND_ID:CUDA,NVHPC>:
          -Mflushz
          -Mfpapprox=div
          -Mfpapprox=sqrt>
          $<$<COMPILE_LANG_AND_ID:CUDA,Clang>:
          -fcuda-flush-denormals-to-zero
          -fcuda-approx-transcendentals
          -Xclang -fcuda-allow-variadic-functions>)
target_compile_options(
  quda PRIVATE $<$<COMPILE_LANG_AND_ID:CUDA,Clang>:-Wno-unknown-cuda-version> $<$<COMPILE_LANG_AND_ID:CUDA,NVIDIA>:
               -Wno-deprecated-gpu-targets  --expt-relaxed-constexpr>)

target_compile_options(quda PRIVATE $<$<COMPILE_LANG_AND_ID:CUDA,NVIDIA>: -ftz=true -prec-div=false -prec-sqrt=false>)
target_compile_options(quda PRIVATE $<$<COMPILE_LANG_AND_ID:CUDA,NVIDIA>: -Wno-deprecated-gpu-targets
                                    --expt-relaxed-constexpr>)
target_compile_options(quda PRIVATE $<$<COMPILE_LANG_AND_ID:CUDA,Clang>: --cuda-path=${CUDAToolkit_TARGET_DIR}>)
target_link_options(quda PUBLIC $<$<CUDA_COMPILER_ID:Clang>: --cuda-path=${CUDAToolkit_TARGET_DIR}>)

if(QUDA_VERBOSE_BUILD)
  target_compile_options(quda PRIVATE $<$<COMPILE_LANG_AND_ID:CUDA,NVIDIA>:--ptxas-options=-v>)
endif(QUDA_VERBOSE_BUILD)

if (${CMAKE_CUDA_COMPILER_ID} MATCHES "NVHPC" AND NOT ${CMAKE_BUILD_TYPE} MATCHES "DEBUG")
  target_compile_options(quda PRIVATE "$<$<COMPILE_LANG_AND_ID:CUDA,NVHPC>:SHELL: -gpu=nodebug" >)
endif()


# workaround for 10.2
if(CMAKE_CUDA_COMPILER_ID MATCHES "NVIDIA"
   AND ${CMAKE_CUDA_COMPILER_VERSION} VERSION_GREATER_EQUAL "10.2"
   AND ${CMAKE_CUDA_COMPILER_VERSION} VERSION_LESS "10.3")
  target_compile_options(
    quda PRIVATE "$<$<COMPILE_LANG_AND_ID:CUDA,NVIDIA>:SHELL: -Xcicc \"--Xllc -dag-vectorize-ops=1\" " >)
endif()
target_compile_options(quda PRIVATE $<$<COMPILE_LANG_AND_ID:CUDA,Clang>:--cuda-path=${CUDAToolkit_TARGET_DIR}>)

target_include_directories(quda PRIVATE .)
target_include_directories(quda PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(quda SYSTEM PRIVATE ../include/externals)
target_include_directories(quda SYSTEM PUBLIC $<$<COMPILE_LANGUAGE:CUDA>:${CUDAToolkit_INCLUDE_DIRS}>)
target_link_libraries(quda PRIVATE $<BUILD_INTERFACE:Eigen>)
target_include_directories(quda PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/>
                                       $<INSTALL_INTERFACE:include/>)
target_include_directories(quda PUBLIC $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include> $<INSTALL_INTERFACE:include>)

target_include_directories(quda_cpp SYSTEM PRIVATE ../include/externals)
target_include_directories(quda_cpp SYSTEM PUBLIC ${CUDAToolkit_INCLUDE_DIRS})
target_link_libraries(quda_cpp PRIVATE $<BUILD_INTERFACE:Eigen>)
target_compile_definitions(quda_cpp PRIVATE $<TARGET_PROPERTY:quda,COMPILE_DEFINITIONS>)
target_include_directories(quda_cpp PRIVATE $<TARGET_PROPERTY:quda,INCLUDE_DIRECTORIES>)
target_compile_options(quda_cpp PRIVATE $<TARGET_PROPERTY:quda,COMPILE_OPTIONS>)

# nvshmem enabled parts need CUDA_SEPARABLE_COMPILATION ...
if(QUDA_NVSHMEM)
  add_library(quda_pack OBJECT ${QUDA_DSLASH_OBJS})
  target_include_directories(quda_pack PRIVATE dslash_core)
  target_include_directories(quda_pack SYSTEM PRIVATE ../include/externals)
  target_include_directories(quda_pack PRIVATE .)
  set_target_properties(quda_pack PROPERTIES POSITION_INDEPENDENT_CODE ${QUDA_BUILD_SHAREDLIB})
  target_compile_definitions(quda_pack PRIVATE $<TARGET_PROPERTY:quda,COMPILE_DEFINITIONS>)
  target_include_directories(quda_pack PRIVATE $<TARGET_PROPERTY:quda,INCLUDE_DIRECTORIES>)
  target_compile_options(quda_pack PRIVATE $<TARGET_PROPERTY:quda,COMPILE_OPTIONS>)
  set_target_properties(quda_pack PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
  set_property(TARGET quda PROPERTY CUDA_RESOLVE_DEVICE_SYMBOLS ON)
endif()

# add target specific files
if(${QUDA_TARGET_TYPE} STREQUAL "CUDA")
  add_subdirectory(targets/cuda)
  target_include_directories(quda PRIVATE ../include/targets/cuda)
  target_include_directories(quda PUBLIC $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include/targets/cuda> $<INSTALL_INTERFACE:include/targets/cuda>)
  target_include_directories(quda SYSTEM PRIVATE ../include/targets/cuda/externals)
endif()
if(${QUDA_TARGET_TYPE} STREQUAL "HIP")
  add_subdirectory(targets/hip)
  target_include_directories(quda PRIVATE ../include/targets/hip)
endif()

add_subdirectory(targets/generic)
target_include_directories(quda PRIVATE ../include/targets/generic)

add_subdirectory(interface)

# propagate CXX flags to CUDA host compiler
#TODO: Do we still need that? 
if(${QUDA_PROPAGATE_CXX_FLAGS})

  # Pick the right set of flags Apparently I cannot do this with generator expressions
  if(CMAKE_BUILD_TYPE STREQUAL "DEVEL")
    set(PROPAGATED_FLAGS "${CMAKE_CXX_FLAGS_DEVEL}")
  elseif(CMAKE_BUILD_TYPE STREQUAL "STRICT")
    set(PROPAGATED_FLAGS "${CMAKE_CXX_FLAGS_STRICT}")
  elseif(CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    set(PROPAGATED_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
  elseif(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    set(PROPAGATED_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  elseif(CMAKE_BUILD_TYPE STREQUAL "HOSTDEBUG")
    set(PROPAGATED_FLAGS "${CMAKE_CXX_FLAGS_HOSTDEBUG}")
  elseif(CMAKE_BUILD_TYPE STREQUAL "SANITIZE")
    set(PROPAGATED_FLAGS "${CMAKE_CXX_FLAGS_SANITIZE}")
  endif()

  # Turne the flags into a CMAKE list
  string(REPLACE " " ";" PROPAGATED_FLAG_LIST ${PROPAGATED_FLAGS})

  foreach(FLAG ${PROPAGATED_FLAG_LIST})
    target_compile_options(quda PRIVATE $<$<COMPILE_LANG_AND_ID:CUDA,NVIDIA>: -Xcompiler=${FLAG}>)
  endforeach()
endif()

# Specific config dependent warning suppressions and lineinfo forwarding
target_compile_options(quda PRIVATE $<$<COMPILE_LANG_AND_ID:CUDA,NVIDIA>:
                            -Wreorder
                            $<$<CXX_COMPILER_ID:Clang>: -Xcompiler=-Wno-unused-function -Xcompiler=-Wno-unknown-pragmas>
                            $<$<CXX_COMPILER_ID:GNU>: -Xcompiler=-Wno-unknown-pragmas>
                            $<$<CONFIG:DEVEL>:-Xptxas -warn-lmem-usage,-warn-spills -lineinfo>
                            $<$<CONFIG:STRICT>: -Werror=all-warnings -lineinfo>
                            $<$<CONFIG:HOSTDEBUG>:-lineinfo>
                            $<$<CONFIG:SANITIZE>:-lineinfo>
                            >)

target_compile_options(quda PRIVATE $<$<COMPILE_LANG_AND_ID:CUDA,NVHPC>:
                            -gpu=lineinfo
                            >)

target_compile_options(quda PRIVATE $<$<COMPILE_LANG_AND_ID:CUDA,Clang>:
                            -Wall -Wextra -Wno-unknown-pragmas
                            $<$<CONFIG:STRICT>:-Werror -Wno-error=pass-failed>
                            $<$<CONFIG:SANITIZE>:-fsanitize=address -fsanitize=undefined>
          >)

# some clang warnings should be warning even when turning warnings into errors
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(quda_cpp PUBLIC $<$<COMPILE_LANGUAGE:CXX>: -Wno-error=unused-function>)

  # this is a hack to get colored diagnostics back when using Ninja and clang
  if(CMAKE_GENERATOR MATCHES "Ninja")
    target_compile_options(quda PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-fcolor-diagnostics>)
  endif()
endif()

# QUDA FEATURES
if(QUDA_DIRAC_WILSON)
  target_compile_definitions(quda PUBLIC GPU_WILSON_DIRAC)
endif(QUDA_DIRAC_WILSON)

if(QUDA_DIRAC_DOMAIN_WALL)
  target_compile_definitions(quda PUBLIC GPU_DOMAIN_WALL_DIRAC)
endif(QUDA_DIRAC_DOMAIN_WALL)

if(QUDA_DIRAC_STAGGERED)
  target_compile_definitions(quda PUBLIC GPU_STAGGERED_DIRAC GPU_FATLINK GPU_UNITARIZE)
endif(QUDA_DIRAC_STAGGERED)

if(QUDA_DIRAC_CLOVER)
  target_compile_definitions(quda PUBLIC GPU_CLOVER_DIRAC GPU_WILSON_DIRAC GPU_GAUGE_TOOLS)
endif(QUDA_DIRAC_CLOVER)

if(QUDA_DIRAC_TWISTED_MASS)
  target_compile_definitions(quda PUBLIC GPU_TWISTED_MASS_DIRAC GPU_WILSON_DIRAC)
endif(QUDA_DIRAC_TWISTED_MASS)

if(QUDA_DIRAC_TWISTED_CLOVER)
  target_compile_definitions(quda PUBLIC GPU_TWISTED_CLOVER_DIRAC GPU_CLOVER_DIRAC GPU_TWISTED_MASS_DIRAC
                                         GPU_WILSON_DIRAC GPU_GAUGE_TOOLS)
endif(QUDA_DIRAC_TWISTED_CLOVER)

if(QUDA_DIRAC_CLOVER_HASENBUSCH)
  target_compile_definitions(quda PUBLIC GPU_CLOVER_HASENBUSCH_TWIST GPU_TWISTED_CLOVER_DIRAC GPU_CLOVER_DIRAC
                                         GPU_WILSON_DIRAC GPU_GAUGE_TOOLS)
endif(QUDA_DIRAC_CLOVER_HASENBUSCH)

if(QUDA_DIRAC_NDEG_TWISTED_MASS)
  target_compile_definitions(quda PUBLIC GPU_NDEG_TWISTED_MASS_DIRAC)
endif(QUDA_DIRAC_NDEG_TWISTED_MASS)

if(${QUDA_BUILD_NATIVE_LAPACK} STREQUAL "ON")
  target_link_libraries(quda PUBLIC ${CUDA_cublas_LIBRARY})
  target_compile_definitions(quda PRIVATE NATIVE_LAPACK_LIB)
endif()

if(QUDA_MULTIGRID)
  target_compile_definitions(quda PRIVATE GPU_MULTIGRID)
endif(QUDA_MULTIGRID)

if(QUDA_GAUGE_ALG)
  target_compile_definitions(quda PUBLIC GPU_GAUGE_ALG GPU_GAUGE_TOOLS GPU_UNITARIZE)
  target_link_libraries(quda PUBLIC ${CUDA_cufft_LIBRARY})
endif(QUDA_GAUGE_ALG)

if(QUDA_SSTEP)
  target_compile_definitions(quda PRIVATE SSTEP)
endif()

if(QUDA_BLOCKSOLVER)
  target_compile_definitions(quda PRIVATE BLOCKSOLVER)
endif()

if(QUDA_FORCE_GAUGE)
  target_compile_definitions(quda PUBLIC GPU_GAUGE_FORCE GPU_GAUGE_TOOLS)
endif(QUDA_FORCE_GAUGE)

if(QUDA_FORCE_HISQ)
  target_compile_definitions(quda PUBLIC GPU_HISQ_FORCE GPU_STAGGERED_OPROD GPU_GAUGE_TOOLS)
endif(QUDA_FORCE_HISQ)

if(QUDA_GAUGE_TOOLS)
  target_compile_definitions(quda PUBLIC GPU_GAUGE_TOOLS)
endif(QUDA_GAUGE_TOOLS)

if(QUDA_GAUGE_ALG)
  target_compile_definitions(quda PUBLIC GPU_GAUGE_ALG GPU_GAUGE_TOOLS GPU_UNITARIZE)
endif(QUDA_GAUGE_ALG)

if(QUDA_INTERFACE_QDP OR QUDA_INTERFACE_ALL)
  target_compile_definitions(quda PUBLIC BUILD_QDP_INTERFACE)
endif(QUDA_INTERFACE_QDP OR QUDA_INTERFACE_ALL)

if(QUDA_INTERFACE_MILC OR QUDA_INTERFACE_ALL)
  target_compile_definitions(quda PUBLIC BUILD_MILC_INTERFACE)
endif(QUDA_INTERFACE_MILC OR QUDA_INTERFACE_ALL)

if(QUDA_INTERFACE_CPS OR QUDA_INTERFACE_ALL)
  target_compile_definitions(quda PUBLIC BUILD_CPS_INTERFACE)
endif(QUDA_INTERFACE_CPS OR QUDA_INTERFACE_ALL)

if(QUDA_INTERFACE_QDPJIT OR QUDA_INTERFACE_ALL)
  target_compile_definitions(quda PUBLIC BUILD_QDPJIT_INTERFACE)
endif(QUDA_INTERFACE_QDPJIT OR QUDA_INTERFACE_ALL)

if(QUDA_INTERFACE_BQCD OR QUDA_INTERFACE_ALL)
  target_compile_definitions(quda PUBLIC BUILD_BQCD_INTERFACE)
endif(QUDA_INTERFACE_BQCD OR QUDA_INTERFACE_ALL)

if(QUDA_INTERFACE_TIFR OR QUDA_INTERFACE_ALL)
  target_compile_definitions(quda PUBLIC BUILD_TIFR_INTERFACE)
endif(QUDA_INTERFACE_TIFR OR QUDA_INTERFACE_ALL)

if(QUDA_CONTRACT)
  target_compile_definitions(quda PUBLIC GPU_CONTRACT)
endif(QUDA_CONTRACT)

if(QUDA_COVDEV)
  target_compile_definitions(quda PUBLIC GPU_COVDEV)
endif(QUDA_COVDEV)

if(QUDA_LAPLACE)
  target_compile_definitions(quda PUBLIC GPU_LAPLACE)
endif(QUDA_LAPLACE)

# MULTI GPU AND USQCD
if(QUDA_MPI OR QUDA_QMP)
  target_compile_definitions(quda PUBLIC MULTI_GPU)
endif()

if(QUDA_MPI)
  target_compile_definitions(quda PUBLIC MPI_COMMS)
  target_link_libraries(quda PUBLIC MPI::MPI_CXX)
endif()

if(QUDA_QMP)
  target_compile_definitions(quda PUBLIC QMP_COMMS)
  target_link_libraries(quda PUBLIC QMP::qmp)
  target_link_libraries(quda PUBLIC MPI::MPI_CXX)
endif()

if(QUDA_NVSHMEM)
  target_link_libraries(quda PUBLIC MPI::MPI_C)
  target_compile_definitions(quda PUBLIC NVSHMEM_COMMS)
  if(QUDA_DOWNLOAD_NVSHMEM)
    add_dependencies(quda NVSHMEM)
    add_dependencies(quda_cpp NVSHMEM)
    add_dependencies(quda_pack NVSHMEM)
  endif()
  get_filename_component(NVSHMEM_LIBPATH ${NVSHMEM_LIBS} DIRECTORY)
  target_link_libraries(quda PUBLIC -L${NVSHMEM_LIBPATH} -lnvshmem)
  target_include_directories(quda SYSTEM PUBLIC $<BUILD_INTERFACE:${NVSHMEM_INCLUDE}>)
endif()

if(QUDA_QIO)
  target_compile_definitions(quda PUBLIC HAVE_QIO)
  target_link_libraries(quda PUBLIC QIO::qio)
endif()

if(QUDA_QDPJIT)
  target_compile_definitions(quda PUBLIC USE_QDPJIT)
  target_include_directories(quda SYSTEM PUBLIC $<BUILD_INTERFACE:${QUDA_QDPJITHOME}/include>)
  target_link_libraries(
    quda
    INTERFACE ${QDP_LDFLAGS}
              ${QDP_LIB}
              ${QDP_LIBS}
              ${QIO_LIB}
              ${LIME_LIB}
              ${QUDA_QMP_LDFLAGS}
              ${QMP_LIB}
              MPI::MPI_CXX)
endif()

if(QUDA_ARPACK)
  target_compile_definitions(quda PRIVATE ARPACK_LIB)
  if(QUDA_ARPACK_LOGGING)
    # ARPACK-NG does not suppport logging - we must warn the user
    message(
      WARNING
        "Specifying QUDA_ARPACK_LOGGING with ARPACK-NG package will cause link failures. Please ensure that QUDA_ARPACK_LOGGING=OFF if downloading ARPACK-NG or using system installed ARPACK-NG"
    )
    target_compile_definitions(ARPACK_LOGGING)
  endif()
  if(QUDA_DOWNLOAD_ARPACK)
    target_link_libraries(quda PUBLIC arpack-ng)
    target_link_libraries(quda_cpp PUBLIC arpack-ng)
    if(QUDA_MPI OR QUDA_QMP)
      target_link_libraries(quda PUBLIC parpack-ng)
      target_link_libraries(quda_cpp PUBLIC parpack-ng)
    endif()
  else()
    target_link_libraries(quda INTERFACE ${ARPACK})
    if(QUDA_MPI OR QUDA_QMP)
      target_link_libraries(quda INTERFACE ${PARPACK} MPI::MPI_Fortran)
    endif()
  endif()
endif()

if(QUDA_OPENBLAS)
  target_compile_definitions(quda PRIVATE OPENBLAS_LIB)
  
  if(QUDA_DOWNLOAD_OPENBLAS)
    target_link_libraries(quda PUBLIC openblas)
    target_link_libraries(quda_cpp PUBLIC openblas)
  else()
    target_link_libraries(quda INTERFACE ${OPENBLAS})
  endif()
endif()

if(QUDA_USE_EIGEN)
  target_compile_definitions(quda PRIVATE EIGEN)
endif()

if(QUDA_OPENMP)
  target_link_libraries(quda PUBLIC OpenMP::OpenMP_CXX)
endif()

if(QUDA_MAGMA)
  target_link_libraries(quda PUBLIC MAGMA::MAGMA)
endif()

# set which precisions to enable
target_compile_definitions(quda PUBLIC QUDA_PRECISION=${QUDA_PRECISION})
target_compile_definitions(quda PUBLIC QUDA_RECONSTRUCT=${QUDA_RECONSTRUCT})

if(QUDA_FAST_COMPILE_REDUCE)
  target_compile_definitions(quda PRIVATE QUDA_FAST_COMPILE_REDUCE)
endif()

if(QUDA_FAST_COMPILE_DSLASH)
  target_compile_definitions(quda PRIVATE QUDA_FAST_COMPILE_DSLASH)
endif()

if(QUDA_JITIFY)
  target_compile_definitions(quda PRIVATE JITIFY)
  find_package(LibDL)
  target_link_libraries(quda PUBLIC ${CUDA_nvrtc_LIBRARY})
  target_link_libraries(quda PUBLIC ${LIBDL_LIBRARIES})
  target_include_directories(quda PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)
endif()

if(QUDA_MPI_NVTX)
  target_sources(quda_cpp PRIVATE nvtx_pmpi.c)
  set(QUDA_NVTX ON)
endif(QUDA_MPI_NVTX)

if(QUDA_INTERFACE_NVTX)
  target_compile_definitions(quda PRIVATE INTERFACE_NVTX)
  set(QUDA_NVTX ON)
endif(QUDA_INTERFACE_NVTX)

if(QUDA_NVTX)
  find_path(NVTX3 "nvtx3/nvToolsExt.h" PATHS ${CUDA_TOOLKIT_INCLUDE} NO_DEFAULT_PATH)
  if(NVTX3)
    target_compile_definitions(quda PRIVATE QUDA_NVTX_VERSION=3)
  else()
    target_link_libraries(quda PUBLIC ${CUDA_nvToolsExt_LIBRARY})
  endif(NVTX3)
endif(QUDA_NVTX)

if(QUDA_BACKWARDS)
  target_include_directories(quda_cpp SYSTEM PRIVATE ${backward-cpp_SOURCE_DIR})
  set_property(
    SOURCE comm_common.cpp
    APPEND
    PROPERTY COMPILE_DEFINITIONS ${BACKWARD_DEFINITIONS})
  set_property(SOURCE comm_common.cpp APPEND PROPERTY COMPILE_DEFINITIONS QUDA_BACKWARDSCPP)
  target_link_libraries(quda PUBLIC ${BACKWARD_LIBRARIES})
endif()

if(QUDA_NUMA_NVML)
  target_compile_definitions(quda PRIVATE NUMA_NVML)
  target_sources(quda_cpp PRIVATE numa_affinity.cpp)
  find_package(NVML REQUIRED)
  target_include_directories(quda PRIVATE SYSTEM NVML_INCLUDE_DIR)
  target_link_libraries(quda PUBLIC ${NVML_LIBRARY})
endif(QUDA_NUMA_NVML)

if(QUDA_NVML)
  target_link_libraries(quda PUBLIC ${NVML_LIBRARY})
endif()

configure_file(../include/quda_define.h.in ../include/quda_define.h @ONLY)
install(FILES "${CMAKE_BINARY_DIR}/include/quda_define.h" DESTINATION include/)

if(QUDA_JITIFY)
  configure_file(../include/targets/cuda/jitify_options.hpp.in ../include/targets/cuda/jitify_options.hpp)
  install(FILES "${CMAKE_BINARY_DIR}/include/targets/cuda/jitify_options.hpp" DESTINATION include/)
endif()

# until we define an install step copy the include directory to the build directory
add_custom_command(
  TARGET quda
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include)

# some hackery to prevent having old shared / static builds of quda messing with the current build
add_custom_command(
  TARGET quda
  PRE_LINK
  COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/libquda.a ${CMAKE_CURRENT_BINARY_DIR}/libquda.so)

set_target_properties(quda PROPERTIES INSTALL_RPATH_USE_LINK_PATH ON )

install(
  TARGETS quda
  EXPORT QUDATargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include)

install(EXPORT QUDATargets
	FILE QUDATargets.cmake
	NAMESPACE QUDA::
	DESTINATION lib/cmake/QUDA)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/qudaConfigVersion.cmake"
  VERSION ${QUDA_VERSION}
  COMPATIBILITY AnyNewerVersion)
