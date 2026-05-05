#ifndef RECURRINGPAYMENT_H
#define RECURRINGPAYMENT_H

#include <string>
#include <memory>
#include "PaymentTypes.h"
#include "RecurringSchedule.h"
#include "PaymentInstance.h"

class RecurringPayment
{
public:
    RecurringPayment(const std::string& id,
                     const std::string& name,
                     double amount,
                     PaymentCategory category,
                     std::unique_ptr<RecurringSchedule> schedule,
                     const std::string& bankAccountNumber = "");

    // Copy/Move constructors
    RecurringPayment(const RecurringPayment& other);
    RecurringPayment(RecurringPayment&&) noexcept = default;

    // Copy/Move assignments
    RecurringPayment& operator=(const RecurringPayment& other);
    RecurringPayment& operator=(RecurringPayment&&) noexcept = default;

    ~RecurringPayment() = default;

    // Getters
    const std::string& getId() const { return m_id; }
    const std::string& getName() const { return m_name; }
    double getAmount() const { return m_amount; }
    PaymentCategory getCategory() const { return m_category; }
    const std::string& getBankAccountNumber() const { return m_bankAccountNumber; }
    bool isActive() const { return m_isActive && m_schedule->isActive(); }
    const RecurringSchedule* getSchedule() const { return m_schedule.get(); }

    DateTime getStartDate() const { return m_schedule->getStartDate(); }
    std::optional<DateTime> getEndDate() const { return m_schedule->getEndDate(); }
    std::string getScheduleDescription() const { return m_schedule->getDescription(); }

    // Setters
    void setName(const std::string& name);
    void setAmount(double amount);
    void setCategory(PaymentCategory category);
    void setBankAccountNumber(const std::string& accountNumber);
    void setSchedule(std::unique_ptr<RecurringSchedule> schedule);
    void setActive(bool active);

    bool isValid() const;
    std::string toString() const;

    std::unique_ptr<PaymentInstance> generateNextInstance(const DateTime& fromDate) const;
    std::vector<PaymentInstance> generateInstances(const DateTime& fromDate, const DateTime& toDate) const;

private:
    std::string m_id;
    std::string m_name;
    double m_amount;
    PaymentCategory m_category;
    std::string m_bankAccountNumber;
    std::unique_ptr<RecurringSchedule> m_schedule;
    bool m_isActive;

private:
    std::string generateInstanceId(int number) const;
};

#endif // RECURRINGPAYMENT_H
