#ifndef INMEMORYREPOSITORY_H
#define INMEMORYREPOSITORY_H

#include "IPaymentRepository.h"
#include <unordered_map>
#include <vector>
#include <mutex>

class InMemoryRepository : public IPaymentRepository
{
public:
    InMemoryRepository() = default;
    virtual ~InMemoryRepository() = default;

    // Recurring Payment operations
    void addRecurringPayment(const RecurringPayment& recurringPayment);
    RecurringPayment* findRecurringPayment(const std::string& id);
    std::vector<RecurringPayment> getAllRecurringPayments() const;
    std::vector<RecurringPayment> getActiveRecurringPayments() const;
    void updateRecurringPayment(const RecurringPayment& recurringPayment);
    bool removeRecurringPayment(const std::string& id);
    bool recurringPaymentExists(const std::string& id) const;

    // Payment Instance operations
    void addPaymentInstance(const PaymentInstance& instance);
    PaymentInstance* findPaymentInstance(const std::string& id);
    std::vector<PaymentInstance> getPaymentInstances(const std::string& recurringPaymentId) const;
    std::vector<PaymentInstance> getAllPaymentInstances() const;
    std::vector<PaymentInstance> getPaymentInstancesByStatus(PaymentStatus status) const;
    std::vector<PaymentInstance> getPaymentInstancesDueBefore(const DateTime& date) const;
    void updatePaymentInstance(const PaymentInstance& instance);
    bool removePaymentInstance(const std::string& id);
    bool paymentInstanceExists(const std::string& id) const;

    // Utility operations
    void clearAll();
    size_t getRecurringPaymentCount() const;
    size_t getPaymentInstanceCount() const;

private:
    std::unordered_map<std::string, RecurringPayment> m_recurringPayments;
    std::unordered_map<std::string, PaymentInstance> m_paymentInstances;
    mutable std::mutex m_mtx;
};

#endif // INMEMORYREPOSITORY_H
