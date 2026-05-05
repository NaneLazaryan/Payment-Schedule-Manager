#include "YearlySchedule.h"
#include "MonthlySchedule.h"
#include <algorithm>
#include <stdexcept>
#include <sstream>
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

YearlySchedule::YearlySchedule(const DateTime& startDate, int interval)
    : RecurringSchedule(startDate), m_interval(interval)
{
    if (interval <= 0)
        throw std::invalid_argument("Interval must be positive");

    auto time = std::chrono::system_clock::to_time_t(m_start);
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
    specificMonth = tmPtr->tm_mon + 1;  // tm_mon is 0-11, we want 1-12
    specificDayOfMonth = tmPtr->tm_mday;
}

YearlySchedule::YearlySchedule(const DateTime& startDate, int month, int dayOfMonth, int interval)
    : RecurringSchedule(startDate), m_interval(interval), specificMonth(month), specificDayOfMonth(dayOfMonth)
{
    if (interval <= 0)
        throw std::invalid_argument("Interval must be positive");

    if (month < 1 || month > 12)
        throw std::invalid_argument("Month must be between 1 and 12");

    if (dayOfMonth < 1 || dayOfMonth > 31)
        throw std::invalid_argument("Day of month must be between 1 and 31");
}

YearlySchedule::YearlySchedule(const DateTime& startDate, const DateTime& endDate, int month, int dayOfMonth, int interval)
    : RecurringSchedule(startDate, endDate), m_interval(interval), specificMonth(month), specificDayOfMonth(dayOfMonth)
{
    if (interval <= 0)
        throw std::invalid_argument("Interval must be positive");

    if (month < 1 || month > 12)
        throw std::invalid_argument("Month must be between 1 and 12");

    if (dayOfMonth < 1 || dayOfMonth > 31)
        throw std::invalid_argument("Day of month must be between 1 and 31");
}

void YearlySchedule::setSpecificMonth(int month)
{
    if (month < 1 || month > 12)
        throw std::invalid_argument("Month must be between 1 and 12");
    specificMonth = month;
}

void YearlySchedule::setSpecificDayOfMonth(int day)
{
    if (day < 1 || day > 31)
        throw std::invalid_argument("Day of month must be between 1 and 31");
    specificDayOfMonth = day;
}

void YearlySchedule::setIntervalYears(int interval)
{
    if (interval <= 0)
        throw std::invalid_argument("Interval must be positive");
    m_interval = interval;
}


DateTime YearlySchedule::addYears(const DateTime& date, int years) const
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

    tmPtr->tm_year += years;

    if (tmPtr->tm_mon == 1 && tmPtr->tm_mday == 29) {
        int targetYear = tmPtr->tm_year + 1900;
        bool isLeap = (targetYear % 4 == 0 && targetYear % 100 != 0) || (targetYear % 400 == 0);
        if (!isLeap) {
            tmPtr->tm_mday = 28;
        }
    }

    return std::chrono::system_clock::from_time_t(std::mktime(tmPtr));
}

