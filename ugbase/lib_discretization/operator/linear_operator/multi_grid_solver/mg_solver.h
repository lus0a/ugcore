/*
 * mg_solver.h
 *
 *  Created on: 07.12.2009
 *      Author: andreasvogel
 */

#ifndef __H__LIB_DISCRETIZATION__MULTI_GRID_SOLVER__MG_SOLVER__
#define __H__LIB_DISCRETIZATION__MULTI_GRID_SOLVER__MG_SOLVER__

// extern includes
#include <vector>
#include <iostream>

// other ug4 modules
#include "common/common.h"
#include "lib_grid/lg_base.h"
#include "lib_algebra/lib_algebra.h"

// library intern headers
#include "lib_discretization/lib_discretization.h"

namespace ug{

template <typename TApproximationSpace, typename TAlgebra>
class AssembledMultiGridCycle :
	virtual public ILinearizedIteratorOperator<typename TApproximationSpace::function_type, typename TApproximationSpace::function_type> {
	public:
		typedef typename TApproximationSpace::domain_type phys_domain_type;

		typedef typename TApproximationSpace::function_type function_type;

		typedef TAlgebra algebra_type;

		typedef AssembledLinearizedOperator<function_type> operator_type;

		typedef ProlongationOperator<function_type> prolongation_operator_type;

		typedef ProjectionOperator<function_type> projection_operator_type;

	private:
		typedef TApproximationSpace approximation_space_type;

		typedef ILinearizedOperatorInverse<function_type, function_type> base_solver_type;

		typedef ILinearizedIteratorOperator<function_type, function_type> smoother_type;

	public:
		// constructore
		AssembledMultiGridCycle(	IAssemble<function_type, algebra_type>& ass, approximation_space_type& approxSpace,
									size_t surfaceLevel, size_t baseLevel, int cycle_type,
									smoother_type& smoother, int nu1, int nu2, base_solver_type& baseSolver, bool grid_changes = true);

		virtual bool init(ILinearizedOperator<function_type, function_type>& A);

		// This functions allocates the Memory for the solver
		// and assembles coarse grid matrices using 'ass'
		virtual bool prepare(function_type &u, function_type& d, function_type &c);

		// This function performes one multi-grid cycle step
		// A correction c is returned as well as the updated defect d := d - A*c
		virtual bool apply(function_type& d, function_type &c, bool updateDefect);

		ILinearizedIteratorOperator<function_type,function_type>* clone()
		{
			AssembledMultiGridCycle<TApproximationSpace, TAlgebra>* clone =
				new AssembledMultiGridCycle<TApproximationSpace, TAlgebra>(	m_ass, m_approxSpace,
																			m_surfaceLevel, m_baseLevel, m_cycle_type,
																			*(m_smoother[0]), m_nu1, m_nu2, m_baseSolver, m_grid_changes);

			return dynamic_cast<ILinearizedIteratorOperator<function_type,function_type>* >(clone);
		}

		// This functions deallocates the Memory for the solver
		~AssembledMultiGridCycle();

 	protected:
 		// smooth on level l, restrict defect, call lmgc (..., l-1) and interpolate correction
		bool lmgc(size_t lev);

		// smmoth help function: perform smoothing on level l, nu times
		bool smooth(function_type& d, function_type& c, size_t lev, int nu);

		bool allocate_memory();
		bool free_memory();

	protected:
		// operator to invert (surface level)
		AssembledLinearizedOperator<function_type>* m_Op;

		IAssemble<function_type, algebra_type>& m_ass;
		approximation_space_type& m_approxSpace;
		phys_domain_type& m_domain;

		size_t m_surfaceLevel;
		size_t m_baseLevel;
		int m_cycle_type;

		int m_nu1;
		int m_nu2;

		std::vector<smoother_type*> m_smoother;
		base_solver_type& m_baseSolver;

		std::vector<operator_type*> m_A;
		std::vector<projection_operator_type*> m_P;
		std::vector<prolongation_operator_type*> m_I;

		std::vector<function_type*> m_u;
		std::vector<function_type*> m_c;
		std::vector<function_type*> m_d;
		std::vector<function_type*> m_t;

		// true -> allocate new matrices on every prepare
		bool m_grid_changes;
		bool m_allocated;

#ifdef UG_PARALLEL
		// communicator
		pcl::ParallelCommunicator<IndexLayout> m_Com;
#endif
};

}

#include "mg_solver_impl.hpp"


#endif /* __H__LIB_DISCRETIZATION__MULTI_GRID_SOLVER__MG_SOLVER__ */
