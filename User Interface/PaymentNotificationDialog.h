#ifndef PAYMENTNOTIFICATIONDIALOG_H
#define PAYMENTNOTIFICATIONDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <vector>
#include "PaymentInstance.h"
#include "PaymentService.h"

class PaymentNotificationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PaymentNotificationDialog(const std::vector<PaymentInstance>& duePayments, PaymentService* service, QWidget *parent = nullptr);
    ~PaymentNotificationDialog() = default;

signals:
    void paymentMarkedAsPaid();

private slots:
    void onMarkAsPaid();
    void onRemindLater();

private:
    void setupUI();

    std::vector<PaymentInstance> m_duePayments;
    PaymentService* m_service;

    QListWidget* m_paymentList;
    QPushButton* m_markPaidBtn;
    QPushButton* m_remindLaterBtn;
    QLabel* m_totalLabel;
};

#endif
