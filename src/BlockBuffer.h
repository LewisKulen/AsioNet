#include "MemPool.h"

namespace AsioNet {
	constexpr unsigned int DEFAULT_SEND_BUFFER_SIZE = 1024 * 8;
	const unsigned int DEFAULT_SEND_BUFFER_POOL_EXTEND_SIZE = 8;

	template<size_t V_BUFFER_SIZE>
	struct BlockElem {
		size_t Write(const char* data, size_t trans)	// 返回实际拷贝的数据量
		{
			size_t remain = V_BUFFER_SIZE >= pos ? V_BUFFER_SIZE - pos : 0;
			if (remain)
			{
				size_t copied = trans;
				if (remain < trans)	// 剩余的不够了
				{
					copied = remain;	// 只能拷贝这么多
				}
				memcpy_s(buffer + pos, remain, data, copied);
				pos += copied;
				return copied;
			}
			return 0;
		}
		bool IsFull()
		{
			return pos >= V_BUFFER_SIZE;
		}
		char buffer[V_BUFFER_SIZE];
		size_t pos;
		BlockElem<V_BUFFER_SIZE>* next;
	};

	template<size_t V_BUFFER_SIZE, size_t V_BUFFER_POOL_EXTEND_SIZE>
	class BlockSendBuffer {
	public:
		BlockSendBuffer() :
			m_pool(V_BUFFER_POOL_EXTEND_SIZE), head(nullptr), tail(nullptr), detachedHead(nullptr)
		{};

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
		void Push(const char* data, size_t trans)
		{
			if (trans <= 0)
			{
				return;
			}

			if (!tail)
			{
				tail = m_pool.New();
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
		}
	private:
		BlockElem<V_BUFFER_SIZE>* head, * tail, * detachedHead;
		MemPool_ThreadUnsafe<BlockElem<V_BUFFER_SIZE>> m_pool;
	};
}