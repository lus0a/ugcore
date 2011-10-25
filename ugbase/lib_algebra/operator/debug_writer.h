/*
 * debug_writer.h
 *
 *  Created on: 18.01.2011
 *      Author: andreasvogel
 */

#ifndef __H__LIB_ALGEBRA__OPERATOR__DEBUG_WRITER__
#define __H__LIB_ALGEBRA__OPERATOR__DEBUG_WRITER__

#include "common/math/ugmath.h"
#include "lib_algebra/common/connection_viewer_output.h"

namespace ug{

/// base class for all debug writer
/**
 * This is the base class for debug output of algebraic vectors and matrices.
 */
template <typename TAlgebra>
class IDebugWriter
{
	public:
	///	type of algebra
		typedef TAlgebra algebra_type;

	///	type of vector
		typedef typename TAlgebra::vector_type vector_type;

	///	type of matrix
		typedef typename TAlgebra::matrix_type matrix_type;

	public:
	///	Constructor
		IDebugWriter() : m_currentDim(-1) {}

	///	write vector
		virtual bool write_vector(const vector_type& vec,
		                          const char* name) = 0;

	///	write matrix
		virtual bool write_matrix(const matrix_type& mat,
		                          const char* name) = 0;

	///	returns the current dimension
		int current_dimension() const {return m_currentDim;}

	///	returns the positions (only available for current dimension)
		template <int dim>
		const std::vector<MathVector<dim> >& get_positions() const
		{
			if(m_currentDim != dim) throw(UGFatalError("Current dim is different."));
			return get_pos(Int2Type<dim>());
		}

	///	sets the current positions
		template <int dim>
		void set_positions(const std::vector<MathVector<dim> >& vPos)
		{
			get_pos(Int2Type<dim>()) = vPos; m_currentDim = dim;
		}

	/// virtual destructor
		virtual ~IDebugWriter(){}

	protected:
	///	returns the positions and sets the current dim
		template <int dim>
		std::vector<MathVector<dim> >& get_positions()
		{
			m_currentDim = dim; return get_pos(Int2Type<dim>());
		}

	///	current dimension
		int m_currentDim;

	///	vectors of positions
		std::vector<MathVector<1> > m_vPos1d;
		std::vector<MathVector<2> > m_vPos2d;
		std::vector<MathVector<3> > m_vPos3d;

	///	help function to get local ips
		std::vector<MathVector<1> >& get_pos(Int2Type<1>) {return m_vPos1d;}
		std::vector<MathVector<2> >& get_pos(Int2Type<2>) {return m_vPos2d;}
		std::vector<MathVector<3> >& get_pos(Int2Type<3>) {return m_vPos3d;}
		const std::vector<MathVector<1> >& get_pos(Int2Type<1>) const {return m_vPos1d;}
		const std::vector<MathVector<2> >& get_pos(Int2Type<2>) const {return m_vPos2d;}
		const std::vector<MathVector<3> >& get_pos(Int2Type<3>) const {return m_vPos3d;}
};

template <typename TAlgebra, int dim>
class AlgebraDebugWriter
	: public IDebugWriter<TAlgebra>
{
	public:
	///	type of matrix
		typedef TAlgebra algebra_type;

	///	type of vector
		typedef typename algebra_type::vector_type vector_type;

	///	type of matrix
		typedef typename algebra_type::matrix_type matrix_type;

	///	type of positions
		typedef MathVector<dim> position_type;

	public:
	///	Constructor
		AlgebraDebugWriter() : m_pPositions(NULL),  m_numPos(-1)
		{}

	///	sets the function
		void set_positions(position_type* pos, int n)
		{
			m_pPositions = pos;
			m_numPos = n;
		}

	///	write vector
		virtual bool write_vector(const vector_type& vec,
		                          const char* filename)
		{
		//	check
			if(m_pPositions == NULL)
			{
				UG_LOG("ERROR in 'AlgebraDebugWriter::write_vector':"
						" No reference positions set.\n");
				return false;
			}

		//	check number of positions
			if(vec.size() != (size_t)m_numPos)
			{
				UG_LOG("ERROR in 'AlgebraDebugWriter::write_vector':"
						" Number of positions does not match.\n");
				return false;
			}

		//	get fresh name
			std::string name(filename);

		//	search for ending and remove
			size_t found = name.find_first_of(".");
			if(found != std::string::npos) name.resize(found);

		#ifdef UG_PARALLEL
		//	add process number
			int rank = pcl::GetProcRank();
			char ext[20];
			sprintf(ext, "_p%04d", rank);
			name.append(ext);
		#endif

		//	add ending
			name.append(".vec");

		//	write to file
			WriteVectorToConnectionViewer<vector_type, position_type>
				(name.c_str(), vec, m_pPositions, dim);

		//	we're done
			return true;
		}

	///	write matrix
		virtual bool write_matrix(const matrix_type& mat,
		                          const char* filename)
		{
		//	check
			if(m_pPositions == NULL)
			{
				UG_LOG("ERROR in 'AlgebraDebugWriter::write_matrix':"
						" No reference positions set.\n");
				return false;
			}

		//	check number of positions
			if(mat.num_rows() != (size_t)m_numPos || mat.num_cols() != (size_t)m_numPos)
			{
				UG_LOG("ERROR in 'AlgebraDebugWriter::write_matrix':"
						" Number of positions does not match.\n");
				return false;
			}

		//	get fresh name
			std::string name(filename);

		//	search for ending and remove
			size_t found = name.find_first_of(".");
			if(found != std::string::npos) name.resize(found);

		#ifdef UG_PARALLEL
		//	add process number
			int rank = pcl::GetProcRank();
			char ext[20];
			sprintf(ext, "_p%04d", rank);
			name.append(ext);
		#endif

		//	add ending
			name.append(".mat");

		//	write to file
			WriteMatrixToConnectionViewer<matrix_type, position_type>
				(name.c_str(), mat, m_pPositions, dim);

		//  we're done
			return true;
		}

	protected:
	//	Positions used in output
		position_type* m_pPositions;

	//	number of positions
		int m_numPos;
};


} // end namespace ug

#endif /* __H__LIB_ALGEBRA__OPERATOR__DEBUG_WRITER__ */
