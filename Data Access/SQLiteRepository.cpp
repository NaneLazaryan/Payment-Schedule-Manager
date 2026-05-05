#include "SQLiteRepository.h"
#include "DailySchedule.h"
#include "WeeklySchedule.h"
#include "MonthlySchedule.h"
#include "YearlySchedule.h"
#include <QVariant>
#include <QSqlRecord>
#include <sstream>
#include <QStringList>
#include <iomanip>

SQLiteRepository::SQLiteRepository(const QString& dbPath)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if(!m_db.open()){
        qWarning() <<"Failed to open databes: "<<m_db.lastError().text();
        throw std::runtime_error("Failed to open database: " + m_db.lastError().text().toStdString());
    }

    if(!initializeDatabase()){
        throw std::runtime_error("Failed to initialize database schema");
    }
}

SQLiteRepository::~SQLiteRepository()
{
    clearCache();
    if(m_db.isOpen()){
        m_db.close();
    }
}

bool SQLiteRepository::initializeDatabase()
{
    QSqlQuery query(m_db);

    if(!query.exec(R"(
        CREATE TABLE IF NOT EXISTS recurring_payments (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            amount REAL NOT NULL,
            bank_account TEXT NOT NULL,
            category INTEGER NOT NULL,
            schedule_type TEXT NOT NULL,
            schedule_data TEXT NOT NULL,
            start_date TEXT NOT NULL,
            end_date TEXT,
            is_active INTEGER NOT NULL DEFAULT 1,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            updated_at TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )")) {
        qWarning() << "Failed to create recurring_payments table:" << query.lastError().text();
        return false;
    }

    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS payment_instances (
            id TEXT PRIMARY KEY,
            recurring_payment_id TEXT NOT NULL,
            amount REAL NOT NULL,
            due_date TEXT NOT NULL,
            paid_date TEXT,
            bank_account TEXT,
            category INTEGER NOT NULL,
            payment_method INTEGER,
            status INTEGER NOT NULL,
            failure_reason TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            updated_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (recurring_payment_id) REFERENCES recurring_payments(id) ON DELETE CASCADE
        )
    )")) {
        qWarning() << "Failed to create payment_instances table:" << query.lastError().text();
        return false;
    }

    // Backfill columns if database already existed without the new fields
    query.exec("ALTER TABLE recurring_payments ADD COLUMN bank_account TEXT");
    query.exec("ALTER TABLE payment_instances ADD COLUMN bank_account TEXT");

    query.exec("CREATE INDEX IF NOT EXISTS idx_recurring_active ON recurring_payments(is_active)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_instance_recurring ON payment_instances(recurring_payment_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_instance_status ON payment_instances(status)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_instance_due_date ON payment_instances(due_date)");

    return true;
}

bool SQLiteRepository::isDatabaseOpen() const
{
    return m_db.isOpen();
}

QString SQLiteRepository::dateTimeToString(const DateTime &dt) const
{
    auto time = std::chrono::system_clock::to_time_t(dt);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return QString::fromStdString(oss.str());
}

