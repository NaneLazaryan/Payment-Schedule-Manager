#include "DateTime.h"
#include <stdexcept>
#include <sstream>
#include <ctime>
#include <iomanip>

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

DateTime DateTimeUtils::now()
{
    return std::chrono::system_clock::now();
}

DateTime DateTimeUtils::createDateTime(int year, int month, int day, int hour, int minute, int second)
{
    std::tm tm{};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1; // [0-11]
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    tm.tm_isdst = -1;

    auto timeT = std::mktime(&tm);
    if(timeT == -1){
        throw std::invalid_argument("Invalid date/time components");
    }

    return std::chrono::system_clock::from_time_t(timeT);
}

std::string DateTimeUtils::formatDateTime(const DateTime& date, const std::string& format)
{
    auto timeT = std::chrono::system_clock::to_time_t(date);

#ifdef _WIN32
    std::tm tm;
    localtime_s(&tm, &timeT);
    std::tm* tmPtr = &tm;
#else
    std::tm* tmPtr = std::localtime(&timeT);
    if (!tmPtr) {
        return "Invalid date";
    }
#endif

    std::ostringstream oss;
    oss << std::put_time(tmPtr, format.c_str());
    return oss.str();
}


bool DateTimeUtils::isSameDay(const DateTime& date1, const DateTime& date2)
{
    auto timeT1 = std::chrono::system_clock::to_time_t(date1);
    auto timeT2 = std::chrono::system_clock::to_time_t(date2);

#ifdef _WIN32
    std::tm tm1, tm2;
    localtime_s(&tm1, &timeT1);
    localtime_s(&tm2, &timeT2);
    std::tm* tm1Ptr = &tm1;
    std::tm* tm2Ptr = &tm2;
#else
    std::tm* tm1Ptr = std::localtime(&timeT1);
    std::tm* tm2Ptr = std::localtime(&timeT2);
    if (!tm1Ptr || !tm2Ptr) {
        return false;
    }
#endif
    return tm1Ptr->tm_year == tm2Ptr->tm_year &&
           tm1Ptr->tm_mon == tm2Ptr->tm_mon &&
           tm1Ptr->tm_mday == tm2Ptr->tm_mday;
}

DateTime DateTimeUtils::startOfDay(const DateTime& date)
{
    auto timeT = std::chrono::system_clock::to_time_t(date);

#ifdef _WIN32
    std::tm tm;
    localtime_s(&tm, &timeT);
    std::tm* tmPtr = &tm;
#else
    std::tm* tmPtr = std::localtime(&timeT);
    if (!tmPtr) {
        return dt;
    }
#endif

    tmPtr->tm_hour = 0;
    tmPtr->tm_min = 0;
    tmPtr->tm_sec = 0;

    auto startTimeT = std::mktime(tmPtr);
    return std::chrono::system_clock::from_time_t(startTimeT);
}
