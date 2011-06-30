// created by Sebastian Reiter
// s.b.reiter@googlemail.com
// 25.05.2011 (m,d,y)

#ifndef __H__UG__domain_distribution__
#define __H__UG__domain_distribution__

#include "lib_discretization/domain.h"
#include "lib_grid/tools/partition_map.h"

#ifdef UG_PARALLEL
	#include "lib_grid/parallelization/load_balancing.h"
	#include "lib_grid/parallelization/parallelization.h"
#endif

namespace ug
{

class PartitionMap;

///	partitions a domain by repeatedly cutting it along the different axis
template <typename TDomain>
static bool PartitionDomain_Bisection(TDomain& domain, PartitionMap& partitionMap,
									  int firstAxisToCut);

///	partitions a domain by sorting all elements into a regular grid
template <typename TDomain>
static bool PartitionDomain_RegularGrid(TDomain& domain, PartitionMap& partitionMap,
										int numCellsX, int numCellsY,
										bool surfaceOnly);

///	partitions a domain by using graph-based partitioning by METIS
template <typename TDomain>
static bool
PartitionDomain_MetisKWay(TDomain& domain, PartitionMap& partitionMap,
						  int numPartitions, size_t baseLevel = 0,
						  int hWeight = 1, int vWeight = 1);


///	distributes a already distributed domain onto the specified processes
template <typename TDomain>
static bool RedistributeDomain(TDomain& domainOut,
							   PartitionMap& partitionMap,
							   bool createVerticalInterfaces);


///	distributes the domain on process 0 to all active processes...
/**	DEPRECIATED. This method should at least take a partition map.
 * In the near future however we will probably replace DistributeDomain
 * with RedistributeDomain completely.*/
template <typename TDomain>
static bool DistributeDomain(TDomain& domainOut);

}//	end of namespace

////////////////////////////////
//	include implementation
#include "domain_distribution_impl.hpp"

#endif
