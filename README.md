# BlockingQueue
A toy message queue for homework

## features
队列头尾各有一把锁，在队列中元素个数大于 1 的时候读写操作互不干扰。

## APIs
template<class T>
bool BlockingQueue<T>::IsEmpty() const;
判断队列是否为空。非线程安全。

template<class T>
void BlockingQueue<T>::Push(T data); 
向队列中添加 data。
  
template<class T>
bool BlockingQueue<T>::TryPush(T data);
尝试向队列中添加元素。成功时返回 true，失败时返回 false。
  
template<class T>
T BlockingQueue<T>::Pop();
从队列中弹出 data。若队列为空则阻塞线程。
  
template<class T>
bool TryPop(T& data);
尝试从队列中弹出 data。成功时返回 true，失败时返回 false。
