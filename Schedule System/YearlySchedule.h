#ifndef YEARLYSCHEDULE_H
#define YEARLYSCHEDULE_H

#include "RecurringSchedule.h"

class YearlySchedule : public RecurringSchedule
{
public:
    YearlySchedule(const DateTime&, int intervale = 1);
    YearlySchedule(const DateTime&, int month, int dayOfMonth, int interval);
    YearlySchedule(const DateTime&, const DateTime&, int month, int dayOfMonth, int interval);

    // Setter
    void setSpecificMonth(int);
    void setSpecificDayOfMonth(int);
    void setIntervalYears(int);

    // Getter
    int getIntervalYears() const
    {
        return m_interval;
    }

    std::optional<int> getSpecificMonth() const
    {
        return specificMonth;
    }

    std::optional<int> getSpecificDayOfMonth() const
    {
        return specificDayOfMonth;
    }

    DateTime getNextPayment(const DateTime&) const;
    std::unique_ptr<RecurringSchedule> clone() const;
    std::string getDescription() const;

    bool isValid() const;

private:
    int m_interval;
    std::optional<int> specificMonth;		// 1-12
    std::optional<int> specificDayOfMonth;  // 1-31

private:
    DateTime addYears(const DateTime&, int) const;
    std::string getMonthName(int month) const;
};

#endif // YEARLYSCHEDULE_H
