/*
 * data_linker.h
 *
 *  Created on: 12.11.2011
 *      Author: andreasvogel
 */

#ifndef __H__UG__LIB_DISC__SPATIAL_DISC__DATA_LINKER__
#define __H__UG__LIB_DISC__SPATIAL_DISC__DATA_LINKER__

#include "ip_data.h"
#include "lib_disc/common/groups_util.h"

namespace ug{

/// combines several IPDatas to a new IPData of a specified type
/**
 * This class provides data at integration points and implements the
 * DependentIPData interface.
 *
 * \tparam 	TData		output Data type
 * \tparam 	dim			World dimension
 */
template <typename TData, int dim>
class DataLinker
	: public DependentIPData<TData, dim>
{
	public:
	///	Base class type
		typedef DependentIPData<TData, dim> base_type;

	//	explicitly forward methods of IIPData
		using base_type::num_series;
		using base_type::num_ip;
		using base_type::time;

	//	explicitly forward methods of IPData
		using base_type::ip;
		using base_type::value;

	public:
	///	constructor
		DataLinker() {m_vpIIPData.clear(); m_vpIDependData.clear();}

	///	compute method
		virtual void compute(bool bDeriv) = 0;

	///	returns if derivative is zero
		virtual bool zero_derivative() const;

	///	returns if the derivative of the i'th input is zero
		bool zero_derivative(size_t i) const
		{
			if(!m_vpIIPData[i].valid()) return true;
			return m_vpIIPData[i]->zero_derivative();
		}

	///	sets the number of inputs
		void set_num_input(size_t num)
		{
			m_vpIIPData.resize(num, NULL);
			m_vpIDependData.resize(num, NULL);
		}

	///	sets an input
		virtual void set_input(size_t i, SmartPtr<IIPDimData<dim> > input)
		{
			UG_ASSERT(i < m_vpIIPData.size(), "invalid index");
			m_vpIIPData[i] = input;
			m_vpIDependData[i] = input.template cast_dynamic<IDependentIPData>();
		}

	///	number of inputs
		virtual size_t num_input() const {return num_needed_data();}

	///	number of other Data this data depends on
		virtual size_t num_needed_data() const {return m_vpIIPData.size();}

	///	return needed data
		virtual SmartPtr<IIPData> needed_data(size_t i)
		{
			UG_ASSERT(i < m_vpIIPData.size(), "Input not needed");
			UG_ASSERT(m_vpIIPData[i].valid(), "Data input not valid");
			return m_vpIIPData[i];
		}

	///	returns if data is ok
		virtual bool is_ready() const;

	///	updates the function group
		void update_function_group();

	protected:
	///	returns number of functions the input depends on
		size_t input_num_fct(size_t i) const
		{
			UG_ASSERT(i < m_vpIDependData.size(), "Input invalid");
			if(!m_vpIDependData[i].valid()) return 0;
			return m_vpIDependData[i]->num_fct();
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
		virtual void local_ip_series_added(const size_t newNumSeries);

	///	forwards the local positions to the data inputs
		virtual void local_ips_changed(const size_t seriesID, const size_t newNumIP);

	///	forwards the global positions to the data inputs
		virtual void global_ips_changed(size_t s, const MathVector<dim>* vPos, size_t numIP);

	protected:
	///	data input
		std::vector<SmartPtr<IIPDimData<dim> > > m_vpIIPData;

	///	data input casted to IDependend data
		std::vector<SmartPtr<IDependentIPData> > m_vpIDependData;

	///	common functions the data depends on
		FunctionGroup m_commonFctGroup;

	///	Function mapping for each input relative to common FunctionGroup
		std::vector<FunctionIndexMapping> m_vMap;

	///	series id the linker uses to get data from input
		std::vector<std::vector<size_t> > m_vvSeriesID;
};

/// combines several IPDatas of the one data type to a new IPData of a second type
/**
 * This class provides data at integration points and implements the
 * DependentIPData interface. It gets passed several data of one C++-type
 * and combines them to a new value of a second type. In addition also the
 * derivative w.r.t. the primal unknowns are passed.
 *
 * \tparam 	TData		output Data type
 * \tparam 	dim			World dimension
 * \tparam	TDataIn		Input data type
 */
template <typename TData, int dim, typename TDataIn>
class DataLinkerEqualData
	: public DataLinker<TData, dim>
{
	public:
	///	Base class type
		typedef DataLinker<TData, dim> base_type;

	//	explicitly forward methods of IIPData
		using base_type::num_series;
		using base_type::num_ip;
		using base_type::time;

	//	explicitly forward methods of IPData
		using base_type::ip;
		using base_type::value;

	//	explicitly forward methods of IDataLinker
		using base_type::num_input;
		using base_type::set_input;
		using base_type::series_id;

	public:
	///	constructor
		DataLinkerEqualData()
		{
			m_vpIPData.clear();
			m_vpDependData.clear();
		}

	///	compute method
		virtual void compute(bool bDeriv) = 0;

	///	set number of needed inputs
		void set_num_input(size_t num);

	///	set input i
		void set_input(size_t i, SmartPtr<IPData<TDataIn, dim> > data);
		void set_input(size_t i, number val);

	protected:
	///	data at ip of input
		const TDataIn& input_value(size_t i, size_t s, size_t ip) const
		{
			UG_ASSERT(i < m_vpIPData.size(), "Input not needed");
			UG_ASSERT(m_vpIPData[i].valid(), "Input invalid");
			return m_vpIPData[i]->value(series_id(i,s), ip);
		}

	///	data at ip of input
		TDataIn& input_value(size_t i, size_t s, size_t ip)
		{
			UG_ASSERT(i < m_vpIPData.size(), "Input not needed");
			UG_ASSERT(m_vpIPData[i].valid(), "Input invalid");
			return m_vpIPData[i]->value(series_id(i,s), ip);
		}

	///	derivative of data at input at ip
		const TDataIn& input_deriv(size_t i, size_t s, size_t ip, size_t fct, size_t dof) const
		{
			UG_ASSERT(i < m_vpDependData.size(), "Input not needed");
			UG_ASSERT(m_vpDependData[i].valid(), "Input invalid");
			return m_vpDependData[i]->deriv(series_id(i,s), ip, fct, dof);
		}

	///	derivative of data at input at ip
		TDataIn& input_deriv(size_t i, size_t s, size_t ip, size_t fct, size_t dof)
		{
			UG_ASSERT(i < m_vpDependData.size(), "Input not needed");
			UG_ASSERT(m_vpDependData[i].valid(), "Input invalid");
			return m_vpDependData[i]->deriv(series_id(i,s), ip, fct, dof);
		}

	protected:
	///	data input
		std::vector<SmartPtr<IPData<TDataIn, dim> > > m_vpIPData;

	///	data input casted to dependend data
		std::vector<SmartPtr<DependentIPData<TDataIn, dim> > > m_vpDependData;
};

/////////////////////////////////////////////////
// Linker Traits to allow generic programming
/////////////////////////////////////////////////

/// Linker Traits
template <typename TData, typename TDataIn>
struct linker_traits
{
///	computes out += s * in1 (with appropriate '*')
	static void mult_add(TData& out, const TData& in1, const TDataIn& s);
};

template <>
struct linker_traits<number, number>
{
	static void mult_add(number& out, const number& in1, const number& s)
	{
		out += in1 * s;
	}
};

template <std::size_t dim>
struct linker_traits< MathVector<dim>, number >
{
	static void mult_add(MathVector<dim>& out,
	                     const MathVector<dim>& in1,
						 const number& s)
	{
		VecScaleAppend(out, s, in1);
	}
};

template <std::size_t dim>
struct linker_traits< MathVector<dim>, MathMatrix<dim,dim> >
{
	static void mult_add(MathVector<dim>& out,
	                     const MathVector<dim>& in1,
						 const MathMatrix<dim,dim>& s)
	{
		MatVecMultAppend(out, s, in1);
	}
};

template <std::size_t dim>
struct linker_traits< MathMatrix<dim,dim>, number >
{
	static void mult_add(MathMatrix<dim,dim>& out,
	                     const MathMatrix<dim,dim>& in1,
						 const number& s)
	{
		MatScaleAppend(out, s, in1);
	}
};

template <std::size_t dim>
struct linker_traits< MathTensor<4,dim>, number >
{
	static void mult_add(MathTensor<4,dim>& out,
	                     const MathTensor<4, dim>& in1,
						 const number& s)
	{
		out = in1;
		throw(UGError("linker_traits for MathTensor4 not implemented"));
	}
};

template <typename TData, int dim, typename TDataScale, typename TImpl>
class StdDataLinkerEqualData
	: 	public DataLinkerEqualData<TData,dim, TDataScale>
{
	public:
		StdDataLinkerEqualData() {}

		virtual void operator() (TData& value,
		                         const MathVector<dim>& globIP,
		                         number time, int si) const
		{
			getImpl().evaluate(value,globIP,time,si);
		}

		virtual void operator() (TData vValue[],
		                         const MathVector<dim> vGlobIP[],
		                         number time, int si, const size_t nip) const
		{
			for(size_t ip = 0; ip < nip; ++ip)
				getImpl().evaluate(vValue[ip],vGlobIP[ip],time,si);
		}

		////////////////
		// one value
		////////////////

		virtual void operator() (TData& value,
		                         const MathVector<dim>& globIP,
		                         number time, int si,
		                         LocalVector& u,
		                         GeometricObject* elem,
		                         const MathVector<dim> vCornerCoords[],
		                         const MathVector<1>& locIP) const
		{
			getImpl().template evaluate<1>(value,globIP,time,si,u,elem,vCornerCoords,locIP);
		}

		virtual void operator() (TData& value,
		                         const MathVector<dim>& globIP,
		                         number time, int si,
		                         LocalVector& u,
		                         GeometricObject* elem,
		                         const MathVector<dim> vCornerCoords[],
		                         const MathVector<2>& locIP) const
		{
			getImpl().template evaluate<2>(value,globIP,time,si,u,elem,vCornerCoords,locIP);
		}

		virtual void operator() (TData& value,
		                         const MathVector<dim>& globIP,
		                         number time, int si,
		                         LocalVector& u,
		                         GeometricObject* elem,
		                         const MathVector<dim> vCornerCoords[],
		                         const MathVector<3>& locIP) const
		{
			getImpl().template evaluate<3>(value,globIP,time,si,u,elem,vCornerCoords,locIP);
		}

		////////////////
		// vector of values
		////////////////

		virtual void operator()(TData vValue[],
		                        const MathVector<dim> vGlobIP[],
		                        number time, int si,
		                        LocalVector& u,
		                        GeometricObject* elem,
		                        const MathVector<dim> vCornerCoords[],
		                        const MathVector<1> vLocIP[],
		                        const size_t nip,
		                        const MathMatrix<1, dim>* vJT = NULL) const
		{
			getImpl().template evaluate<1>(vValue,vGlobIP,time,si,u,elem,
			                               vCornerCoords,vLocIP,nip, vJT);
		}

		virtual void operator()(TData vValue[],
		                        const MathVector<dim> vGlobIP[],
		                        number time, int si,
		                        LocalVector& u,
		                        GeometricObject* elem,
		                        const MathVector<dim> vCornerCoords[],
		                        const MathVector<2> vLocIP[],
		                        const size_t nip,
		                        const MathMatrix<2, dim>* vJT = NULL) const
		{
			getImpl().template evaluate<2>(vValue,vGlobIP,time,si,u,elem,
			                               vCornerCoords,vLocIP,nip, vJT);
		}

		virtual void operator()(TData vValue[],
		                        const MathVector<dim> vGlobIP[],
		                        number time, int si,
		                        LocalVector& u,
		                        GeometricObject* elem,
		                        const MathVector<dim> vCornerCoords[],
		                        const MathVector<3> vLocIP[],
		                        const size_t nip,
		                        const MathMatrix<3, dim>* vJT = NULL) const
		{
			getImpl().template evaluate<3>(vValue,vGlobIP,time,si,u,elem,
			                               vCornerCoords,vLocIP,nip, vJT);
		}

	///	returns that a grid function is needed for evaluation
		virtual bool requires_grid_fct() const
		{
			bool bRet = false;
			for(size_t i = 0; i < this->m_vpIPData.size(); ++i)
				bRet |= this->m_vpIPData[i]->requires_grid_fct();
			return bRet;
		}

	///	sets the associated function pattern
		virtual void set_function_pattern(const FunctionPattern& fctPatt)
		{
			for(size_t i = 0; i < this->m_vpIPData.size(); ++i)
				this->m_vpIPData[i]->set_function_pattern(fctPatt);
		}

	protected:
	///	access to implementation
		TImpl& getImpl() {return static_cast<TImpl&>(*this);}

	///	const access to implementation
		const TImpl& getImpl() const {return static_cast<const TImpl&>(*this);}
};

/////////////////////////////////////////////////
// Scaled adding of Data
/////////////////////////////////////////////////

/**
 * This linker recombines the data like
 *
 * l = s_1 * i_1 + ... + s_k * i_k
 *
 * where, l, i_1, ..., i_k are of the same data type and the s_i are some
 * scaling factor of a (possibly) different type, that is applyable to the
 * data type
 *
 * \tparam		TData		exported and combined Data type
 * \tparam		dim			world dimension
 * \tparam		TDataScale 	type of scaling data
 */
template <typename TData, int dim, typename TDataScale>
class ScaleAddLinker
	: public StdDataLinkerEqualData<TData, dim, TDataScale, ScaleAddLinker<TData, dim, TDataScale> >
{
	public:
	//	type of base class
		typedef DataLinker<TData, dim> base_type;

	//	explicitly forward methods of IIPData
		using base_type::num_series;
		using base_type::num_ip;
		using base_type::time;

	//	explicitly forward methods of IPData
		using base_type::ip;
		using base_type::value;

	//	explicitly forward methods of IDependentIPData
		using base_type::num_fct;

	//	explicitly forward methods of DependentIPData
		using base_type::num_sh;
		using base_type::deriv;

	//	explicitly forward methods of DataLinker
		using base_type::series_id;

	public:
	///	constructor
		ScaleAddLinker() {}

	///	adds an input to the list of summands scaled by a user data factor
	///	\{
		void add(SmartPtr<IPData<TDataScale, dim> > scale,
		         SmartPtr<IPData<TData, dim> > data);
		void add(number scale,
		         SmartPtr<IPData<TData, dim> > data);
		void add(SmartPtr<IPData<TDataScale, dim> > scale,
		         number data);
		void add(number scale,
		         number data);
	/// \}

	///	computes the value
		virtual void compute(bool bDeriv);

		inline void evaluate (TData& value,
		                      const MathVector<dim>& globIP,
		                      number time, int si) const
		{
			//	reset value
			value = 0.0;

			TData valData;
			TDataScale valScale;

			//	add contribution of each summand
				for(size_t c = 0; c < m_vpIPData.size(); ++c)
				{
					(*m_vpIPData[c])(valData, globIP, time, si);
					(*m_vpScaleData[c])(valScale, globIP, time, si);

					linker_traits<TData, TDataScale>::
					mult_add(value, valData, valScale);
				}
		}

		template <int refDim>
		inline void evaluate (TData& value,
		                      const MathVector<dim>& globIP,
		                      number time, int si,
		                      LocalVector& u,
		                      GeometricObject* elem,
		                      const MathVector<dim> vCornerCoords[],
		                      const MathVector<refDim>& locIP) const
		{
			//	reset value
			value = 0.0;

			TData valData;
			TDataScale valScale;

			//	add contribution of each summand
				for(size_t c = 0; c < m_vpIPData.size(); ++c)
				{
					(*m_vpIPData[c])(valData, globIP, time, si, u, elem, vCornerCoords, locIP);
					(*m_vpScaleData[c])(valScale, globIP, time, si, u, elem, vCornerCoords, locIP);

					linker_traits<TData, TDataScale>::
					mult_add(value, valData, valScale);
				}
		}

		template <int refDim>
		inline void evaluate(TData vValue[],
		                     const MathVector<dim> vGlobIP[],
		                     number time, int si,
		                     LocalVector& u,
		                     GeometricObject* elem,
		                     const MathVector<dim> vCornerCoords[],
		                     const MathVector<refDim> vLocIP[],
		                     const size_t nip,
		                     const MathMatrix<refDim, dim>* vJT = NULL) const
		{
			//	reset value
			for(size_t ip = 0; ip < nip; ++ip)
				vValue[ip] = 0.0;

			std::vector<TData> vValData(nip);
			std::vector<TDataScale> vValScale(nip);

			//	add contribution of each summand
				for(size_t c = 0; c < m_vpIPData.size(); ++c)
				{
					(*m_vpIPData[c])(&vValData[0], vGlobIP, time, si, u,
									elem, vCornerCoords, vLocIP, nip, vJT);
					(*m_vpScaleData[c])(&vValScale[0], vGlobIP, time, si, u,
										elem, vCornerCoords, vLocIP, nip, vJT);

					for(size_t ip = 0; ip < nip; ++ip)
						linker_traits<TData, TDataScale>::
						mult_add(vValue[ip], vValData[ip], vValScale[ip]);
				}
		}

	protected:
	///	resizes the scaling arrays
		void resize_scaling(size_t num)
		{
			m_vpIPData.resize(num, NULL);
			m_vpDependData.resize(num, NULL);
			m_vpScaleData.resize(num, NULL);
			m_vpScaleDependData.resize(num, NULL);
		}

	///	data at ip of input
		const TData& input_value(size_t i, size_t s, size_t ip) const
		{
			UG_ASSERT(i < m_vpIPData.size(), "Input not needed");
			UG_ASSERT(m_vpIPData[i].valid(), "Input invalid");
			return m_vpIPData[i]->value(series_id(2*i,s), ip);
		}

	///	derivative of data at input at ip
		const TData& input_deriv(size_t i, size_t s, size_t ip, size_t fct, size_t dof) const
		{
			UG_ASSERT(i < m_vpDependData.size(), "Input not needed");
			UG_ASSERT(m_vpDependData[i].valid(), "Input invalid");
			return m_vpDependData[i]->deriv(series_id(2*i,s), ip, fct, dof);
		}

	///	scale at ip of input
		const TDataScale& scale_value(size_t i, size_t s, size_t ip) const
		{
			UG_ASSERT(i < m_vpScaleData.size(), "Input not needed");
			UG_ASSERT(m_vpScaleData[i].valid(), "Input invalid");
			return m_vpScaleData[i]->value(series_id(2*i+1,s), ip);
		}

	///	derivative of data at input at ip
		const TDataScale& scale_deriv(size_t i, size_t s, size_t ip, size_t fct, size_t dof) const
		{
			UG_ASSERT(i < m_vpScaleDependData.size(), "Input not needed");
			UG_ASSERT(m_vpScaleDependData[i].valid(), "Input invalid");
			return m_vpScaleDependData[i]->deriv(series_id(2*i+1,s), ip, fct, dof);
		}

	///	returns number of functions the input depends on
		size_t input_num_fct(size_t i) const
		{
			return base_type::input_num_fct(2*i);
		}

	///	returns the number in the common FctGrp for a fct of an input
		size_t input_common_fct(size_t i, size_t fct) const
		{
			return base_type::input_common_fct(2*i, fct);
		}

	///	returns number of functions the scaling depends on
		size_t scale_num_fct(size_t i) const
		{
			return base_type::input_num_fct(2*i+1);
		}

	///	returns the number in the common FctGrp for a fct of a scaling
		size_t scale_common_fct(size_t i, size_t fct) const
		{
			return base_type::input_common_fct(2*i+1, fct);
		}

	protected:
	///	data input
		std::vector<SmartPtr<IPData<TDataScale, dim> > > m_vpScaleData;

	///	data input casted to dependend data
		std::vector<SmartPtr<DependentIPData<TDataScale, dim> > > m_vpScaleDependData;

	///	data input
		std::vector<SmartPtr<IPData<TData, dim> > > m_vpIPData;

	///	data input casted to dependend data
		std::vector<SmartPtr<DependentIPData<TData, dim> > > m_vpDependData;
};

} // end namespace ug

//	include implementation
#include "data_linker_impl.h"

#endif /* __H__UG__LIB_DISC__SPATIAL_DISC__DATA_LINKER__ */
