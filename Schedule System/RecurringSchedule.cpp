#include "RecurringSchedule.h"
#include "DateTime.h"
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

RecurringSchedule::RecurringSchedule(const DateTime& startDate)
    : m_start(startDate), m_active(true) {}

RecurringSchedule::RecurringSchedule(const DateTime& startDate, const DateTime& endDate)
    : m_start(startDate), m_end(endDate), m_active(true)
{
    if (endDate <= startDate)
        throw std::invalid_argument("End date must be after start date");
}

DateTime RecurringSchedule::addDays(const DateTime& date, int days) const
{
    return date + std::chrono::hours(24 * days);
}


void RecurringSchedule::setEndDate(const DateTime& endDate)
{
    if (endDate <= m_start)
        throw std::invalid_argument("End date must be after start date");

    m_end = endDate;
}

void RecurringSchedule::setActive(bool isActive)
{
    m_active = isActive;
}
void RecurringSchedule::clearEndDate()
{
    m_end = std::nullopt;
}

bool RecurringSchedule::isSameDay(const DateTime& d1, const DateTime& d2) const
{
    return DateTimeUtils::isSameDay(d1, d2);
}

int RecurringSchedule::getDayOfMonth(const DateTime& date) const
{
    auto time = std::chrono::system_clock::to_time_t(date);
#ifdef _WIN32
    std::tm tm;
    localtime_s(&tm, &time);
    std::tm* tmPtr = &tm;
#else
    std::tm* tmPtr = std::localtime(&time);
    if (!tmPtr) {
        return 1;  // Default fallback
    }
#endif
    return tmPtr->tm_mday;
}

DayOfWeek RecurringSchedule::getDayOfWeek(const DateTime& date) const
{
    auto time = std::chrono::system_clock::to_time_t(date);
#ifdef _WIN32
    std::tm tm;
    localtime_s(&tm, &time);
    std::tm* tmPtr = &tm;
#else
    std::tm* tmPtr = std::localtime(&time);
    if (!tmPtr) {
        return DayOfWeek::SUNDAY;  // Default fallback
    }
#endif
    // tm_wday is 0-6 (Sunday-Saturday), which matches our DayOfWeek enum
    return static_cast<DayOfWeek>(tmPtr->tm_wday);
}

bool RecurringSchedule::isValid() const
{
    if (m_end.has_value() && m_end.value() <= m_start)
        return false;
    return true;
}

std::vector<DateTime> RecurringSchedule::getUpcomingDates(int count) const
{
    return getUpcomingDates(count, std::chrono::system_clock::now());
}

std::vector<DateTime> RecurringSchedule::getUpcomingDates(int count, const DateTime& fromDate) const
{
    std::vector<DateTime> dates;
    DateTime current = fromDate;

    for (int i = 0; i < count; ++i) {
        try {
            current = getNextPayment(current);
            dates.push_back(current);
            current = addDays(current, 1);
        }
        catch (const std::runtime_error&) {
            break;
        }
    }
    return dates;
}
