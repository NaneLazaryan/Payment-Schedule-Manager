#ifndef PAYMENTDETAILSDIALOG_H
#define PAYMENTDETAILSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include "RecurringPayment.h"

class PaymentDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PaymentDetailsDialog(RecurringPayment* payment, QWidget *parent = nullptr);
    ~PaymentDetailsDialog() = default;

private:
    void setupUI();
    void displayPaymentInfo();

    RecurringPayment* m_payment;
    QTextEdit *m_detailsText;
};

#endif // PAYMENTDETAILSDIALOG_H