DateTime SQLiteRepository::stringToDateTime(const QString& str) const
{
    if (str.isEmpty()) {
        return std::chrono::system_clock::now();
    }

    std::tm tm = {};
    std::istringstream ss(str.toStdString());
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    if (ss.fail()) {
        return std::chrono::system_clock::now();
    }

    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::string SQLiteRepository::serializeSchedule(const RecurringSchedule* schedule) const
{
    if(!schedule) return "";

    std::ostringstream oss;

    if(auto daily = dynamic_cast<const DailySchedule*>(schedule)){
        oss<<"DAILY|"<<daily->getIntervalDays();
    }
    else if(auto weekly = dynamic_cast<const WeeklySchedule*>(schedule)){
        oss<<"WEEKLY|"<<weekly->getIntervalWeeks();
        if(weekly->getSpecificDay().has_value()){
            oss << "|" << static_cast<int>(weekly->getSpecificDay().value());
        }
    }
    else if (auto monthly = dynamic_cast<const MonthlySchedule*>(schedule)) {
        oss << "MONTHLY|" << monthly->getIntervalMonths();
        if (monthly->getSpecificDayOfMonth().has_value()) {
            oss << "|" << monthly->getSpecificDayOfMonth().value();
        }
    }
    else if (auto yearly = dynamic_cast<const YearlySchedule*>(schedule)) {
        oss << "YEARLY|" << yearly->getIntervalYears();
        if (yearly->getSpecificMonth().has_value() && yearly->getSpecificDayOfMonth().has_value()) {
            oss << "|" << yearly->getSpecificMonth().value() << "|" << yearly->getSpecificDayOfMonth().value();
        }
    }

    return oss.str();
}

std::unique_ptr<RecurringSchedule> SQLiteRepository::deserializeSchedule(const QString& scheduleData) const
{
    QStringList parts = scheduleData.split('|');
    if(parts.isEmpty()) return nullptr;

    QString type = parts[0];
    DateTime start = DateTimeUtils::now();
    std::optional<DateTime> end = std::nullopt;

    try{
        if(type == "DAILY" && parts.size() >= 2){
            int interval = parts[1].toInt();
            return std::make_unique<DailySchedule>(start, interval);
        }
        else if (type == "WEEKLY" && parts.size() >= 2) {
            int interval = parts[1].toInt();
            if (parts.size() >= 3) {
                DayOfWeek day = static_cast<DayOfWeek>(parts[2].toInt());
                return std::make_unique<WeeklySchedule>(start, day, interval);
            }
            return std::make_unique<WeeklySchedule>(start, interval);
        }
        else if (type == "MONTHLY" && parts.size() >= 2) {
            int interval = parts[1].toInt();
            if (parts.size() >= 3) {
                int dayOfMonth = parts[2].toInt();
                return std::make_unique<MonthlySchedule>(start, dayOfMonth, interval);
            }
            return std::make_unique<MonthlySchedule>(start, interval);
        }
        else if (type == "YEARLY" && parts.size() >= 2) {
            int interval = parts[1].toInt();
            if (parts.size() >= 4) {
                int month = parts[2].toInt();
                int day = parts[3].toInt();
                return std::make_unique<YearlySchedule>(start, month, day, interval);
            }
            return std::make_unique<YearlySchedule>(start, interval);
        }
    }
    catch (const std::exception& e) {
        qWarning() << "Failed to deserialize schedule:" << e.what();
    }

    return nullptr;
}

void SQLiteRepository::addRecurringPayment(const RecurringPayment& recurringPayment)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    if (recurringPaymentExists(recurringPayment.getId())) {
        throw std::invalid_argument("Recurring payment with ID '" + recurringPayment.getId() + "' already exists");
    }

    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO recurring_payments
        (id, name, amount, bank_account, category, schedule_type, schedule_data, start_date, end_date, is_active)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    query.addBindValue(QString::fromStdString(recurringPayment.getId()));
    query.addBindValue(QString::fromStdString(recurringPayment.getName()));
    query.addBindValue(recurringPayment.getAmount());
    query.addBindValue(QString::fromStdString(recurringPayment.getBankAccountNumber()));
    query.addBindValue(static_cast<int>(recurringPayment.getCategory()));

    // Serialize schedule
    std::string scheduleData = serializeSchedule(nullptr);
    QStringList parts = QString::fromStdString(scheduleData).split('|');
    QString scheduleType = parts.isEmpty() ? "DAILY" : parts[0];

    // Get schedule info from description
    std::string desc = recurringPayment.getScheduleDescription();
    if (desc.find("day") != std::string::npos) scheduleType = "DAILY";
    else if (desc.find("week") != std::string::npos) scheduleType = "WEEKLY";
    else if (desc.find("month") != std::string::npos) scheduleType = "MONTHLY";
    else if (desc.find("year") != std::string::npos) scheduleType = "YEARLY";

    query.addBindValue(scheduleType);
    query.addBindValue(QString::fromStdString(desc));
    query.addBindValue(dateTimeToString(recurringPayment.getStartDate()));
    query.addBindValue(recurringPayment.getEndDate().has_value() ? dateTimeToString(recurringPayment.getEndDate().value()) : QVariant());
    query.addBindValue(recurringPayment.isActive() ? 1 : 0);

    if (!query.exec()) {
        throw std::runtime_error("Failed to add recurring payment: " + query.lastError().text().toStdString());
    }

    m_recurringCache.emplace(recurringPayment.getId(), std::move(recurringPayment));
}

RecurringPayment* SQLiteRepository::findRecurringPayment(const std::string& id)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    // Check cache first
    auto it = m_recurringCache.find(id);
    if (it != m_recurringCache.end()) {
        return &it->second;
    }

    // Load from database
    loadRecurringPaymentToCache(id);

    it = m_recurringCache.find(id);
    return (it != m_recurringCache.end()) ? &it->second : nullptr;
}

