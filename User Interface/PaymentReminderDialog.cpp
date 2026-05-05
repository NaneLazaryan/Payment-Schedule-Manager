#include "PaymentReminderDialog.h"
#include "PaymentTypes.h"
#include "DateTime.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QBrush>

PaymentReminderDialog::PaymentReminderDialog(const std::vector<PaymentInstance>& duePayments, QWidget *parent)
    : QDialog(parent), m_duePayments(duePayments)
{
    setupUI();
    populateTable(duePayments);

    setWindowTitle("⚠️ Payment Reminders");
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    resize(800, 500);
}

void PaymentReminderDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Header with warning icon and message
    QLabel *headerLabel = new QLabel("⚠️ You have payments that need attention!");
    headerLabel->setStyleSheet(
        "QLabel { "
        "background-color: #fff3cd; "
        "border: 2px solid #ffc107; "
        "border-radius: 8px; "
        "padding: 15px; "
        "font-size: 16px; "
        "font-weight: bold; "
        "color: #856404; "
        "}"
        );
    mainLayout->addWidget(headerLabel);

    // Summary label
    m_summaryLabel = new QLabel();
    m_summaryLabel->setStyleSheet("font-size: 14px; color: #333; font-weight: 500;");
    mainLayout->addWidget(m_summaryLabel);

    // Table of due payments
    m_paymentsTable = new QTableWidget();
    m_paymentsTable->setColumnCount(6);
    m_paymentsTable->setHorizontalHeaderLabels({
        "Payment Name",
        "Amount",
        "Bank Account",
        "Due Date",
        "Days Overdue",
        "Category"
    });
    m_paymentsTable->horizontalHeader()->setStretchLastSection(true);
    m_paymentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_paymentsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_paymentsTable->setAlternatingRowColors(true);
    m_paymentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_paymentsTable->verticalHeader()->setVisible(false);
    m_paymentsTable->setStyleSheet(
        "QTableWidget { "
        "border: 2px solid #e0e0e0; "
        "border-radius: 5px; "
        "gridline-color: #e0e0e0; "
        "background-color: white; "
        "selection-background-color: #3498db; "
        "font-size: 13px; "
        "}"
        "QHeaderView::section { "
        "background-color: #34495e; "
        "color: white; "
        "padding: 10px; "
        "border: none; "
        "font-weight: bold; "
        "}"
        );
    mainLayout->addWidget(m_paymentsTable);

    // Action buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_markPaidBtn = new QPushButton("✅ Mark Selected as Paid");
    m_markPaidBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #27ae60; "
        "color: white; "
        "padding: 10px 20px; "
        "border-radius: 5px; "
        "font-weight: bold; "
        "font-size: 13px; "
        "} "
        "QPushButton:hover { background-color: #229954; } "
        "QPushButton:pressed { background-color: #1e8449; }"
        );

    m_snoozeBtn = new QPushButton("⏰ Snooze (Remind Later)");
    m_snoozeBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #f39c12; "
        "color: white; "
        "padding: 10px 20px; "
        "border-radius: 5px; "
        "font-weight: bold; "
        "font-size: 13px; "
        "} "
        "QPushButton:hover { background-color: #e67e22; } "
        "QPushButton:pressed { background-color: #d35400; }"
        );

    m_closeBtn = new QPushButton("Close");
    m_closeBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #95a5a6; "
        "color: white; "
        "padding: 10px 20px; "
        "border-radius: 5px; "
        "font-weight: bold; "
        "font-size: 13px; "
        "} "
        "QPushButton:hover { background-color: #7f8c8d; } "
        "QPushButton:pressed { background-color: #6c7a7b; }"
        );

    buttonLayout->addWidget(m_markPaidBtn);
    buttonLayout->addWidget(m_snoozeBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeBtn);

    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(m_markPaidBtn, &QPushButton::clicked, this, &PaymentReminderDialog::onMarkAsPaid);
    connect(m_snoozeBtn, &QPushButton::clicked, this, &PaymentReminderDialog::onSnooze);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    // Store instance IDs for lookup
    m_paymentsTable->setProperty("instanceIds", QVariant());
}

