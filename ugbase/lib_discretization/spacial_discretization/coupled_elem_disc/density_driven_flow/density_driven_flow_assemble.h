
#ifndef __H__LIB_DISCRETIZATION__SPACIAL_DISCRETIZATION__COUPLED_ELEM_DISC__DENSITY_DRIVEN_FLOW__CPL_DENSITY_DRIVEN_FLOW_ASSEMBLE__
#define __H__LIB_DISCRETIZATION__SPACIAL_DISCRETIZATION__COUPLED_ELEM_DISC__DENSITY_DRIVEN_FLOW__CPL_DENSITY_DRIVEN_FLOW_ASSEMBLE__

// other ug4 modules
#include "common/common.h"
#include "lib_grid/lg_base.h"
#include "lib_algebra/lib_algebra.h"

// library intern headers
#include "lib_discretization/spacial_discretization/disc_helper/fvgeom.h"

#include "../coupled_elem_disc_interface.h"
#include "../disc_coupling/element_data.h"

namespace ug{

template<typename TDomain, int ref_dim, typename TAlgebra>
class CplDensityDrivenFlowElemDisc : public ICoupledElemDisc<TAlgebra>
//	public DataExportingClass<MathVector<TDomain::dim>, MathVector<ref_dim>, TAlgebra>
{
	public:
		// domain type
		typedef TDomain domain_type;

		// world dimension
		static const int dim = TDomain::dim;

		// position type
		typedef typename TDomain::position_type position_type;

		// algebra type
		typedef TAlgebra algebra_type;

		// local matrix type
		typedef typename algebra_type::matrix_type::local_matrix_type local_matrix_type;

		// local vector type
		typedef typename algebra_type::vector_type::local_vector_type local_vector_type;

		// local index type
		typedef typename algebra_type::vector_type::local_index_type local_index_type;

	protected:
		typedef void (*Pososity_fct)(number&);
		typedef void (*Viscosity_fct)(number&, number);
		typedef void (*Density_fct)(number&, number);
		typedef void (*D_Density_fct)(number&, number);
		typedef void (*Mol_Diff_Tensor_fct)(MathMatrix<dim,dim>&);
		typedef void (*Permeability_Tensor_fct)(MathMatrix<dim,dim>&);
		typedef void (*Gravity_fct)(MathVector<dim>&);

	public:
		CplDensityDrivenFlowElemDisc(	TDomain& domain, number upwind_amount,
							Pososity_fct Porosity, Viscosity_fct Viscosity, Density_fct Density, D_Density_fct D_Density,
							Mol_Diff_Tensor_fct Mol_Diff, Permeability_Tensor_fct Permeability_Tensor, Gravity_fct Gravity);

	public:
		virtual size_t num_imports() {return 0;}

		virtual DataImportItem* import(size_t i) {return NULL;}

		virtual bool register_exports(DataContainer& Cont)
		{
			if(Cont.register_item(m_DarcyVelocity) != true)
			{
				UG_ASSERT(0, "Must work.");
				return false;
			}
			return true;
		}
		virtual bool unregister_exports(DataContainer& Cont)
		{
			if(Cont.unregister_item(m_DarcyVelocity) != true)
			{
				UG_ASSERT(0, "Must work.");
				return false;
			}
			return true;
		}

		virtual bool register_imports(DataContainer& Cont) {return true;}

		virtual bool unregister_imports(DataContainer& Cont){ return true;}

	protected:
		DataClassExportPossibility<MathVector<dim>, MathVector<ref_dim>, algebra_type>  m_DarcyVelocity;

	public:
		// number of fundamental functions required for this assembling
		virtual size_t num_fct(){return 2;}

		// local shape function set required for the 'i'-th fundamental function
		virtual LocalShapeFunctionSetID local_shape_function_set_id(size_t loc_fct) {return LSFS_LAGRANGEP1;}

	public:
		template <typename TElem>
		inline size_t num_total_sh(){		typedef typename reference_element_traits<TElem>::reference_element_type ref_elem_type;
											return 2*ref_elem_type::num_corners;};

		template <typename TElem>
		inline size_t num_sh(size_t loc_fct){		typedef typename reference_element_traits<TElem>::reference_element_type ref_elem_type;
													return ref_elem_type::num_corners;};

		template <typename TElem>
		inline bool prepare_element_loop();

		template <typename TElem>
		inline bool finish_element_loop();

		template <typename TElem>
		inline bool prepare_element(TElem* elem, const local_vector_type& u, const local_index_type& glob_ind);

		template <typename TElem>
		inline bool assemble_JA(local_matrix_type& J, const local_vector_type& u, number time=0.0);

		template <typename TElem>
		inline bool assemble_JM(local_matrix_type& J, const local_vector_type& u, number time=0.0);

		template <typename TElem>
		inline bool assemble_A(local_vector_type& d, const local_vector_type& u, number time=0.0);

		template <typename TElem>
		inline bool assemble_M(local_vector_type& d, const local_vector_type& u, number time=0.0);

		template <typename TElem>
		inline bool assemble_f(local_vector_type& d, number time=0.0);


	private:
		// domain
		TDomain& m_domain;

		// position access
		template <typename TElem>
		ReferenceMapping<typename reference_element_traits<TElem>::reference_element_type, dim>& get_mapping()
		{
			static ReferenceMapping<typename reference_element_traits<TElem>::reference_element_type, dim> mapping;
			return mapping;
		}

		// position access
		typename TDomain::position_type* m_corners;
		typename TDomain::position_accessor_type m_aaPos;

		// Finite Volume Element Geometry
		template <typename TElem>
		inline FVElementGeometry<TElem, dim>& get_fvgeom()
		{
			static FVElementGeometry<TElem, dim> geo;
			return geo;
		}

		// amount of upwind (1.0 == full upwind, 0.0 == no upwind)
		number m_upwind_amount;

		// User functions
		Pososity_fct m_Porosity;
		Viscosity_fct m_Viscosity;
		Density_fct m_Density;
		D_Density_fct m_D_Density;
		Mol_Diff_Tensor_fct m_Mol_Diff_Tensor;
		Permeability_Tensor_fct m_Permeability_Tensor;
		Gravity_fct m_Gravity;

		// constant values, read in once
		number m_porosity;

	private:
		// local constants for readability (local function 0 == _C_, local function 1 == _P_)
		static const size_t _C_ = 0;
		static const size_t _P_ = 1;

	private:
		template <typename TElem>
		void data_export(std::vector<MathVector<dim> >& val, std::vector<std::vector<MathVector<dim> > >& deriv,
						const std::vector<MathVector<ref_dim> >& pos, const local_vector_type& u, bool compute_derivatives)
		{
		#define u(fct, i)    ( (u)[ref_elem_type::num_corners*(fct) + (i)])
			typedef typename reference_element_traits<TElem>::reference_element_type ref_elem_type;
			const LocalShapeFunctionSet<ref_elem_type>& TrialSpace =
					LocalShapeFunctionSetFactory::inst().get_local_shape_function_set<ref_elem_type>(LSFS_LAGRANGEP1);
			const size_t num_co = ref_elem_type::num_corners;

			number c_ip;
			MathVector<TDomain::dim> grad_p_ip, grad_p_local;
			MathMatrix<TDomain::dim, ref_elem_type::dim> JTInv;
			std::vector<number> shape_ip(val.size());
			std::vector<MathVector<TDomain::dim> > grad_local_ip(val.size());
			std::vector<MathVector<TDomain::dim> > grad_global_ip(val.size());

			get_mapping().update(m_corners);
			for(size_t ip = 0; ip < val.size(); ++ip)
			{
				VecSet(grad_p_ip, 0.0); c_ip = 0.0;

				if(get_mapping().jacobian_transposed_inverse(pos[ip], JTInv) == false) UG_ASSERT(0, "");
				for(size_t co = 0; co < num_co; ++co)
				{
					if(TrialSpace.evaluate(co, pos[ip], shape_ip[co]) == false) UG_ASSERT(0, "");
					if(TrialSpace.evaluate_grad(co, pos[ip], grad_local_ip[co]) == false)  UG_ASSERT(0, "");
					c_ip += u(_C_, co) * shape_ip[co];
					MatVecMult(grad_global_ip[co], JTInv, grad_local_ip[co]);
					VecScaleAppend(grad_p_ip, u(_P_,co), grad_global_ip[co]);
				}

				number s, mu_ip;
				MathVector<dim> vel, gravity;
				MathMatrix<dim, dim> K;

				m_Density(s, c_ip);
				m_Gravity(gravity);
				m_Viscosity(mu_ip, c_ip);
				m_Permeability_Tensor(K);
				VecScale(vel, gravity, s);
				VecSubtract(vel, vel, grad_p_ip);
				MatVecMult(val[ip], K, vel);
				VecScale(val[ip], val[ip], 1./mu_ip);

				if(compute_derivatives == true)
				{
					m_D_Density(s, c_ip);
					MathVector<dim> D_vel_c[num_co], D_vel_p[num_co];
					for(size_t co = 0; co < num_co; ++co)
					{

						VecScale(D_vel_c[co], gravity, s*shape_ip[co]);
						VecScale(D_vel_p[co], grad_global_ip[co], -1);
						MatVecMult(deriv[ip][co], K, D_vel_c[co]);
						MatVecMult(deriv[ip][num_co + co], K, D_vel_p[co]);

						VecScale(deriv[ip][co],deriv[ip][co],1./mu_ip);
						VecScale(deriv[ip][num_co + co],deriv[ip][num_co + co],1./mu_ip);
					}
				}
			}

		#undef u
		}

		bool compute_ip_Darcy_velocity(MathVector<dim>& Darcy_vel, number c_ip, const MathVector<dim>& grad_p_ip);


		template <typename TElem>
		bool compute_D_ip_Darcy_velocity(	const SubControlVolumeFace<TElem, dim>& scvf,
											MathVector<dim>& Darcy_vel, MathVector<dim> D_Darcy_vel_c[], MathVector<dim> D_Darcy_vel_p[],
											number c_ip, const MathVector<dim>& grad_p_ip);
};

} // end namespace ug

#include "density_driven_flow_assemble_impl.h"

#endif /*__H__LIB_DISCRETIZATION__SPACIAL_DISCRETIZATION__COUPLED_ELEM_DISC__DENSITY_DRIVEN_FLOW__CPL_DENSITY_DRIVEN_FLOW_ASSEMBLE__*/