void SQLiteRepository::loadRecurringPaymentToCache(const std::string& id) const
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM recurring_payments WHERE id = ?");
    query.addBindValue(QString::fromStdString(id));

    if (!query.exec() || !query.next()) {
        return;
    }

    try {
        std::string paymentId = query.value("id").toString().toStdString();
        std::string name = query.value("name").toString().toStdString();
        double amount = query.value("amount").toDouble();
        PaymentCategory category = static_cast<PaymentCategory>(query.value("category").toInt());
        DateTime startDate = stringToDateTime(query.value("start_date").toString());
        std::string bankAccount = "N/A";
        int bankIndex = query.record().indexOf("bank_account");
        if (bankIndex != -1) {
            bankAccount = query.value(bankIndex).toString().toStdString();
        }

        // Create a simple daily schedule as placeholder
        auto schedule = std::make_unique<DailySchedule>(startDate, 1);

        if (!query.value("end_date").isNull()) {
            DateTime endDate = stringToDateTime(query.value("end_date").toString());
            schedule->setEndDate(endDate);
        }

        bool isActive = query.value("is_active").toInt() == 1;
        schedule->setActive(isActive);

        RecurringPayment payment(paymentId, name, amount, category, std::move(schedule), bankAccount.empty() ? "N/A" : bankAccount);
        payment.setActive(isActive);

        m_recurringCache.emplace(paymentId, std::move(payment));
    }
    catch (const std::exception& e) {
        qWarning() << "Failed to load recurring payment from cache:" << e.what();
    }
}

std::vector<RecurringPayment> SQLiteRepository::getAllRecurringPayments() const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    std::vector<RecurringPayment> payments;
    QSqlQuery query(m_db);

    if (!query.exec("SELECT * FROM recurring_payments ORDER BY created_at DESC")) {
        qWarning() << "Failed to get all recurring payments:" << query.lastError().text();
        return payments;
    }

    while (query.next()) {
        std::string id = query.value("id").toString().toStdString();

        auto it = m_recurringCache.find(id);
        if (it == m_recurringCache.end()) {
            loadRecurringPaymentToCache(id);
            it = m_recurringCache.find(id);
        }

        if (it != m_recurringCache.end()) {
            payments.push_back(it->second);
        }
    }

    return payments;
}

std::vector<RecurringPayment> SQLiteRepository::getActiveRecurringPayments() const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    std::vector<RecurringPayment> payments;
    QSqlQuery query(m_db);

    if (!query.exec("SELECT * FROM recurring_payments WHERE is_active = 1 ORDER BY created_at DESC")) {
        qWarning() << "Failed to get active recurring payments:" << query.lastError().text();
        return payments;
    }

    while (query.next()) {
        std::string id = query.value("id").toString().toStdString();

        auto it = m_recurringCache.find(id);
        if (it == m_recurringCache.end()) {
            loadRecurringPaymentToCache(id);
            it = m_recurringCache.find(id);
        }

        if (it != m_recurringCache.end()) {
            payments.push_back(it->second);
        }
    }

    return payments;
}

void SQLiteRepository::updateRecurringPayment(const RecurringPayment& recurringPayment)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE recurring_payments
        SET name = ?, amount = ?, bank_account = ?, category = ?, is_active = ?, updated_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )");

    query.addBindValue(QString::fromStdString(recurringPayment.getName()));
    query.addBindValue(recurringPayment.getAmount());
    query.addBindValue(QString::fromStdString(recurringPayment.getBankAccountNumber()));
    query.addBindValue(static_cast<int>(recurringPayment.getCategory()));
    query.addBindValue(recurringPayment.isActive() ? 1 : 0);
    query.addBindValue(QString::fromStdString(recurringPayment.getId()));

    if (!query.exec()) {
        throw std::runtime_error("Failed to update recurring payment: " + query.lastError().text().toStdString());
    }

    m_recurringCache.emplace(recurringPayment.getId(), recurringPayment);
}

bool SQLiteRepository::removeRecurringPayment(const std::string& id)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM recurring_payments WHERE id = ?");
    query.addBindValue(QString::fromStdString(id));

    bool success = query.exec() && query.numRowsAffected() > 0;

    if (success) {
        m_recurringCache.erase(id);
    }

    return success;
}

