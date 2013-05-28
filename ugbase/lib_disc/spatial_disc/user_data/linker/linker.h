/*
 * linker.h
 *
 *  Created on: 12.11.2011
 *      Author: andreasvogel
 */

#ifndef __H__UG__LIB_DISC__SPATIAL_DISC__DATA_LINKER__
#define __H__UG__LIB_DISC__SPATIAL_DISC__DATA_LINKER__

#include "../std_user_data.h"
#include "lib_disc/common/groups_util.h"

namespace ug{

/// combines several UserDatas to a new UserData of a specified type
/**
 * This class provides data at integration points and implements the
 * DependentUserData interface.
 *
 * \tparam 	TData		output Data type
 * \tparam 	dim			World dimension
 */
template <typename TImpl, typename TData, int dim>
class StdDataLinker
	: public StdUserData< StdDataLinker<TImpl, TData, dim>,
						  TData, dim, void,
						  DependentUserData<TData, dim> >
{
	public:
		virtual void operator() (TData& value,
								 const MathVector<dim>& globIP,
								 number time, int si) const;

		virtual void operator()(TData vValue[],
								const MathVector<dim> vGlobIP[],
								number time, int si, const size_t nip) const;

		template <int refDim>
		inline void evaluate(TData vValue[],
							 const MathVector<dim> vGlobIP[],
							 number time, int si,
							 GeometricObject* elem,
							 const MathVector<dim> vCornerCoords[],
							 const MathVector<refDim> vLocIP[],
							 const size_t nip,
							 LocalVector* u,
							 const MathMatrix<refDim, dim>* vJT = NULL) const;

		virtual void compute(LocalVector* u, GeometricObject* elem,
							 const MathVector<dim> vCornerCoords[], bool bDeriv = false);

	protected:
		template <int refDim>
		void eval_deriv(LocalVector* u, GeometricObject* elem,
		                const MathVector<dim> vCornerCoords[], bool bDeriv = false);

	public:
	///	returns that a grid function is needed for evaluation
		virtual bool requires_grid_fct() const;

	///	returns if provided data is continuous over geometric object boundaries
		virtual bool continuous() const;

	///	returns if derivative is zero
		virtual bool zero_derivative() const;

	public:
	///	returns if the derivative of the i'th input is zero
		bool zero_derivative(size_t i) const
		{
			if(!m_vspICplUserData[i].valid()) return true;
			return m_vspICplUserData[i]->zero_derivative();
		}

	///	sets the number of inputs
		void set_num_input(size_t num)
		{
			m_vspICplUserData.resize(num, NULL);
			m_vspUserDataInfo.resize(num, NULL);
		}

	///	sets an input
		virtual void set_input(size_t i,
		                       SmartPtr<ICplUserData<dim> > input,
		                       SmartPtr<UserDataInfo> info)
		{
			UG_ASSERT(i < m_vspICplUserData.size(), "invalid index");
			m_vspICplUserData[i] = input;
			m_vspUserDataInfo[i] = info;
		}

	///	number of inputs
		virtual size_t num_input() const {return num_needed_data();}

	///	number of other Data this data depends on
		virtual size_t num_needed_data() const {return m_vspICplUserData.size();}

	///	return needed data
		virtual SmartPtr<ICplUserData<dim> > needed_data(size_t i)
		{
			UG_ASSERT(i < m_vspICplUserData.size(), "Input not needed");
			UG_ASSERT(m_vspICplUserData[i].valid(), "Data input not valid");
			return m_vspICplUserData[i];
		}

	///	returns if data is ok
		virtual void check_setup() const;

	///	updates the function group
		virtual void set_function_pattern(ConstSmartPtr<FunctionPattern> fctPatt);

	protected:
	///	returns number of functions the input depends on
		size_t input_num_fct(size_t i) const
		{
			UG_ASSERT(i < m_vspUserDataInfo.size(), "Input invalid");
			if(!m_vspUserDataInfo[i].valid()) return 0;
			return m_vspUserDataInfo[i]->num_fct();
		}

	///	returns the number in the common FctGrp for a fct of an input
		size_t input_common_fct(size_t i, size_t fct) const
		{
			UG_ASSERT(i < m_vMap.size(), "Input Map invalid");
			UG_ASSERT(fct < m_vMap[i].num_fct(), "Input Map invalid for fct");
			return m_vMap[i][fct];
		}

	///	returns the series id set for the i'th input
		size_t series_id(size_t i, size_t s) const
		{
			UG_ASSERT(i < m_vvSeriesID.size(), "invalid index");
			UG_ASSERT(s < m_vvSeriesID[i].size(), "invalid index");
			return m_vvSeriesID[i][s];
		}

	///	requests series id's from input data
		virtual void local_ip_series_added(const size_t seriesID);

	///	forwards the local positions to the data inputs
		virtual void local_ips_changed(const size_t seriesID, const size_t newNumIP);

	///	forwards the global positions to the data inputs
		virtual void global_ips_changed(const size_t seriesID, const MathVector<dim>* vPos, const size_t numIP);

	protected:
	///	data input
		std::vector<SmartPtr<ICplUserData<dim> > > m_vspICplUserData;

	///	data input casted to IDependend data
		std::vector<SmartPtr<UserDataInfo> > m_vspUserDataInfo;

	///	Function mapping for each input relative to common FunctionGroup
		std::vector<FunctionIndexMapping> m_vMap;

	///	series id the linker uses to get data from input
		std::vector<std::vector<size_t> > m_vvSeriesID;

	protected:
	///	access to implementation
		TImpl& getImpl() {return static_cast<TImpl&>(*this);}

	///	const access to implementation
		const TImpl& getImpl() const {return static_cast<const TImpl&>(*this);}
};

} // end namespace ug

//	include implementation
#include "linker_impl.h"

#endif /* __H__UG__LIB_DISC__SPATIAL_DISC__DATA_LINKER__ */