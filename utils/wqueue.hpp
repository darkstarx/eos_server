#ifndef WQUEUE_HPP
#define WQUEUE_HPP

#include <pthread.h>
#include <list>

template <typename T>
class wqueue
{
public:
	wqueue(): m_active(true)
	{
		pthread_mutex_init(&m_mutex, NULL);
		pthread_cond_init(&m_cond, NULL);
	}
	~wqueue()
	{
		pthread_mutex_destroy(&m_mutex);
		pthread_cond_destroy(&m_cond);
	}

	void push(T item)
	{
		pthread_mutex_lock(&m_mutex);
		m_queue.push_back(item);
		pthread_cond_signal(&m_cond);
		pthread_mutex_unlock(&m_mutex);
	}

	T pop()
	{
		pthread_mutex_lock(&m_mutex);
		while (m_queue.empty() && m_active) {
			pthread_cond_wait(&m_cond, &m_mutex);
		}
		T item;
		// Queue can be empty here only after release_consumers
		if (m_queue.empty()) item = T();
		else {
			item = m_queue.front();
			m_queue.pop_front();
		}
		pthread_mutex_unlock(&m_mutex);
		return item;
	}

	void release_consumers()
	{
		pthread_mutex_lock(&m_mutex);
		m_active = false;
		pthread_cond_broadcast(&m_cond);
		pthread_mutex_unlock(&m_mutex);
	}

	void resume_consuming()
	{
		pthread_mutex_lock(&m_mutex);
		m_active = true;
		pthread_mutex_unlock(&m_mutex);
	}

	size_t size() {
		pthread_mutex_lock(&m_mutex);
		size_t size = m_queue.size();
		pthread_mutex_unlock(&m_mutex);
		return size;
	}

private:
	bool m_active;
	std::list<T> m_queue;
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
};

#endif // WQUEUE_HPP
