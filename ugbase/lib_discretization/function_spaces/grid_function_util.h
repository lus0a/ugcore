/*
 * grid_function_util.h
 *
 *  Created on: 17.08.2010
 *      Author: andreasvogel
 */

#ifndef __H__LIBDISCRETIZATION__FUNCTION_SPACE__GRID_FUNCTION_UTIL__
#define __H__LIBDISCRETIZATION__FUNCTION_SPACE__GRID_FUNCTION_UTIL__

#include <boost/function.hpp>

#include "./grid_function.h"
#include "lib_algebra/cpu_algebra/sparsematrix_print.h"
#include "lib_algebra/operator/debug_writer.h"
#include "lib_algebra/operator/vector_writer.h"
#include "lib_discretization/io/vtkoutput.h"
#include "lib_discretization/spatial_discretization/constraints/constraint_interface.h"
#include <vector>
#include <string>
#include "lib_algebra/common/connection_viewer_output.h"

#ifdef UG_PARALLEL
	#include "pcl/pcl.h"
#endif

namespace ug{

///	writes positions of vertex dofs into a std::vector
template <typename TFunction>
void ExtractPositionsVertex(const TFunction &u,
                           std::vector<MathVector<TFunction::domain_type::dim> >& vPos)
{
//	domain type
	typedef typename TFunction::domain_type domain_type;

//	get position accessor
	const typename domain_type::position_accessor_type& aaPos
			= u.get_approximation_space().get_domain().get_position_accessor();

//	iterator
	geometry_traits<VertexBase>::const_iterator iter, iterEnd;

//	algebra indices vector
	typename TFunction::algebra_index_vector_type ind;

//	loop all subsets
	for(int si = 0; si < u.num_subsets(); ++si)
	{
	//	get iterators
		iter = u.template begin<VertexBase>(si);
		iterEnd = u.template end<VertexBase>(si);

	//	loop all vertices
		for(;iter != iterEnd; ++iter)
		{
		//	get vertex
			VertexBase* v = *iter;

		//	load indices associated with vertex
			u.inner_algebra_indices(v, ind);

		//	write position
			for(size_t i = 0; i < ind.size(); ++i)
			{
				const size_t index = ind[i];
				vPos[index] = aaPos[v];
			}
		}
	}
}

///	writes positions of element dofs into a std::vector
template <typename TFunction, typename TElem>
void ExtractPositionsElem(const TFunction &u,
                          std::vector<MathVector<TFunction::domain_type::dim> >& vPos)
{
	typename geometry_traits<TElem>::const_iterator iter, iterEnd;

//	vector for positions
	std::vector<MathVector<TFunction::domain_type::dim> > vElemPos;

//	algebra indices vector
	typename TFunction::multi_index_vector_type ind;

//	loop all subsets
	for(int si = 0; si < u.num_subsets(); ++si)
	{
	//	get iterators
		iter = u.template begin<TElem>(si);
		iterEnd = u.template end<TElem>(si);

	//	loop all functions
		for(size_t fct = 0; fct < u.num_fct(); ++fct)
		{
		//	skip non-used function
			if(!u.is_def_in_subset(fct,si)) continue;

		//	loop all elements
			for(;iter != iterEnd; ++iter)
			{
			//	get vertex
				TElem* elem = *iter;

			//	load indices associated with element function
				u.inner_multi_indices(elem, fct, ind);

			//	load positions associated with element and function
				u.inner_dof_positions(elem, fct, vElemPos);

			//	check correct size
				UG_ASSERT(ind.size() == vElemPos.size(), "Num MultiIndex ("<<ind.size()
						  <<") and Num Position ("<<vElemPos.size()<<") must match."
						 "GeomObject dim="<<geometry_traits<TElem>::BASE_OBJECT_TYPE_ID);

			//	write position
				for(size_t sh = 0; sh < ind.size(); ++sh)
				{
					const size_t index = ind[sh][0];
					vPos[index] = vElemPos[sh];
				}
			}
		}
	}
}

template <typename TFunction>
void ExtractPositions(const TFunction &u,
                      std::vector<MathVector<TFunction::domain_type::dim> >& vPos)
{
//	number of total dofs
	int nr = u.num_indices();

//	resize positions
	vPos.resize(nr);

//	extract for all element types
	ExtractPositionsVertex<TFunction>(u, vPos);
	ExtractPositionsElem<TFunction, EdgeBase>(u, vPos);
	ExtractPositionsElem<TFunction, Face>(u, vPos);
	ExtractPositionsElem<TFunction, Volume>(u, vPos);
}

template <class TFunction>
bool WriteMatrixToConnectionViewer(const char *filename,
                                   const typename TFunction::algebra_type::matrix_type &A,
                                   const TFunction &u)
{
//	check that extension '.mat' is chosen
	const char * p = strstr(filename, ".mat");
	if(p == NULL)
	{
		UG_LOG("Currently only '.mat' format supported for matrices.\n");
		return false;
	}

//	extended filename
	std::string name(filename);

//	add parallel ending
#ifdef UG_PARALLEL
//	search for ending
	size_t found = name.find_first_of(".");

//	remove endings
	name.resize(found);

//	add new ending, containing process number
	int rank = pcl::GetProcRank();
	char ext[20];
	sprintf(ext, "_p%04d.mat", rank);
	name.append(ext);
#endif

//	position array
	const static int dim = TFunction::domain_type::dim;
	std::vector<MathVector<dim> > positions;

// 	get positions of vertices
	ExtractPositions(u, positions);

// 	write matrix
	WriteMatrixToConnectionViewer(name.c_str(), A, &positions[0], dim);

//	we're done
	return true;
}


template <typename TGridFunction>
bool SaveMatrixForConnectionViewer(	TGridFunction& u,
									MatrixOperator<typename TGridFunction::vector_type,
													typename TGridFunction::vector_type,
													typename TGridFunction::algebra_type::matrix_type>& A,
									const char* filename)
{
//	forward
	return WriteMatrixToConnectionViewer(filename, A.get_matrix(), u);
}

template <class TFunction>
bool WriteVectorToConnectionViewer(const char *filename,
                                   const typename TFunction::algebra_type::vector_type &b,
                                   const TFunction &u)
{
//	get dimension
	const static int dim = TFunction::domain_type::dim;

//	check name

	std::string name(filename);
	size_t iExtPos = name.find_last_of(".");
	if(iExtPos == std::string::npos || name.substr(iExtPos).compare(".vec") != 0)
	{
		UG_LOG("Only '.vec' format supported for vectors.\n");
		return false;
	}

//	extended filename
//	add p000X extension in parallel
#ifdef UG_PARALLEL
	name.resize(iExtPos);
	int rank = pcl::GetProcRank();
	char ext[20];
	sprintf(ext, "_p%04d.vec", rank);
	name.append(ext);
#endif

// 	get positions of vertices
	std::vector<MathVector<dim> > positions;
	ExtractPositions(u, positions);

//	write vector
	WriteVectorToConnectionViewer(name.c_str(), b, &positions[0], dim);

//	we're done
	return true;
}

template <typename TGridFunction>
bool SaveVectorForConnectionViewer(	TGridFunction& b,
									const char* filename)
{
	return WriteVectorToConnectionViewer(filename, b, b);
}


template <typename TGridFunction>
class GridFunctionDebugWriter
	: public IDebugWriter<typename TGridFunction::algebra_type>
{
	public:
	///	type of matrix
		typedef typename TGridFunction::algebra_type algebra_type;

	///	type of vector
		typedef typename algebra_type::vector_type vector_type;

	///	type of matrix
		typedef typename algebra_type::matrix_type matrix_type;

	public:
	///	Constructor
		GridFunctionDebugWriter() :
			m_pGridFunc(NULL),
			bConnViewerOut(true), bVTKOut(true)
		{}

	///	sets the function
		void set_reference_grid_function(const TGridFunction& u)
		{
			m_pGridFunc = &u;
		}

	//	sets if writing to vtk
		void set_vtk_output(bool b) {bVTKOut = b;}

	//	sets if writing to conn viewer
		void set_conn_viewer_output(bool b) {bConnViewerOut = b;}

	///	write vector
		virtual bool write_vector(const vector_type& vec,
		                          const char* filename)
		{
			bool bRet = true;

		//	write to conn viewer
			if(bConnViewerOut)
				bRet &= write_vector_to_conn_viewer(vec, filename);

		//	write to vtk
			if(bVTKOut)
				bRet &= write_vector_to_vtk(vec, filename);

		//	we're done
			return bRet;
		}

	///	write matrix
		virtual bool write_matrix(const matrix_type& mat,
		                          const char* filename)
		{
		//	something to do ?
			if(!bConnViewerOut) return true;

		//	check
			if(m_pGridFunc == NULL)
			{
				UG_LOG("ERROR in 'GridFunctionDebugWriter::write_vector':"
						" No reference grid function set.\n");
				return false;
			}

		//	get fresh name
			std::string name(filename);

		//	search for ending and remove
			size_t found = name.find_first_of(".");
			if(found != std::string::npos) name.resize(found);

		//	add ending
			name.append(".mat");

		//	write to file
			WriteMatrixToConnectionViewer(name.c_str(), mat, *m_pGridFunc);

		//  we're done
			return true;
		}

	protected:
	///	write vector
		virtual bool write_vector_to_conn_viewer(const vector_type& vec,
		                                         const char* filename)
		{
		//	check
			if(m_pGridFunc == NULL)
			{
				UG_LOG("ERROR in 'GridFunctionDebugWriter::write_vector':"
						" No reference grid function set.\n");
				return false;
			}

		//	get fresh name
			std::string name(filename);

		//	search for ending and remove
			size_t found = name.find_first_of(".");
			if(found != std::string::npos) name.resize(found);

		//	add ending
		//	\todo: Introduce a ending *.vec for Connection Viewer
			name.append(".mat");

		//	write to file
			WriteVectorToConnectionViewer(name.c_str(), vec, *m_pGridFunc);

		//	we're done
			return true;
		}

		bool write_vector_to_vtk(const vector_type& vec, const char* filename)
		{
		//	check
			if(m_pGridFunc == NULL)
			{
				UG_LOG("ERROR in 'GridFunctionDebugWriter::write_vector':"
						" No reference grid function set.\n");
				return false;
			}

		//	assign all patterns and sizes
			m_vtkFunc.clone_pattern((*m_pGridFunc));
			m_vtkFunc = (*m_pGridFunc);

		//	overwrite values with vector
			m_vtkFunc.assign(vec);

		//	create vtk output
			VTKOutput<TGridFunction> out;

		//	write
			return out.print(filename, m_vtkFunc);
		}

	protected:
	// 	grid function used as reference
		const TGridFunction* m_pGridFunc;

	// 	dummy vector for vtk output
		TGridFunction m_vtkFunc;

	//	flag if write to conn viewer
		bool bConnViewerOut;

	//	flag if write to vtk
		bool bVTKOut;
};

template <typename TGridFunction>
class GridFunctionPositionProvider
	: public IPositionProvider<TGridFunction::domain_type::dim>
{
	public:
	///	Constructor
		GridFunctionPositionProvider() : m_pGridFunc(NULL)
		{
		}

		void set_reference_grid_function(const TGridFunction& u)
		{
			m_pGridFunc = &u;
		}

		virtual bool get_positions(std::vector<MathVector<TGridFunction::domain_type::dim> >&vec)
		{
			UG_ASSERT(m_pGridFunc != NULL, "provide a grid function with set_reference_grid_function");
			ExtractPositions(*m_pGridFunc, vec);
			return true;
		}

	protected:
		const TGridFunction *m_pGridFunc;
};


template <typename TGridFunction, typename TVector>
class GridFunctionVectorWriter
	: public IVectorWriter<TVector>
{
	public:
		typedef typename TVector::value_type value_type;
		typedef typename TGridFunction::domain_type domain_type;
		typedef TVector vector_type;
		typedef boost::function<void (number& value,
		                              const MathVector<domain_type::dim>& x,
		                              number time)> NumberFunctor;

	public:
	///	Constructor
		GridFunctionVectorWriter() :
			m_pGridFunc(NULL)
		{}

		void set_user_data(const NumberFunctor& userData)
		{
			m_userData = userData;
		}

		void set_reference_grid_function(const TGridFunction& u)
		{
			m_pGridFunc = &u;
		}

		/*virtual double calculate(MathVector<3> &v, double time)
		{
		}*/

		virtual bool update(vector_type &vec)
		{
			UG_ASSERT(m_pGridFunc != NULL, "provide a grid function with set_reference_grid_function");
			UG_ASSERT(m_userData != NULL, "provide user data with set_user_data");

			const TGridFunction &u = *m_pGridFunc;

		//	get position accessor

			const typename domain_type::position_accessor_type& aaPos
					= u.get_approximation_space().get_domain().get_position_accessor();

		//	number of total dofs
			int nr = u.num_indices();

		//	resize positions
			vec.resize(nr);

		//	loop all subsets
			for(int si = 0; si < u.num_subsets(); ++si)
			{
			//	loop all vertices
				geometry_traits<VertexBase>::const_iterator iter
													= u.template begin<VertexBase>(si);
				for(;iter != u.template end<VertexBase>(si); ++iter)
				{
				//	get vertex
					VertexBase* v = *iter;

				//	algebra indices vector
					typename TGridFunction::algebra_index_vector_type ind;

				//	load indices associated with vertex
					u.inner_algebra_indices(v, ind);

					number t = 0.0;

					number d;
					m_userData(d, aaPos[v], t);

				//	write
					for(size_t i = 0; i < ind.size(); ++i)
					{
						const size_t index = ind[i];
						for(size_t alpha = 0; alpha < GetSize(vec[index]); ++alpha)
							BlockRef(vec[index], alpha) = d;
					}
				}
			}
			return true;
		}

	protected:
		const TGridFunction *m_pGridFunc;
		NumberFunctor m_userData;
};


template <typename TGridFunction>
class GridFunctionVectorWriterDirichlet0
	: public IVectorWriter<typename TGridFunction::algebra_type::vector_type>
{
	public:
		typedef typename TGridFunction::domain_type domain_type;

		typedef typename TGridFunction::approximation_space_type approximation_space_type;
		typedef typename approximation_space_type::dof_distribution_type dof_distribution_type;

		typedef typename TGridFunction::algebra_type algebra_type;
		typedef typename algebra_type::vector_type vector_type;
		typedef typename vector_type::value_type value_type;

		typedef IConstraint<typename dof_distribution_type::implementation_type, algebra_type>
			post_process_type;
	public:
	///	Constructor
		GridFunctionVectorWriterDirichlet0() :
			m_pApproxSpace(NULL), m_pPostProcess(NULL), m_level(-1)
		{}


		void set_level(size_t level)
		{
			m_level = level;
		}

		void init(post_process_type &pp, approximation_space_type& approxSpace)
		{
			m_pPostProcess = &pp;
			m_pApproxSpace = &approxSpace;
		}


		/*virtual double calculate(MathVector<3> &v, double time)
		{
		}*/

		virtual bool update(vector_type &vec)
		{
			UG_ASSERT(m_pPostProcess != NULL, "provide a post process with init");
			UG_ASSERT(m_pApproxSpace != NULL, "provide approximation space init");

			dof_distribution_type *pDOF;
			if(m_level == (size_t)-1)
				pDOF = &m_pApproxSpace->get_surface_dof_distribution();
			else
				pDOF = &m_pApproxSpace->get_level_dof_distribution(m_level) ;

			vec.resize(pDOF->num_indices());
			vec.set(1.0);

			return m_pPostProcess->adjust_defect(vec, vec, *pDOF);
		}

	protected:
		approximation_space_type * m_pApproxSpace;
		post_process_type *m_pPostProcess;
		size_t m_level;
};



} // end namespace ug

#endif /* __H__LIBDISCRETIZATION__FUNCTION_SPACE__GRID_FUNCTION_UTIL__ */
