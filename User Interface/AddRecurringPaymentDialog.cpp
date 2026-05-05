#include "AddRecurringPaymentDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>

AddRecurringPaymentDialog::AddRecurringPaymentDialog(PaymentService* service, QWidget *parent)
    : QDialog(parent), m_service(service)
{
    setupUI();
    setWindowTitle("Add Recurring Payment");
    resize(500, 600);
}

void AddRecurringPaymentDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Basic Information Group
    QGroupBox *basicGroup = new QGroupBox("Basic Information");
    QFormLayout *basicLayout = new QFormLayout(basicGroup);

    m_idEdit = new QLineEdit();
    m_idEdit->setPlaceholderText("Unique identifier");
    basicLayout->addRow("ID*:", m_idEdit);

    m_nameEdit = new QLineEdit();
    m_nameEdit->setPlaceholderText("e.g., Netflix Subscription");
    basicLayout->addRow("Name*:", m_nameEdit);

    m_amountSpin = new QDoubleSpinBox();
    m_amountSpin->setRange(0.01, 999999.99);
    m_amountSpin->setDecimals(2);
    m_amountSpin->setPrefix("$ ");
    m_amountSpin->setValue(10.00);
    basicLayout->addRow("Amount*:", m_amountSpin);

    m_bankAccountEdit = new QLineEdit();
    m_bankAccountEdit->setPlaceholderText("e.g., 1234-5678-9012");
    basicLayout->addRow("Bank Account*:", m_bankAccountEdit);

    m_categoryCombo = new QComboBox();
    m_categoryCombo->addItem("Rent", (int)PaymentCategory::RENT);
    m_categoryCombo->addItem("Subscription", (int)PaymentCategory::SUBSCRIPTION);
    m_categoryCombo->addItem("Insurance", (int)PaymentCategory::INSURANCE);
    m_categoryCombo->addItem("Entertainment", (int)PaymentCategory::ENTERTAINMENT);
    m_categoryCombo->addItem("Other", (int)PaymentCategory::OTHER);
    basicLayout->addRow("Category*:", m_categoryCombo);

    mainLayout->addWidget(basicGroup);

    // Schedule Information Group
    QGroupBox *scheduleGroup = new QGroupBox("Schedule Information");
    QFormLayout *scheduleLayout = new QFormLayout(scheduleGroup);

    m_scheduleTypeCombo = new QComboBox();
    m_scheduleTypeCombo->addItem("Daily", 0);
    m_scheduleTypeCombo->addItem("Weekly", 1);
    m_scheduleTypeCombo->addItem("Monthly", 2);
    m_scheduleTypeCombo->addItem("Yearly", 3);
    scheduleLayout->addRow("Schedule Type*:", m_scheduleTypeCombo);

    m_startDateEdit = new QDateEdit(QDate::currentDate());
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDisplayFormat("yyyy-MM-dd");
    scheduleLayout->addRow("Start Date*:", m_startDateEdit);

    m_hasEndDateCheck = new QCheckBox("Set End Date");
    scheduleLayout->addRow("", m_hasEndDateCheck);

    m_endDateEdit = new QDateEdit(QDate::currentDate().addYears(1));
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setDisplayFormat("yyyy-MM-dd");
    m_endDateEdit->setEnabled(false);
    scheduleLayout->addRow("End Date:", m_endDateEdit);

    mainLayout->addWidget(scheduleGroup);

    // Schedule Options
    m_scheduleOptionsWidget = new QWidget();
    m_scheduleOptionsLayout = new QFormLayout(m_scheduleOptionsWidget);

    m_intervalSpin = new QSpinBox();
    m_intervalSpin->setRange(1, 365);
    m_intervalSpin->setValue(1);

    m_dayOfWeekCombo = new QComboBox();
    m_dayOfWeekCombo->addItem("Sunday", 0);
    m_dayOfWeekCombo->addItem("Monday", 1);
    m_dayOfWeekCombo->addItem("Tuesday", 2);
    m_dayOfWeekCombo->addItem("Wednesday", 3);
    m_dayOfWeekCombo->addItem("Thursday", 4);
    m_dayOfWeekCombo->addItem("Friday", 5);
    m_dayOfWeekCombo->addItem("Saturday", 6);

    m_dayOfMonthSpin = new QSpinBox();
    m_dayOfMonthSpin->setRange(1, 31);
    m_dayOfMonthSpin->setValue(1);

    m_monthSpin = new QSpinBox();
    m_monthSpin->setRange(1, 12);
    m_monthSpin->setValue(1);

    QGroupBox *optionsGroup = new QGroupBox("Schedule Options");
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsGroup);
    optionsLayout->addWidget(m_scheduleOptionsWidget);
    mainLayout->addWidget(optionsGroup);

    updateScheduleOptions();

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *cancelBtn = new QPushButton("Cancel");
    QPushButton *saveBtn = new QPushButton("Save");
    saveBtn->setStyleSheet("QPushButton { background-color: #27ae60; color: white; padding: 10px 20px; font-weight: bold; }");

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(saveBtn);

    mainLayout->addLayout(buttonLayout);

    // Connections
    connect(m_scheduleTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddRecurringPaymentDialog::onScheduleTypeChanged);
    connect(m_hasEndDateCheck, &QCheckBox::toggled, m_endDateEdit, &QDateEdit::setEnabled);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &AddRecurringPaymentDialog::onAccept);

    // styling
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #3498db;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
    )");
}

