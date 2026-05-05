#include "PaymentInstance.h"
#include <sstream>
#include <chrono>
#include <stdexcept>

PaymentInstance::PaymentInstance(const std::string& name,
                                 const std::string& recurringPaymentId,
                                 double amount,
                                 DateTime dueDate,
                                 PaymentCategory category,
                                 const std::string& bankAccountNumber)
    : m_name(name),
    m_recurringPaymentId(recurringPaymentId),
    m_amount(amount),
    m_dueDate(dueDate),
    m_category(category),
    m_bankAccountNumber(bankAccountNumber),
    m_status(PaymentStatus::PENDING)
{
    if (amount <= 0) {
        throw std::invalid_argument("Payment amount must be positive");
    }
    if (name.empty()) {
        throw std::invalid_argument("Payment instance ID cannot be empty");
    }
    if (recurringPaymentId.empty()) {
        throw std::invalid_argument("Recurring payment ID cannot be empty");
    }
}

void PaymentInstance::setAmount(double amount)
{
    if (amount <= 0) {
        throw std::invalid_argument("Payment amount must be positive");
    }
    m_amount = amount;
}

void PaymentInstance::markAsPaid(PaymentMethod method) {
    if (m_status == PaymentStatus::CANCELLED) {
        throw std::runtime_error("Cannot mark cancelled payment as paid");
    }

    m_status = PaymentStatus::PAID;
    m_paidDate = std::chrono::system_clock::now();
    m_payMethod = method;
}

void PaymentInstance::markAsFailed(const std::string& reason) {
    m_status = PaymentStatus::FAILED;
    if (!reason.empty()) {
        m_failureReason = reason;
    }
}

void PaymentInstance::markAsOverdue() {
    if (m_status == PaymentStatus::PENDING) {
        m_status = PaymentStatus::OVERDUE;
    }
}

void PaymentInstance::processPayment() {
    if (m_status != PaymentStatus::PENDING && m_status != PaymentStatus::OVERDUE) {
        throw std::runtime_error("Can only process pending or overdue payments");
    }

    m_status = PaymentStatus::PROCESSING;
}

void PaymentInstance::cancelPayment() {
    if (m_status == PaymentStatus::PAID) {
        throw std::runtime_error("Cannot cancel a paid payment");
    }

    m_status = PaymentStatus::CANCELLED;
}

bool PaymentInstance::isOverdue() const {
    if (m_status == PaymentStatus::OVERDUE) {
        return true;
    }

    if (m_status == PaymentStatus::PENDING) {
        auto now = DateTimeUtils::now(); //std::chrono::system_clock::now();
        return m_dueDate < now;
    }

    return false;
}

bool PaymentInstance::isDueToday() const {
    return DateTimeUtils::isSameDay(m_dueDate, DateTimeUtils::now());
}

bool PaymentInstance::isDueBefore(const DateTime& date) const {
    return m_dueDate < date;
}

std::string PaymentInstance::toString() const {
    std::ostringstream os;
    os << "Payment Instance [" << m_name << "]\n"
       << "  Recurring Payment ID: " << m_recurringPaymentId << "\n"
       << "  Amount: $" << m_amount << "\n"
       << "  Due Date: " << DateTimeUtils::formatDateTime(m_dueDate) << "\n"
       << "  Status: " << PaymentTypeUtils::statusToString(m_status) << "\n"
       << "  Bank Account: " << (m_bankAccountNumber.empty() ? "N/A" : m_bankAccountNumber) << "\n"
       << "  Category: " << PaymentTypeUtils::categoryToString(m_category);

    if (m_paidDate.has_value()) {
        os << "\n  Paid Date: " << DateTimeUtils::formatDateTime(m_paidDate.value());
    }

    if (m_payMethod.has_value()) {
        os << "\n  Payment Method: " << PaymentTypeUtils::paymentMethodString(m_payMethod.value());
    }

    if (m_failureReason.has_value()) {
        os << "\n  Failure Reason: " << m_failureReason.value();
    }

    return os.str();
}
