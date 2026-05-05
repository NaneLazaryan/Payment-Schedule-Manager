#ifndef NOTIFICATIONCENTER_H
#define NOTIFICATIONCENTER_H

#include <vector>
#include <algorithm>
#include "PaymentInstance.h"

// Observer
class NotificationObserver
{
public:
    virtual ~NotificationObserver() = default;
    virtual void onPaymentsDue(const std::vector<PaymentInstance>& duePayments) = 0;
};


// Observable
class NotificationCenter
{
public:
    void addObserver(NotificationObserver* observer)
    {
        if (observer && std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end()) {
            m_observers.push_back(observer);
        }
    }

    void removeObserver(NotificationObserver* observer)
    {
        m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer), m_observers.end());
    }

    void notifyPaymentsDue(const std::vector<PaymentInstance>& duePayments) const
    {
        for (auto* obs : m_observers) {
            if (obs) {
                obs->onPaymentsDue(duePayments);
            }
        }
    }

private:
    std::vector<NotificationObserver*> m_observers;
};

#endif // NOTIFICATIONCENTER_H

