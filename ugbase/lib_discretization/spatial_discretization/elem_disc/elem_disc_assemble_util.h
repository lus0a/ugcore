/*
 * assemble_elem_disc.h
 *
 *  Created on: 08.07.2010
 *      Author: andreasvogel
 */

#ifndef __H__LIB_DISCRETIZATION__SPATIAL_DISCRETIZATION__ELEM_DISC__ELEM_DISC_ASSEMBLE_UTIL__
#define __H__LIB_DISCRETIZATION__SPATIAL_DISCRETIZATION__ELEM_DISC__ELEM_DISC_ASSEMBLE_UTIL__

// extern includes
#include <iostream>
#include <vector>

// other ug4 modules
#include "common/common.h"
#include "lib_grid/lg_base.h"

// intern headers
#include "../../reference_element/reference_element.h"
#include "./elem_disc_interface.h"
#include "lib_discretization/common/function_group.h"
#include "lib_discretization/spatial_discretization/ip_data/data_evaluator.h"

#define PROFILE_ELEM_LOOP
#ifdef PROFILE_ELEM_LOOP
	#define EL_PROFILE_FUNC()		PROFILE_FUNC()
	#define EL_PROFILE_BEGIN(name)	PROFILE_BEGIN(name)
	#define EL_PROFILE_END()		PROFILE_END()
#else
	#define EL_PROFILE_FUNC()
	#define EL_PROFILE_BEGIN(name)
	#define EL_PROFILE_END()
#endif