bool SQLiteRepository::recurringPaymentExists(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM recurring_payments WHERE id = ?");
    query.addBindValue(QString::fromStdString(id));

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

void SQLiteRepository::addPaymentInstance(const PaymentInstance& instance)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    if (paymentInstanceExists(instance.getName())) {
        throw std::invalid_argument("Payment instance with ID '" + instance.getName() + "' already exists");
    }

    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO payment_instances
        (id, recurring_payment_id, amount, due_date, paid_date, bank_account, category, payment_method, status, failure_reason)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    query.addBindValue(QString::fromStdString(instance.getName()));
    query.addBindValue(QString::fromStdString(instance.getRecurringPaymentId()));
    query.addBindValue(instance.getAmount());
    query.addBindValue(dateTimeToString(instance.getDueDate()));
    query.addBindValue(instance.getPaidDate().has_value() ? dateTimeToString(instance.getPaidDate().value()) : QVariant());
    query.addBindValue(QString::fromStdString(instance.getBankAccountNumber()));
    query.addBindValue(static_cast<int>(instance.getCategory()));
    query.addBindValue(instance.getPaymentMethod().has_value() ? static_cast<int>(instance.getPaymentMethod().value()) : QVariant());
    query.addBindValue(static_cast<int>(instance.getStatus()));
    query.addBindValue(instance.getFailureReason().has_value() ? QString::fromStdString(instance.getFailureReason().value()) : QVariant());

    if (!query.exec()) {
        throw std::runtime_error("Failed to add payment instance: " + query.lastError().text().toStdString());
    }

    m_instanceCache.emplace(instance.getName(), instance);
}

PaymentInstance* SQLiteRepository::findPaymentInstance(const std::string& id)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    auto it = m_instanceCache.find(id);
    if (it != m_instanceCache.end()) {
        return &it->second;
    }

    loadPaymentInstanceToCache(id);

    it = m_instanceCache.find(id);
    return (it != m_instanceCache.end()) ? &it->second : nullptr;
}

void SQLiteRepository::loadPaymentInstanceToCache(const std::string& id) const
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM payment_instances WHERE id = ?");
    query.addBindValue(QString::fromStdString(id));

    if (!query.exec() || !query.next()) {
        return;
    }

    try {
        std::string instanceId = query.value("id").toString().toStdString();
        std::string recurringId = query.value("recurring_payment_id").toString().toStdString();
        double amount = query.value("amount").toDouble();
        DateTime dueDate = stringToDateTime(query.value("due_date").toString());
        PaymentCategory category = static_cast<PaymentCategory>(query.value("category").toInt());
        std::string bankAccount = "";
        int bankIndex = query.record().indexOf("bank_account");
        if (bankIndex != -1) {
            bankAccount = query.value(bankIndex).toString().toStdString();
        }

        PaymentInstance instance(instanceId, recurringId, amount, dueDate, category, bankAccount);

        // Set optional fields
        if (!query.value("paid_date").isNull()) {
            DateTime paidDate = stringToDateTime(query.value("paid_date").toString());
            PaymentMethod method = PaymentMethod::OTHER;
            if (!query.value("payment_method").isNull()) {
                method = static_cast<PaymentMethod>(query.value("payment_method").toInt());
            }
            instance.markAsPaid(method);
        }

        // Set status
        PaymentStatus status = static_cast<PaymentStatus>(query.value("status").toInt());
        if (status == PaymentStatus::FAILED && !query.value("failure_reason").isNull()) {
            instance.markAsFailed(query.value("failure_reason").toString().toStdString());
        } else if (status == PaymentStatus::OVERDUE) {
            instance.markAsOverdue();
        } else if (status == PaymentStatus::CANCELLED) {
            instance.cancelPayment();
        }

        m_instanceCache.emplace(instanceId, std::move(instance));
    }
    catch (const std::exception& e) {
        qWarning() << "Failed to load payment instance:" << e.what();
    }
}

std::vector<PaymentInstance> SQLiteRepository::getPaymentInstances(const std::string& recurringPaymentId) const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    std::vector<PaymentInstance> instances;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM payment_instances WHERE recurring_payment_id = ? ORDER BY due_date DESC");
    query.addBindValue(QString::fromStdString(recurringPaymentId));

    if (!query.exec()) {
        return instances;
    }

    while (query.next()) {
        std::string id = query.value("id").toString().toStdString();

        auto it = m_instanceCache.find(id);
        if (it == m_instanceCache.end()) {
            loadPaymentInstanceToCache(id);
            it = m_instanceCache.find(id);
        }

        if (it != m_instanceCache.end()) {
            instances.push_back(it->second);
        }
    }

    return instances;
}

