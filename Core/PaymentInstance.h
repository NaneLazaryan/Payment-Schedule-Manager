#ifndef PAYMENTINSTANCE_H
#define PAYMENTINSTANCE_H

#include "DateTime.h"
#include "PaymentTypes.h"

class PaymentInstance
{
public:
    PaymentInstance(const std::string& id,
                    const std::string& recurringPaymentId,
                    double amount,
                    DateTime dueDate,
                    PaymentCategory category = PaymentCategory::OTHER,
                    const std::string& bankAccountNumber = "");

    // Copy/Move constructors
    PaymentInstance(const PaymentInstance&) = default;
    PaymentInstance(PaymentInstance&&) noexcept = default;

    // Copy/Move assignments
    PaymentInstance& operator=(const PaymentInstance&) = default;
    PaymentInstance& operator=(PaymentInstance&&) noexcept = default;

    ~PaymentInstance() = default;

    // Getters
    const std::string& getName() const { return m_name; }
    const std::string& getRecurringPaymentId() const { return m_recurringPaymentId; }
    double getAmount() const { return m_amount; }
    DateTime getDueDate() const { return m_dueDate; }
    PaymentCategory getCategory() const { return m_category; }
    const std::string& getBankAccountNumber() const { return m_bankAccountNumber; }
    std::optional<PaymentMethod> getPaymentMethod() const { return m_payMethod;}
    PaymentStatus getStatus() const { return m_status; }
    std::optional<DateTime> getPaidDate() const { return m_paidDate; }
    std::optional<std::string> getFailureReason() const { return m_failureReason; }

    // Setters
    void setDueDate(const DateTime& dueDate) { m_dueDate = dueDate; }
    void setAmount(double amount);
    void setCategory(PaymentCategory category) { m_category = category; }

    // State management
    void markAsPaid(PaymentMethod method = PaymentMethod::OTHER);
    void markAsFailed(const std::string& reason = "");
    void markAsOverdue();
    void processPayment();
    void cancelPayment();

    // Status checks
    bool isPending() const { return m_status == PaymentStatus::PENDING; }
    bool isPaid() const { return m_status == PaymentStatus::PAID; }
    bool isFailed() const { return m_status == PaymentStatus::FAILED; }
    bool isCancelled() const { return m_status == PaymentStatus::CANCELLED; }
    bool isProcessing() const { return m_status == PaymentStatus::PROCESSING; }
    bool isOverdue() const;
    bool isDueToday() const;
    bool isDueBefore(const DateTime& date) const;

    // Formatting
    std::string toString() const;

private:
    std::string m_name;
    std::string m_recurringPaymentId;
    double m_amount;
    DateTime m_dueDate;
    std::optional<DateTime> m_paidDate;
    PaymentCategory m_category;
    std::string m_bankAccountNumber;
    std::optional<PaymentMethod> m_payMethod;
    PaymentStatus m_status;
    std::optional<std::string> m_failureReason;
};

#endif // PAYMENTINSTANCE_H
