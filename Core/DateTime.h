#ifndef DATETIME_H
#define DATETIME_H

#include <chrono>
#include <optional>
#include <string>

using DateTime = std::chrono::system_clock::time_point;
using Days = std::chrono::days;

namespace DateTimeUtils {
    // Current time
    DateTime now();

    /*
     Create a DateTime from date components
     Month (1-12)
     Day of month (1-31)
     Year (e.g., 2024)
     Hour (0-23), default 0
     Minute (0-59), default 0
     Second (0-59), default 0
     */
    DateTime createDateTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0);
    std::string formatDateTime(const DateTime& date, const std::string& fromat = "%Y-%m-%d %H:%M:%S");

    bool isSameDay(const DateTime& date1, const DateTime& date2);
    DateTime startOfDay(const DateTime& date);
}

#endif // DATETIME_H
