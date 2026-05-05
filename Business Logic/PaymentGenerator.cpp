#include "PaymentGenerator.h"
#include "DateTime.h"
#include <sstream>
#include <chrono>
#include <algorithm>

std::unique_ptr<PaymentInstance> PaymentGenerator::generateNext(const RecurringPayment& recurringPayment, const DateTime& fromDate) const
{
    return recurringPayment.generateNextInstance(fromDate);
}

std::vector<PaymentInstance> PaymentGenerator::generateInRange(const RecurringPayment& recurringPayment, const DateTime& fromDate, const DateTime& toDate) const
{
    return recurringPayment.generateInstances(fromDate, toDate);
}

bool PaymentGenerator::instanceExistsForDate(const std::vector<PaymentInstance>& existingInstances, const DateTime& dueDate) const
{
    return std::any_of(existingInstances.begin(), existingInstances.end(), [&dueDate](const PaymentInstance& instance){
        return DateTimeUtils::isSameDay(instance.getDueDate(), dueDate);
    });
}

std::string PaymentGenerator::generateInstanceId(const std::string& recurringPaymentId, int instanceNumber) const
{
    std::ostringstream os;
    os << recurringPaymentId << "_inst_" << instanceNumber;
    return os.str();
}


std::vector<PaymentInstance> PaymentGenerator::generateUpTo(const RecurringPayment& recurringPayment,const DateTime& targetDate,const std::vector<PaymentInstance>& existingInstances) const
{
    std::vector<PaymentInstance> newInstances;

    if(!recurringPayment.isActive()){
        return newInstances;
    }

    DateTime startDate = recurringPayment.getStartDate();
    if(!existingInstances.empty()){
        auto latestIt = std::max_element(
            existingInstances.begin(),
            existingInstances.end(),
            [](const PaymentInstance& a, const PaymentInstance& b) {
                return a.getDueDate() < b.getDueDate();
        });
        if(latestIt != existingInstances.end()){
            // Start from the day after latest instance
            startDate = latestIt->getDueDate();
            startDate += std::chrono::hours(24);
        }
    }

    DateTime currentDate = startDate;
    int instanceNumber = static_cast<int>(existingInstances.size());
    
    // If start date is today or in the past and no instances exist yet, include it as first payment
    DateTime scheduleStart = recurringPayment.getStartDate();
    auto now = DateTimeUtils::now();
    bool startDateAdded = false;
    if (existingInstances.empty() && scheduleStart <= now && scheduleStart <= targetDate && !instanceExistsForDate(existingInstances, scheduleStart)) {
        std::string instanceId = generateInstanceId(recurringPayment.getId(), ++instanceNumber);
        newInstances.emplace_back(instanceId,
                                  recurringPayment.getId(),
                                  recurringPayment.getAmount(),
                                  scheduleStart,
                                  recurringPayment.getCategory(),
                                  recurringPayment.getBankAccountNumber());
        startDateAdded = true;
    }

    while(currentDate <= targetDate){
        try{
            DateTime nextDueDate = recurringPayment.getSchedule()->getNextPayment(currentDate);

            if(nextDueDate > targetDate) break;
            
            // Skip start date if we already added it
            if (startDateAdded && DateTimeUtils::isSameDay(nextDueDate, scheduleStart)) {
                currentDate = nextDueDate;
                currentDate += std::chrono::hours(24);
                continue;
            }

            if(!instanceExistsForDate(existingInstances, nextDueDate)){
                std::string instanceId = generateInstanceId(recurringPayment.getId(), ++instanceNumber);

                newInstances.emplace_back(instanceId,
                                          recurringPayment.getId(),
                                          recurringPayment.getAmount(),
                                          nextDueDate,
                                          recurringPayment.getCategory(),
                                          recurringPayment.getBankAccountNumber());
            }

            currentDate = nextDueDate;
            currentDate += std::chrono::hours(24);
        }
        catch (const std::runtime_error&) {
            // No more occurrences
            break;
        }
    }

    return newInstances;
}
