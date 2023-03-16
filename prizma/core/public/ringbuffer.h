#pragma once

// кольцевой буффер никогда не имеет конца, если при обращении мы выходим за фактический размер буффера то начинаем с конца, то есть если i = 5, а размер = 3, то мы попадем в элемент 5-3 = 2
// планирую использовать в звуках, для записи/воспроизведении звука
template <typename T>
class RingBuffer
{
	struct cnt
	{
		T* data;
		size_t size;
		size_t refs;
	};
public:
	RingBuffer(size_t size);
	RingBuffer(const RingBuffer origin)
	{
		container = origin.container;
		container->refs++;
	}
	~RingBuffer();
	T& operator [](size_t i);
private:
	cnt* container;
};

template<typename T>
inline RingBuffer<T>::RingBuffer(size_t size)
{
}

template<typename T>
inline RingBuffer<T>::~RingBuffer()
{
}

template<typename T>
inline T& RingBuffer<T>::operator[](size_t i)
{
	return container->data[i % container->size];
}

