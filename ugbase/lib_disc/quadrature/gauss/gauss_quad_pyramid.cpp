//  This file is parsed from UG 3.9.
//  It provides the Gauss Quadratures for a reference pyramid.


#include "../quadrature.h"
#include "gauss_quad_pyramid.h"
#include "common/util/provider.h"

namespace ug{

template <>
number GaussQuadBase<GaussQuadrature<ReferencePyramid, 2>, 3, 2, 8>::m_vWeight[8] =
{
1./3. * 0.12500000000000000000000000000000,
1./3. * 0.12500000000000000000000000000000,
1./3. * 0.12500000000000000000000000000000,
1./3. * 0.12500000000000000000000000000000,
1./3. * 0.12500000000000000000000000000000,
1./3. * 0.12500000000000000000000000000000,
1./3. * 0.12500000000000000000000000000000,
1./3. * 0.12500000000000000000000000000000
};

template <>
MathVector<3> GaussQuadBase<GaussQuadrature<ReferencePyramid, 2>, 3, 2, 8>::m_vPoint[8] =
{
MathVector<3>(0.58541020000000000000000000000000, 0.72819660000000000000000000000000, 0.13819660000000000000000000000000),
MathVector<3>(0.13819660000000000000000000000000, 0.72819660000000000000000000000000, 0.13819660000000000000000000000000),
MathVector<3>(0.13819660000000000000000000000000, 0.27630920000000000000000000000000, 0.58541020000000000000000000000000),
MathVector<3>(0.13819660000000000000000000000000, 0.27630920000000000000000000000000, 0.13819660000000000000000000000000),
MathVector<3>(0.72819660000000000000000000000000, 0.13819660000000000000000000000000, 0.13819660000000000000000000000000),
MathVector<3>(0.72819660000000000000000000000000, 0.58541020000000000000000000000000, 0.13819660000000000000000000000000),
MathVector<3>(0.27630920000000000000000000000000, 0.13819660000000000000000000000000, 0.58541020000000000000000000000000),
MathVector<3>(0.27630920000000000000000000000000, 0.13819660000000000000000000000000, 0.13819660000000000000000000000000)
};




template <>
FlexGaussQuadrature<ReferencePyramid>::FlexGaussQuadrature(int order)
{
	switch(order)
	{
	case 0:
	case 1:
	case 2:
		const static GaussQuadrature<ReferencePyramid, 2>& q2 
			= Provider<GaussQuadrature<ReferencePyramid, 2> >::get();

		m_order = q2.order();
		m_numPoints = q2.size();
		m_pvPoint = q2.points();
		m_pvWeight = q2.weights();
		break;

	default: UG_THROW("Order "<<order<<" not available for GaussQuadrature of pyramid.");
	}
}
}; // namespace ug
