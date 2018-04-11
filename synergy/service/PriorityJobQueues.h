#pragma once

#include <queue>
#include <vector>

template <typename T>
class PriorityJobQueues {
public:
    PriorityJobQueues(int numberOfQueue = 0) :
        m_queues(numberOfQueue) {
    }

    size_t size() { return m_size; }

    void appendQueue() {
        m_queues.emplace_back();
    }

    bool appendJob(typename std::queue<T>::size_type index, T job) {
        if (index >= m_queues.size()) {
            return false;
        }

        m_queues[index].emplace(std::move(job));
        ++m_size;
        return true;
    }

    bool hasJob() const noexcept {
        return m_size;
    }

    T& currentJob() {
        if (m_currentJobQueueIndex <= -1 ||
            m_currentJobQueueIndex >= m_queues.size()) {
            throw;
        }

        return m_queues[m_currentJobQueueIndex].front();
    }

    T& nextJob() {
        int index = 0;
        for (auto& q : m_queues) {
            if (!q.empty()) {
                m_currentJobQueueIndex = index;
                return q.front();
            }

            index += 1;
        }

        throw;
    }

    void popJob() {
        if (m_currentJobQueueIndex <= -1 ||
            m_currentJobQueueIndex >= m_queues.size()) {
            throw;
        }

        m_queues[m_currentJobQueueIndex].pop();

        m_size -= 1;
    }

private:
    std::vector<std::queue<T>> m_queues;
    size_t m_size = 0;
    int m_currentJobQueueIndex = -1;
};