std::vector<PaymentInstance> SQLiteRepository::getAllPaymentInstances() const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    std::vector<PaymentInstance> instances;
    QSqlQuery query(m_db);

    if (!query.exec("SELECT * FROM payment_instances ORDER BY due_date DESC")) {
        return instances;
    }

    while (query.next()) {
        std::string id = query.value("id").toString().toStdString();

        auto it = m_instanceCache.find(id);
        if (it == m_instanceCache.end()) {
            loadPaymentInstanceToCache(id);
            it = m_instanceCache.find(id);
        }

        if (it != m_instanceCache.end()) {
            instances.push_back(it->second);
        }
    }

    return instances;
}

std::vector<PaymentInstance> SQLiteRepository::getPaymentInstancesByStatus(PaymentStatus status) const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    std::vector<PaymentInstance> instances;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM payment_instances WHERE status = ? ORDER BY due_date DESC");
    query.addBindValue(static_cast<int>(status));

    if (!query.exec()) {
        return instances;
    }

    while (query.next()) {
        std::string id = query.value("id").toString().toStdString();

        auto it = m_instanceCache.find(id);
        if (it == m_instanceCache.end()) {
            loadPaymentInstanceToCache(id);
            it = m_instanceCache.find(id);
        }

        if (it != m_instanceCache.end()) {
            instances.push_back(it->second);
        }
    }

    return instances;
}

std::vector<PaymentInstance> SQLiteRepository::getPaymentInstancesDueBefore(const DateTime& date) const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    std::vector<PaymentInstance> instances;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM payment_instances WHERE due_date < ? ORDER BY due_date DESC");
    query.addBindValue(dateTimeToString(date));

    if (!query.exec()) {
        return instances;
    }

    while (query.next()) {
        std::string id = query.value("id").toString().toStdString();

        auto it = m_instanceCache.find(id);
        if (it == m_instanceCache.end()) {
            loadPaymentInstanceToCache(id);
            it = m_instanceCache.find(id);
        }

        if (it != m_instanceCache.end()) {
            instances.push_back(it->second);
        }
    }

    return instances;
}

void SQLiteRepository::updatePaymentInstance(const PaymentInstance& instance)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE payment_instances
        SET amount = ?, due_date = ?, paid_date = ?, bank_account = ?, category = ?,
            payment_method = ?, status = ?, failure_reason = ?, updated_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )");

    query.addBindValue(instance.getAmount());
    query.addBindValue(dateTimeToString(instance.getDueDate()));
    query.addBindValue(instance.getPaidDate().has_value() ? dateTimeToString(instance.getPaidDate().value()) : QVariant());
    query.addBindValue(QString::fromStdString(instance.getBankAccountNumber()));
    query.addBindValue(static_cast<int>(instance.getCategory()));
    query.addBindValue(instance.getPaymentMethod().has_value() ? static_cast<int>(instance.getPaymentMethod().value()) : QVariant());
    query.addBindValue(static_cast<int>(instance.getStatus()));
    query.addBindValue(instance.getFailureReason().has_value() ? QString::fromStdString(instance.getFailureReason().value()) : QVariant());
    query.addBindValue(QString::fromStdString(instance.getName()));

    if (!query.exec()) {
        throw std::runtime_error("Failed to update payment instance: " + query.lastError().text().toStdString());
    }

    m_instanceCache.emplace(instance.getName(), instance);
}

bool SQLiteRepository::removePaymentInstance(const std::string& id)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM payment_instances WHERE id = ?");
    query.addBindValue(QString::fromStdString(id));

    bool success = query.exec() && query.numRowsAffected() > 0;

    if (success) {
        m_instanceCache.erase(id);
    }

    return success;
}

bool SQLiteRepository::paymentInstanceExists(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM payment_instances WHERE id = ?");
    query.addBindValue(QString::fromStdString(id));

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

void SQLiteRepository::clearAll()
{
    std::lock_guard<std::mutex> lock(m_mtx);

    QSqlQuery query(m_db);
    query.exec("DELETE FROM payment_instances");
    query.exec("DELETE FROM recurring_payments");

    clearCache();
}

void SQLiteRepository::clearCache()
{
    m_recurringCache.clear();
    m_instanceCache.clear();
}

size_t SQLiteRepository::getRecurringPaymentCount() const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    QSqlQuery query(m_db);
    if (query.exec("SELECT COUNT(*) FROM recurring_payments") && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

size_t SQLiteRepository::getPaymentInstanceCount() const
{
    std::lock_guard<std::mutex> lock(m_mtx);

    QSqlQuery query(m_db);
    if (query.exec("SELECT COUNT(*) FROM payment_instances") && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}
