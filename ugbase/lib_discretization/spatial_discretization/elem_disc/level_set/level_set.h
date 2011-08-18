/*
 * level_set_util.h
 *
 *  Created on: 01.07.2011
 *      Author: Christian Wehner  christian.wehner@gcsc.uni-frankfurt.de
 */

#ifndef LEVEL_SET_UTIL_H_
#define LEVEL_SET_UTIL_H_

#include "common/common.h"

#include <vector>
#include "lib_discretization/spatial_discretization/disc_util/finite_volume_geometry.h"
#include <boost/function.hpp>

namespace ug{

template<typename TGridFunction>
class FV1LevelSetDisc
{
	    enum UserDataType {HardcodedData,FunctorData,VectorData,ConstantData};
	///	domain type
		typedef typename TGridFunction::domain_type domain_type;
		
	///	algebra type
		typedef typename TGridFunction::algebra_type algebra_type;
	
	/// dof distribution type	
		typedef typename TGridFunction::dof_distribution_type dof_distribution_type;

		typedef typename TGridFunction::dof_distribution_type::implementation_type
				dof_distribution_impl_type;

		typedef boost::function<void (number& value,
				                              const MathVector<domain_type::dim>& x,
				                              number time)> NumberFunctor;// see grid_function_util.h

	///	world dimension
		static const int dim = domain_type::dim;

		///for debug	type of scv-size attachment
			//	typedef typename Grid::VertexAttachmentAccessor<Attachment<number> > aaDiv;

	///	grid type
		typedef typename domain_type::grid_type grid_type;

	///	type of scv-size attachment
		typedef typename Grid::VertexAttachmentAccessor<Attachment<number> > aaSCV;

	///	type of gradient attachment
		typedef typename Grid::VertexAttachmentAccessor<Attachment<MathVector<dim> > > aaGrad;

		// 	Type of multi index vector
		typedef typename dof_distribution_type::multi_index_vector_type multi_index_vector_type;
		
		// edge iterator
		typedef geometry_traits<EdgeBase>::const_iterator EdgeBaseConstIterator;

		// vertex base iterator
		typedef geometry_traits<VertexBase>::const_iterator VertexBaseConstIterator;

	 public:
    ///	Constructor
      	    ///	Destructor
      	~FV1LevelSetDisc() {};

    ///	set time step
     	FV1LevelSetDisc() :
      		m_dt(0.0), m_time(0), m_gamma(1.0), m_delta(0.0),
      		m_reinit(false), m_analyticalSolution(true),
      		m_analyticalVelocity(false),m_externalVelocity(true),m_analyticalSource(false),m_divFree(false),
      		m_source(false),
      		m_nrOfSteps(1),m_bdryCondition(0),
      		m_maxCFL(0),m_print(false),m_timestep_nr(0),m_limiter(false),
		    m_vel_x_fct(0),
		    m_vel_y_fct(0),
		    m_vel_z_fct(0),
		    m_source_fct(0),
		    m_solution_fct(0),
		    m_vel_x_vec(0),
		    m_vel_y_vec(0),
		    m_vel_z_vec(0),
		    m_source_vec(0),
		    m_source_constant(0),
		    m_constantv_x(0),
		    m_constantv_y(0),
		    m_constantv_z(0),
		    m_dirichlet_constant(0),
		    m_source_type(HardcodedData),
		    m_velocity_type(HardcodedData),
      	    m_dirichlet_data_type(HardcodedData),
      	    m_interpolate_v_in_ip(true)
      	{}

        void set_dt(number deltaT){ UG_LOG("Set dt="<<deltaT<<"\n"); m_dt=deltaT; };

	/// set scale parameters for external velocity and velocity in normal direction
    	void set_vel_scale(number gamma,number delta){ };
    	void set_reinit(size_t n){ m_reinit=1;m_gamma=0;m_delta=1;m_nrOfSteps=n; };
		void set_divfree_bool(bool b){m_divFree=b;};
		void set_delta(number delta){m_delta =delta;}
		void set_time(double t){m_time = t;}
		number get_time(){return m_time;};
		void set_info(bool b){m_print=b;};
		void set_limiter(bool b){m_limiter=b;};
		void set_timestep_nr(size_t n){m_timestep_nr = n;};
		// set nr of time steps to perform in advect_lsf (default is 1)
		void set_nr_of_steps(size_t n){m_nrOfSteps = n;};
		///	adds a post process to be used when stepping the level set function
//		void add_post_process(IConstraint<dof_distribution_type, algebra_type>& pp){m_vPP.push_back(&pp);}
	    bool compute_error(TGridFunction& numsol);
		bool advect_lsf(TGridFunction& uNew,TGridFunction& u);
	    bool init_function(TGridFunction& u);
	    void set_neumann_boundary(const char* subsets){m_neumannSubsets = subsets;}
	///	adds a post process to be used when stepping the level set function
		void add_post_process(IConstraint<dof_distribution_impl_type, algebra_type>& pp) {m_vPP.push_back(&pp);}