DateTime YearlySchedule::getNextPayment(const DateTime& fromDate) const
{
    if (!m_active)
        throw std::runtime_error("Schedule is not active");

    DateTime candidate = (fromDate < m_start) ? m_start : fromDate;
    auto candidateTime = std::chrono::system_clock::to_time_t(candidate);
#ifdef _WIN32
    std::tm candidateTm;
    localtime_s(&candidateTm, &candidateTime);
    std::tm targetTm = candidateTm;
#else
    std::tm* candidateTmPtr = std::localtime(&candidateTime);
    if (!candidateTmPtr) {
        throw std::runtime_error("Failed to convert time");
    }
    std::tm targetTm = *candidateTmPtr;
#endif

    if (specificMonth.has_value()) {
        targetTm.tm_mon = specificMonth.value() - 1;  // tm_mon is 0-11, specificMonth is 1-12
    }
    if (specificDayOfMonth.has_value()) {
        targetTm.tm_mday = specificDayOfMonth.value();
    }

    int maxDayInMonth = 31;
    switch (targetTm.tm_mon) {
    case 1: { // February
        int year = targetTm.tm_year + 1900;
        bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        maxDayInMonth = isLeap ? 29 : 28;
        break;
    }
    case 3: case 5: case 8: case 10: // Apr, Jun, Sep, Nov
        maxDayInMonth = 30;
        break;
    default: // Jan, Mar, May, Jul, Aug, Oct, Dec
        maxDayInMonth = 31;
    }

    if (targetTm.tm_mday > maxDayInMonth) {
        targetTm.tm_mday = maxDayInMonth;
    }

    targetTm.tm_hour = 0;
    targetTm.tm_min = 0;
    targetTm.tm_sec = 0;
    targetTm.tm_isdst = -1;

    DateTime nextDate = std::chrono::system_clock::from_time_t(std::mktime(&targetTm));

    if (nextDate <= candidate) {
        nextDate = addYears(nextDate, m_interval);

        auto nextTime = std::chrono::system_clock::to_time_t(nextDate);
#ifdef _WIN32
        std::tm nextTm;
        localtime_s(&nextTm, &nextTime);
        std::tm* nextTmPtr = &nextTm;
#else
        std::tm* nextTmPtr = std::localtime(&nextTime);
        if (!nextTmPtr) {
            throw std::runtime_error("Failed to convert time");
        }
#endif

        if (specificMonth.has_value()) {
            nextTmPtr->tm_mon = specificMonth.value() - 1;
        }
        if (specificDayOfMonth.has_value()) {
            maxDayInMonth = 31;
            switch (nextTmPtr->tm_mon) {
            case 1: {
                int year = nextTmPtr->tm_year + 1900;
                bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
                maxDayInMonth = isLeap ? 29 : 28;
                break;
            }
            case 3: case 5: case 8: case 10:
                maxDayInMonth = 30;
                break;
            }
            nextTmPtr->tm_mday = std::min(specificDayOfMonth.value(), maxDayInMonth);
        }

        nextTmPtr->tm_hour = 0;
        nextTmPtr->tm_min = 0;
        nextTmPtr->tm_sec = 0;
        nextTmPtr->tm_isdst = -1;

        nextDate = std::chrono::system_clock::from_time_t(std::mktime(nextTmPtr));
    }

    if (m_end.has_value() && nextDate > m_end.value()) {
        throw std::runtime_error("No more occurrences (end date reached)");
    }

    if (nextDate < m_start) {
        nextDate = m_start;
    }

    return nextDate;
}

std::unique_ptr<RecurringSchedule> YearlySchedule::clone() const
{
    if (m_end.has_value() && specificMonth.has_value() && specificDayOfMonth.has_value())
        return std::make_unique<YearlySchedule>(m_start, m_end.value(), specificMonth.value(), specificDayOfMonth.value(), m_interval);
    else if (specificMonth.has_value() && specificDayOfMonth.has_value())
        return std::make_unique<YearlySchedule>(m_start, specificMonth.value(), specificDayOfMonth.value(), m_interval);

    return std::make_unique<YearlySchedule>(m_start, m_interval);
}

bool YearlySchedule::isValid() const
{
    if (!RecurringSchedule::isValid() || m_interval <= 0)
        return false;

    if (specificMonth.has_value()) {
        int month = specificMonth.value();
        if (month < 1 || month > 12) return false;
    }

    if (specificDayOfMonth.has_value()) {
        int day = specificDayOfMonth.value();
        if (day < 1 || day > 31) return false;

        if (specificMonth.has_value()) {
            int month = specificMonth.value();
            if (month == 2 && day > 29) return false;  // February max 29
            if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) {
                return false;  // Apr, Jun, Sep, Nov max 30
            }
        }
    }

    return true;
}

std::string YearlySchedule::getMonthName(int month) const {
    switch (month) {
    case 1: return "January";
    case 2: return "February";
    case 3: return "March";
    case 4: return "April";
    case 5: return "May";
    case 6: return "June";
    case 7: return "July";
    case 8: return "August";
    case 9: return "September";
    case 10: return "October";
    case 11: return "November";
    case 12: return "December";
    default: return "Unknown";
    }
}

std::string YearlySchedule::getDescription() const {
    std::ostringstream oss;

    if (m_interval == 1) {
        oss << "Every year";
    }
    else {
        oss << "Every " << m_interval << " years";
    }

    // Add month and day information if specified
    if (specificMonth.has_value() && specificDayOfMonth.has_value()) {
        oss << " on " << getMonthName(specificMonth.value())
        << " " << specificDayOfMonth.value();
    }
    else if (specificMonth.has_value()) {
        oss << " in " << getMonthName(specificMonth.value());
    }
    else if (specificDayOfMonth.has_value()) {
        oss << " on day " << specificDayOfMonth.value();
    }

    return oss.str();
}
