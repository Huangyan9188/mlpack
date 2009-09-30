/* MLPACK 0.1
 *
 * Copyright (c) 2008 Alexander Gray,
 *                    Garry Boyer,
 *                    Ryan Riegel,
 *                    Nikolaos Vasiloglou,
 *                    Dongryeol Lee,
 *                    Chip Mappus, 
 *                    Nishant Mehta,
 *                    Hua Ouyang,
 *                    Parikshit Ram,
 *                    Long Tran,
 *                    Wee Chin Wong
 *
 * Copyright (c) 2008 Georgia Institute of Technology
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
/**
 * @author Parikshit Ram (pram@cc.gatech.edu)
 * @file phi.h
 *
 * This file computes the Gaussian probability
 * density function
 */
#include <cmath>
#include "fastlib/fastlib.h"
//#include "fastlib/fastlib_int.h"

/**
 * Calculates the multivariate Gaussian probability density function
 * 
 * Example use:
 * @code
 * Vector x, mean;
 * Matrix cov;
 * ....
 * long double f = phi(x, mean, cov);
 * @endcode
 */

long double phi(Vector& x , Vector& mean , Matrix& cov) {
	
  long double det, f;
  double exponent;
  index_t dim;
  Matrix inv;
  Vector diff, tmp;
	
    dim = x.length();
  la::InverseInit(cov, &inv);
  det = la::Determinant(cov);
  
  if( det < 0){
    det = -det;
  }
  la::SubInit(mean,x,&diff);
  la::MulInit(inv, diff, &tmp);
  exponent = la::Dot(diff, tmp);
  long double tmp1, tmp2, tmp3;
  tmp1 = 1;
  tmp2 = dim;
  tmp2 = tmp2/2;
  tmp2 = pow((2*(math::PI)),tmp2);
  tmp1 = tmp1/tmp2;
  tmp3 = 1;
  tmp2 = sqrt(det);
  tmp3 = tmp3/tmp2;
  tmp2 = -exponent;
  tmp2 = tmp2 / 2;
  f = (tmp1*tmp3*exp(tmp2));

  return f;
}

/**
 * Calculates the univariate Gaussian probability density function
 * 
 * Example use:
 * @code
 * double x, mean, var;
 * ....
 * long double f = phi(x, mean, var);
 * @endcode
 */

long double phi(double x, double mean, double var) {

  long double f;
	
  f = exp(-1.0*((x-mean)*(x-mean)/(2*var)))/sqrt(2*math::PI*var);
  return f;
}

/**
 * Calculates the multivariate Gaussian probability density function 
 * and also the gradients with respect to the mean and the variance
 *
 * Example use:
 * @code
 * Vector x, mean, g_mean, g_cov;
 * ArrayList<Matrix> d_cov; // the dSigma
 * ....
 * long double f = phi(x, mean, cov, d_cov, &g_mean, &g_cov);
 * @endcode
 */

long double phi(Vector& x, Vector& mean, Matrix& cov, ArrayList<Matrix>& d_cov, Vector *g_mean, Vector *g_cov){
	
  long double det, f;
  double exponent;
  index_t dim;
  Matrix inv;
  Vector diff, tmp;
	
  dim = x.length();
  la::InverseInit(cov, &inv);
  det = la::Determinant(cov);

  if( det < 0){
    det = -det;
  }
  la::SubInit(mean,x,&diff);
  la::MulInit(inv, diff, &tmp);
  exponent = la::Dot(diff, tmp);
  long double tmp1, tmp2, tmp3;
  tmp1 = 1;
  tmp2 = dim;
  tmp2 = tmp2/2;
  tmp2 = pow((2*(math::PI)),tmp2);
  tmp1 = tmp1/tmp2;
  tmp3 = 1;
  tmp2 = sqrt(det);
  tmp3 = tmp3/tmp2;
  tmp2 = -exponent;
  tmp2 = tmp2 / 2;
  f = (tmp1*tmp3*exp(tmp2));

  // Calculating the g_mean values  which would be a (1 X dim) vector
  la::ScaleInit(f,tmp,g_mean);
	
  // Calculating the g_cov values which would be a (1 X (dim*(dim+1)/2)) vector
  double *g_cov_tmp;
  g_cov_tmp = (double*)malloc(d_cov.size()*sizeof(double));
  for(index_t i = 0; i < d_cov.size(); i++){
    Vector tmp_d;
    Matrix inv_d;
    long double tmp_d_cov_d_r;
		
    la::MulInit(d_cov[i],tmp,&tmp_d);
    tmp_d_cov_d_r = la::Dot(tmp_d,tmp);
    la::MulInit(inv,d_cov[i],&inv_d);
    for(index_t j = 0; j < dim; j++)
      tmp_d_cov_d_r += inv_d.get(j,j);
    g_cov_tmp[i] = f*tmp_d_cov_d_r/2;
  }
  g_cov->Copy(g_cov_tmp,d_cov.size());
	
  return f;
}
