#ifndef PAYMENTSERVICE_H
#define PAYMENTSERVICE_H

#include "IPaymentRepository.h"
#include "PaymentGenerator.h"
#include "RecurringPayment.h"
#include "PaymentInstance.h"
#include "RecurringSchedule.h"
#include "PaymentTypes.h"
#include "DateTime.h"
#include <memory>
#include <vector>
#include <string>

class PaymentService
{
public:
    explicit PaymentService(std::unique_ptr<IPaymentRepository> repository);
    ~PaymentService() = default;

    void createRecurringPayment(const std::string& name,
                                const std::string& id,
                                double amount,
                                PaymentCategory category,
                                const std::string& bankAccountNumber,
                                std::unique_ptr<RecurringSchedule> schedule);

    RecurringPayment* getRecurringPayment(const std::string& id);
    std::vector<RecurringPayment> getAllRecurringPayments() const;

    void updateRecurringPayment(const RecurringPayment& recurringPayment);
    void deactivateRecurringPayment(const std::string& id);
    void activateRecurringPayment(const std::string& id);
    bool deleteRecurringPayment(const std::string& id);

    std::vector<PaymentInstance> getPaymentInstances(const std::string& recurringPaymentId) const;
    std::vector<PaymentInstance> getAllPaymentInstances() const;
    std::vector<PaymentInstance> getPaymentInstancesByStatus(PaymentStatus status) const;
    std::vector<PaymentInstance> getPendingPayments() const;
    std::vector<PaymentInstance> getOverduePayments();
    std::vector<PaymentInstance> getUpcomingPayments(const DateTime& fromDate, const DateTime& toDate) const;

    void markPaymentAsPaid(const std::string& instanceId, PaymentMethod method = PaymentMethod::OTHER);
    void markPaymentAsFailed(const std::string& instanceId, const std::string& reason = "");
    void cancelPaymentInstance(const std::string& instanceId);

    void generatePaymentsUpTo(const DateTime& targetDate);
    void generatePaymentsForRecurringPayment(const std::string& recurringPaymentId, const DateTime& targetDate);
    std::unique_ptr<PaymentInstance> getNextPayment(const std::string& recurringPaymentId) const;

private:
    std::unique_ptr<IPaymentRepository> m_repo;
    PaymentGenerator m_generator;

private:
    void updateOverduePayments();
};

#endif // PAYMENTSERVICE_H
