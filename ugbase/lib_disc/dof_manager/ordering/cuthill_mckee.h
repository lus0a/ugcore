/*
 * cuthill_mckee.h
 *
 *  Created on: 21.03.2011
 *      Author: andreasvogel
 */

#ifndef __H__UG__LIB_DISC__DOF_MANAGER__CUTHILL_MCKEE__
#define __H__UG__LIB_DISC__DOF_MANAGER__CUTHILL_MCKEE__

#include <vector>
#include "lib_disc/function_spaces/approximation_space.h"

namespace ug{

/// returns an array describing the needed index mapping for Cuthill-McKee ordering
/**
 * This function computes a index mapping, that transforms a index-graph into
 * Cuthill-McKee ordering. For each index an vector of all adjacent indices
 * must be passed. (If no adjacent index is passed for an index, this index is
 * skipped and not sorted). On exit the index field vNewIndex is filled with
 * the index mapping: newInd = vNewIndex[oldInd]
 *
 * \param[out]	vNewIndex		vector returning new index for old index
 * \param[in]	vvNeighbour		vector of adjacent indices for each index
 * \param[in]	bReverse		flag if "reverse Cuthill-McKee" is used
 * \returns		flag if ordering was successful
 */
void ComputeCuthillMcKeeOrder(std::vector<size_t>& vNewIndex,
                              std::vector<std::vector<size_t> >& vvNeighbour,
                              bool bReverse = true);

/// orders the dof distribution using Cuthill-McKee
inline void OrderCuthillMcKee(DoFDistribution& dofDistr, bool bReverse);

/// orders the all DofDistributions of the ApproximationSpace using Cuthill-McKee
template <typename TDomain>
void OrderCuthillMcKee(ApproximationSpace<TDomain>& approxSpace, bool bReverse);

} // end namespace ug

#endif /* __H__UG__LIB_DISC__DOF_MANAGER__CUTHILL_MCKEE__ */