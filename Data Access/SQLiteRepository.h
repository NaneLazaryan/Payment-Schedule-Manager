#ifndef SQLITEREPOSITORY_H
#define SQLITEREPOSITORY_H

#include "IPaymentRepository.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <mutex>
#include <unordered_map>

class SQLiteRepository : public IPaymentRepository
{
public:
    explicit SQLiteRepository(const QString& dbPath = "payments.db");
    virtual ~SQLiteRepository();

    // Recurring Payment operations
    void addRecurringPayment(const RecurringPayment& recurringPayment);
    RecurringPayment* findRecurringPayment(const std::string& id);
    std::vector<RecurringPayment> getAllRecurringPayments() const;
    std::vector<RecurringPayment> getActiveRecurringPayments() const;
    void updateRecurringPayment(const RecurringPayment& recurringPayment);
    bool removeRecurringPayment(const std::string& id);
    bool recurringPaymentExists(const std::string& id) const;

    // Payment Instance operations
    void addPaymentInstance(const PaymentInstance& instance);
    PaymentInstance* findPaymentInstance(const std::string& id);
    std::vector<PaymentInstance> getPaymentInstances(const std::string& recurringPaymentId) const;
    std::vector<PaymentInstance> getAllPaymentInstances() const;
    std::vector<PaymentInstance> getPaymentInstancesByStatus(PaymentStatus status) const;
    std::vector<PaymentInstance> getPaymentInstancesDueBefore(const DateTime& date) const;
    void updatePaymentInstance(const PaymentInstance& instance);
    bool removePaymentInstance(const std::string& id);
    bool paymentInstanceExists(const std::string& id) const;

    // Utility operations
    void clearAll();
    size_t getRecurringPaymentCount() const;
    size_t getPaymentInstanceCount() const;

    bool initializeDatabase();
    bool isDatabaseOpen() const;

private:
    QSqlDatabase m_db;
    mutable std::mutex m_mtx;
    mutable std::unordered_map<std::string, RecurringPayment> m_recurringCache;
    mutable std::unordered_map<std::string, PaymentInstance> m_instanceCache;

    std::string serializeSchedule(const RecurringSchedule* schedule) const;
    std::unique_ptr<RecurringSchedule> deserializeSchedule(const QString& scheduleData) const;

    QString dateTimeToString(const DateTime& dt) const;
    DateTime stringToDateTime(const QString& str) const;

    void loadRecurringPaymentToCache(const std::string& id) const;
    void loadPaymentInstanceToCache(const std::string& id) const;
    void clearCache();
};

#endif // SQLITEREPOSITORY_H
