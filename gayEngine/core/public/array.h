#pragma once


template <typename T>
class Array
{
	struct data_t
	{
		T* data;
		size_t size;
		size_t refs;
	};
#
public:
	Array(size_t size = 0);
	~Array();
	T& operator [] (size_t i)
	{
		return cont->data[i];
	}
	T El(size_t i)
	{
		return cont->data[i];
	}
	void SetSize(size_t size)
	{

	}
	size_t GetSize()
	{
		return cont.size;
	}
private:
	data_t cont;
};

template<typename T>
inline Array<T>::Array(size_t size)
{
	cont = (data_t)malloc(sizeof(data_t)*size);
	cont->size = size;
	cont->refs = 1;
}

template<typename T>
inline Array<T>::~Array()
{
	if (cont->refs <= 1)
	{
		free(cont);
	}
	else
		cont->refs--;
}