		void set_vel_x(){m_velocity_type=HardcodedData;};
		void set_vel_y(){m_velocity_type=HardcodedData;};
		void set_vel_z(){m_velocity_type=HardcodedData;};

		void set_vel_x(const NumberFunctor& v){m_velocity_type=FunctorData;m_vel_x_fct = v;};
		void set_vel_y(const NumberFunctor& v){m_vel_y_fct = v;};
		void set_vel_z(const NumberFunctor& v){m_vel_z_fct = v;};

		void set_vel_x(TGridFunction& v){m_velocity_type=VectorData;m_vel_x_vec = &v;};
		void set_vel_y(TGridFunction& v){m_vel_y_vec = &v;};
		void set_vel_z(TGridFunction& v){m_vel_z_vec = &v;};

		void set_vel_x(number v){m_velocity_type=ConstantData;m_constantv_x = v;};
		void set_vel_y(number v){m_constantv_y = v;};
		void set_vel_z(number v){m_constantv_z = v;};

		void set_source(){m_source_type=HardcodedData;};
		void set_source(const NumberFunctor& s){m_source_type=FunctorData;m_source_fct= s;};
		void set_source(TGridFunction& s){m_source_type=VectorData;m_source_vec = &s;};
		void set_source(number s){m_source_type=ConstantData;m_source_constant=s;}

		void set_dirichlet_data(){m_dirichlet_data_type=HardcodedData;};
		void set_dirichlet_data(const NumberFunctor& s){m_dirichlet_data_type=FunctorData;m_solution_fct=s;}
		void set_dirichlet_data(number d){m_dirichlet_data_type=ConstantData;m_dirichlet_constant=d;};

		bool fill_v_vec(TGridFunction& vel,int component);
		
		bool runtimetest (TGridFunction& u);

	 protected:
	    number analytic_solution(number,MathVector<dim>);
		number analytic_source(number,MathVector<dim>);
	    bool analytic_velocity(MathVector<dim>&,number, MathVector<dim>);

	///	fills the scvVolume attachment for all element types
	    bool calculate_vertex_vol(TGridFunction& u, aaSCV& aaScvVolume);

//	    template <typename TElem>
//   	    bool calculate_vertex_grad_vol(grid_type& grid,TGridFunction& u, aaGrad& aaGradient, aaSCV& aaVolume );

	    bool calculate_vertex_grad_vol(TGridFunction& u, aaGrad& aaGradient, aaSCV& aaVolume );

		template <typename TElem>
		bool assemble_element(TElem& elem, DimFV1Geometry<dim>& geo, grid_type& grid,TGridFunction& uNew,const TGridFunction& uOld,aaGrad& aaGradient, aaSCV& aaVolume );

//fordebug		template <typename TElem>
//		bool assemble_divergence(TElem& elem,grid_type& grid,TGridFunction& uNew,aaDiv& aaDivergence,aaGrad& aaGradient );

		bool assign_dirichlet(TGridFunction&,int);
		bool limit_grad(TGridFunction& uOld, aaGrad& aaGradient);

		//bool limit_grad_alpha(TGridFunction& uOld,aaGrad& aaGradient,aaAlpha& aaAlpha);

	private:
	///	vector holding all scheduled post processes
		std::vector<IConstraint<dof_distribution_impl_type, algebra_type>*> m_vPP;
      	number m_dt;
		number m_time;
    	number m_gamma;
      	number m_delta;
    	bool m_reinit;
    	bool m_analyticalSolution;
	    bool m_analyticalVelocity;
	    bool m_externalVelocity;
		bool m_analyticalSource;
    	bool m_divFree;
		bool m_source;
     	size_t m_nrOfSteps;
    	size_t m_bdryCondition;
		number m_maxCFL;
		bool m_print;
		size_t m_timestep_nr;
		size_t m_limiter;
		std::string m_neumannSubsets;
		NumberFunctor m_vel_x_fct;
		NumberFunctor m_vel_y_fct;
		NumberFunctor m_vel_z_fct;
		NumberFunctor m_source_fct;
		NumberFunctor m_solution_fct;
		TGridFunction* m_vel_x_vec;
		TGridFunction* m_vel_y_vec;
		TGridFunction* m_vel_z_vec;
		TGridFunction* m_source_vec;
		number m_source_constant;
		number m_constantv_x;
		number m_constantv_y;
		number m_constantv_z;
		number m_dirichlet_constant;
		UserDataType m_source_type;
		UserDataType m_velocity_type;
		UserDataType m_dirichlet_data_type;
		bool m_interpolate_v_in_ip;
};

} // end namespace ug

// include implementation
#include "level_set_impl.h"

#endif /* LEVEL_SET_UTIL_H_ */
