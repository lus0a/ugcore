/*
 * integrate_bridge.cpp
 *
 *  Created on: 21.05.2012
 *      Author: andreasvogel
 */

// extern headers
#include <iostream>
#include <sstream>
#include <string>

// include bridge
#include "bridge/bridge.h"
#include "bridge/util.h"

// lib_disc includes
#include "lib_disc/function_spaces/grid_function.h"
#include "lib_disc/dof_manager/surface_dof_distribution.h"
#include "lib_disc/function_spaces/integrate.h"

using namespace std;

namespace ug{
namespace bridge{
namespace Integrate{

/**
 * Class exporting the functionality. All functionality that is to
 * be used in scripts or visualization must be registered here.
 */
struct Functionality
{

/**
 * Function called for the registration of Domain and Algebra dependent parts.
 * All Functions and Classes depending on both Domain and Algebra
 * are to be placed here when registering. The method is called for all
 * available Domain and Algebra types, based on the current build options.
 *
 * @param reg				registry
 * @param parentGroup		group for sorting of functionality
 */
template <typename TDomain, typename TAlgebra>
static void DomainAlgebra(Registry& reg, string grp)
{
//	typedef
	static const int dim = TDomain::dim;
	typedef GridFunction<TDomain, SurfaceDoFDistribution, TAlgebra> TFct;

//	Integral
	{
		reg.add_function("Integral", static_cast<number (*)(SmartPtr<IDirectIPData<number,dim> >, SmartPtr<TFct>, number, int, const char*)>(&Integral<TFct>), grp);
		reg.add_function("Integral", static_cast<number (*)(number, SmartPtr<TFct>, number, int, const char*)>(&Integral<TFct>), grp);
#ifdef UG_FOR_LUA
		reg.add_function("Integral", static_cast<number (*)(const char*, SmartPtr<TFct>, number, int, const char*)>(&Integral<TFct>), grp);
#endif
	}

//	L2Error
	{
		reg.add_function("L2Error",static_cast<number (*)(IPData<number, dim>&, TFct&, const char*, number, int, const char*)>(&L2Error<TFct>), grp);
#ifdef UG_FOR_LUA
		reg.add_function("L2Error",static_cast<number (*)(const char*, TFct&, const char*, number, int, const char*)>(&L2Error<TFct>), grp);
#endif
	}

//	L2Norm
	{
		typedef number (*fct_type)(TFct&, const char*, int, const char*);
		reg.add_function("L2Norm",static_cast<fct_type>(&L2Norm<TFct>),grp);
	}

//	StdFuncIntegral
	{
		typedef number (*fct_type)(TFct&, const char*, int, const char*);
		reg.add_function("StdFuncIntegral",static_cast<fct_type>(&StdFuncIntegral<TFct>),grp);
	}

//	IntegrateFluxOnBoundary
	{
		typedef number (*fct_type)(TFct&, const char*, const char*, const char*);
		reg.add_function("IntegrateFluxOnBoundary",static_cast<fct_type>(&IntegrateFluxOnBoundary<TFct>),grp,
		                 "Integral", "GridFunction#Component#BoundarySubset#InnerSubset");
	}



}

}; // end Functionality
}// end Integrate

void RegisterBridge_Integrate(Registry& reg, string grp)
{
	grp.append("/Discretization");
	typedef Integrate::Functionality Functionality;

	try{
		RegisterDomainAlgebraDependent<Functionality>(reg,grp);
	}
	UG_REGISTRY_CATCH_THROW(grp);
}

}//	end of namespace bridge
}//	end of namespace ug
