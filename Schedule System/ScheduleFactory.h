#ifndef SCHEDULEFACTORY_H
#define SCHEDULEFACTORY_H

#include "RecurringSchedule.h"
#include "DailySchedule.h"
#include "WeeklySchedule.h"
#include "MonthlySchedule.h"
#include "YearlySchedule.h"

#include <memory>

// ABSTRACT FACTORY

class ScheduleFactory
{
public:
    virtual ~ScheduleFactory() = default;
    virtual std::unique_ptr<RecurringSchedule> create(const DateTime&) const = 0;
    virtual std::unique_ptr<RecurringSchedule> create(const DateTime&, const DateTime&) const = 0;
};

class DailyScheduleFactory : public ScheduleFactory
{
public:
    explicit DailyScheduleFactory(int interval = 1) : intervalDays(interval) {}

    std::unique_ptr<RecurringSchedule> create(const DateTime& start) const
    {
        return std::make_unique<DailySchedule>(start, intervalDays);
    }

    std::unique_ptr<RecurringSchedule> create(const DateTime& start, const DateTime& end) const
    {
        return std::make_unique<DailySchedule>(start, end, intervalDays);
    }

private:
    int intervalDays;
};

class WeeklyScheduleFactory : public ScheduleFactory
{
public:
    WeeklyScheduleFactory(int interval = 1, std::optional<DayOfWeek> day = std::nullopt)
        : intervalWeeks(interval), specificDay(day) {}

    std::unique_ptr<RecurringSchedule> create(const DateTime& start) const
    {
        if (specificDay.has_value())
            return std::make_unique<WeeklySchedule>(start, specificDay.value(), intervalWeeks);
        return std::make_unique<WeeklySchedule>(start, intervalWeeks);
    }

    std::unique_ptr<RecurringSchedule> create(const DateTime& start, const DateTime& end) const
    {
        if (specificDay.has_value())
            return std::make_unique<WeeklySchedule>(start, end, specificDay.value(), intervalWeeks);

        auto schedule = std::make_unique<WeeklySchedule>(start, intervalWeeks);
        schedule->setEndDate(end);
        return schedule;
    }

private:
    int intervalWeeks;
    std::optional<DayOfWeek> specificDay;

};

class MonthlyScheduleFactory : public ScheduleFactory
{
public:
    MonthlyScheduleFactory(int interval = 1, std::optional<int> dayOfMonth = std::nullopt)
        :intervalMonths(interval), specificDay(dayOfMonth) {}

    std::unique_ptr<RecurringSchedule> create(const DateTime& start) const
    {
        if (specificDay.has_value())
            return std::make_unique<MonthlySchedule>(start, specificDay.value(), intervalMonths);
        return std::make_unique<MonthlySchedule>(start, intervalMonths);
    }

    std::unique_ptr<RecurringSchedule> create(const DateTime& start, const DateTime& end) const
    {
        if (specificDay.has_value())
            return std::make_unique<MonthlySchedule>(start, end, specificDay.value(), intervalMonths);

        auto schedule = std::make_unique<MonthlySchedule>(start, intervalMonths);
        schedule->setEndDate(end);
        return schedule;
    }
private:
    int intervalMonths;
    std::optional<int> specificDay;
};

class YearlyScheduleFactory : public ScheduleFactory
{
public:
    YearlyScheduleFactory(int interval = 1, std::optional<int> month = std::nullopt, std::optional<int> dayOfMonth = std::nullopt)
        : intervalYears(interval), specificMonth(month), specificDayOfMonth(dayOfMonth) {}

    std::unique_ptr<RecurringSchedule> create(const DateTime& start) const
    {
        if (specificMonth.has_value() && specificDayOfMonth.has_value())
            return std::make_unique<YearlySchedule>(start, specificMonth.value(), specificDayOfMonth.value(), intervalYears);
        return std::make_unique<YearlySchedule>(start, intervalYears);
    }

    std::unique_ptr<RecurringSchedule> create(const DateTime& start, const DateTime& end) const
    {
        if (specificMonth.has_value() && specificDayOfMonth.has_value())
            return std::make_unique<YearlySchedule>(start, specificMonth.value(), specificDayOfMonth.value(), intervalYears);

        auto schedule = std::make_unique<YearlySchedule>(start, intervalYears);
        schedule->setEndDate(end);
        return schedule;
    }

private:
    int intervalYears;
    std::optional<int> specificMonth;
    std::optional<int> specificDayOfMonth;
};


#endif // SCHEDULEFACTORY_H
