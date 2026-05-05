#ifndef PAYMENTGENERATOR_H
#define PAYMENTGENERATOR_H

#include "RecurringPayment.h"
#include "PaymentInstance.h"
#include "DateTime.h"
#include <vector>
#include <memory>

class PaymentGenerator
{
public:
    PaymentGenerator() = default;
    ~PaymentGenerator() = default;

    std::unique_ptr<PaymentInstance> generateNext(const RecurringPayment& recurringPayment, const DateTime& fromDate) const;
    std::vector<PaymentInstance> generateInRange(const RecurringPayment& recurringPayment, const DateTime& fromDate, const DateTime& toDate) const;
    std::vector<PaymentInstance> generateUpTo(const RecurringPayment& recurringPayment,const DateTime& targetDate,const std::vector<PaymentInstance>& existingInstances = {}) const;

private:
    // Check payment instance already exists
    bool instanceExistsForDate(const std::vector<PaymentInstance>& existingInstances, const DateTime& dueDate) const;
    // Unique ID for payment
    std::string generateInstanceId(const std::string& recurringPaymentId, int instanceNumber) const;
};

#endif // PAYMENTGENERATOR_H
