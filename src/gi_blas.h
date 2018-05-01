/*
 *  OpenGI: Library for Parameterization and Geometry Image creation
 *  Copyright (C) 2008-2011  Christian Rau
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published 
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Christian Rau
 *
 *     rauy@users.sourceforge.net
 */

/** \internal
 *  \file
 *  \brief Declaration of common BLAS functions.
 */

#ifndef __GI_BLAS_H__
#define __GI_BLAS_H__

#if HAVE_CONFIG_H
	#include <config.h>
#endif


/*************************************************************************/
/* Functions */

/** \name Level 1 BLAS routines
 *  \{
 */
void drotg(double *a, double *b, double *c, double *s);
void drot(int n, double *x, int incx, double *y, int incy, double c, double s);
void dswap(int n, double *x, int incx, double *y, int incy);
void dscal(int n, double a, double *x, int incx);
void dcopy(int n, const double *x, int incx, double *y, int incy);
void daxpy(int n, double a, const double *x, int incx, double *y, int incy);
double ddot(int n, const double *x, int incx, const double *y, int incy);
double dnrm2(int n, const double *x, int incx);
double dasum(int n, const double *x, int incx);
int idamax(int n, const double *x, int incx);
/** \} */

/** \name Level 2 BLAS routines
 *  \{
 */
void dgemv(char trans, int m, int n, double alpha, const double *A, int lda, 
	const double *x, int incx, double beta, double *y, int incy);
void dsymv(char uplo, int n, double alpha, const double *A, int lda, 
	const double *x, int incx, double beta, double *y, int incy);
void dger(int m, int n, double alpha, const double *x, int incx, 
	const double *y, int incy, double *A, int lda);
void dsyr(char uplo, int n, double alpha, 
	const double *x, int incx, double *A, int lda);
void dsyr2(char uplo, int n, double alpha, const double *x, 
	int incx, const double *y, int incy, double *A, int lda);
void dtpsv(char uplo, char trans, char diag, int n, 
	const double *A, double *x, int incx);
/** \} */

/** \name Level 3 BLAS routines
 *  \{
 */
void dgemm(char transa, char transb, int m, int n, int k, 
	double alpha, const double *A, int lda, const double *B, 
	int ldb, double beta, double *C, int ldc);
/** \} */


#endif
