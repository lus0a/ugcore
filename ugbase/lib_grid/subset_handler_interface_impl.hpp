//	created by Sebastian Reiter
//	s.b.reiter@googlemail.com
//	y08 m11 d24

#ifndef __H__LIBGRID__SUBSET_HANDLER_INTERFACE_IMPL__
#define __H__LIBGRID__SUBSET_HANDLER_INTERFACE_IMPL__

#include "subset_handler_interface.h"

namespace ug
{
template <>
inline AttachmentPipe<VertexBase*, ISubsetHandler>&
ISubsetHandler::get_attachment_pipe<VertexBase>(int subsetIndex)
{
	assert(subset_attachments_are_enabled() && "ERROR - you have to enable subset-attachments for this subset-handler before executing this mehtod.");
	return *m_vertexAttachmentPipes[subsetIndex];
}

template <>
inline AttachmentPipe<EdgeBase*, ISubsetHandler>&
ISubsetHandler::get_attachment_pipe<EdgeBase>(int subsetIndex)
{
	assert(subset_attachments_are_enabled() && "ERROR - you have to enable subset-attachments for this subset-handler before executing this mehtod.");
	return *m_edgeAttachmentPipes[subsetIndex];
}

template <>
inline AttachmentPipe<Face*, ISubsetHandler>&
ISubsetHandler::get_attachment_pipe<Face>(int subsetIndex)
{
	assert(subset_attachments_are_enabled() && "ERROR - you have to enable subset-attachments for this subset-handler before executing this mehtod.");
	return *m_faceAttachmentPipes[subsetIndex];
}

template <>
inline AttachmentPipe<Volume*, ISubsetHandler>&
ISubsetHandler::get_attachment_pipe<Volume>(int subsetIndex)
{
	assert(subset_attachments_are_enabled() && "ERROR - you have to enable subset-attachments for this subset-handler before executing this mehtod.");
	return *m_volumeAttachmentPipes[subsetIndex];
}


inline void ISubsetHandler::
subset_assigned(VertexBase* v, iterator iter, int subsetIndex)
{
	if(subset_attachments_are_enabled())
	{
		if(get_subset_index(v) != -1)
			get_attachment_pipe<VertexBase>(subsetIndex).unregister_element(v);
			
		if(subsetIndex != -1)
			get_attachment_pipe<VertexBase>(subsetIndex).register_element(v);
	}

	m_aaIteratorVRT[v] = iter;
	m_aaSubsetIndexVRT[v] = subsetIndex;
}

inline void ISubsetHandler::
subset_assigned(EdgeBase* e, iterator iter, int subsetIndex)
{
	if(subset_attachments_are_enabled())
	{
		if(get_subset_index(e) != -1)
			get_attachment_pipe<EdgeBase>(subsetIndex).unregister_element(e);
			
		if(subsetIndex != -1)
			get_attachment_pipe<EdgeBase>(subsetIndex).register_element(e);
	}

	m_aaIteratorEDGE[e] = iter;
	m_aaSubsetIndexEDGE[e] = subsetIndex;
}

inline void 
ISubsetHandler::
subset_assigned(Face* f, iterator iter, int subsetIndex)
{
	if(subset_attachments_are_enabled())
	{
		if(get_subset_index(f) != -1)
			get_attachment_pipe<Face>(subsetIndex).unregister_element(f);
			
		if(subsetIndex != -1)
			get_attachment_pipe<Face>(subsetIndex).register_element(f);
	}

	m_aaIteratorFACE[f] = iter;
	m_aaSubsetIndexFACE[f] = subsetIndex;
}

inline void 
ISubsetHandler::
subset_assigned(Volume* v, iterator iter, int subsetIndex)
{
	if(subset_attachments_are_enabled())
	{
		if(get_subset_index(v) != -1)
			get_attachment_pipe<Volume>(subsetIndex).unregister_element(v);
			
		if(subsetIndex != -1)
			get_attachment_pipe<Volume>(subsetIndex).register_element(v);
	}

	m_aaIteratorVOL[v] = iter;
	m_aaSubsetIndexVOL[v] = subsetIndex;
}

template <class TIterator>
void ISubsetHandler::
assign_subset(TIterator iterBegin, TIterator iterEnd, int subsetIndex)
{
	typename TIterator::value_type elem;
	while(iterBegin != iterEnd)
	{
		elem = *iterBegin;
		++iterBegin;
		assign_subset(elem, subsetIndex);
	}
}

inline void
ISubsetHandler::
subset_info_required(int index)
{
	if(index >= (int)m_subsetInfos.size())
		create_required_subset_infos(index);
}


////////////////////////////////////////////////////////////////////////
//	attachment handling
template <class TGeomObjClass>
void ISubsetHandler::
attach_to(IAttachment& attachment, int subsetIndex)
{
	assert(subset_attachments_are_enabled() && "ERROR - you have to enable subset-attachments for this subset-handler before executing this mehtod.");
	STATIC_ASSERT(geometry_traits<TGeomObjClass>::BASE_OBJECT_TYPE_ID != -1,
			invalid_GeomObjClass);

	subset_info_required(subsetIndex);

	int objType = geometry_traits<TGeomObjClass>::BASE_OBJECT_TYPE_ID;
	switch(objType)
	{
		case VERTEX:
			get_attachment_pipe<VertexBase>(subsetIndex).attach(attachment, 0);
			break;
		case EDGE:
			get_attachment_pipe<EdgeBase>(subsetIndex).attach(attachment, 0);
			break;
		case FACE:
			get_attachment_pipe<Face>(subsetIndex).attach(attachment, 0);
			break;
		case VOLUME:
			get_attachment_pipe<Volume>(subsetIndex).attach(attachment, 0);
			break;
	};
}

template <class TGeomObjClass, class TAttachment>
void ISubsetHandler::
attach_to_dv(TAttachment& attachment, int subsetIndex,
			const typename TAttachment::ValueType& defaultValue)
{
	assert(subset_attachments_are_enabled() && "ERROR - you have to enable subset-attachments for this subset-handler before executing this mehtod.");
	STATIC_ASSERT(geometry_traits<TGeomObjClass>::BASE_OBJECT_TYPE_ID != -1,
			invalid_GeomObjClass);

	subset_info_required(subsetIndex);

	int objType = geometry_traits<TGeomObjClass>::BASE_OBJECT_TYPE_ID;
	switch(objType)
	{
		case VERTEX:
			get_attachment_pipe<VertexBase>(subsetIndex).attach(attachment, defaultValue, 0);
			break;
		case EDGE:
			get_attachment_pipe<EdgeBase>(subsetIndex).attach(attachment, defaultValue, 0);
			break;
		case FACE:
			get_attachment_pipe<Face>(subsetIndex).attach(attachment, defaultValue, 0);
			break;
		case VOLUME:
			get_attachment_pipe<Volume>(subsetIndex).attach(attachment, defaultValue, 0);
			break;
	};
}

template <class TGeomObjClass>
void ISubsetHandler::detach_from(IAttachment& attachment, int subsetIndex)
{
	assert(subset_attachments_are_enabled() && "ERROR - you have to enable subset-attachments for this subset-handler before executing this mehtod.");
	STATIC_ASSERT(geometry_traits<TGeomObjClass>::BASE_OBJECT_TYPE_ID != -1,
				invalid_GeomObjClass);

	assert(subsetIndex >= 0 && subsetIndex < num_subset_infos() && "bad subset index.");

	int objType = geometry_traits<TGeomObjClass>::BASE_OBJECT_TYPE_ID;
	switch(objType)
	{
		case VERTEX:
			get_attachment_pipe<VertexBase>(subsetIndex).detach(attachment);
			break;
		case EDGE:
			get_attachment_pipe<EdgeBase>(subsetIndex).detach(attachment);
			break;
		case FACE:
			get_attachment_pipe<Face>(subsetIndex).detach(attachment);
			break;
		case VOLUME:
			get_attachment_pipe<Volume>(subsetIndex).detach(attachment);
			break;
	};
}


template <class TAttachment>
typename TAttachment::ContainerType*
ISubsetHandler::get_attachment_data_container(TAttachment& attachment, int subsetIndex,
											  const VertexBase*)
{
	assert(subset_attachments_are_enabled() && "ERROR - you have to enable subset-attachments for this subset-handler before executing this mehtod.");

	return get_attachment_pipe<VertexBase>(subsetIndex).get_data_container(attachment);
}

template <class TAttachment>
typename TAttachment::ContainerType*
ISubsetHandler::get_attachment_data_container(TAttachment& attachment, int subsetIndex,
											  const EdgeBase*)
{
	assert(subset_attachments_are_enabled() && "ERROR - you have to enable subset-attachments for this subset-handler before executing this mehtod.");

	return get_attachment_pipe<EdgeBase>(subsetIndex).get_data_container(attachment);
}

template <class TAttachment>
typename TAttachment::ContainerType*
ISubsetHandler::get_attachment_data_container(TAttachment& attachment, int subsetIndex,
											  const Face*)
{
	assert(subset_attachments_are_enabled() && "ERROR - you have to enable subset-attachments for this subset-handler before executing this mehtod.");

	return get_attachment_pipe<Face>(subsetIndex).get_data_container(attachment);
}

template <class TAttachment>
typename TAttachment::ContainerType*
ISubsetHandler::get_attachment_data_container(TAttachment& attachment, int subsetIndex,
											  const Volume*)
{
	assert(subset_attachments_are_enabled() && "ERROR - you have to enable subset-attachments for this subset-handler before executing this mehtod.");

	return get_attachment_pipe<Volume>(subsetIndex).get_data_container(attachment);
}

////////////////////////////////////////////////////////////////////////
//	attachments_traits
inline uint 
attachment_traits<VertexBase*, ISubsetHandler>::
get_data_index(ElemHandlerPtr pHandler, ConstElemPtr elem)
{
	return pHandler->get_attachment_data_index(elem);
}

inline void 
attachment_traits<VertexBase*, ISubsetHandler>::
set_data_index(ElemHandlerPtr pHandler, ElemPtr elem, uint index)
{
	pHandler->set_attachment_data_index(elem, index);
}

inline uint 
attachment_traits<EdgeBase*, ISubsetHandler>::
get_data_index(ElemHandlerPtr pHandler, ConstElemPtr elem)
{
	return pHandler->get_attachment_data_index(elem);
}

inline void 
attachment_traits<EdgeBase*, ISubsetHandler>::
set_data_index(ElemHandlerPtr pHandler, ElemPtr elem, uint index)
{
	pHandler->set_attachment_data_index(elem, index);
}

inline uint 
attachment_traits<Face*, ISubsetHandler>::
get_data_index(ElemHandlerPtr pHandler, ConstElemPtr elem)
{
	return pHandler->get_attachment_data_index(elem);
}

inline void 
attachment_traits<Face*, ISubsetHandler>::
set_data_index(ElemHandlerPtr pHandler, ElemPtr elem, uint index)
{
	pHandler->set_attachment_data_index(elem, index);
}

inline uint 
attachment_traits<Volume*, ISubsetHandler>::
get_data_index(ElemHandlerPtr pHandler, ConstElemPtr elem)
{
	return pHandler->get_attachment_data_index(elem);
}

inline void 
attachment_traits<Volume*, ISubsetHandler>::
set_data_index(ElemHandlerPtr pHandler, ElemPtr elem, uint index)
{
	pHandler->set_attachment_data_index(elem, index);
}

}//	end of namespace

#endif
