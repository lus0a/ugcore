/*
 * groups_util.cpp
 *
 *  Created on: 18.10.2010
 *      Author: andreasvogel
 */

#include "groups_util.h"
#include "common/util/string_util.h"

#include <algorithm>
#include <limits>

using namespace std;

namespace ug{

void
CreateFunctionIndexMapping(FunctionIndexMapping& map,
                           const FunctionGroup& grpFromSmall,
                           const FunctionGroup& grpToLarge)
{
//	clear map
	map.clear();

//	check that groups are based on same function pattern
	if(grpToLarge.function_pattern() != grpFromSmall.function_pattern())
		UG_THROW("CreateFunctionIndexMapping: groups are not based on same "
				"function pattern, thus no mapping can be created.");

//	check that "from group" is contained in "to group"
	if(!grpToLarge.contains(grpFromSmall))
		UG_THROW("CreateFunctionIndexMapping: smaller FunctionGroup "
				<< grpFromSmall << " is not contained in larger Group " <<
				grpToLarge<<". Cannot create Mapping.");

//	loop all functions on grpFrom
	for(size_t from = 0; from < grpFromSmall.size(); ++from)
	{
	//	get unique id of function
		const size_t uniqueID = grpFromSmall[from];

	//	find unique id of function in grpTo
		const size_t locIndex = grpToLarge.local_index(uniqueID);

	//	set mapping
		map.push_back(locIndex);
	}
}

void CreateFunctionIndexMapping(FunctionIndexMapping& map,
                                const FunctionGroup& grpFrom,
                                ConstSmartPtr<FunctionPattern> fctPattern)
{
	FunctionGroup commonFctGroup(fctPattern);
	commonFctGroup.add_all();
	CreateFunctionIndexMapping(map, grpFrom, commonFctGroup);
}


/**
 * This function create the union of function groups. Container is clear at beginning.
 *
 * \param[out]		fctGrp		Union of Functions
 * \param[in]		vFctGrp		Vector of function group (may contain NULL)
 * \param[in]		sortFct		flag if group should be sorted after adding
 */
void CreateUnionOfFunctionGroups(FunctionGroup& fctGrp,
                                 const vector<const FunctionGroup*>& vFctGrp,
                                 bool sortFct)
{
//	clear group
	fctGrp.clear();

//	if empty, nothing to do
	if(vFctGrp.empty()) return;

//	set underlying subsetHandler
	size_t grp = 0;
	for(; grp < vFctGrp.size(); ++grp)
	{
		if(vFctGrp[grp] == NULL) continue;

		ConstSmartPtr<FunctionPattern> pFctPat = vFctGrp[grp]->function_pattern();
		if(pFctPat.invalid())
			UG_THROW("CreateUnionOfFunctionGroups: Function group "
					<<grp<<" has NULL as underlying FunctionPattern.");

		fctGrp.set_function_pattern(pFctPat);
		break;
	}

//	if no function group given
	if(grp == vFctGrp.size()) return;

//	add all Subset groups of the element discs
	for(size_t i = 0; i < vFctGrp.size(); ++i)
	{
	//	add subset group of elem disc
		if(vFctGrp[i] != NULL)
		{
			try{
				fctGrp.add(*vFctGrp[i]);
			}UG_CATCH_THROW("Cannot add functions of the Function Group "<< i << ".");
		}
	}

//	sort iff required
	if(sortFct) fctGrp.sort();
}


} // end namespace ug
