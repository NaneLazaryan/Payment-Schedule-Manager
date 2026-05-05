#include "MonthlySchedule.h"
#include <sstream>
#include <stdexcept>
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

MonthlySchedule::MonthlySchedule(const DateTime& startDate, int interval)
    : RecurringSchedule(startDate), m_interval(interval)
{
    if (interval <= 0)
        throw std::invalid_argument("Interval must be positive");
}

MonthlySchedule::MonthlySchedule(const DateTime& startDate, int dayOfMonth, int interval)
    : RecurringSchedule(startDate), m_interval(interval), specificDay(dayOfMonth)
{
    if (interval <= 0 || dayOfMonth < 1 || dayOfMonth > 31)
        throw std::invalid_argument("Invalid interval or day of month");
}

MonthlySchedule::MonthlySchedule(const DateTime& startDate, const DateTime& endDate, int dayOfMonth, int interval)
    : RecurringSchedule(startDate, endDate), m_interval(interval), specificDay(dayOfMonth)
{
    if (interval <= 0 || dayOfMonth < 1 || dayOfMonth > 31)
        throw std::invalid_argument("Invalid interval or day of month");
}

void MonthlySchedule::setIntervalMonths(int months)
{
    if (months <= 0)
        throw std::invalid_argument("Interval must be positive");
    m_interval = months;
}

void MonthlySchedule::setSpecificDay(int day)
{
    if (day < 1 || day > 31)
        throw std::invalid_argument("Day of month must be between 1 and 31");
    specificDay = day;
}

void MonthlySchedule::clearSpecificDay()
{
    specificDay = std::nullopt;
}

DateTime MonthlySchedule::getNextPayment(const DateTime& fromDate) const
{
    if (!m_active) {
        throw std::runtime_error("Schedule is not active");
    }

    DateTime nextDate = (fromDate < m_start) ? m_start : addDays(fromDate, 1);
    nextDate = addMonths(nextDate, m_interval);

    if (specificDay.has_value()) {
        auto time = std::chrono::system_clock::to_time_t(nextDate);
#ifdef _WIN32
        std::tm tm;
        localtime_s(&tm, &time);
        std::tm* tmPtr = &tm;
#else
        std::tm* tmPtr = std::localtime(&time);
        if (!tmPtr) {
            throw std::runtime_error("Failed to convert time");
        }
        std::tm tm = *tmPtr;
        tmPtr = &tm;
#endif
        tmPtr->tm_mday = specificDay.value();
        nextDate = std::chrono::system_clock::from_time_t(std::mktime(tmPtr));
    }

    if (m_end.has_value() && nextDate > m_end.value()) {
        throw std::runtime_error("No more occurrences (end date reached)");
    }

    return nextDate;
}

std::unique_ptr<RecurringSchedule> MonthlySchedule::clone() const
{
    if (m_end.has_value() && specificDay.has_value())
        return std::make_unique<MonthlySchedule>(m_start, m_end.value(), specificDay.value(), m_interval);
    else if (specificDay.has_value())
        return std::make_unique<MonthlySchedule>(m_start, specificDay.value(), m_interval);

    return std::make_unique<MonthlySchedule>(m_start, m_interval);
}

bool MonthlySchedule::isValid() const
{
    if (!RecurringSchedule::isValid() || m_interval <= 0)
        return false;

    if (specificDay.has_value()) {
        int day = specificDay.value();
        if (day < 1 || day > 31) return false;
    }

    return true;
}

DateTime MonthlySchedule::addMonths(const DateTime& date, int months) const
{
    auto time = std::chrono::system_clock::to_time_t(date);
#ifdef _WIN32
    std::tm tm;
    localtime_s(&tm, &time);
    std::tm* tmPtr = &tm;
#else
    std::tm* tmPtr = std::localtime(&time);
    if (!tmPtr) {
        throw std::runtime_error("Failed to convert time");
    }
    std::tm tm = *tmPtr;
    tmPtr = &tm;
#endif

    tmPtr->tm_mon += months;
    while (tmPtr->tm_mon >= 12) {
        tmPtr->tm_mon -= 12;
        tmPtr->tm_year++;
    }

    while (tmPtr->tm_mon < 0) {
        tmPtr->tm_mon += 12;
        tmPtr->tm_year--;
    }

    int maxDay = 31;
    if (tmPtr->tm_mon == 1) { // February
        bool isLeap = (tmPtr->tm_year % 4 == 0 && tmPtr->tm_year % 100 != 0) || (tmPtr->tm_year % 400 == 0);
        maxDay = isLeap ? 29 : 28;
    }
    else if (tmPtr->tm_mon == 3 || tmPtr->tm_mon == 5 || tmPtr->tm_mon == 8 || tmPtr->tm_mon == 10) {
        maxDay = 30;
    }
    if (tmPtr->tm_mday > maxDay) {
        tmPtr->tm_mday = maxDay;
    }

    return std::chrono::system_clock::from_time_t(std::mktime(tmPtr));
}

std::string MonthlySchedule::getDescription() const {
    std::ostringstream oss;
    if (m_interval == 1) {
        oss << "Every month";
    }
    else {
        oss << "Every " << m_interval << " months";
    }

    if (specificDay.has_value()) {
        oss << " on day " << specificDay.value();
    }

    return oss.str();
}
