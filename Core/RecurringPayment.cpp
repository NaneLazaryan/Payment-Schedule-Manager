#include "RecurringPayment.h"
#include <stdexcept>
#include <sstream>

RecurringPayment::RecurringPayment(const std::string& id,
                                   const std::string& name,
                                   double amount,
                                   PaymentCategory category,
                                   std::unique_ptr<RecurringSchedule> schedule,
                                   const std::string& bankAccountNumber)
    : m_id(id),
    m_name(name),
    m_amount(amount),
    m_category(category),
    m_bankAccountNumber(bankAccountNumber),
    m_schedule(std::move(schedule)),
    m_isActive(true)
{
    if (amount <= 0) {
        throw std::invalid_argument("Payment amount must be positive");
    }
    if(id.empty()){
        throw std::invalid_argument("Recurring payment ID cannot be empty");
    }
    if (name.empty()) {
        throw std::invalid_argument("Recurring payment name cannot be empty");
    }
    if(!m_schedule){
        throw std::invalid_argument("Schedule cannot be null");
    }
    if(!m_schedule->isValid()){
        throw std::invalid_argument("Invalid schedule provided");
    }
    if (m_bankAccountNumber.empty()) {
        throw std::invalid_argument("Bank account number cannot be empty");
    }
}

RecurringPayment::RecurringPayment(const RecurringPayment& other)
    : m_id(other.m_id),
    m_name(other.m_name),
    m_amount(other.m_amount),
    m_category(other.m_category),
    m_bankAccountNumber(other.m_bankAccountNumber),
    m_schedule(other.m_schedule ? other.m_schedule->clone() : nullptr),
    m_isActive(other.m_isActive)
{}

RecurringPayment& RecurringPayment::operator=(const RecurringPayment& other)
{
    if (this == &other) {
        return *this;
    }

    m_id = other.m_id;
    m_name = other.m_name;
    m_amount = other.m_amount;
    m_category = other.m_category;
    m_bankAccountNumber = other.m_bankAccountNumber;
    m_schedule = other.m_schedule ? other.m_schedule->clone() : nullptr;
    m_isActive = other.m_isActive;

    return *this;
}

// Setters
void RecurringPayment::setName(const std::string& name)
{
    if (name.empty()) {
        throw std::invalid_argument("Recurring payment name cannot be empty");
    }
    m_name = name;
}

void RecurringPayment::setAmount(double amount)
{
    if (amount <= 0) {
        throw std::invalid_argument("Payment amount must be positive");
    }
    m_amount = amount;
}
void RecurringPayment::setCategory(PaymentCategory category)
{
    m_category = category;
}
void RecurringPayment::setBankAccountNumber(const std::string& accountNumber)
{
    if (accountNumber.empty()) {
        throw std::invalid_argument("Bank account number cannot be empty");
    }
    m_bankAccountNumber = accountNumber;
}
void RecurringPayment::setSchedule(std::unique_ptr<RecurringSchedule> schedule)
{
    if (!schedule) {
        throw std::invalid_argument("Schedule cannot be null");
    }
    if (!schedule->isValid()) {
        throw std::invalid_argument("Invalid schedule provided");
    }
    m_schedule = std::move(schedule);
}

void RecurringPayment::setActive(bool active)
{
    m_isActive = active;
    if (m_schedule) {
        m_schedule->setActive(active);
    }
}

bool RecurringPayment::isValid() const
{
    if(m_id.empty() || m_name.empty()) return false;
    if(m_amount<=0) return false;
    if(!m_schedule) return false;
    if(m_bankAccountNumber.empty()) return false;
    return m_schedule->isValid();
}

std::string RecurringPayment::toString() const
{
    std::ostringstream os;
    os << "Recurring Payment [" << m_id << "]\n"
       << "  Name: " << m_name << "\n"
       << "  Amount: $" << m_amount << "\n"
       << "  Category: " << PaymentTypeUtils::categoryToString(m_category) << "\n"
       << "  Bank Account: " << m_bankAccountNumber << "\n"
       << "  Schedule: " << (m_schedule ? m_schedule->getDescription() : "None") << "\n"
       << "  Active: " << (m_isActive ? "Yes" : "No")
       << "  Start Date: " << DateTimeUtils::formatDateTime(getStartDate());

    if (getEndDate().has_value()) {
        os << "\n  End Date: " << DateTimeUtils::formatDateTime(getEndDate().value());
    }

    return os.str();
}

std::unique_ptr<PaymentInstance> RecurringPayment::generateNextInstance(const DateTime& fromDate) const
{
    if(!m_isActive || !m_schedule->isActive()){
        return nullptr;
    }

    try{
        DateTime nextDueDate = m_schedule->getNextPayment(fromDate);

        static int instanceCounter = 0;
        std::string instId = generateInstanceId(++instanceCounter);

        return std::make_unique<PaymentInstance>(instId, m_id, m_amount, nextDueDate, m_category, m_bankAccountNumber);
    }
    catch(const std::runtime_error&){
        return nullptr; // No more occurences
    }
}

std::vector<PaymentInstance> RecurringPayment::generateInstances(const DateTime& fromDate, const DateTime& toDate) const
{
    std::vector<PaymentInstance> instances;
    if(!m_isActive || !m_schedule->isActive()){
        return instances;
    }

    if(fromDate > toDate) return instances;

    DateTime currentDate = fromDate;
    int instanceCounter = 0;

    while(currentDate <= toDate){
        try{
            DateTime nextDueDate = m_schedule->getNextPayment(currentDate);

            if(nextDueDate > toDate) break;
            std::string instId = generateInstanceId(++instanceCounter);

            instances.emplace_back(instId, m_id, m_amount, nextDueDate, m_category, m_bankAccountNumber);

            currentDate = nextDueDate;
            currentDate += std::chrono::hours(24);  // day after payment
        }
        catch(const std::runtime_error&){
            break;  // No more occurences
        }
    }

    return instances;
}

std::string RecurringPayment::generateInstanceId(int instanceNumber) const {
    std::ostringstream os;
    os << m_id << "_instance_" << instanceNumber;
    return os.str();
}
