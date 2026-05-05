#ifndef PAYMENTREMINDERDIALOG_H
#define PAYMENTREMINDERDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <vector>
#include "PaymentInstance.h"

class PaymentReminderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PaymentReminderDialog(const std::vector<PaymentInstance>& duePayments, QWidget *parent = nullptr);
    ~PaymentReminderDialog() = default;

signals:
    void markPaymentAsPaid(const std::string& instanceId);
    void snoozeReminder();

private:
    void setupUI();
    void populateTable(const std::vector<PaymentInstance>& duePayments);
    QString getCategoryColor(PaymentCategory category);

    std::vector<PaymentInstance> m_duePayments;
    QTableWidget *m_paymentsTable;
    QLabel *m_summaryLabel;
    QPushButton *m_markPaidBtn;
    QPushButton *m_snoozeBtn;
    QPushButton *m_closeBtn;

private slots:
    void onMarkAsPaid();
    void onSnooze();
};

#endif // PAYMENTREMINDERDIALOG_H
