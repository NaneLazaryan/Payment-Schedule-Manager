#ifndef RECURRINGSCHEDULE_H
#define RECURRINGSCHEDULE_H

#include "DateTime.h"
#include <optional>
#include <chrono>
#include <ctime>
#include <vector>
#include <memory>
#include <string>

enum class DayOfWeek {
    SUNDAY = 0,
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY
};

class RecurringSchedule
{
public:
    RecurringSchedule(const DateTime&);
    RecurringSchedule(const DateTime&, const DateTime&);
    virtual ~RecurringSchedule() = default;

    // Getters
    DateTime getStartDate() const { return m_start; }
    std::optional<DateTime> getEndDate() const { return m_end; }
    bool isActive() const { return m_active; }

    // Setters
    void setEndDate(const DateTime&);
    void setActive(bool);
    void clearEndDate();

    virtual DateTime getNextPayment(const DateTime&) const = 0;
    virtual std::unique_ptr<RecurringSchedule> clone() const = 0;
    virtual std::string getDescription() const = 0;

    virtual bool isValid() const;
    virtual std::vector<DateTime> getUpcomingDates(int count) const;
    virtual std::vector<DateTime> getUpcomingDates(int count, const DateTime& fromDate) const;

protected:
    DateTime m_start;
    std::optional<DateTime> m_end;
    bool m_active;

    // Helper functions
    DateTime addDays(const DateTime&, int) const;
    bool isSameDay(const DateTime&, const DateTime&) const;
    int getDayOfMonth(const DateTime&) const;
    DayOfWeek getDayOfWeek(const DateTime&) const;
};

#endif // RECURRINGSCHEDULE_H
