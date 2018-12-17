#pragma once

#include <exception>
#include <mutex>

namespace jojo {
	namespace _internal {
		template <class T>
		class INode {
		public:
			INode<T> * next;

			INode(): next(nullptr) { }
			INode(const INode&) = default;
			INode(INode&&) = default;
			INode& operator= (const INode& rhs) = default;
			INode& operator= (INode&& rhs) = default;
			virtual ~INode() = 0 {}
		};
	}
	using namespace _internal;

	template<class T>
	class BlockingQueue {
	public:
		class Node : public INode<T>{
		public :
			T data;

			Node() = default;
			explicit Node(T data): INode<T>(), data(data) { }
			~Node() override = default;
		}; 

	public:
		BlockingQueue(): head(new SentryNode), tail(new SentryNode) {
			head->next = tail;
			tail->next = head;
		}

		~BlockingQueue()
		{
			clear();
		}

		void clear()
		{
			std::lock_guard<std::mutex> lock_head(head->mtx);
			std::lock_guard<std::mutex> lock_tail(tail->mtx);

			while (head->next != tail)
			{
				Node* node = dynamic_cast<Node*>(head->next);
				head->next = node->next;
				delete node;
			}
		}

		bool IsEmpty() const
		{
			return head->next == tail;
		}

		void Push(const T& data)
		{
			std::lock_guard<std::mutex> lock_tail(tail->mtx);

			Node* node = new Node(data);
			node->next = tail;

			if (tail->next != head)
			{
				tail->next->next = node;
			}
			else // empty
			{
				std::lock_guard<std::mutex> lock_head(head->mtx);

				if (head->next == tail) // now we can't promise that head->next == tail even tail->next == head (which means the queue is empty) just before because of multi-threads
				{
					head->next = node;
				}

				head->cond.notify_one();
			}
			tail->next = node;
		}

		bool TryPush(const T& data)
		{
			if (!tail->mtx.try_lock())
				return false;

			if (tail->next == head) // empty
			{
				if (!head->mtx.try_lock())
				{
					tail->mtx.unlock();
					return false;
				}

				Node* node = new Node(data);
				node->next = tail;
				if (head->next == tail) // now we can't promise that head->next == tail even tail->next == head (which means the queue is empty) just before because of multi-threads
				{
					head->next = node;
				}

				head->cond.notify_one();
				head->mtx.unlock();
			}
			else
			{
				Node* node = new Node(data);
				node->next = tail;
				tail->next->next = node;
			}

			tail->mtx.unlock();
			return true;
		}

		T Pop()
		{
			std::unique_lock<std::mutex> lock_head(head->mtx);
			head->cond.wait(lock_head, [this]() { return this->head->next != this->tail; });

			Node* node = dynamic_cast<Node*>(head->next);
			head->next = node->next;

			if (node->next == tail)
			{
				std::lock_guard<std::mutex> lock_tail(tail->mtx);

				tail->next = head;
			}

			T data = node->data;
			delete node;
			return data;
		}

		bool TryPop(T& data)
		{
			if (!head->mtx.try_lock())
				return false;

			if (head->next == tail) // empty
			{
				head->mtx.unlock();
				return false;
			}

			if (head->next->next == tail)
			{
				if (!tail->mtx.try_lock())
				{
					head->mtx.unlock();
					return false;
				}
				if (head->next->next == tail)
				{
					tail->next = head;
				}
				tail->mtx.unlock();
			}

			Node* node = dynamic_cast<Node*>(head->next);
			data = std::move(node->data);
			head->next = node->next;
			delete node;

			head->mtx.unlock();
			return true;
		}

	private:
		class SentryNode : public INode<T>{
		public:
			std::condition_variable cond;
			std::mutex mtx;

			~SentryNode() override = default;
		};

	private:
		SentryNode * const head;
		SentryNode * const tail; // tail->next means "tail->prev" in fact
	};
}
