#ifndef DAILYSCHEDULE_H
#define DAILYSCHEDULE_H

#include "RecurringSchedule.h"

class DailySchedule : public RecurringSchedule
{
public:
    DailySchedule(const DateTime&, int interval = 1);
    DailySchedule(const DateTime&, const DateTime&, int interval = 1);

    ~DailySchedule() = default;

    // Getter
    int getIntervalDays() const
    {
        return m_interval;
    }

    // Setter
    void setIntervalDays(int);

    bool isValid() const;

    DateTime getNextPayment(const DateTime&) const;
    std::unique_ptr<RecurringSchedule> clone() const;
    std::string getDescription() const;

private:
    int m_interval;
};
#endif // DAILYSCHEDULE_H
