/*
 * domain_discretization.h
 *
 *  Created on: 29.06.2010
 *      Author: andreasvogel
 */

#ifndef __H__LIB_DISCRETIZATION__SPACIAL_DISCRETIZATION__DOMAIN_DISCRETIZATION__
#define __H__LIB_DISCRETIZATION__SPACIAL_DISCRETIZATION__DOMAIN_DISCRETIZATION__

// other ug4 modules
#include "common/common.h"
#include "lib_algebra/lib_algebra.h"

// library intern headers
#include "lib_discretization/spacial_discretization/domain_discretization_interface.h"
#include "elem_disc/elem_disc_assemble_util.h"

namespace ug {

template <	typename TDiscreteFunction,
			typename TAlgebra = typename TDiscreteFunction::algebra_type >
class DomainDiscretization :
	public IDomainDiscretization<TDiscreteFunction, TAlgebra>
{
	protected:
		// type of algebra
		typedef TDiscreteFunction discrete_function_type;

		// type of algebra
		typedef TAlgebra algebra_type;

		// type of algebra matrix
		typedef typename algebra_type::matrix_type matrix_type;

		// type of algebra vector
		typedef typename algebra_type::vector_type vector_type;

	public:
		DomainDiscretization()
		{};

		IAssembleReturn assemble_jacobian(matrix_type& J, const discrete_function_type& u)
		{
			if(!check_solution(u)) return IAssemble_ERROR;
			for(size_t i = 0; i < m_disc.size(); ++i)
				if(!AssembleJacobian(*m_disc[i].disc, J, u, m_disc[i].u_comp, m_disc[i].si))
					return IAssemble_ERROR;

			for(size_t i = 0; i < m_dirichletDisc.size(); ++i)
				if((m_dirichletDisc[i].disc)->clear_dirichlet_jacobian(J, u, m_dirichletDisc[i].si) != IAssemble_OK)
					return IAssemble_ERROR;
			return IAssemble_OK;
		}
		IAssembleReturn assemble_defect(vector_type& d, const discrete_function_type& u)
		{
			if(!check_solution(u)) return IAssemble_ERROR;
			for(size_t i = 0; i < m_disc.size(); ++i)
				if(!AssembleDefect(*m_disc[i].disc, d, u, m_disc[i].u_comp, m_disc[i].si))
					return IAssemble_ERROR;

			for(size_t i = 0; i < m_dirichletDisc.size(); ++i)
				if((m_dirichletDisc[i].disc)->clear_dirichlet_defect(d, u, m_dirichletDisc[i].si) != IAssemble_OK)
					return IAssemble_ERROR;
			return IAssemble_OK;
		}
		IAssembleReturn assemble_linear(matrix_type& mat, vector_type& rhs, const discrete_function_type& u)
		{
			if(!check_solution(u)) return IAssemble_ERROR;
			for(size_t i = 0; i < m_disc.size(); ++i)
				if(!AssembleLinear(*m_disc[i].disc, mat, rhs, u, m_disc[i].u_comp, m_disc[i].si))
					return IAssemble_ERROR;

			for(size_t i = 0; i < m_dirichletDisc.size(); ++i)
				if((m_dirichletDisc[i].disc)->set_dirichlet_linear(mat, rhs, u, m_dirichletDisc[i].si) != IAssemble_OK)
					return IAssemble_ERROR;
			return IAssemble_OK;
		}
		IAssembleReturn assemble_solution(discrete_function_type& u)
		{
			if(!check_solution(u)) return IAssemble_ERROR;
			for(size_t i = 0; i < m_dirichletDisc.size(); ++i)
				if((m_dirichletDisc[i].disc)->set_dirichlet_solution(u, m_dirichletDisc[i].si) != IAssemble_OK)
					return IAssemble_ERROR;
			return IAssemble_OK;
		}

		///////////////////////
		// Time dependent part
		///////////////////////
		IAssembleReturn assemble_jacobian(matrix_type& J, const discrete_function_type& u, number time, number s_m, number s_a)
		{
			if(!check_solution(u)) return IAssemble_ERROR;
			for(size_t i = 0; i < m_disc.size(); ++i)
				if(!AssembleJacobian(*m_disc[i].disc, J, u, m_disc[i].u_comp, m_disc[i].si, time, s_m, s_a))
					return IAssemble_ERROR;

			for(size_t i = 0; i < m_dirichletDisc.size(); ++i)
				if((m_dirichletDisc[i].disc)->clear_dirichlet_jacobian(J, u, m_dirichletDisc[i].si, time) != IAssemble_OK)
					return IAssemble_ERROR;
			return IAssemble_OK;
		}
		IAssembleReturn assemble_defect(vector_type& d, const discrete_function_type& u, number time, number s_m, number s_a)
		{
			if(!check_solution(u)) return IAssemble_ERROR;
			for(size_t i = 0; i < m_disc.size(); ++i)
				if(!AssembleDefect(*m_disc[i].disc, d, u, m_disc[i].u_comp, m_disc[i].si, time, s_m, s_a))
					return IAssemble_ERROR;

			for(size_t i = 0; i < m_dirichletDisc.size(); ++i)
				if((m_dirichletDisc[i].disc)->clear_dirichlet_defect(d, u, m_dirichletDisc[i].si, time) != IAssemble_OK)
					return IAssemble_ERROR;
			return IAssemble_OK;
		}
		IAssembleReturn assemble_linear(matrix_type& mat, vector_type& rhs, const discrete_function_type& u, number time, number s_m, number s_a)
		{
			if(!check_solution(u)) return IAssemble_ERROR;
			for(size_t i = 0; i < m_disc.size(); ++i)
				if(!AssembleLinear(*m_disc[i].disc, mat, rhs, u, m_disc[i].u_comp, m_disc[i].si, time, s_m, s_a))
					return IAssemble_ERROR;

			for(size_t i = 0; i < m_dirichletDisc.size(); ++i)
				if((m_dirichletDisc[i].disc)->set_dirichlet_linear(mat, rhs, u, m_dirichletDisc[i].si, time) != IAssemble_OK)
					return IAssemble_ERROR;
			return IAssemble_OK;
		}
		IAssembleReturn assemble_solution(discrete_function_type& u, number time)
		{
			if(!check_solution(u)) return IAssemble_ERROR;
			for(size_t i = 0; i < m_dirichletDisc.size(); ++i)
				if((m_dirichletDisc[i].disc)->set_dirichlet_solution(u, m_dirichletDisc[i].si, time) != IAssemble_OK)
					return IAssemble_ERROR;
			return IAssemble_OK;
		}

	protected:
		typedef std::vector<size_t> comp_vec_type;

	public:
		bool add_disc(IElemDisc<TAlgebra>& elemDisc, const comp_vec_type& u_comp, int si)
		{
			// check if number of functions match
			if(elemDisc.num_fct() != u_comp.size())
			{
				UG_LOG("Wrong number of local functions given for Elemet Discretization.\n");
				UG_LOG("Needed: " << elemDisc.num_fct() << ", given: " << u_comp.size() << ".\n");
				UG_LOG("Cannot add element discretization.\n");
				return false;
			}
			m_disc.push_back(ElemDisc(si, elemDisc, u_comp));
			return true;
		}

	protected:
		struct ElemDisc
		{
			ElemDisc(int s, IElemDisc<TAlgebra>& di, const comp_vec_type& cmp) :
				si(s), disc(&di), u_comp(cmp) {};

			int si;
			IElemDisc<TAlgebra>* disc;
			comp_vec_type u_comp;
		};

		std::vector<ElemDisc> m_disc;

		bool check_solution(const discrete_function_type& u)
		{
			for(size_t i = 0; i < m_disc.size(); ++i)
			{
				for(size_t fct = 0; fct < (m_disc[i].disc)->num_fct(); ++fct)
				{
					if((u.get_local_shape_function_set_id((m_disc[i].u_comp)[fct]) !=	(m_disc[i].disc)->local_shape_function_set_id(fct)))
					{
						UG_LOG("Solution does not match requirements of discretization.\n");
						return false;
					}
				}
			}
			return true;
		}

	public:
		bool add_dirichlet_bnd(IDirichletBoundaryValues<discrete_function_type>& bnd_disc, int si)
		{
			m_dirichletDisc.push_back(DirichletDisc(si, &bnd_disc));
			return true;
		}

	protected:
		struct DirichletDisc
		{
			DirichletDisc(int s, IDirichletBoundaryValues<discrete_function_type>* di) :
				si(s), disc(di) {};

			int si;
			IDirichletBoundaryValues<discrete_function_type>* disc;
		};

		std::vector<DirichletDisc> m_dirichletDisc;

	protected:
		// TODO: What is this function used for???? Do we have to include it
		virtual size_t num_fct() const
		{
			size_t sum = 0;
			for(size_t i = 0; i < m_disc.size(); ++i)
				sum += m_disc[i].u_comp.size();

			return sum;
		}

		virtual bool is_dirichlet(int si, size_t fct)
		{
			for(size_t i = 0; i < m_dirichletDisc.size(); ++i)
			{
				if(m_dirichletDisc[i].si != si) continue;
				if((m_dirichletDisc[i].disc)->is_dirichlet(fct) == true) return true;
			}
			return false;
		}
};

} // end namespace ug

#endif /* __H__LIB_DISCRETIZATION__SPACIAL_DISCRETIZATION__DOMAIN_DISCRETIZATION__ */
