#pragma once

#include "MemPool.h"
#include <queue>

template<size_t V_BUFFER_SIZE>
struct BlockElem {
	// 数据写入这个Block，直到buffer写满
	// 返回实际拷贝的数据量
	size_t Write(const char* data, size_t trans)
	{
		size_t remain = V_BUFFER_SIZE >= wpos ? V_BUFFER_SIZE - wpos : 0;
		if (remain)
		{
			size_t copied = trans;
			if (remain < trans)	// 剩余的不够了
			{
				copied = remain;	// 只能拷贝这么多
			}
			memcpy_s(buffer + wpos, remain, data, copied);
			wpos += copied;
			return copied;
		}
		return 0;
	}

	bool IsFull()
	{
		return wpos >= V_BUFFER_SIZE;
	}

	char buffer[V_BUFFER_SIZE];
	size_t wpos;	// 写偏移
	BlockElem<V_BUFFER_SIZE>* next;
	bool done;
};

template<size_t V_BUFFER_SIZE/*每个buffer的大小*/,
			size_t V_EXTEND_NUM/*每次扩充的大小*/>
class BlockSendBuffer {
public:
	// 每次扩充几个buffer
	BlockSendBuffer() :
		m_pool(V_EXTEND_NUM), head(nullptr), tail(nullptr), detachedHead(nullptr)
	{}
	~BlockSendBuffer()
	{}
	bool Empty()
	{
		return head == nullptr;
	}
	BlockElem<V_BUFFER_SIZE>* DetachHead()
	{
		if (!detachedHead && head) {
			detachedHead = head;
			if (tail && tail == detachedHead) {
				tail = tail->next;	// nil
			}
			head = head->next;	// could be nil
			return detachedHead;
		}
		return nullptr;
	}
	void FreeDeatched()
	{
		if (detachedHead)
		{
			m_pool.Del(detachedHead);
			detachedHead = nullptr;
		}
	}
	// 数据可能分散在多个block上
	bool Push(const char* data, size_t trans)
	{
		if (trans <= 0)
		{
			return false;
		}

		if (!tail)
		{
			tail = m_pool.New();	// this will memset the block
			if (!head)	// first push
			{
				head = tail;
			}
		}

		size_t copied = 0;
		do {
			copied += tail->Write(data + copied, trans - copied);

			if (tail->IsFull())	// 用完了就再分配一个把
			{
				tail->next = m_pool.New();
				tail = tail->next;
			}
		} while (copied < trans);
		return true;
	}

	void Clear()
	{
		m_pool.Clear();
		detachedHead = head = tail = nullptr;
	}
private:
	BlockElem<V_BUFFER_SIZE>* head, * tail, * detachedHead;
	MemPool_ThreadUnsafe<BlockElem<V_BUFFER_SIZE>> m_pool;
};


template<size_t V_BUFFER_SIZE>
struct BlockElem_1 {
	// 尝试将数据全部写入这个Block，如果剩余buffer不足，则不写入
	size_t WriteAll(const char* data, size_t trans)
	{
		size_t remain = V_BUFFER_SIZE >= wpos ? V_BUFFER_SIZE - wpos : 0;
		if (remain >= trans)
		{
			memcpy_s(buffer + wpos, remain, data, trans);
			wpos += trans;
			return trans;
		}
		return 0;
	}
	bool IsFull()
	{
		return wpos >= V_BUFFER_SIZE;
	}
	char* Read(size_t r)
	{
		char* data = buffer + rpos;
		rpos += r;
		return data;
	}
	bool Empty()
	{
		return rpos >= wpos;
	}
	bool IsDone()
	{
		return done;
	}
	void Done()
	{
		done = true;
	}
	char buffer[V_BUFFER_SIZE];
	size_t wpos;	// 写偏移
	size_t rpos;	// 读偏移,max = V_BUFFER_SIZE
	BlockElem_1<V_BUFFER_SIZE>* next;
	bool done;
};

template<size_t V_BUFFER_SIZE/*每个buffer的大小*/,
			size_t V_EXTEND_NUM/*每次扩充的大小*/>
class BlockBuffer {
public:
	// 每次扩充几个buffer
	BlockBuffer() :
		m_pool(V_EXTEND_NUM),head(nullptr), tail(nullptr)
	{}
	~BlockBuffer()
	{}
	bool Empty()
	{
		return head == nullptr;
	}

	// 保证数据在一个block上
	bool Push(const char* data, size_t trans)
	{
		// 保证写入的大小在一个block里能放得下
		if (trans <= 0 || trans > V_BUFFER_SIZE)
		{
			return false;
		}
		if (!tail)
		{
			tail = m_pool.New();	// this will memset the block
			if (!head)	// first push
			{
				head = tail;
			}
		}

		if(tail->WriteAll(data,trans) != trans)
		{
			tail->Done();
			tail->next = m_pool.New();
			tail = tail->next;
			tail->WriteAll(data,trans);
		}
		que.push(trans);
		return true;
	}

	// 将数据写入传进的buffer中
	bool Pop(char** in,size_t* s)
	{
		if (!head || que.empty())
		{
			return false;
		}
		size_t len = que.front();
		char* buf = head->Read(len);
		memcpy_s(*in,len,buf,len);
		*s = len;

		if(head->IsDone() && head->Empty())
		{
			auto p = head;
			m_pool.Del(p);
			head = head->next;
		}

		que.pop();
		return true;
	}

	std::string PopToString()
	{
		if (!head || que.empty())
		{
			return "";
		}
		size_t len = que.front();
		char* buf = head->Read(len);
		if(head->IsDone() && head->Empty())
		{
			auto p = head;
			m_pool.Del(p);
			head = head->next;
		}
		que.pop();
		return std::string(buf,len);
	}
private:
	BlockElem_1<V_BUFFER_SIZE>* head, * tail;
	MemPool_ThreadUnsafe<BlockElem_1<V_BUFFER_SIZE>> m_pool;
	std::queue<size_t> que;
};