void PaymentReminderDialog::populateTable(const std::vector<PaymentInstance>& duePayments)
{
    m_paymentsTable->setRowCount(0);

    auto now = DateTimeUtils::now();
    int overdueCount = 0;
    int dueTodayCount = 0;
    double totalAmount = 0.0;

    for (const auto& payment : duePayments) {
        int row = m_paymentsTable->rowCount();
        m_paymentsTable->insertRow(row);

        // Payment Name
        QTableWidgetItem *nameItem = new QTableWidgetItem(QString::fromStdString(payment.getRecurringPaymentId()));
        nameItem->setFont(QFont("Arial", 10, QFont::Bold));
        m_paymentsTable->setItem(row, 0, nameItem);

        // Amount
        QTableWidgetItem *amountItem = new QTableWidgetItem(QString("$%1").arg(payment.getAmount(), 0, 'f', 2));
        amountItem->setFont(QFont("Arial", 10, QFont::Bold));
        amountItem->setForeground(QBrush(QColor("#e74c3c")));
        m_paymentsTable->setItem(row, 1, amountItem);

        // Bank Account
        m_paymentsTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(payment.getBankAccountNumber())));

        // Due Date
        QString dueDate = QString::fromStdString(DateTimeUtils::formatDateTime(payment.getDueDate(), "%Y-%m-%d"));
        QTableWidgetItem *dueDateItem = new QTableWidgetItem(dueDate);
        if (payment.isOverdue()) {
            dueDateItem->setBackground(QBrush(QColor("#fadbd8")));
        }
        m_paymentsTable->setItem(row, 3, dueDateItem);

        // Days Overdue / Status
        QString overdueText;
        QTableWidgetItem *overdueItem;
        if (payment.isDueToday()) {
            overdueText = "DUE TODAY";
            overdueItem = new QTableWidgetItem(overdueText);
            overdueItem->setForeground(QBrush(QColor("#f39c12")));
            overdueItem->setFont(QFont("Arial", 10, QFont::Bold));
            dueTodayCount++;
        } else if (payment.isOverdue()) {
            auto duration = std::chrono::duration_cast<std::chrono::hours>(now - payment.getDueDate());
            int daysOverdue = duration.count() / 24;
            overdueText = QString::number(daysOverdue) + " days";
            overdueItem = new QTableWidgetItem(overdueText);
            overdueItem->setForeground(QBrush(QColor("#e74c3c")));
            overdueItem->setFont(QFont("Arial", 10, QFont::Bold));
            overdueCount++;
        } else {
            overdueItem = new QTableWidgetItem("-");
        }
        m_paymentsTable->setItem(row, 4, overdueItem);

        // Category
        QTableWidgetItem *categoryItem = new QTableWidgetItem(
            QString::fromStdString(PaymentTypeUtils::categoryToString(payment.getCategory()))
            );
        categoryItem->setForeground(QBrush(QColor(getCategoryColor(payment.getCategory()))));
        m_paymentsTable->setItem(row, 5, categoryItem);

        totalAmount += payment.getAmount();
    }

    m_paymentsTable->resizeColumnsToContents();

    // Update summary
    QString summary = QString("Total: %1 payment(s) • Overdue: %2 • Due Today: %3 • Total Amount: $%4")
                          .arg(duePayments.size())
                          .arg(overdueCount)
                          .arg(dueTodayCount)
                          .arg(totalAmount, 0, 'f', 2);
    m_summaryLabel->setText(summary);
}

QString PaymentReminderDialog::getCategoryColor(PaymentCategory category)
{
    switch (category) {
    case PaymentCategory::RENT: return "#e74c3c";
    case PaymentCategory::SUBSCRIPTION: return "#3498db";
    case PaymentCategory::INSURANCE: return "#9b59b6";
    case PaymentCategory::ENTERTAINMENT: return "#f39c12";
    case PaymentCategory::OTHER: return "#95a5a6";
    default: return "#000000";
    }
}

void PaymentReminderDialog::onMarkAsPaid()
{
    int row = m_paymentsTable->currentRow();
    if (row < 0 || row >= static_cast<int>(m_duePayments.size())) {
        return;
    }

    const PaymentInstance& payment = m_duePayments[row];
    emit markPaymentAsPaid(payment.getName());

    // Remove from table
    m_paymentsTable->removeRow(row);
    m_duePayments.erase(m_duePayments.begin() + row);

    // If no more payments, close dialog
    if (m_duePayments.empty()) {
        accept();
    } else {
        // Update summary
        populateTable(m_duePayments);
    }
}

void PaymentReminderDialog::onSnooze()
{
    emit snoozeReminder();
    accept();
}
