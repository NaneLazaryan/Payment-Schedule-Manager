#ifndef MONTHLYSCHEDULE_H
#define MONTHLYSCHEDULE_H

#include "RecurringSchedule.h"

class MonthlySchedule : public RecurringSchedule
{

public:
    MonthlySchedule(const DateTime&, int interval = 1);
    MonthlySchedule(const DateTime&, int dayOfMonth, int interval);
    MonthlySchedule(const DateTime&, const DateTime&, int dayOfMonth, int interval);

    // Setter
    void setIntervalMonths(int);
    void setSpecificDay(int);

    void clearSpecificDay();

    // Getter
    int getIntervalMonths() const
    {
        return m_interval;
    }

    std::optional<int> getSpecificDayOfMonth() const
    {
        return specificDay;
    }

    DateTime getNextPayment(const DateTime&) const;
    std::unique_ptr<RecurringSchedule> clone() const;
    std::string getDescription() const;

    bool isValid() const;

private:
    int m_interval;
    std::optional<int> specificDay;

    DateTime addMonths(const DateTime&, int) const;
};
#endif // MONTHLYSCHEDULE_H
