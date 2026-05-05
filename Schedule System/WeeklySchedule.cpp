#include "WeeklySchedule.h"
#include <sstream>
#include <stdexcept>
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

WeeklySchedule::WeeklySchedule(const DateTime& startDate, int interval)
    : RecurringSchedule(startDate), m_interval(interval)
{
    if (interval <= 0)
        throw std::invalid_argument("Interval must be positive");
}

WeeklySchedule::WeeklySchedule(const DateTime& startDate, DayOfWeek day, int interval)
    : RecurringSchedule(startDate), specificDay(day), m_interval(interval)
{
    if (interval <= 0)
        throw std::invalid_argument("Interval must be positive");
}

WeeklySchedule::WeeklySchedule(const DateTime& startDate, const DateTime& endDate, DayOfWeek day, int interval)
    : RecurringSchedule(startDate, endDate), m_interval(interval), specificDay(day)
{
    if (interval <= 0)
        throw std::invalid_argument("Interval must be positive");
}

void WeeklySchedule::setIntervalWeeks(int weeks)
{
    if (weeks <= 0)
        throw std::invalid_argument("Interval must be positive");
    m_interval = weeks;
}

void WeeklySchedule::setSpecificDay(DayOfWeek day)
{
    specificDay = day;
}


void WeeklySchedule::clearSpecificDay()
{
    specificDay = std::nullopt;
}


DateTime WeeklySchedule::getNextPayment(const DateTime& fromDate) const
{
    if (!m_active)
        throw std::runtime_error("Schedule is not active");

    DateTime nextDate = (fromDate < m_start) ? m_start : addDays(fromDate, 1);

    if (specificDay.has_value()) {
        DayOfWeek targetDay = specificDay.value();

        while (getDayOfWeek(nextDate) != targetDay) {
            nextDate = addDays(nextDate, 1);
        }

        auto weeksSinceStart = std::chrono::duration_cast<std::chrono::hours>(nextDate - m_start).count() / (24 * 7);

        if (weeksSinceStart % m_interval != 0) {
            int weeksToAdd = m_interval - (weeksSinceStart % m_interval);
            nextDate = addDays(nextDate, weeksToAdd * 7);
        }
    }
    else {
        nextDate = addDays(nextDate, m_interval * 7);
    }

    if (m_end.has_value() && nextDate > m_end.value())
        throw std::runtime_error("No more occurrences (end date reached)");

    return nextDate;
}

std::unique_ptr<RecurringSchedule> WeeklySchedule::clone() const
{
    if (m_end.has_value() && specificDay.has_value())
        return std::make_unique<WeeklySchedule>(m_start, m_end.value(), specificDay.value(), m_interval);
    else if (specificDay.has_value())
        return std::make_unique<WeeklySchedule>(m_start, specificDay.value(), m_interval);

    return std::make_unique<WeeklySchedule>(m_start, m_interval);

}

bool WeeklySchedule::isValid() const
{
    return RecurringSchedule::isValid() && m_interval > 0;
}

std::string WeeklySchedule::dayOfWeekToString(DayOfWeek day) const
{
    switch (day) {
    case DayOfWeek::SUNDAY: return "Sunday";
    case DayOfWeek::MONDAY: return "Monday";
    case DayOfWeek::TUESDAY: return "Tuesday";
    case DayOfWeek::WEDNESDAY: return "Wednesday";
    case DayOfWeek::THURSDAY: return "Thursday";
    case DayOfWeek::FRIDAY: return "Friday";
    case DayOfWeek::SATURDAY: return "Saturday";
    default: return "Unknown";
    }
}

std::string WeeklySchedule::getDescription() const {
    std::ostringstream oss;
    if (m_interval == 1) {
        oss << "Every week";
    }
    else {
        oss << "Every " << m_interval << " weeks";
    }

    if (specificDay.has_value()) {
        oss << " on " << dayOfWeekToString(specificDay.value());
    }

    return oss.str();
}
