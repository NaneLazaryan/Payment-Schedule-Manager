#include "DailySchedule.h"
#include <sstream>
#include <stdexcept>
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

DailySchedule::DailySchedule(const DateTime& startDate, int interval)
    : RecurringSchedule(startDate), m_interval(interval)
{
    if (interval <= 0)
        throw std::invalid_argument("Interval must be positive");
}


DailySchedule::DailySchedule(const DateTime& startDate, const DateTime& endDate, int interval)
    : RecurringSchedule(startDate, endDate), m_interval(interval)
{
    if (interval <= 0)
        throw std::invalid_argument("Interval must be positive");
}

void DailySchedule::setIntervalDays(int days)
{
    if (days <= 0)
        throw std::invalid_argument("Interval must be positive");
    m_interval = days;
}

bool DailySchedule::isValid() const
{
    return RecurringSchedule::isValid() && m_interval > 0;
}

DateTime DailySchedule::getNextPayment(const DateTime& fromDate) const
{
    if (!m_active)
        throw std::runtime_error("Schedule is not active");

    DateTime nextDate = (fromDate < m_start) ? m_start : fromDate;

    auto diff = std::chrono::duration_cast<std::chrono::hours>(nextDate - m_start).count() / 24;

    long long daysSinceStart = diff;
    long long remainder = daysSinceStart % m_interval;

    if (remainder == 0 && nextDate > m_start) {
        nextDate = addDays(nextDate, m_interval);
    }
    else if (remainder > 0) {
        nextDate = addDays(nextDate, m_interval - remainder);
    }

    if (m_end.has_value() && nextDate > m_end.value()) {
        throw std::runtime_error("No more occurrences (end date reached)");
    }

    return nextDate;
}

std::unique_ptr<RecurringSchedule> DailySchedule::clone() const
{
    if (m_end.has_value()) {
        return std::make_unique<DailySchedule>(m_start, m_end.value(), m_interval);
    }
    return std::make_unique<DailySchedule>(m_start, m_interval);
}

std::string DailySchedule::getDescription() const {
    std::ostringstream oss;
    if (m_interval == 1) {
        oss << "Every day";
    }
    else {
        oss << "Every " << m_interval << " days";
    }
    return oss.str();
}
