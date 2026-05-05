#include "InMemoryRepository.h"
#include <stdexcept>
#include <algorithm>

void InMemoryRepository::addRecurringPayment(const RecurringPayment& recurringPayment)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    if(m_recurringPayments.count(recurringPayment.getId()) > 0){
        throw std::invalid_argument("Recurring payment with ID '" + recurringPayment.getId() + "' already exists");
    }

    m_recurringPayments.emplace(recurringPayment.getId(), recurringPayment);
}

RecurringPayment* InMemoryRepository::findRecurringPayment(const std::string& id)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    auto it = m_recurringPayments.find(id);
    if(it != m_recurringPayments.end()){
        return &it->second;
    }
    return nullptr;
}
std::vector<RecurringPayment> InMemoryRepository::getAllRecurringPayments() const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    std::vector<RecurringPayment> res;
    res.reserve(m_recurringPayments.size());

    for (const auto& pair : m_recurringPayments) {
        res.push_back(pair.second);
    }

    return res;
}
std::vector<RecurringPayment> InMemoryRepository::getActiveRecurringPayments() const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    std::vector<RecurringPayment> res;

    for (const auto& pair : m_recurringPayments) {
        if (pair.second.isActive()) {
            res.push_back(pair.second);
        }
    }

    return res;
}
void InMemoryRepository::updateRecurringPayment(const RecurringPayment& recurringPayment)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    auto it = m_recurringPayments.find(recurringPayment.getId());
    if (it == m_recurringPayments.end()) {
        throw std::runtime_error("Recurring payment with ID '" + recurringPayment.getId() + "' not found");
    }

    it->second = recurringPayment;
}

bool InMemoryRepository::removeRecurringPayment(const std::string& id)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    return m_recurringPayments.erase(id) > 0;
}

bool InMemoryRepository::recurringPaymentExists(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    return m_recurringPayments.count(id) > 0;
}


void InMemoryRepository::addPaymentInstance(const PaymentInstance& instance)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    if(m_paymentInstances.count(instance.getName()) > 0){
        throw std::invalid_argument("Payment instance with ID '" + instance.getName() + "' already exists");
    }

    m_paymentInstances.emplace(instance.getName(), instance);
}

PaymentInstance* InMemoryRepository::findPaymentInstance(const std::string& id)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    auto it = m_paymentInstances.find(id);
    if(it!=m_paymentInstances.end()){
        return &it->second;
    }
    return nullptr;
}

std::vector<PaymentInstance> InMemoryRepository::getPaymentInstances(const std::string& recurringPaymentId) const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    std::vector<PaymentInstance> res;

    for (const auto& pair : m_paymentInstances) {
        if (pair.second.getRecurringPaymentId() == recurringPaymentId) {
            res.push_back(pair.second);
        }
    }

    return res;
}

std::vector<PaymentInstance> InMemoryRepository::getAllPaymentInstances() const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    std::vector<PaymentInstance> result;
    result.reserve(m_paymentInstances.size());

    for (const auto& pair : m_paymentInstances) {
        result.push_back(pair.second);
    }

    return result;
}

std::vector<PaymentInstance> InMemoryRepository::getPaymentInstancesByStatus(PaymentStatus status) const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    std::vector<PaymentInstance> result;

    for (const auto& pair : m_paymentInstances) {
        if (pair.second.getStatus() == status) {
            result.push_back(pair.second);
        }
    }

    return result;
}

std::vector<PaymentInstance> InMemoryRepository::getPaymentInstancesDueBefore(const DateTime& date) const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    std::vector<PaymentInstance> result;

    for (const auto& pair : m_paymentInstances) {
        if (pair.second.isDueBefore(date)) {
            result.push_back(pair.second);
        }
    }

    return result;
}

void InMemoryRepository::updatePaymentInstance(const PaymentInstance& instance)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    auto it = m_paymentInstances.find(instance.getName());
    if (it == m_paymentInstances.end()) {
        throw std::runtime_error("Payment instance with ID '" + instance.getName() + "' not found");
    }

    it->second = instance;
}

bool InMemoryRepository::removePaymentInstance(const std::string& id)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    return m_paymentInstances.erase(id) > 0;
}

bool InMemoryRepository::paymentInstanceExists(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    return m_paymentInstances.count(id) > 0;
}

void InMemoryRepository::clearAll() {
    std::lock_guard<std::mutex> lock(m_mtx);

    m_recurringPayments.clear();
    m_paymentInstances.clear();
}

size_t InMemoryRepository::getRecurringPaymentCount() const {
    std::lock_guard<std::mutex> lock(m_mtx);

    return m_recurringPayments.size();
}

size_t InMemoryRepository::getPaymentInstanceCount() const {
    std::lock_guard<std::mutex> lock(m_mtx);

    return m_paymentInstances.size();
}
