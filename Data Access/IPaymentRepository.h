#ifndef IPAYMENTREPOSITORY_H
#define IPAYMENTREPOSITORY_H

#include "RecurringPayment.h"
#include "PaymentInstance.h"
#include <vector>
#include <string>
#include <memory>

// Payment data repository
class IPaymentRepository
{
public:
    virtual ~IPaymentRepository() = default;

    virtual void addRecurringPayment(const RecurringPayment& recurringPayment) = 0;
    virtual RecurringPayment* findRecurringPayment(const std::string& id) = 0;
    virtual std::vector<RecurringPayment> getAllRecurringPayments() const = 0;
    virtual std::vector<RecurringPayment> getActiveRecurringPayments() const = 0;
    virtual void updateRecurringPayment(const RecurringPayment& recurringPayment) = 0;
    virtual bool removeRecurringPayment(const std::string& id) = 0;
    virtual bool recurringPaymentExists(const std::string& id) const = 0;

    virtual void addPaymentInstance(const PaymentInstance& instance) = 0;
    virtual PaymentInstance* findPaymentInstance(const std::string& id) = 0;
    virtual std::vector<PaymentInstance> getPaymentInstances(const std::string& recurringPaymentId) const = 0;
    virtual std::vector<PaymentInstance> getAllPaymentInstances() const = 0;
    virtual std::vector<PaymentInstance> getPaymentInstancesByStatus(PaymentStatus status) const = 0;
    virtual std::vector<PaymentInstance> getPaymentInstancesDueBefore(const DateTime& date) const = 0;
    virtual void updatePaymentInstance(const PaymentInstance& instance) = 0;
    virtual bool removePaymentInstance(const std::string& id) = 0;
    virtual bool paymentInstanceExists(const std::string& id) const = 0;

    // Utility operations
    virtual void clearAll() = 0;
    virtual size_t getRecurringPaymentCount() const = 0;
    virtual size_t getPaymentInstanceCount() const = 0;
};

#endif // IPAYMENTREPOSITORY_H
