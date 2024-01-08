#pragma once
#include <vector>
#include <mutex>


// Caution:Thread Unsafe
// T Should be a fixed size elem
template<class T>
class MemPool_ThreadUnsafe {
	struct Elem {
		T data;
		Elem* next;
	};
public:
	MemPool_ThreadUnsafe(size_t uiExtendSize)
	{
		m_extendSize = uiExtendSize;
		m_freeHead = nullptr;
		// ExtendPool();
	}
	~MemPool_ThreadUnsafe()
	{
		Clear();
	}
	T* New()
	{
		if (m_freeHead == nullptr)
		{
			ExtendPool();
		}
		T* data = &(m_freeHead->data);
		memset(data, 0, sizeof(T));
		m_freeHead = m_freeHead->next;
		return data;
	}
	void Del(T* p)
	{
		((Elem*)p)->next = m_freeHead;
		m_freeHead = (Elem*)p;
	}
	void Clear()
	{
		for (size_t i = 0; i < m_pool.size(); i++)
		{
			delete[] m_pool[i];
		}
		m_freeHead = nullptr;
		m_pool.clear();
	}
protected:
	void ExtendPool()
	{
		Elem* extendPool = new Elem[m_extendSize];
		for (size_t i = 0; i < m_extendSize - 1; i++)
		{
			extendPool[i].next = &extendPool[i + 1];
		}
		extendPool[m_extendSize - 1].next = m_freeHead;
		m_freeHead = &extendPool[0];

		m_pool.push_back(extendPool);
	}
private:
	std::vector<Elem*> m_pool;
	Elem* m_freeHead;
	size_t m_extendSize;
};

template<class T>
class MemPool {
	struct Elem {
		T data;
		Elem* next;
	};
public:
	/*
	MemPool<T>* GetInstance()	// for singleton
	{
		static MemPool<T> m_instance;
		return m_instance;
	}
	*/
	MemPool(size_t uiExtendSize)
	{
		m_extendSize = uiExtendSize;
		m_freeHead = nullptr;
		ExtendPool();
	}
	~MemPool()
	{
		for (size_t i = 0; i < m_pool.size(); i++)
		{
			delete[] m_pool[i];
		}
	}
	T* New()
	{
		std::lock_guard<std::mutex> guard(lock);
		if (m_freeHead == nullptr)
		{
			ExtendPool();
		}
		T* data = &(m_freeHead->data);
		memset(data, 0, sizeof(T));	// Caution:Only Fixed Size Elem Can Use,Pointer Inside T May Cause Leak Of Memory!!!
		m_freeHead = m_freeHead->next;
		return data;
	}
	void Del(T* p)
	{
		std::lock_guard<std::mutex> guard(lock);
		((Elem*)p)->next = m_freeHead;
		m_freeHead = (Elem*)p;
	}
protected:
	void ExtendPool()
	{
		Elem* extendPool = new Elem[m_extendSize];
		for (size_t i = 0; i < m_extendSize - 1; i++)
		{
			extendPool[i].next = &extendPool[i + 1];
		}
		extendPool[m_extendSize - 1].next = m_freeHead;
		m_freeHead = &extendPool[0];
		m_pool.push_back(extendPool);
	}
private:
	std::vector<Elem*> m_pool;
	Elem* m_freeHead;
	size_t m_extendSize;
	std::mutex lock;
};