void AddRecurringPaymentDialog::onScheduleTypeChanged(int index)
{
    updateScheduleOptions();
}

void AddRecurringPaymentDialog::updateScheduleOptions()
{
    // Clear existing widgets
    while (m_scheduleOptionsLayout->count() > 0) {
        QLayoutItem *item = m_scheduleOptionsLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->setParent(nullptr);
        }
        delete item;
    }

    int scheduleType = m_scheduleTypeCombo->currentData().toInt();

    switch (scheduleType) {
    case 0: // Daily
        m_scheduleOptionsLayout->addRow("Repeat Every (days):", m_intervalSpin);
        m_intervalSpin->setParent(m_scheduleOptionsWidget);
        m_intervalSpin->show();
        break;

    case 1: // Weekly
        m_scheduleOptionsLayout->addRow("Repeat Every (weeks):", m_intervalSpin);
        m_scheduleOptionsLayout->addRow("Day of Week:", m_dayOfWeekCombo);
        m_intervalSpin->setParent(m_scheduleOptionsWidget);
        m_dayOfWeekCombo->setParent(m_scheduleOptionsWidget);
        m_intervalSpin->show();
        m_dayOfWeekCombo->show();
        break;

    case 2: // Monthly
        m_scheduleOptionsLayout->addRow("Repeat Every (months):", m_intervalSpin);
        m_scheduleOptionsLayout->addRow("Day of Month:", m_dayOfMonthSpin);
        m_intervalSpin->setParent(m_scheduleOptionsWidget);
        m_dayOfMonthSpin->setParent(m_scheduleOptionsWidget);
        m_intervalSpin->show();
        m_dayOfMonthSpin->show();
        break;

    case 3: // Yearly
        m_scheduleOptionsLayout->addRow("Repeat Every (years):", m_intervalSpin);
        m_scheduleOptionsLayout->addRow("Month:", m_monthSpin);
        m_scheduleOptionsLayout->addRow("Day:", m_dayOfMonthSpin);
        m_intervalSpin->setParent(m_scheduleOptionsWidget);
        m_monthSpin->setParent(m_scheduleOptionsWidget);
        m_dayOfMonthSpin->setParent(m_scheduleOptionsWidget);
        m_intervalSpin->show();
        m_monthSpin->show();
        m_dayOfMonthSpin->show();
        break;
    }
}

std::unique_ptr<RecurringSchedule> AddRecurringPaymentDialog::createSchedule(std::unique_ptr<ScheduleFactory> factory)
{
    QDate startDate = m_startDateEdit->date();
    DateTime startDateTime = DateTimeUtils::createDateTime(
        startDate.year(), startDate.month(), startDate.day()
        );

    std::unique_ptr<RecurringSchedule> schedule = factory->create(startDateTime);

    if (m_hasEndDateCheck->isChecked()) {
        QDate endDate = m_endDateEdit->date();
        DateTime endDateTime = DateTimeUtils::createDateTime(
            endDate.year(), endDate.month(), endDate.day()
            );
        schedule->setEndDate(endDateTime);
    }

    return schedule;
}

void AddRecurringPaymentDialog::onAccept()
{
    // Validation
    if (m_idEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter an ID.");
        return;
    }

    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter a name.");
        return;
    }

    if (m_amountSpin->value() <= 0) {
        QMessageBox::warning(this, "Validation Error", "Amount must be greater than 0.");
        return;
    }
    if (m_bankAccountEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter a bank account number.");
        return;
    }

    try {
        std::string id = m_idEdit->text().trimmed().toStdString();
        std::string name = m_nameEdit->text().trimmed().toStdString();
        double amount = m_amountSpin->value();
        std::string bankAccount = m_bankAccountEdit->text().trimmed().toStdString();
        PaymentCategory category = static_cast<PaymentCategory>(m_categoryCombo->currentData().toInt());

        int scheduleType = m_scheduleTypeCombo->currentData().toInt();
        int interval = m_intervalSpin->value();

        std::unique_ptr<ScheduleFactory> factory;

        switch (scheduleType) {
        case 0: // Daily
            factory = std::make_unique<DailyScheduleFactory>(interval);
            break;

        case 1: { // Weekly
            DayOfWeek dayOfWeek = static_cast<DayOfWeek>(m_dayOfWeekCombo->currentData().toInt());
            factory = std::make_unique<WeeklyScheduleFactory>(interval,dayOfWeek);
            break;
        }

        case 2: { // Monthly
            int dayOfMonth = m_dayOfMonthSpin->value();
            factory = std::make_unique<MonthlyScheduleFactory>(interval, dayOfMonth);
            break;
        }

        case 3: { // Yearly
            int month = m_monthSpin->value();
            int day = m_dayOfMonthSpin->value();
            factory = std::make_unique<YearlyScheduleFactory>(interval, month, day);
            break;
        }
        }
        auto schedule = createSchedule(std::move(factory));

        m_service->createRecurringPayment(name, id, amount, category, bankAccount, std::move(schedule));

        accept();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to create payment: %1").arg(e.what()));
    }
}
