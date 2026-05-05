#include "PaymentTypes.h"

using namespace PaymentTypeUtils;

std::string PaymentTypeUtils::statusToString(PaymentStatus s)
{
    switch (s) {
    case PaymentStatus::PENDING: return "Pending";
    case PaymentStatus::PROCESSING: return "Processing";
    case PaymentStatus::PAID: return "Paied";
    case PaymentStatus::FAILED: return "Failed";
    case PaymentStatus::CANCELLED: return "Cancelled";
    case PaymentStatus::OVERDUE: return "Overdue";
    default: return "Unknown";
    }
}

std::string PaymentTypeUtils::categoryToString(PaymentCategory c)
{
    switch (c) {
    case PaymentCategory::RENT: return "Rent";
    case PaymentCategory::SUBSCRIPTION: return "Subscription";
    case PaymentCategory::INSURANCE: return "Insurance";
    case PaymentCategory::ENTERTAINMENT: return "Entertainment";
    case PaymentCategory::OTHER: return "Other";
    default: return "Unknown";
    }
}

std::string PaymentTypeUtils::paymentMethodString(PaymentMethod method)
{
    switch (method) {
    case PaymentMethod::CASH: return "Cash";
    case PaymentMethod::CREDIT_CARD: return "Credit Card";
    case PaymentMethod::BANK_TRANSFER: return "Bank Transfer";
    case PaymentMethod::DIGITAL_WALLET: return "Digital Wallet";
    case PaymentMethod::CHECK: return "Check";
    case PaymentMethod::OTHER: return "Other";
    default: return "Unknown";
    }
}

