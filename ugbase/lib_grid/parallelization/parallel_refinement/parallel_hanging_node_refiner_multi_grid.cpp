// created by Sebastian Reiter
// s.b.reiter@googlemail.com
// 09.02.2011 (m,d,y)
 
#include "parallel_hanging_node_refiner_multi_grid.h"
#include "../util/compol_selection.h"

using namespace std;

namespace ug{

ParallelHangingNodeRefiner_MultiGrid::
ParallelHangingNodeRefiner_MultiGrid(
		IRefinementCallback* refCallback) :
	BaseClass(refCallback),
	m_pDistGridMgr(NULL),
	m_bNewInterfaceEdgesMarked(false),
	m_bNewInterfaceFacesMarked(false),
	m_bNewInterfaceVolumesMarked(false)
{
}

ParallelHangingNodeRefiner_MultiGrid::
ParallelHangingNodeRefiner_MultiGrid(
		DistributedGridManager& distGridMgr,
		IRefinementCallback* refCallback) :
	BaseClass(*distGridMgr.get_assigned_grid(), refCallback),
	m_pDistGridMgr(&distGridMgr),
	m_bNewInterfaceEdgesMarked(false),
	m_bNewInterfaceFacesMarked(false),
	m_bNewInterfaceVolumesMarked(false)
{
}

ParallelHangingNodeRefiner_MultiGrid::
~ParallelHangingNodeRefiner_MultiGrid()
{

}

void ParallelHangingNodeRefiner_MultiGrid::
set_distributed_grid_manager(DistributedGridManager& distGridMgr)
{
	m_pDistGridMgr = &distGridMgr;
}

void ParallelHangingNodeRefiner_MultiGrid::
clear_marks()
{
	BaseClass::clear_marks();
	m_bNewInterfaceEdgesMarked = false;
	m_bNewInterfaceFacesMarked = false;
	m_bNewInterfaceVolumesMarked = false;
}

void ParallelHangingNodeRefiner_MultiGrid::
mark_for_refinement(EdgeBase* e)
{
	if((!m_selMarkedElements.is_selected(e))
		&& m_pDistGridMgr->is_interface_element(e))
		m_bNewInterfaceEdgesMarked = true;
	BaseClass::mark_for_refinement(e);
}

void ParallelHangingNodeRefiner_MultiGrid::
mark_for_refinement(Face* f)
{
	if((!m_selMarkedElements.is_selected(f))
		&& m_pDistGridMgr->is_interface_element(f))
		m_bNewInterfaceFacesMarked = true;
	BaseClass::mark_for_refinement(f);
}

void ParallelHangingNodeRefiner_MultiGrid::
mark_for_refinement(Volume* v)
{
	if((!m_selMarkedElements.is_selected(v))
		&& m_pDistGridMgr->is_interface_element(v))
		m_bNewInterfaceVolumesMarked = true;
	BaseClass::mark_for_refinement(v);
}

void ParallelHangingNodeRefiner_MultiGrid::
collect_objects_for_refine()
{
//todo: This method could be improved.
//		In its current implementation a little too much
//		serial work is done.

//	the layoutmap is used for communication
	GridLayoutMap& layoutMap = m_pDistGridMgr->grid_layout_map();

//	first we'll call the base implementation
	while(1){
	//	we call collect_objects_for_refine in each iteration.
	//	This might be a bit of an overkill, since only a few normally
	//	have changed...
		BaseClass::collect_objects_for_refine();

	//	we now have to inform all processes whether interface elements
	//	were marked on any process.
		int newlyMarkedElems = 0;
		if(m_bNewInterfaceEdgesMarked ||
			m_bNewInterfaceFacesMarked ||
			m_bNewInterfaceVolumesMarked)
		{
			newlyMarkedElems = 1;
		}

		UG_LOG("newly marked elems: " << newlyMarkedElems << endl);

		int exchangeFlag;
		m_procCom.allreduce(&newlyMarkedElems, &exchangeFlag, 1,
							PCL_DT_INT, PCL_RO_LOR);

		UG_LOG("exchange flag: " << exchangeFlag << endl);

	//	before we continue we'll set all flags to false
		m_bNewInterfaceEdgesMarked = false;
		m_bNewInterfaceFacesMarked = false;
		m_bNewInterfaceVolumesMarked = false;

		if(exchangeFlag){

		//	we have to communicate the marks.
		//	do this by first gather selection at master nodes
		//	and then distribute them to slaves.
			ComPol_Selection<EdgeLayout> compolSelEDGE(m_selMarkedElements);
			ComPol_Selection<FaceLayout> compolSelFACE(m_selMarkedElements);



		//	send data SLAVE -> MASTER
			m_intfComEDGE.exchange_data(layoutMap, INT_SLAVE, INT_MASTER,
										compolSelEDGE);

			m_intfComFACE.exchange_data(layoutMap, INT_SLAVE, INT_MASTER,
										compolSelFACE);

			m_intfComEDGE.communicate();
			m_intfComFACE.communicate();

		//	and now MASTER -> SLAVE (the selection has been adjusted on the fly)
			m_intfComEDGE.exchange_data(layoutMap, INT_MASTER, INT_SLAVE,
										compolSelEDGE);

			m_intfComFACE.exchange_data(layoutMap, INT_MASTER, INT_SLAVE,
										compolSelFACE);

			m_intfComEDGE.communicate();
			m_intfComFACE.communicate();
		}
		else
			break;
	}
}


bool ParallelHangingNodeRefiner_MultiGrid::
refinement_is_allowed(VertexBase* elem)
{
	return !m_pDistGridMgr->is_ghost(elem);
}

bool ParallelHangingNodeRefiner_MultiGrid::
refinement_is_allowed(EdgeBase* elem)
{
	return !m_pDistGridMgr->is_ghost(elem);
}

bool ParallelHangingNodeRefiner_MultiGrid::
refinement_is_allowed(Face* elem)
{
	return !m_pDistGridMgr->is_ghost(elem);
}

bool ParallelHangingNodeRefiner_MultiGrid::
refinement_is_allowed(Volume* elem)
{
	return !m_pDistGridMgr->is_ghost(elem);
}

void ParallelHangingNodeRefiner_MultiGrid::
pre_refine()
{
	m_pDistGridMgr->begin_ordered_element_insertion();
	BaseClass::pre_refine();
}

void ParallelHangingNodeRefiner_MultiGrid::
post_refine()
{
	BaseClass::post_refine();
	m_pDistGridMgr->end_ordered_element_insertion();
}

void ParallelHangingNodeRefiner_MultiGrid::
set_involved_processes(pcl::ProcessCommunicator com)
{
	m_procCom = com;
}

void ParallelHangingNodeRefiner_MultiGrid::refine()
{
	UG_ASSERT(m_pDistGridMgr, "a distributed grid manager has to be assigned");
	if(!m_pDistGridMgr){
		throw(UGError("No distributed grid manager assigned."));
	}

	BaseClass::refine();
}

}// end of namespace
