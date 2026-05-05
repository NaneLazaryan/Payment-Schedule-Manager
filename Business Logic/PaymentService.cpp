#include "PaymentService.h"
#include <stdexcept>
#include <algorithm>
#include <chrono>

PaymentService::PaymentService(std::unique_ptr<IPaymentRepository> repository)
    : m_repo(std::move(repository))
{
    if(!m_repo){
        throw std::invalid_argument("Repository connot be null");
    }
}

void PaymentService::createRecurringPayment(const std::string& name,
                                            const std::string& id,
                                            double amount,
                                            PaymentCategory category,
                                            const std::string& bankAccountNumber,
                                            std::unique_ptr<RecurringSchedule> schedule)
{
    if (id.empty()) {
        throw std::invalid_argument("Recurring payment ID cannot be empty");
    }
    if (name.empty()) {
        throw std::invalid_argument("Recurring payment name cannot be empty");
    }
    if (bankAccountNumber.empty()) {
        throw std::invalid_argument("Bank account number cannot be empty");
    }
    if (amount <= 0) {
        throw std::invalid_argument("Payment amount must be positive");
    }
    if (!schedule) {
        throw std::invalid_argument("Schedule cannot be null");
    }

    if (m_repo->recurringPaymentExists(id)) {
        throw std::invalid_argument("Recurring payment with ID '" + id + "' already exists");
    }

    RecurringPayment recurringPayment(id, name, amount, category, std::move(schedule), bankAccountNumber);
    m_repo->addRecurringPayment(recurringPayment);
}

RecurringPayment* PaymentService::getRecurringPayment(const std::string& id)
{
    return m_repo->findRecurringPayment(id);
}

std::vector<RecurringPayment> PaymentService::getAllRecurringPayments() const
{
    return m_repo->getActiveRecurringPayments();
}

void PaymentService::updateRecurringPayment(const RecurringPayment& recurringPayment)
{
    if(!m_repo->recurringPaymentExists(recurringPayment.getId())){
        throw std::runtime_error("Recurring payment not found");
    }
    m_repo->updateRecurringPayment(recurringPayment);
}

void PaymentService::deactivateRecurringPayment(const std::string& id)
{
    RecurringPayment* payment = m_repo->findRecurringPayment(id);
    if(!payment){
        throw std::runtime_error("Recurring payment not found");
    }

    payment->setActive(false);
    m_repo->updateRecurringPayment(*payment);
}

void PaymentService::activateRecurringPayment(const std::string& id)
{
    RecurringPayment* payment = m_repo->findRecurringPayment(id);
    if(!payment){
        throw std::runtime_error("Recurring payment not found");
    }

    payment->setActive(true);
    m_repo->updateRecurringPayment(*payment);
}

bool PaymentService::deleteRecurringPayment(const std::string& id)
{
    return m_repo->removeRecurringPayment(id);
}

std::vector<PaymentInstance> PaymentService::getPaymentInstances(const std::string& recurringPaymentId) const
{
    return m_repo->getPaymentInstances(recurringPaymentId);
}

std::vector<PaymentInstance> PaymentService::getAllPaymentInstances() const
{
    return m_repo->getAllPaymentInstances();
}

std::vector<PaymentInstance> PaymentService::getPaymentInstancesByStatus(PaymentStatus status) const
{
    return m_repo->getPaymentInstancesByStatus(status);
}

std::vector<PaymentInstance> PaymentService::getPendingPayments() const
{
    return m_repo->getPaymentInstancesByStatus(PaymentStatus::PENDING);
}

std::vector<PaymentInstance> PaymentService::getOverduePayments()
{
    updateOverduePayments();

    auto overdue = m_repo->getPaymentInstancesByStatus(PaymentStatus::OVERDUE);
    auto pending = m_repo->getPaymentInstancesByStatus(PaymentStatus::PENDING);

    auto now = std::chrono::system_clock::now();
    for(const auto& payment : pending){
        if(payment.isOverdue())
            overdue.push_back(payment);
    }

    return overdue;
}

std::vector<PaymentInstance> PaymentService::getUpcomingPayments(const DateTime& fromDate, const DateTime& toDate) const
{
    auto allInstances = m_repo->getAllPaymentInstances();
    std::vector<PaymentInstance> result;

    for(const auto& instance : allInstances){
        const DateTime& dueDate = instance.getDueDate();
        if(dueDate >= fromDate && dueDate <= toDate){
            if(instance.getStatus() == PaymentStatus::PENDING || instance.getStatus() == PaymentStatus::OVERDUE){
                result.push_back(instance);
            }
        }
    }

    std::sort(result.begin(), result.end(), [](const PaymentInstance& a, const PaymentInstance& b){
        return a.getDueDate() < b.getDueDate();
    });

    return result;
}

void PaymentService::markPaymentAsPaid(const std::string& instanceId, PaymentMethod method)
{
    PaymentInstance* instance = m_repo->findPaymentInstance(instanceId);
    if(!instance){
        throw std::runtime_error("Payment instance not found");
    }

    instance->markAsPaid(method);
    m_repo->updatePaymentInstance(*instance);
}

void PaymentService::markPaymentAsFailed(const std::string& instanceId, const std::string& reason)
{
    PaymentInstance* instance = m_repo->findPaymentInstance(instanceId);
    if(!instance){
        throw std::runtime_error("Payment instance not found");
    }

    instance->markAsFailed(reason);
    m_repo->updatePaymentInstance(*instance);
}

void PaymentService::cancelPaymentInstance(const std::string& instanceId)
{
    PaymentInstance* instance = m_repo->findPaymentInstance(instanceId);
    if(!instance){
        throw std::runtime_error("Payment instance not found");
    }

    instance->cancelPayment();
    m_repo->updatePaymentInstance(*instance);
}

void PaymentService::generatePaymentsUpTo(const DateTime& targetDate)
{
    auto activePayments = m_repo->getActiveRecurringPayments();

    for(const auto& recurringPayment : activePayments){
        generatePaymentsForRecurringPayment(recurringPayment.getId(), targetDate);
    }
}

void PaymentService::generatePaymentsForRecurringPayment(const std::string& recurringPaymentId, const DateTime& targetDate)
{
    RecurringPayment* recurrinfgPayment = m_repo->findRecurringPayment(recurringPaymentId);
    if(!recurrinfgPayment){
        throw std::runtime_error("Recurring payment not found");
    }

    auto existingInstances = m_repo->getPaymentInstances(recurringPaymentId);
    auto newInstances = m_generator.generateUpTo(*recurrinfgPayment, targetDate, existingInstances);

    for(const auto& instance : newInstances){
        m_repo->addPaymentInstance(instance);
    }
}

std::unique_ptr<PaymentInstance> PaymentService::getNextPayment(const std::string& recurringPaymentId) const
{
    RecurringPayment* recurringPayment = m_repo->findRecurringPayment(recurringPaymentId);
    if(!recurringPayment){
        return nullptr;
    }

    auto now = std::chrono::system_clock::now();
    return m_generator.generateNext(*recurringPayment, now);
}

void PaymentService::updateOverduePayments()
{
    auto pendingPayments = m_repo->getPaymentInstancesByStatus(PaymentStatus::PENDING);
    auto now = std::chrono::system_clock::now();

    for(const auto& payment : pendingPayments){
        if(payment.isOverdue()){
            PaymentInstance* instance = m_repo->findPaymentInstance(payment.getName());
            if(instance && instance->getStatus() == PaymentStatus::PENDING){
                instance->markAsOverdue();
                m_repo->updatePaymentInstance(*instance);
            }
        }
    }
}
