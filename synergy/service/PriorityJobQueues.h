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

    T& nextJob() {
        for (auto& q : m_queues) {
            if (!q.empty()) {
                return q.front();
            }
        }

        throw;
    }

    void popJob() {
        for (auto& q : m_queues) {
            if (!q.empty()) {
                q.pop();
                --m_size;
                return;
            }
        }
    }

private:
    std::vector<std::queue<T>> m_queues;
    size_t m_size = 0;
};