namespace ug {

////////////////////////////////////////////////////////////////////////////////
// Assemble Stiffness Matrix
////////////////////////////////////////////////////////////////////////////////

/**
 * This function adds to the Stiffness matrix the entries of one subset for all passed
 * element discretizations.
 *
 * \param[in]		vElemDisc		element discretizations
 * \param[in]		dofDistr		DoF Distribution
 * \param[in]		si				subset index
 * \param[in]		bNonRegularGrid flag to indicate if non regular grid is used
 * \param[in,out]	A				Stiffness matrix
 * \param[in]		u				solution
 */
template <	typename TElem,
			typename TDoFDistribution,
			typename TAlgebra>
bool
AssembleStiffnessMatrix(	const std::vector<IElemDisc*>& vElemDisc,
                        	const IDoFDistribution<TDoFDistribution>& dofDistr,
                        	int si, bool bNonRegularGrid,
                        	typename TAlgebra::matrix_type& A,
                        	const typename TAlgebra::vector_type& u,
                        	ISelector* sel = NULL)
{
// 	check if at least on element exist, else return
	if(dofDistr.template num<TElem>(si) == 0) return true;

//	create data evaluator
	DataEvaluator Eval;

//	prepare for given elem discs
	if(!Eval.set_elem_discs(vElemDisc, dofDistr.get_function_pattern(), bNonRegularGrid))
	{
		UG_LOG("ERROR in 'AssembleStiffnessMatrix': "
				"Cannot init DataEvaluator with IElemDiscs.\n");
		return false;
	}

	Eval.set_time_dependent(false);

// 	local indices and local algebra
	LocalIndices ind; LocalVector locU; LocalMatrix locA;

//	prepare element discs
	if(!Eval.template prepare_elem_loop<TElem>(ind))
	{
		UG_LOG("ERROR in 'AssembleStiffnessMatrix': "
				"Cannot prepare element loop.\n");
		return false;
	}

//	get element iterator
	typename geometry_traits<TElem>::const_iterator iter, iterBegin, iterEnd;
	iterBegin = dofDistr.template begin<TElem>(si);
	iterEnd = dofDistr.template end<TElem>(si);

// 	Loop over all elements
	for(iter = iterBegin; iter != iterEnd; ++iter)
	{
	// 	get Element
		TElem* elem = *iter;

	//	check if elem is skipped from assembling
		if(sel) if(!sel->is_selected(elem)) continue;

	// 	get global indices
		dofDistr.indices(elem, ind, Eval.use_hanging());

	// 	adapt local algebra
		locU.resize(ind); locA.resize(ind);

	// 	read local values of u
		GetLocalVector(locU, u);

	// 	prepare element
		if(!Eval.prepare_element(elem, locU, ind))
		{
			UG_LOG("ERROR in 'AssembleStiffnessMatrix': "
					"Cannot prepare element.\n");
			return false;
		}

	//	Compute element data
		if(!Eval.compute_elem_data(locU, ind, true))
		{
			UG_LOG("ERROR in 'AssembleStiffnessMatrix': "
					"Cannot compute element data.\n");
			return false;
		}

	//	Evaluate lin defect A
		if(!Eval.compute_lin_defect_JA(locU, ind))
		{
			UG_LOG("ERROR in 'AssembleStiffnessMatrix': "
					"Cannot compute lin_defect_JA.\n");
			return false;
		}

	// 	Assemble JA
		locA = 0.0;
		if(!Eval.assemble_JA(locA, locU))
		{
			UG_LOG("ERROR in 'AssembleStiffnessMatrix': "
					"Cannot compute element contribution to Jacobian (A).\n");
			return false;
		}

	//	add couplings
		Eval.add_coupl_JA(locA, ind);

	// 	send local to global matrix
		AddLocalMatrix(A, locA);
	}

// 	finish element loop
	if(!Eval.finish_element_loop())
	{
		UG_LOG("ERROR in 'AssembleStiffnessMatrix': "
				"Cannot finish element loop.\n");
		return false;
	}

//	we're done
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Assemble Mass Matrix
////////////////////////////////////////////////////////////////////////////////
/**
 * This function adds to the Mass matrix the entries of one subset for all passed
 * element discretizations.
 *
 * \param[in]		vElemDisc		element discretizations
 * \param[in]		dofDistr		DoF Distribution
 * \param[in]		si				subset index
 * \param[in]		bNonRegularGrid flag to indicate if non regular grid is used
 * \param[in,out]	M				Mass matrix
 * \param[in]		u				solution
 */
template <	typename TElem,
			typename TDoFDistribution,
			typename TAlgebra>
bool
AssembleMassMatrix(	const std::vector<IElemDisc*>& vElemDisc,
					const IDoFDistribution<TDoFDistribution>& dofDistr,
					int si, bool bNonRegularGrid,
					typename TAlgebra::matrix_type& M,
					const typename TAlgebra::vector_type& u,
                	ISelector* sel = NULL)
{
// 	check if at least on element exist, else return
	if(dofDistr.template num<TElem>(si) == 0) return true;

//	create data evaluator
	DataEvaluator Eval;

//	prepare for given elem discs
	if(!Eval.set_elem_discs(vElemDisc, dofDistr.get_function_pattern(), bNonRegularGrid))
	{
		UG_LOG("ERROR in 'AssembleMassMatrix': "
				"Cannot init DataEvaluator with IElemDiscs.\n");
		return false;
	}

	Eval.set_time_dependent(false);

// 	local indices and local algebra
	LocalIndices ind; LocalVector locU; LocalMatrix locM;

//	prepare element discs
	if(!Eval.template prepare_elem_loop<TElem>(ind))
	{
		UG_LOG("ERROR in 'AssembleMassMatrix': "
				"Cannot prepare element loop.\n");
		return false;
	}

//	get element iterator
	typename geometry_traits<TElem>::const_iterator iter, iterBegin, iterEnd;
	iterBegin = dofDistr.template begin<TElem>(si);
	iterEnd = dofDistr.template end<TElem>(si);

// 	Loop over all elements
	for(iter = iterBegin; iter != iterEnd; ++iter)
	{
	// 	get Element
		TElem* elem = *iter;

	//	check if elem is skipped from assembling
		if(sel) if(!sel->is_selected(elem)) continue;

	// 	get global indices
		dofDistr.indices(elem, ind, Eval.use_hanging());

	// 	adapt local algebra
		locU.resize(ind); locM.resize(ind);

	// 	read local values of u
		GetLocalVector(locU, u);

	// 	prepare element
		if(!Eval.prepare_element(elem, locU, ind))
		{
			UG_LOG("ERROR in 'AssembleMassMatrix': "
					"Cannot prepare element.\n");
			return false;
		}

	//	Compute element data
		if(!Eval.compute_elem_data(locU, ind, true))
		{
			UG_LOG("ERROR in 'AssembleMassMatrix': "
					"Cannot compute element data.\n");
			return false;
		}

	//	Evaluate lin defect M
		if(!Eval.compute_lin_defect_JM(locU, ind))
		{
			UG_LOG("ERROR in 'AssembleMassMatrix': "
					"Cannot compute lin_defect_JA.\n");
			return false;
		}

	// 	Assemble JM
		locM = 0.0;
		if(!Eval.assemble_JM(locM, locU))
		{
			UG_LOG("ERROR in 'AssembleMassMatrix': "
					"Cannot compute element contribution to Jacobian (A).\n");
			return false;
		}

	//	add couplings
		Eval.add_coupl_JM(locM, ind);

	// 	send local to global matrix
		AddLocalMatrix(M, locM);
	}

// 	finish element loop
	if(!Eval.finish_element_loop())
	{
		UG_LOG("ERROR in 'AssembleMassMatrix': "
				"Cannot finish element loop.\n");
		return false;
	}

//	we're done
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Assemble (stationary) Jacobian
////////////////////////////////////////////////////////////////////////////////

/**
 * This function adds to the jacobian the entries of one subset for all passed
 * element discretizations.
 *
 * \param[in]		vElemDisc		element discretizations
 * \param[in]		dofDistr		DoF Distribution
 * \param[in]		si				subset index
 * \param[in]		bNonRegularGrid flag to indicate if non regular grid is used
 * \param[in,out]	J				jacobian
 * \param[in]		u				solution
 */
template <	typename TElem,
			typename TDoFDistribution,
			typename TAlgebra>
bool
AssembleJacobian(	const std::vector<IElemDisc*>& vElemDisc,
					const IDoFDistribution<TDoFDistribution>& dofDistr,
					int si, bool bNonRegularGrid,
					typename TAlgebra::matrix_type& J,
					const typename TAlgebra::vector_type& u,
                	ISelector* sel = NULL)
{
// 	check if at least on element exist, else return
	if(dofDistr.template num<TElem>(si) == 0) return true;

//	create data evaluator
	DataEvaluator Eval;

//	prepare for given elem discs
	if(!Eval.set_elem_discs(vElemDisc, dofDistr.get_function_pattern(), bNonRegularGrid))
	{
		UG_LOG("ERROR in '(stationary) AssembleJacobian': "
				"Cannot init DataEvaluator with IElemDiscs.\n");
		return false;
	}

	Eval.set_time_dependent(false);

// 	local indices and local algebra
	LocalIndices ind; LocalVector locU; LocalMatrix locJ;

//	prepare element discs
	if(!Eval.template prepare_elem_loop<TElem>(ind))
	{
		UG_LOG("ERROR in '(stationary) AssembleJacobian': "
				"Cannot prepare element loop.\n");
		return false;
	}

//	get element iterator
	typename geometry_traits<TElem>::const_iterator iter, iterBegin, iterEnd;
	iterBegin = dofDistr.template begin<TElem>(si);
	iterEnd = dofDistr.template end<TElem>(si);

// 	Loop over all elements
	for(iter = iterBegin; iter != iterEnd; ++iter)
	{
	// 	get Element
		TElem* elem = *iter;

	//	check if elem is skipped from assembling
		if(sel) if(!sel->is_selected(elem)) continue;

	// 	get global indices
		dofDistr.indices(elem, ind, Eval.use_hanging());

	// 	adapt local algebra
		locU.resize(ind); locJ.resize(ind);

	// 	read local values of u
		GetLocalVector(locU, u);

	// 	prepare element
		if(!Eval.prepare_element(elem, locU, ind))
		{
			UG_LOG("ERROR in '(stationary) AssembleJacobian': "
					"Cannot prepare element.\n");
			return false;
		}

	//	Compute element data
		if(!Eval.compute_elem_data(locU, ind, true))
		{
			UG_LOG("ERROR in '(stationary) AssembleJacobian': "
					"Cannot compute element data.\n");
			return false;
		}

	//	Evaluate lin defect A
		if(!Eval.compute_lin_defect_JA(locU, ind))
		{
			UG_LOG("ERROR in '(stationary) AssembleJacobian': "
					"Cannot compute lin_defect_JA.\n");
			return false;
		}

	// 	Assemble JA
		locJ = 0.0;
		if(!Eval.assemble_JA(locJ, locU))
		{
			UG_LOG("ERROR in '(stationary) AssembleJacobian': "
					"Cannot compute element contribution to Jacobian (A).\n");
			return false;
		}

	//	add couplings
		Eval.add_coupl_JA(locJ, ind);

	// 	send local to global matrix
		AddLocalMatrix(J, locJ);
	}

// 	finish element loop
	if(!Eval.finish_element_loop())
	{
		UG_LOG("ERROR in '(stationary) AssembleJacobian': "
				"Cannot finish element loop.\n");
		return false;
	}

//	we're done
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Assemble (instationary) Jacobian
////////////////////////////////////////////////////////////////////////////////

/**
 * This function adds to the jacobian the entries of one subset for all passed
 * element discretizations.
 * Note, that it is assumed	to have s_m0 == 1
 *
 * \param[in]		vElemDisc		element discretizations
 * \param[in]		dofDistr		DoF Distribution
 * \param[in]		si				subset index
 * \param[in]		bNonRegularGrid flag to indicate if non regular grid is used
 * \param[in,out]	J				jacobian
 * \param[in]		u				solution
 * \param[in]		s_a0			scaling factor for stiffness part
 * \param[in]		time			current time
 */
template <	typename TElem,
			typename TDoFDistribution,
			typename TAlgebra>
bool
AssembleJacobian(	const std::vector<IElemDisc*>& vElemDisc,
					const IDoFDistribution<TDoFDistribution>& dofDistr,
					int si, bool bNonRegularGrid,
					typename TAlgebra::matrix_type& J,
					const typename TAlgebra::vector_type& u, number time,
	                const SolutionTimeSeries<typename TAlgebra::vector_type>& solList,
					number s_a0,
                	ISelector* sel = NULL)
{
// 	check if at least on element exist, else return
	if(dofDistr.template num<TElem>(si) == 0) return true;

//	create data evaluator
	DataEvaluator Eval;

//	prepare for given elem discs
	if(!Eval.set_elem_discs(vElemDisc, dofDistr.get_function_pattern(), bNonRegularGrid))
	{
		UG_LOG("ERROR in '(instationary) AssembleJacobian': "
				"Cannot init DataEvaluator with IElemDiscs.\n");
		return false;
	}

//	set time-independent
	LocalVectorTimeSeries locTimeSeries;
	bool bNeedLocTimeSeries = Eval.set_time_dependent(true, time, &locTimeSeries);

// 	local indices and local algebra
	LocalIndices ind; LocalVector locU; LocalMatrix locJ;

//	prepare element discs
	if(!Eval.template prepare_elem_loop<TElem>(ind, time))
	{
		UG_LOG("ERROR in '(instationary) AssembleJacobian': "
				"Cannot prepare element loop.\n");
		return false;
	}

//	get element iterator
	typename geometry_traits<TElem>::const_iterator iter, iterBegin, iterEnd;
	iterBegin = dofDistr.template begin<TElem>(si);
	iterEnd = dofDistr.template end<TElem>(si);

// 	Loop over all elements
	for(iter = iterBegin; iter != iterEnd; ++iter)
	{
	// 	get Element
		TElem* elem = *iter;

	//	check if elem is skipped from assembling
		if(sel) if(!sel->is_selected(elem)) continue;

	// 	get global indices
		dofDistr.indices(elem, ind, Eval.use_hanging());

	// 	adapt local algebra
		locU.resize(ind); locJ.resize(ind);

	// 	read local values of u
		GetLocalVector(locU, u);

	//	read local values of time series
		if(bNeedLocTimeSeries) locTimeSeries.read_values(solList, ind);

	// 	prepare element
		if(!Eval.prepare_element(elem, locU, ind))
		{
			UG_LOG("ERROR in '(instationary) AssembleJacobian': "
					"Cannot prepare element.\n");
			return false;
		}

	//	Compute element data
		if(!Eval.compute_elem_data(locU, ind, true))
		{
			UG_LOG("ERROR in '(instationary) AssembleJacobian': "
					"Cannot compute element data.\n");
			return false;
		}

	//	Evaluate lin defect A
		if(!Eval.compute_lin_defect_JA(locU, ind))
		{
			UG_LOG("ERROR in '(instationary) AssembleJacobian': "
					"Cannot compute lin_defect_JA.\n");
			return false;
		}

	//	Evaluate lin defect M
		if(!Eval.compute_lin_defect_JM(locU, ind))
		{
			UG_LOG("ERROR in '(instationary) AssembleJacobian': "
					"Cannot compute lin_defect_JM.\n");
			return false;
		}

	// 	Assemble JA
		locJ = 0.0;
		if(!Eval.assemble_JA(locJ, locU))
		{
			UG_LOG("ERROR in '(instationary) AssembleJacobian': "
					"Cannot compute element contribution to Jacobian (A).\n");
			return false;
		}

	//	add couplings
		Eval.add_coupl_JA(locJ, ind);

	//	scale stiffness part
		locJ *= s_a0;

	// 	Assemble JM
		if(!Eval.assemble_JM(locJ, locU))
		{
			UG_LOG("ERROR in '(instationary) AssembleJacobian': "
					"Cannot compute element contribution to Jacobian (A).\n");
			return false;
		}

	//	add couplings
		Eval.add_coupl_JM(locJ, ind);

	// 	send local to global matrix
		AddLocalMatrix(J, locJ);
	}

// 	finish element loop
	if(!Eval.finish_element_loop())
	{
		UG_LOG("ERROR in '(instationary) AssembleJacobian': "
				"Cannot finish element loop.\n");
		return false;
	}

//	we're done
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Assemble (stationary) Defect
////////////////////////////////////////////////////////////////////////////////

/**
 * This function adds to the defect the entries of one subset for all passed
 * element discretizations.
 *
 * \param[in]		vElemDisc		element discretizations
 * \param[in]		dofDistr		DoF Distribution
 * \param[in]		si				subset index
 * \param[in]		bNonRegularGrid flag to indicate if non regular grid is used
 * \param[in,out]	d				defect
 * \param[in]		u				solution
 */
template <	typename TElem,
			typename TDoFDistribution,
			typename TAlgebra>
bool
AssembleDefect(	const std::vector<IElemDisc*>& vElemDisc,
               	const IDoFDistribution<TDoFDistribution>& dofDistr,
               	int si, bool bNonRegularGrid,
               	typename TAlgebra::vector_type& d,
               	const typename TAlgebra::vector_type& u,
            	ISelector* sel = NULL)
{
// 	check if at least on element exist, else return
	if(dofDistr.template num<TElem>(si) == 0) return true;

//	create data evaluator
	DataEvaluator Eval;

//	prepare for given elem discs
	if(!Eval.set_elem_discs(vElemDisc, dofDistr.get_function_pattern(), bNonRegularGrid))
	{
		UG_LOG("ERROR in '(stationary) AssembleDefect': "
				"Cannot init DataEvaluator with IElemDiscs.\n");
		return false;
	}

//	set time-independent
	Eval.set_time_dependent(false);

// 	local indices and local algebra
	LocalIndices ind; LocalVector locU, locD, tmpLocD;

//	prepare element discs
	if(!Eval.template prepare_elem_loop<TElem>(ind))
	{
		UG_LOG("ERROR in '(stationary) AssembleDefect': "
				"Cannot prepare element loop.\n");
		return false;
	}

//	get element iterator
	typename geometry_traits<TElem>::const_iterator iter, iterBegin, iterEnd;
	iterBegin = dofDistr.template begin<TElem>(si);
	iterEnd = dofDistr.template end<TElem>(si);

// 	Loop over all elements
	for(iter = iterBegin; iter != iterEnd; ++iter)
	{
	// 	get Element
		TElem* elem = *iter;

	//	check if elem is skipped from assembling
		if(sel) if(!sel->is_selected(elem)) continue;

	// 	get global indices
		dofDistr.indices(elem, ind, Eval.use_hanging());

	// 	adapt local algebra
		locU.resize(ind); locD.resize(ind); tmpLocD.resize(ind);

	// 	read local values of u
		GetLocalVector(locU, u);

	// 	prepare element
		if(!Eval.prepare_element(elem, locU, ind))
		{
			UG_LOG("ERROR in '(stationary) AssembleDefect': "
					"Cannot prepare element.\n");
			return false;
		}

	//	Compute element data
		if(!Eval.compute_elem_data(locU, ind, false))
		{
			UG_LOG("ERROR in '(stationary) AssembleDefect': "
					"Cannot compute element data.\n");
			return false;
		}

	// 	Assemble A
		locD = 0.0;
		if(!Eval.assemble_A(locD, locU))
		{
			UG_LOG("ERROR in '(stationary) AssembleDefect': "
					"Cannot compute element contribution to Jacobian (A).\n");
			return false;
		}

	// 	Assemble rhs
		tmpLocD = 0.0;
		if(!Eval.assemble_rhs(tmpLocD))
		{
			UG_LOG("ERROR in '(stationary) AssembleDefect': "
					"Cannot compute element contribution to Rhs.\n");
			return false;
		}
		locD.scale_append(-1, tmpLocD);

	// 	send local to global rhs
		AddLocalVector(d, locD);
	}

// 	finish element loop
	if(!Eval.finish_element_loop())
	{
		UG_LOG("ERROR in '(stationary) AssembleDefect': "
				"Cannot finish element loop.\n");
		return false;
	}

//	we're done
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Assemble (instationary) Defect
////////////////////////////////////////////////////////////////////////////////

/**
 * This function adds to the defect the entries of one subset for all passed
 * element discretizations.
 *
 * \param[in]		vElemDisc		element discretizations
 * \param[in]		dofDistr		DoF Distribution
 * \param[in]		si				subset index
 * \param[in]		bNonRegularGrid flag to indicate if non regular grid is used
 * \param[in,out]	d				defect
 * \param[in]		u				solution
 * \param[in]		s_m				scaling factor for mass part
 * \param[in]		s_a				scaling factor for stiffness part
 * \param[in]		time			current time
 */
template <	typename TElem,
			typename TDoFDistribution,
			typename TAlgebra>
bool
AssembleDefect(	const std::vector<IElemDisc*>& vElemDisc,
               	const IDoFDistribution<TDoFDistribution>& dofDistr,
               	int si, bool bNonRegularGrid,
               	typename TAlgebra::vector_type& d,
               	const typename TAlgebra::vector_type& u, number time,
                const SolutionTimeSeries<typename TAlgebra::vector_type>& solList,
               	number s_m, number s_a,
            	ISelector* sel = NULL)
{
// 	check if at least on element exist, else return
	if(dofDistr.template num<TElem>(si) == 0) return true;

//	create data evaluator
	DataEvaluator Eval;

//	prepare for given elem discs
	if(!Eval.set_elem_discs(vElemDisc, dofDistr.get_function_pattern(), bNonRegularGrid))
	{
		UG_LOG("ERROR in '(instationary) AssembleDefect': "
				"Cannot init DataEvaluator with IElemDiscs.\n");
		return false;
	}

//	set time-independent
	LocalVectorTimeSeries locTimeSeries;
	bool bNeedLocTimeSeries = Eval.set_time_dependent(true, time, &locTimeSeries);

// 	local indices and local algebra
	LocalIndices ind; LocalVector locU, locD, tmpLocD;

//	prepare element discs
	if(!Eval.template prepare_elem_loop<TElem>(ind, time))
	{
		UG_LOG("ERROR in '(instationary) AssembleDefect': "
				"Cannot prepare element loop.\n");
		return false;
	}

//	get element iterator
	typename geometry_traits<TElem>::const_iterator iter, iterBegin, iterEnd;
	iterBegin = dofDistr.template begin<TElem>(si);
	iterEnd = dofDistr.template end<TElem>(si);

// 	Loop over all elements
	for(iter = iterBegin; iter != iterEnd; ++iter)
	{
	// 	get Element
		TElem* elem = *iter;

	//	check if elem is skipped from assembling
		if(sel) if(!sel->is_selected(elem)) continue;

	// 	get global indices
		dofDistr.indices(elem, ind, Eval.use_hanging());

	// 	adapt local algebra
		locU.resize(ind); locU.resize(ind);
		locD.resize(ind); tmpLocD.resize(ind);

	// 	read local values of u
		GetLocalVector(locU, u);

	//	read local values of time series
		if(bNeedLocTimeSeries) locTimeSeries.read_values(solList, ind);

	// 	prepare element
		if(!Eval.prepare_element(elem, locU, ind))
		{
			UG_LOG("ERROR in '(instationary) AssembleDefect': "
					"Cannot prepare element.\n");
			return false;
		}

	//	Compute element data
		if(!Eval.compute_elem_data(locU, ind, false))
		{
			UG_LOG("ERROR in '(instationary) AssembleDefect': "
					"Cannot compute element data.\n");
			return false;
		}

	// 	Assemble M
		locD = 0.0;
		if(!Eval.assemble_M(locD, locU))
		{
			UG_LOG("ERROR in '(instationary) AssembleDefect': "
					"Cannot compute element contribution to Jacobian (A).\n");
			return false;
		}
		locD *= s_m;

	// 	Assemble A
		tmpLocD = 0.0;
		if(!Eval.assemble_A(tmpLocD, locU))
		{
			UG_LOG("ERROR in '(instationary) AssembleDefect': "
					"Cannot compute element contribution to Jacobian (A).\n");
			return false;
		}
		locD.scale_append(s_a, tmpLocD);

	// 	Assemble rhs
		tmpLocD = 0.0;
		if(!Eval.assemble_rhs(tmpLocD))
		{
			UG_LOG("ERROR in '(instationary) AssembleDefect': "
					"Cannot compute element contribution to Rhs.\n");
			return false;
		}
		locD.scale_append( -s_a, tmpLocD);

	// 	send local to global rhs
		AddLocalVector(d, locD);
	}

// 	finish element loop
	if(!Eval.finish_element_loop())
	{
		UG_LOG("ERROR in '(instationary) AssembleDefect': "
				"Cannot finish element loop.\n");
		return false;
	}

//	we're done
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Assemble (stationary) Linear
////////////////////////////////////////////////////////////////////////////////

/**
 * This function adds to the Matrix and to the Rhs the entries of one subset
 * for all passed element discretizations.
 *
 * \param[in]		vElemDisc		element discretizations
 * \param[in]		dofDistr		DoF Distribution
 * \param[in]		si				subset index
 * \param[in]		bNonRegularGrid flag to indicate if non regular grid is used
 * \param[in,out]	A				Matrix
 * \param[in,out]	rhs				Right-hand side
 * \param[in]		u				solution
 */
template <	typename TElem,
			typename TDoFDistribution,
			typename TAlgebra>
bool
AssembleLinear(	const std::vector<IElemDisc*>& vElemDisc,
               	const IDoFDistribution<TDoFDistribution>& dofDistr,
               	int si, bool bNonRegularGrid,
               	typename TAlgebra::matrix_type& A,
               	typename TAlgebra::vector_type& rhs,
               	const typename TAlgebra::vector_type& u,
            	ISelector* sel = NULL)
{
// 	check if at least on element exist, else return
	if(dofDistr.template num<TElem>(si) == 0) return true;

//	create data evaluator
	DataEvaluator Eval;

//	prepare for given elem discs
	if(!Eval.set_elem_discs(vElemDisc, dofDistr.get_function_pattern(), bNonRegularGrid))
	{
		UG_LOG("ERROR in '(stationary) AssembleLinear': "
				"Cannot init DataEvaluator with IElemDiscs.\n");
		return false;
	}

//	set time-independent
	Eval.set_time_dependent(false);

// 	local indices and local algebra
	LocalIndices ind; LocalVector locU, locRhs; LocalMatrix locA;

//	prepare element discs
	if(!Eval.template prepare_elem_loop<TElem>(ind))
	{
		UG_LOG("ERROR in '(stationary) AssembleLinear': "
				"Cannot prepare element loop.\n");
		return false;
	}

//	get element iterator
	typename geometry_traits<TElem>::const_iterator iter, iterBegin, iterEnd;
	iterBegin = dofDistr.template begin<TElem>(si);
	iterEnd = dofDistr.template end<TElem>(si);

// 	Loop over all elements
	for(iter = iterBegin; iter != iterEnd; ++iter)
	{
	// 	get Element
		TElem* elem = *iter;

	//	check if elem is skipped from assembling
		if(sel) if(!sel->is_selected(elem)) continue;

	// 	get global indices
		dofDistr.indices(elem, ind, Eval.use_hanging());

	// 	adapt local algebra
		locU.resize(ind); locRhs.resize(ind); locA.resize(ind);

	// 	read local values of u
		GetLocalVector(locU, u);

	// 	prepare element
		if(!Eval.prepare_element(elem, locU, ind))
		{
			UG_LOG("ERROR in '(stationary) AssembleLinear': "
					"Cannot prepare element.\n");
			return false;
		}

	//	Compute element data
		if(!Eval.compute_elem_data(locU, ind, false))
		{
			UG_LOG("ERROR in '(stationary) AssembleLinear': "
					"Cannot compute element data.\n");
			return false;
		}

	// 	Assemble JA
		locA = 0.0;
		if(!Eval.assemble_JA(locA, locU))
		{
			UG_LOG("ERROR in '(stationary) AssembleLinear': "
					"Cannot compute element contribution to Jacobian (A).\n");
			return false;
		}

	// 	Assemble rhs
		locRhs = 0.0;
		if(!Eval.assemble_rhs(locRhs))
		{
			UG_LOG("ERROR in '(stationary) AssembleLinear': "
					"Cannot compute element contribution to Rhs.\n");
			return false;
		}

	// 	send local to global matrix
		AddLocalMatrix(A, locA);

	// 	send local to global rhs
		AddLocalVector(rhs, locRhs);
	}

// 	finish element loop
	if(!Eval.finish_element_loop())
	{
		UG_LOG("ERROR in '(stationary) AssembleLinear': "
				"Cannot finish element loop.\n");
		return false;
	}

//	we're done
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Assemble (instationary) Linear
////////////////////////////////////////////////////////////////////////////////

/**
 * This function adds to the Matrix and to the Rhs the entries of one subset
 * for all passed element discretizations.
 *
 * \param[in]		vElemDisc		element discretizations
 * \param[in]		dofDistr		DoF Distribution
 * \param[in]		si				subset index
 * \param[in]		bNonRegularGrid flag to indicate if non regular grid is used
 * \param[in,out]	A				Matrix
 * \param[in,out]	rhs				Right-hand side
 * \param[in]		u				solution
 * \param[in]		s_m				scaling factor for mass part
 * \param[in]		s_a				scaling factor for stiffness part
 * \param[in]		time			current time
 */
template <	typename TElem,
			typename TDoFDistribution,
			typename TAlgebra>
bool
AssembleLinear(	const std::vector<IElemDisc*>& vElemDisc,
               	const IDoFDistribution<TDoFDistribution>& dofDistr,
               	int si, bool bNonRegularGrid,
               	typename TAlgebra::matrix_type& A,
               	typename TAlgebra::vector_type& rhs,
               	const typename TAlgebra::vector_type& u, number time,
                const SolutionTimeSeries<typename TAlgebra::vector_type>& solList,
               	number s_m, number s_a,
            	ISelector* sel = NULL)
{
// 	check if at least on element exist, else return
	if(dofDistr.template num<TElem>(si) == 0) return true;

//	create data evaluator
	DataEvaluator Eval;

//	prepare for given elem discs
	if(!Eval.set_elem_discs(vElemDisc, dofDistr.get_function_pattern(), bNonRegularGrid))
	{
		UG_LOG("ERROR in '(instationary) AssembleLinear': "
				"Cannot init DataEvaluator with IElemDiscs.\n");
		return false;
	}

//	set time-independent
	LocalVectorTimeSeries locTimeSeries;
	bool bNeedLocTimeSeries = Eval.set_time_dependent(true, time, &locTimeSeries);

// 	local indices and local algebra
	LocalIndices ind; LocalVector locU, locRhs; LocalMatrix locA, tmpLocA;

//	prepare element discs
	if(!Eval.template prepare_elem_loop<TElem>(ind, time))
	{
		UG_LOG("ERROR in '(instationary) AssembleLinear': "
				"Cannot prepare element loop.\n");
		return false;
	}

//	get element iterator
	typename geometry_traits<TElem>::const_iterator iter, iterBegin, iterEnd;
	iterBegin = dofDistr.template begin<TElem>(si);
	iterEnd = dofDistr.template end<TElem>(si);

// 	Loop over all elements
	for(iter = iterBegin; iter != iterEnd; ++iter)
	{
	// 	get Element
		TElem* elem = *iter;

	//	check if elem is skipped from assembling
		if(sel) if(!sel->is_selected(elem)) continue;

	// 	get global indices
		dofDistr.indices(elem, ind, Eval.use_hanging());

	// 	adapt local algebra
		locU.resize(ind); locRhs.resize(ind);
		locA.resize(ind); tmpLocA.resize(ind);

	// 	read local values of u
		GetLocalVector(locU, u);

	//	read local values of time series
		if(bNeedLocTimeSeries) locTimeSeries.read_values(solList, ind);

	// 	prepare element
		if(!Eval.prepare_element(elem, locU, ind))
		{
			UG_LOG("ERROR in '(instationary) AssembleLinear': "
					"Cannot prepare element.\n");
			return false;
		}

	//	Compute element data
		if(!Eval.compute_elem_data(locU, ind, false))
		{
			UG_LOG("ERROR in '(instationary) AssembleLinear': "
					"Cannot compute element data.\n");
			return false;
		}

	// 	Assemble JM
		locA = 0.0;
		if(!Eval.assemble_JA(locA, locU))
		{
			UG_LOG("ERROR in '(instationary) AssembleLinear': "
					"Cannot compute element contribution to Jacobian (A).\n");
			return false;
		}
		locA *= s_m;

	// 	Assemble JA
		tmpLocA = 0.0;
		if(!Eval.assemble_JA(tmpLocA, locU))
		{
			UG_LOG("ERROR in '(instationary) AssembleLinear': "
					"Cannot compute element contribution to Jacobian (A).\n");
			return false;
		}
		locA.scale_append(s_a, tmpLocA);

	// 	Assemble rhs
		locRhs = 0.0;
		if(!Eval.assemble_rhs(locRhs))
		{
			UG_LOG("ERROR in '(instationary) AssembleLinear': "
					"Cannot compute element contribution to Rhs.\n");
			return false;
		}
		locRhs *= s_a;

	// 	send local to global matrix
		AddLocalMatrix(A, locA);

	// 	send local to global rhs
		AddLocalVector(rhs, locRhs);
	}

// 	finish element loop
	if(!Eval.finish_element_loop())
	{
		UG_LOG("ERROR in '(instationary) AssembleLinear': "
				"Cannot finish element loop.\n");
		return false;
	}

//	we're done
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Assemble Rhs
////////////////////////////////////////////////////////////////////////////////

/**
 * This function adds to the Rhs the entries of one subset
 * for all passed element discretizations.
 *
 * \param[in]		vElemDisc		element discretizations
 * \param[in]		dofDistr		DoF Distribution
 * \param[in]		si				subset index
 * \param[in]		bNonRegularGrid flag to indicate if non regular grid is used
 * \param[in,out]	rhs				Right-hand side
 * \param[in]		u				solution
 */
template <	typename TElem,
			typename TDoFDistribution,
			typename TAlgebra>
bool
AssembleRhs(	const std::vector<IElemDisc*>& vElemDisc,
               	const IDoFDistribution<TDoFDistribution>& dofDistr,
               	int si, bool bNonRegularGrid,
               	typename TAlgebra::vector_type& rhs,
               	const typename TAlgebra::vector_type& u,
            	ISelector* sel = NULL)
{
// 	check if at least on element exist, else return
	if(dofDistr.template num<TElem>(si) == 0) return true;

//	create data evaluator
	DataEvaluator Eval;

//	prepare for given elem discs
	if(!Eval.set_elem_discs(vElemDisc, dofDistr.get_function_pattern(), bNonRegularGrid))
	{
		UG_LOG("ERROR in 'AssembleRhs': "
				"Cannot init DataEvaluator with IElemDiscs.\n");
		return false;
	}

//	set time-independent
	Eval.set_time_dependent(false);

// 	local indices and local algebra
	LocalIndices ind; LocalVector locU, locRhs;

//	prepare element discs
	if(!Eval.template prepare_elem_loop<TElem>(ind))
	{
		UG_LOG("ERROR in 'AssembleRhs': "
				"Cannot prepare element loop.\n");
		return false;
	}

//	get element iterator
	typename geometry_traits<TElem>::const_iterator iter, iterBegin, iterEnd;
	iterBegin = dofDistr.template begin<TElem>(si);
	iterEnd = dofDistr.template end<TElem>(si);

// 	Loop over all elements
	for(iter = iterBegin; iter != iterEnd; ++iter)
	{
	// 	get Element
		TElem* elem = *iter;

	//	check if elem is skipped from assembling
		if(sel) if(!sel->is_selected(elem)) continue;

	// 	get global indices
		dofDistr.indices(elem, ind, Eval.use_hanging());

	// 	adapt local algebra
		locU.resize(ind); locRhs.resize(ind);

	// 	read local values of u
		GetLocalVector(locU, u);

	// 	prepare element
		if(!Eval.prepare_element(elem, locU, ind))
		{
			UG_LOG("ERROR in 'AssembleRhs': "
					"Cannot prepare element.\n");
			return false;
		}

	//	Compute element data
		if(!Eval.compute_elem_data(locU, ind, false))
		{
			UG_LOG("ERROR in 'AssembleRhs': "
					"Cannot compute element data.\n");
			return false;
		}

	// 	Assemble rhs
		locRhs = 0.0;
		if(!Eval.assemble_rhs(locRhs))
		{
			UG_LOG("ERROR in 'AssembleRhs': "
					"Cannot compute element contribution to Rhs.\n");
			return false;
		}

	// 	send local to global rhs
		AddLocalVector(rhs, locRhs);
	}

// 	finish element loop
	if(!Eval.finish_element_loop())
	{
		UG_LOG("ERROR in 'AssembleRhs': "
				"Cannot finish element loop.\n");
		return false;
	}

//	we're done
	return true;
}

} // end namespace ug


#endif /* __H__LIB_DISCRETIZATION__SPATIAL_DISCRETIZATION__ELEM_DISC__ELEM_DISC_ASSEMBLE_UTIL__ */
