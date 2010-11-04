#ifndef __H__UG__SMALL_ALGEBRA__DENSEMATRIX_INVERT_H__
#define __H__UG__SMALL_ALGEBRA__DENSEMATRIX_INVERT_H__

#include "../small_matrix/densematrix.h"
#include "../small_matrix/densevector.h"
#include "../../common/operations.h"

namespace ug{


///////////////////////////////////////////////////////////////////////////////////////
//!
//! smallInverse<size_t n>
//! A class to hold a inverse of a smallMatrix<n>
//! implemented with LAPACKs LU-Decomposition dgetrf
//! (uses double[n*n] for LU and interchange[n] for pivoting
//! functions:
//! setAsInverseOf(const blockDenseMatrix<n> &mat) : init as inverse of mat
//! blockVector<n> * smallInverse<n> = smallInverse<n> * blockVector<n>
//! = A^{-1} b
template<typename TStorage>
class DenseMatrixInverse
{
private: // storage
	DenseMatrix<TStorage> densemat;
	std::vector<lapack_int> interchange;

public:
	inline size_t num_cols() const
	{
		return densemat.num_cols();
	}

	inline size_t num_rows() const
	{
		return densemat.num_rows();
	}

///
public:
	//! initializes this object as inverse of mat
	bool set_as_inverse_of(const DenseMatrix<TStorage> &mat)
	{
		UG_ASSERT(mat.num_rows() == mat.num_cols(), "only for square matrices");

		densemat = mat;
		interchange.resize(mat.num_rows());

		int info = getrf(mat.num_rows(), mat.num_cols(), &densemat(0,0), mat.num_rows(), &interchange[0]);
		UG_ASSERT(info == 0, "info is " << info << ( info > 0 ? ": Matrix singular in U(i,i)" : ": i-th argument had had illegal value"));
		return info == 0;
	}

	template<typename vector_t>
	void apply(DenseVector<vector_t> &vec) const
	{
		int info = getrs(ModeNoTrans, num_rows(), 1, &densemat(0,0), num_rows(), &interchange[0], &vec[0], num_rows());
		(void) info;
		UG_ASSERT(info == 0, "DenseMatrixInverse::mat_mult: getrs failed.");
	}

	// todo: implement operator *=

	template<typename T> friend std::ostream &operator << (std::ostream &out, const DenseMatrixInverse<T> &mat);
};


template<typename T>
std::ostream &operator << (std::ostream &out, const DenseMatrixInverse<T> &mat)
{
	out << "[ ";
	for(size_t r=0; r<mat.num_rows(); ++r)
	{
		for(size_t c=0; c<mat.num_cols(); ++c)
			out << mat.densemat(r, c) << " ";
		if(r != mat.num_rows()-1) out << "| ";
	}
	out << "]";
	out << " (DenseMatrixInverse " << mat.num_rows() << "x" << mat.num_cols() << ", " << ((T::ordering == ColMajor) ? "ColMajor)" : "RowMajor)");

	return out;
}


template<typename T>
struct matrix_algebra_type_traits<DenseMatrixInverse<T> >
{
	static const matrix_algebra_type type = MATRIX_USE_GLOBAL_FUNCTIONS;
};

//! calculates dest = beta1 * A1;
template<typename vector_t, typename matrix_t>
inline void MatMult(DenseVector<vector_t> &dest,
		const number &beta1, const DenseMatrixInverse<matrix_t> &A1, const DenseVector<vector_t> &w1)
{
	if(beta1 == 1.0)
	{
		dest = w1;
		A1.apply(dest);
	}
	else
	{
		DenseVector<vector_t> tmp;
		tmp = w1;
		A1.apply(tmp);
		VecScaleAssign(dest, beta1, dest);
	}
}


//! calculates dest = alpha1*v1 + beta1 * A1 *w1;
template<typename vector_t, typename matrix_t>
inline void MatMultAdd(DenseVector<vector_t> &dest,
		const number &alpha1, const DenseVector<vector_t> &v1,
		const number &beta1, const DenseMatrixInverse<matrix_t> &A1, const DenseVector<vector_t> &w1)
{
	// todo: use dynamic stack here
	DenseVector<vector_t> tmp;
	tmp = w1;
	A1.apply(tmp);
	VecScaleAdd(dest, alpha1, v1, beta1, tmp);
}



template<typename T, eMatrixOrdering TOrdering>
inline bool GetInverse(DenseMatrixInverse<VariableArray2<T, TOrdering> > &inv, const DenseMatrix<VariableArray2<T, TOrdering> > &mat)
{
	return inv.set_as_inverse_of(mat);
}

template<typename T, size_t TBlockSize, eMatrixOrdering TOrdering>
inline bool GetInverse(DenseMatrixInverse<FixedArray2<T, TBlockSize, TBlockSize, TOrdering> > &inv, const DenseMatrix<FixedArray2<T, TBlockSize, TBlockSize, TOrdering> > &mat)
{
	return inv.set_as_inverse_of(mat);
}

}
#endif
