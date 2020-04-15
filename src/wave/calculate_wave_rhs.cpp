//========================================================================================
// Athena++ astrophysical MHD code
// Copyright(C) 2014 James M. Stone <jmstone@princeton.edu> and other code contributors
// Licensed under the 3-clause BSD License, see LICENSE file for details
//========================================================================================
//! \file calculate_wave_rhs.cpp
//  \brief Calculate wave equation RHS

// Athena++ headers
#include "../athena.hpp"
#include "../athena_arrays.hpp"
#include "../coordinates/coordinates.hpp"
#include "../mesh/mesh.hpp"
#include "wave.hpp"

//! \fn void Wave::WaveRHS
//  \brief Calculate RHS for the wave equation using finite-differencing
void Wave::WaveRHS(AthenaArray<Real> & u){
  MeshBlock *pmb = pmy_block;

  AthenaArray<Real> wu, wpi;
  // internal dimension inferred
  wu.InitWithShallowSlice(u, 0, 1);
  wpi.InitWithShallowSlice(u, 1, 1);

  Real c_2 = SQR(c);

  for(int k = mbi.kl; k <= mbi.ku; ++k) {
    for(int j = mbi.jl; j <= mbi.ju; ++j) {
#pragma omp simd
      for(int i = mbi.il; i <= mbi.iu; ++i) {
        rhs(0,k,j,i) = wpi(k,j,i);
        rhs(1,k,j,i) = 0.0;
      }
      for(int a = 0; a < 3; ++a) {
#pragma omp simd
        for(int i = mbi.il; i <= mbi.iu; ++i) {
          rhs(1,k,j,i) += c_2 * FD.Dxx(a, wu(k,j,i));
        }
      }
    }
  }
}

