// created by Sebastian Reiter
// s.b.reiter@gmail.com

#ifndef __H__UG_field_impl__
#define __H__UG_field_impl__

#include <cstring>
#include <algorithm>

namespace ug{

template <class T> Field<T>::
Field() :
	m_width(0),
	m_height(0),
	m_capacity(0),
	m_data(NULL)
{
}

template <class T> Field<T>::
Field(size_t width, size_t height) :
	m_width(width),
	m_height(height)
{
	m_capacity = m_width * m_height;
	m_data = new T[m_capacity];
}

template <class T> Field<T>::
Field(const Field& f){
	m_width = f.width();
	m_height = f.height();
	m_capacity = m_width * m_height;
	m_data = new T[m_capacity];
	memcpy(m_data, f.data(), m_capacity * sizeof(T));
}

template <class T> Field<T>::
~Field(){
	if(m_data)
		delete[] m_data;
}

template <class T> T& Field<T>::
operator=(const Field& f){
	m_width = f.m_width;
	m_height = f.m_height;
	if(m_data && (m_capacity != m_width * m_height)){
		delete[] m_data;
		m_data = NULL;
	}
	m_capacity = m_width * m_height;
	if(!m_data)
		m_data = new T[m_capacity];
	memcpy(m_data, f.data(), m_capacity * sizeof(T));
}

template <class T> void Field<T>::
resize_no_copy(size_t width, size_t height){

	if(width * height > m_capacity){
		if(m_data)
			delete[] m_data;
		m_capacity += width * height;
		m_data = new T[m_capacity];
	}
	m_width = width;
	m_height = height;
}

template <class T> T& Field<T>::
at(size_t x, size_t y){
	return m_data[array_index(x, y)];
}

template <class T> const T& Field<T>::
at(size_t x, size_t y) const{
	return m_data[array_index(x, y)];
}

template <class T> void Field<T>::
fill(size_t x, size_t y, size_t w, size_t h, const T& value)
{
	using std::min;
	using std::max;

	const size_t xEnd = max(x + w, m_width);
	const size_t yEnd = max(y + h, m_height);

	for(size_t iy = max(y, size_t(0)); iy < yEnd; ++iy){
		for(size_t ix = max(x, size_t(0)); ix < yEnd; ++ix){
			m_data[array_index(ix, iy)] = value;
		}
	}
}

template <class T> void Field<T>::
fill_all(const T& value)
{
	const size_t dataSize = size();
	for(size_t i = 0; i < dataSize; ++i)
		m_data[i] = value;
}

template <class T> void Field<T>::
copy(size_t x, size_t y, const Field& f){
	using std::min;
	using std::max;

	const size_t xEnd = max(x + f.width(), m_width);
	const size_t yEnd = max(y + f.height(), m_height);

	for(size_t iy = max(y, size_t(0)); iy < yEnd; ++iy){
		for(size_t ix = max(x, size_t(0)); ix < yEnd; ++ix){
			m_data[array_index(ix, iy)] = f.at(ix, iy);
		}
	}
}

template <class T> void Field<T>::
swap(Field& f){
	using std::swap;
	swap(m_width, f.m_width);
	swap(m_height, f.m_height);
	swap(m_capacity, f.m_capacity);
	swap(m_data, f.m_data);
}


template <class T> size_t Field<T>::
array_index(size_t x, size_t y) const{
	return x + y * m_width;
}

}//	end of namespace

#endif	//__H__UG_field_impl__
