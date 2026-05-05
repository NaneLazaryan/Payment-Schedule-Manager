#ifndef ADDRECURRINGPAYMENTDIALOG_H
#define ADDRECURRINGPAYMENTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QDateEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include "PaymentService.h"
#include "ScheduleFactory.h"

class AddRecurringPaymentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddRecurringPaymentDialog(PaymentService* service, QWidget *parent = nullptr);
    ~AddRecurringPaymentDialog() = default;

private slots:
    void onScheduleTypeChanged(int index);
    void onAccept();

private:
    void setupUI();
    void updateScheduleOptions();
    std::unique_ptr<RecurringSchedule> createSchedule(std::unique_ptr<ScheduleFactory> factory);

    PaymentService* m_service;

    QLineEdit *m_idEdit;
    QLineEdit *m_nameEdit;
    QDoubleSpinBox *m_amountSpin;
    QComboBox *m_categoryCombo;
    QLineEdit *m_bankAccountEdit;
    QComboBox *m_scheduleTypeCombo;
    QDateEdit *m_startDateEdit;
    QCheckBox *m_hasEndDateCheck;
    QDateEdit *m_endDateEdit;

    // Schedule-specific widgets
    QSpinBox *m_intervalSpin;
    QComboBox *m_dayOfWeekCombo;
    QSpinBox *m_dayOfMonthSpin;
    QSpinBox *m_monthSpin;

    QWidget *m_scheduleOptionsWidget;
    QFormLayout *m_scheduleOptionsLayout;
};

#endif // ADDRECURRINGPAYMENTDIALOG_H