//! \fn void Wave:WaveBoundaryRHS
//   \brief Calculate the boundary RHS
void Wave::WaveBoundaryRHS(AthenaArray<Real> & u){
  MeshBlock * pmb = pmy_block;

  if (use_Dirichlet) {

    if(pmb->pbval->block_bcs[BoundaryFace::inner_x1] == BoundaryFlag::extrapolate_outflow ||
       pmb->pbval->block_bcs[BoundaryFace::inner_x1] == BoundaryFlag::outflow)
        WaveBoundaryDirichlet_(u, mbi.il, mbi.il, mbi.jl, mbi.ju, mbi.kl, mbi.ku);
    if(pmb->pbval->block_bcs[BoundaryFace::outer_x1] == BoundaryFlag::extrapolate_outflow ||
       pmb->pbval->block_bcs[BoundaryFace::outer_x1] == BoundaryFlag::outflow)
        WaveBoundaryDirichlet_(u, mbi.iu, mbi.iu, mbi.jl, mbi.ju, mbi.kl, mbi.ku);

    if(pmb->pbval->block_bcs[BoundaryFace::inner_x2] == BoundaryFlag::extrapolate_outflow ||
       pmb->pbval->block_bcs[BoundaryFace::inner_x2] == BoundaryFlag::outflow)
        WaveBoundaryDirichlet_(u, mbi.il, mbi.iu, mbi.jl, mbi.jl, mbi.kl, mbi.ku);
    if(pmb->pbval->block_bcs[BoundaryFace::outer_x2] == BoundaryFlag::extrapolate_outflow ||
       pmb->pbval->block_bcs[BoundaryFace::outer_x2] == BoundaryFlag::outflow)
        WaveBoundaryDirichlet_(u, mbi.il, mbi.iu, mbi.ju, mbi.ju, mbi.kl, mbi.ku);

    if(pmb->pbval->block_bcs[BoundaryFace::inner_x3] == BoundaryFlag::extrapolate_outflow ||
       pmb->pbval->block_bcs[BoundaryFace::inner_x3] == BoundaryFlag::outflow)
        WaveBoundaryDirichlet_(u, mbi.il, mbi.iu, mbi.jl, mbi.ju, mbi.kl, mbi.kl);
    if(pmb->pbval->block_bcs[BoundaryFace::outer_x3] == BoundaryFlag::extrapolate_outflow ||
       pmb->pbval->block_bcs[BoundaryFace::outer_x3] == BoundaryFlag::outflow)
        WaveBoundaryDirichlet_(u, mbi.il, mbi.iu, mbi.jl, mbi.ju, mbi.ku, mbi.ku);

  } else if (use_Sommerfeld) {
    //..
  } else {
    return;
  }

  return;

  // the appropriate condition to apply depends on dimension
  // an assumption is made that any present Sommerfeld conditions are order as
  // x1, x2, ...

  if(pmb->pbval->block_bcs[BoundaryFace::inner_x1] == BoundaryFlag::extrapolate_outflow ||
     pmb->pbval->block_bcs[BoundaryFace::inner_x1] == BoundaryFlag::outflow) {
    switch(pmb->pmy_mesh->ndim) {
    case 1:
      WaveSommerfeld_1d_L_(u);
      break;
    case 2:
      WaveSommerfeld_2d_(u);
      break;
    case 3:
      WaveSommerfeld_3d_(u);
      break;
    }
  }
  if(pmb->pbval->block_bcs[BoundaryFace::outer_x1] == BoundaryFlag::extrapolate_outflow ||
     pmb->pbval->block_bcs[BoundaryFace::outer_x1] == BoundaryFlag::outflow) {
    switch(pmb->pmy_mesh->ndim) {
    case 1:
      WaveSommerfeld_1d_R_(u);
      break;
    case 2:
      WaveSommerfeld_2d_(u);
      break;
    case 3:
      WaveSommerfeld_3d_(u);
      break;
    }
  }
  if(pmb->pbval->block_bcs[BoundaryFace::inner_x2] == BoundaryFlag::extrapolate_outflow ||
     pmb->pbval->block_bcs[BoundaryFace::inner_x2] == BoundaryFlag::outflow) {
    switch(pmb->pmy_mesh->ndim) {
    case 2:
      WaveSommerfeld_2d_(u);
      break;
    case 3:
      WaveSommerfeld_3d_(u);
      break;
    }
  }
  if(pmb->pbval->block_bcs[BoundaryFace::outer_x2] == BoundaryFlag::extrapolate_outflow ||
     pmb->pbval->block_bcs[BoundaryFace::outer_x2] == BoundaryFlag::outflow) {
    switch(pmb->pmy_mesh->ndim) {
    case 2:
      WaveSommerfeld_2d_(u);
      break;
    case 3:
      WaveSommerfeld_3d_(u);
      break;
    }
  }
  if(pmb->pbval->block_bcs[BoundaryFace::inner_x3] == BoundaryFlag::extrapolate_outflow ||
     pmb->pbval->block_bcs[BoundaryFace::inner_x3] == BoundaryFlag::outflow) {
    if(pmb->pmy_mesh->ndim == 3)
      WaveSommerfeld_3d_(u);
  }
  if(pmb->pbval->block_bcs[BoundaryFace::outer_x3] == BoundaryFlag::extrapolate_outflow ||
     pmb->pbval->block_bcs[BoundaryFace::outer_x3] == BoundaryFlag::outflow) {
    if(pmb->pmy_mesh->ndim == 3)
      WaveSommerfeld_3d_(u);
  }


  // coutBoldGreen("wu:\n");
  // AthenaArray<Real> wu;
  // wu.InitWithShallowSlice(u, 0, 1);
  // wu.print_all("%1.10f");
  // printf("\n");

  // coutBoldGreen("wpi:\n");

  // AthenaArray<Real> wpi;
  // wpi.InitWithShallowSlice(u, 1, 1);
  // wpi.print_all("%1.10f");
  // printf("\n");


  // coutBoldGreen("rhs(0,:,:,:):\n");
  // AthenaArray<Real> rh_0;
  // rh_0.InitWithShallowSlice(rhs, 0, 1);
  // rh_0.print_all("%1.10f");
  // printf("\n");

  // coutBoldGreen("rhs(1,:,:,:):\n");
  // AthenaArray<Real> rhs_1;
  // rhs_1.InitWithShallowSlice(rhs, 1, 1);
  // rhs_1.print_all("%1.10f");
  // printf("\n");

}

void Wave::WaveBoundaryDirichlet_(AthenaArray<Real> & u, int il, int iu,
                                  int jl, int ju, int kl, int ku) {
  for(int k = kl; k <= ku; ++k)
    for(int j = jl; j <= ju; ++j)
#pragma omp simd
      for(int i = il; i <= iu; ++i) {
        rhs(0,k,j,i) = 0.;
        rhs(1,k,j,i) = 0.;
        // rhs(0,k,j,i) = std::numeric_limits<double>::quiet_NaN();
        // rhs(1,k,j,i) = std::numeric_limits<double>::quiet_NaN();
      }
}

