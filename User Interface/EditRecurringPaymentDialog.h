#ifndef EDITRECURRINGPAYMENTDIALOG_H
#define EDITRECURRINGPAYMENTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include "PaymentService.h"
#include "RecurringPayment.h"

class EditRecurringPaymentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditRecurringPaymentDialog(PaymentService* service, RecurringPayment* payment, QWidget *parent = nullptr);
    ~EditRecurringPaymentDialog() = default;

private slots:
    void onAccept();

private:
    void setupUI();
    void loadPaymentData();

    PaymentService* m_service;
    RecurringPayment* m_payment;

    QLineEdit *m_nameEdit;
    QDoubleSpinBox *m_amountSpin;
    QComboBox *m_categoryCombo;
    QLineEdit *m_bankAccountEdit;
    QLabel *m_idLabel;
    QLabel *m_scheduleLabel;
};

#endif // EDITRECURRINGPAYMENTDIALOG_H
