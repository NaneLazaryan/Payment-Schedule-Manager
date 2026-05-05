#ifndef PAYMENTTYPES_H
#define PAYMENTTYPES_H

#include "DateTime.h"
#include <string>

enum class PaymentStatus
{
    PENDING,
    PAID,
    CANCELLED,
    FAILED,
    PROCESSING,
    OVERDUE // Past due date
};

enum class PaymentCategory
{
    RENT,
    SUBSCRIPTION,
    INSURANCE,
    ENTERTAINMENT,
    OTHER
};

enum class PaymentMethod {
    CASH,
    CREDIT_CARD,
    BANK_TRANSFER,
    DIGITAL_WALLET,
    CHECK,
    OTHER
};

namespace PaymentTypeUtils
{
    std::string statusToString(PaymentStatus status);
    std::string categoryToString(PaymentCategory category);
    std::string paymentMethodString(PaymentMethod method);
}


#endif // PAYMENTTYPES_H