void Wave::WaveSommerfeld_1d_L_(AthenaArray<Real> & u){
  // For u_tt = c^2 (u_xx1 + .. + u_xxd) with r = sqrt(xx1 ^ 2 + ... + xxd ^2)
  // Sommerfeld conditions should go like:
  // lim r^((d-1)/2)(u_r + 1/c u_t)
  //
  // In particular, in 1d, these are exact.

  AthenaArray<Real> wpi;
  wpi.InitWithShallowSlice(u, 1, 1);

  for(int k = mbi.kl; k <= mbi.ku; ++k)
    for(int j = mbi.jl; j <= mbi.ju; ++j)
#pragma omp simd
      for(int i = mbi.il; i <= mbi.il; ++i) {
        rhs(1,k,j,i) = c * FD.Ds(0, wpi(k,j,i));
      }
}


void Wave::WaveSommerfeld_1d_R_(AthenaArray<Real> & u){
  // For u_tt = c^2 (u_xx1 + .. + u_xxd) with r = sqrt(xx1 ^ 2 + ... + xxd ^2)
  // Sommerfeld conditions should go like:
  // lim r^((d-1)/2)(u_r + 1/c u_t)
  //
  // In particular, in 1d, these are exact.

  AthenaArray<Real> wpi;
  wpi.InitWithShallowSlice(u, 1, 1);

  for(int k = mbi.kl; k <= mbi.ku; ++k)
    for(int j = mbi.jl; j <= mbi.ju; ++j)
#pragma omp simd
      for(int i = mbi.iu; i <= mbi.iu; ++i) {
        rhs(1,k,j,i) = -c * FD.Ds(0, wpi(k,j,i));
      }

}

void Wave::WaveSommerfeld_2d_(AthenaArray<Real> & u){
  // For u_tt = c^2 (u_xx1 + .. + u_xxd) with r = sqrt(xx1 ^ 2 + ... + xxd ^2)
  // Sommerfeld conditions should go like:
  // lim r^((d-1)/2)(u_r + 1/c u_t)

  AthenaArray<Real> wpi;
  wpi.InitWithShallowSlice(u, 1, 1);

  for(int k = mbi.kl; k <= mbi.ku; ++k) {
    for(int j = mbi.jl; j <= mbi.ju; ++j) {
#pragma omp simd
      for(int i = mbi.il; i <= mbi.iu; ++i) {
        // Derivatives of pi
        Real const wpi_x = FD.Ds(0, wpi(k,j,i));
        Real const wpi_y = FD.Ds(1, wpi(k,j,i));

        Real const rr = std::sqrt(SQR(mbi.x1(i)) + SQR(mbi.x2(j)));
        Real const sx = mbi.x1(i);
        Real const sy = mbi.x2(j);

        rhs(1,k,j,i) = -c * (wpi(k,j,i)/rr + 2. * (sx*wpi_x + sy*wpi_y)) / 2.;
      }
    }
  }
}


void Wave::WaveSommerfeld_3d_(AthenaArray<Real> & u){
  // For u_tt = c^2 (u_xx1 + .. + u_xxd) with r = sqrt(xx1 ^ 2 + ... + xxd ^2)
  // Sommerfeld conditions should go like:
  // lim r^((d-1)/2)(u_r + 1/c u_t)

  AthenaArray<Real> wpi;
  wpi.InitWithShallowSlice(u, 1, 1);

  for(int k = mbi.kl; k <= mbi.ku; ++k) {
    for(int j = mbi.jl; j <= mbi.ju; ++j) {
#pragma omp simd
      for(int i = mbi.il; i <= mbi.iu; ++i) {
        // Derivatives of pi
        Real const wpi_x = FD.Ds(0, wpi(k,j,i));
        Real const wpi_y = FD.Ds(1, wpi(k,j,i));
        Real const wpi_z = FD.Ds(2, wpi(k,j,i));

        Real const rr = std::sqrt(SQR(mbi.x1(i)) +
                                  SQR(mbi.x2(j)) + SQR(mbi.x3(k)));
        Real const sx = mbi.x1(i)/rr;
        Real const sy = mbi.x2(j)/rr;
        Real const sz = mbi.x3(k)/rr;

        rhs(1,k,j,i) = -c * (wpi(k,j,i)/rr + sx*wpi_x + sy*wpi_y + sz*wpi_z);
      }
    }
  }
}
