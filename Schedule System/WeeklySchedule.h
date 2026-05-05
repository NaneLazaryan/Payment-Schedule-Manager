#ifndef WEEKLYSCHEDULE_H
#define WEEKLYSCHEDULE_H

#include "RecurringSchedule.h"

class WeeklySchedule : public RecurringSchedule
{
public:
    WeeklySchedule(const DateTime&, int interval = 1);
    WeeklySchedule(const DateTime&, DayOfWeek, int);
    WeeklySchedule(const DateTime&, const DateTime&, DayOfWeek, int);

    ~WeeklySchedule() = default;

    // Getters
    int getIntervalWeeks() const
    {
        return m_interval;
    }

    std::optional<DayOfWeek> getSpecificDay() const
    {
        return specificDay;
    }

    // Setters
    void setIntervalWeeks(int);
    void setSpecificDay(DayOfWeek);

    void clearSpecificDay();
    bool isValid() const;

    DateTime getNextPayment(const DateTime&) const;
    std::unique_ptr<RecurringSchedule> clone() const;
    std::string getDescription() const;

    std::string dayOfWeekToString(DayOfWeek) const;

private:
    int m_interval;
    std::optional<DayOfWeek> specificDay;
};

#endif // WEEKLYSCHEDULE_H
