/*
 * interpolate.h
 *
 *  Created on: 22.02.2010
 *      Author: andreasvogel
 */

#ifndef __H__UG__LIB_DISC__FUNCTION_SPACES__INTERPOLATE__
#define __H__UG__LIB_DISC__FUNCTION_SPACES__INTERPOLATE__

#include "common/common.h"

#include "lib_disc/common/subset_group.h"
#include "lib_disc/local_finite_element/local_shape_function_set.h"
#include <boost/function.hpp>
#include "lib_disc/spatial_disc/ip_data/const_user_data.h"

#ifdef UG_FOR_LUA
#include "bindings/lua/lua_user_data.h"
#endif

namespace ug{

////////////////////////////////////////////////////////////////////////////////
// Interpolate on Vertices only
////////////////////////////////////////////////////////////////////////////////

/// interpolates a function on vertices
template <typename TGridFunction>
void InterpolateOnVertices(SmartPtr<IDirectIPData<number, TGridFunction::dim> > spInterpolFunction,
                           SmartPtr<TGridFunction> spGridFct,
                           size_t fct,
                           number time,
                           const SubsetGroup& ssGrp)
{
//	domain type and position_type
	typedef typename TGridFunction::domain_type domain_type;
	typedef typename domain_type::position_type position_type;

// get position accessor
	const typename domain_type::position_accessor_type& aaPos
										= spGridFct->domain()->position_accessor();

	std::vector<MultiIndex<2> > ind;
	typename TGridFunction::template dim_traits<0>::const_iterator iterEnd, iter;

	for(size_t i = 0; i < ssGrp.num_subsets(); ++i)
	{
	//	get subset index
		const int si = ssGrp[i];

	//	skip if function is not defined in subset
		if(!spGridFct->is_def_in_subset(fct, si)) continue;

	// 	iterate over all elements
		iterEnd = spGridFct->template end<VertexBase>(si);
		iter = spGridFct->template begin<VertexBase>(si);
		for(; iter != iterEnd; ++iter)
		{
		//	get element
			VertexBase* vrt = *iter;

		//	global position
			position_type glob_pos = aaPos[vrt];

		//	value at position
			number val;
			(*spInterpolFunction)(val, glob_pos, time, si);

		//	get multiindices of element
			spGridFct->multi_indices(vrt, fct, ind);

		// 	loop all dofs
			for(size_t i = 0; i < ind.size(); ++i)
			{
			//	set value
				BlockRef((*spGridFct)[ind[i][0]], ind[i][1]) = val;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Interpolate on Elements
////////////////////////////////////////////////////////////////////////////////

/// interpolates a function on an element
template <typename TElem, typename TGridFunction>
void InterpolateOnElements(
		SmartPtr<IDirectIPData<number, TGridFunction::dim> > spInterpolFunction,
		SmartPtr<TGridFunction> spGridFct, size_t fct, int si, number time)
{
//	get reference element type
	typedef typename reference_element_traits<TElem>::reference_element_type
				ref_elem_type;

//	dimension of reference element
	const int dim = ref_elem_type::dim;

//	domain type and position_type
	typedef typename TGridFunction::domain_type domain_type;
	typedef typename domain_type::position_type position_type;

//	get iterators
	typename TGridFunction::template traits<TElem>::const_iterator iterEnd, iter;
	iterEnd = spGridFct->template end<TElem>(si);
	iter = spGridFct->template begin<TElem>(si);

//	check if something to do:
	if(iter == iterEnd) return;

//	id of shape functions used
	LFEID id = spGridFct->local_finite_element_id(fct);

//	get trial space
	const LocalShapeFunctionSet<ref_elem_type>& trialSpace =
			LocalShapeFunctionSetProvider::get<ref_elem_type>(id);

//	number of dofs on element
	const size_t nsh = trialSpace.num_sh();

// 	load local positions of dofs for the trial space on element
	std::vector<MathVector<dim> > loc_pos(nsh);
	for(size_t i = 0; i < nsh; ++i)
		if(!trialSpace.position(i, loc_pos[i]))
			UG_THROW("InterpolateFunctionOnElem: Cannot find meaningful"
					" local positions of dofs.");

//	create a reference mapping
	ReferenceMapping<ref_elem_type, domain_type::dim> mapping;

// 	iterate over all elements
	for( ; iter != iterEnd; ++iter)
	{
	//	get element
		TElem* elem = *iter;

	//	get all corner coordinates
		std::vector<position_type> vCorner;
		CollectCornerCoordinates(vCorner, *elem, *spGridFct->domain());

	//	update the reference mapping for the corners
		mapping.update(&vCorner[0]);

	//	get multiindices of element
		std::vector<MultiIndex<2> > ind;
		spGridFct->multi_indices(elem, fct, ind);

	//	check multi indices
		if(ind.size() != nsh)
			UG_THROW("InterpolateFunctionOnElem: Number of shapes is "
					<<nsh<<", but got "<<ind.size()<<" multi indices.");

	// 	loop all dofs
		for(size_t i = 0; i < nsh; ++i)
		{
		//	global position
			position_type glob_pos;

		//  map local dof position to global position
			mapping.local_to_global(glob_pos, loc_pos[i]);

		//	value at position
			number val;
			(*spInterpolFunction)(val, glob_pos, time, si);

		//	set value
			BlockRef((*spGridFct)[ind[i][0]], ind[i][1]) = val;
		}
	}
}

template <typename TGridFunction>
void InterpolateOnElements(SmartPtr<IDirectIPData<number, TGridFunction::dim> > spInterpolFunction,
                           SmartPtr<TGridFunction> spGridFct,
                           size_t fct,
                           number time,
                           const SubsetGroup& ssGrp)
{
//	loop subsets
	for(size_t i = 0; i < ssGrp.num_subsets(); ++i)
	{
	//	get subset index
		const int si = ssGrp[i];

	//	skip if function is not defined in subset
		if(!spGridFct->is_def_in_subset(fct, si)) continue;

	//	switch dimensions
		try
		{
		const int dim = ssGrp.dim(i);

		if(dim > TGridFunction::dim)
			UG_THROW("InterpolateOnElements: Dimension of subset is "<<dim<<", but "
			         " World Dimension is "<<TGridFunction::dim<<". Cannot interpolate this.");

		switch(dim)
		{
		case DIM_SUBSET_EMPTY_GRID: break;
		case 0: /* \TODO: do nothing may be wrong */	break;
		case 1:
			InterpolateOnElements<Edge, TGridFunction>(spInterpolFunction, spGridFct, fct, si, time);
			break;
		case 2:
			InterpolateOnElements<Triangle, TGridFunction>(spInterpolFunction, spGridFct, fct, si, time);
			InterpolateOnElements<Quadrilateral, TGridFunction>(spInterpolFunction, spGridFct, fct, si, time);
			break;
		case 3:
			InterpolateOnElements<Tetrahedron, TGridFunction>(spInterpolFunction, spGridFct, fct, si, time);
			InterpolateOnElements<Hexahedron, TGridFunction>(spInterpolFunction, spGridFct, fct, si, time);
			InterpolateOnElements<Prism, TGridFunction>(spInterpolFunction, spGridFct, fct, si, time);
			InterpolateOnElements<Pyramid, TGridFunction>(spInterpolFunction, spGridFct, fct, si, time);
			break;
		default: UG_THROW("InterpolateOnElements: Dimension " <<dim<<
		                " not possible for world dim "<<3<<".");
		}
		}
		UG_CATCH_THROW("InterpolateOnElements: Cannot interpolate on elements.");
	}
}

////////////////////////////////////////////////////////////////////////////////
// Interpolate routine
////////////////////////////////////////////////////////////////////////////////


/// interpolates a function on a subset
/**
 * This function interpolates a GridFunction. To evaluate the function on every
 * point a functor must be passed.
 *
 * \param[out]		u			interpolated grid function
 * \param[in]		data		data evaluator
 * \param[in]		name		symbolic name of function
 * \param[in]		time		time point
 * \param[in]		subsets		subsets, where to interpolate
 */
template <typename TGridFunction>
void InterpolateFunction(SmartPtr<IDirectIPData<number, TGridFunction::dim> > spInterpolFunction,
                         SmartPtr<TGridFunction> spGridFct, const char* cmp,
                         number time, const char* subsets)
{
//	get function id of name
	const size_t fct = spGridFct->fct_id_by_name(cmp);

//	check that function found
	if(fct > spGridFct->num_fct())
		UG_THROW("InterpolateFunction: Name of component '"<<cmp<<"' not found.");

//	create subset group
	SubsetGroup ssGrp(spGridFct->domain()->subset_handler());
	if(subsets != NULL)
		ConvertStringToSubsetGroup(ssGrp, spGridFct->domain()->subset_handler(), subsets);
	else // add all if no subset specified
		ssGrp.add_all();

//	forward
	if(spGridFct->local_finite_element_id(fct) == LFEID(LFEID::LAGRANGE, 1))
		InterpolateOnVertices<TGridFunction>(spInterpolFunction, spGridFct, fct, time, ssGrp);
	else
		InterpolateOnElements<TGridFunction>(spInterpolFunction, spGridFct, fct, time, ssGrp);

	//	adjust parallel storage state
#ifdef UG_PARALLEL
	spGridFct->set_storage_type(PST_CONSISTENT);
#endif
}

template <typename TGridFunction>
void InterpolateFunction(SmartPtr<IDirectIPData<number, TGridFunction::dim> > spInterpolFunction,
                         SmartPtr<TGridFunction> spGridFct, const char* cmp,
                         number time)
{InterpolateFunction(spInterpolFunction, spGridFct, cmp, time, NULL);}

///////////////
// const data
///////////////

template <typename TGridFunction>
void InterpolateFunction(number val,
                         SmartPtr<TGridFunction> spGridFct, const char* cmp,
                         number time, const char* subsets)
{
	static const int dim = TGridFunction::dim;
	SmartPtr<IDirectIPData<number, dim> > sp =
			CreateSmartPtr(new ConstUserNumber<dim>(val));
	InterpolateFunction(sp, spGridFct, cmp, time, subsets);
}
template <typename TGridFunction>
void InterpolateFunction(number val,
                         SmartPtr<TGridFunction> spGridFct, const char* cmp,
                         number time)
{InterpolateFunction(val, spGridFct, cmp, time, NULL);}

///////////////
// lua data
///////////////

#ifdef UG_FOR_LUA
template <typename TGridFunction>
void InterpolateFunction(const char* LuaFunction,
                         SmartPtr<TGridFunction> spGridFct, const char* cmp,
                         number time, const char* subsets)
{
	static const int dim = TGridFunction::dim;
	SmartPtr<IDirectIPData<number, dim> > sp =
			LuaUserDataFactory<number, dim>::create(LuaFunction);
	InterpolateFunction(sp, spGridFct, cmp, time, subsets);
}
template <typename TGridFunction>
void InterpolateFunction(const char* LuaFunction,
                         SmartPtr<TGridFunction> spGridFct, const char* cmp,
                         number time)
{InterpolateFunction(LuaFunction, spGridFct, cmp, time, NULL);}
#endif


} // namespace ug

#endif /*__H__UG__LIB_DISC__FUNCTION_SPACES__INTERPOLATE__*/
