#include "PaymentNotificationDialog.h"
#include <QVBoxLayout>
#include <QMessageBox>

PaymentNotificationDialog::PaymentNotificationDialog(const std::vector<PaymentInstance>& duePayments, PaymentService* service, QWidget *parent)
    : QDialog(parent),
    m_duePayments(duePayments),
    m_service(service)
{
    setupUI();
    setWindowTitle("💳 Payments Due Today");
    resize(500, 400);
}

void PaymentNotificationDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Header
    QLabel* headerLabel = new QLabel("⚠️ You have payments due today!");
    headerLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #e74c3c; padding: 10px;");
    mainLayout->addWidget(headerLabel);

    // Payment list
    m_paymentList = new QListWidget();
    m_paymentList->setStyleSheet(R"(
        QListWidget {
            border: 2px solid #3498db;
            border-radius: 5px;
            background-color: #ecf0f1;
            font-size: 13px;
        }
        QListWidget::item {
            padding: 10px;
            margin: 5px;
            background-color: white;
            border-radius: 3px;
        }
        QListWidget::item:selected {
            background-color: #3498db;
            color: white;
        }
    )");

    double totalAmount = 0.0;
    for (const auto& payment : m_duePayments) {
        QString itemText = QString("📋 %1\n💰 Amount: $%2\n🏦 Bank Account: %3")
                               .arg(QString::fromStdString(payment.getRecurringPaymentId()))
                               .arg(payment.getAmount(), 0, 'f', 2)
                               .arg(QString::fromStdString(payment.getBankAccountNumber()));

        QListWidgetItem* item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, QString::fromStdString(payment.getName())); // Store instance ID
        m_paymentList->addItem(item);

        totalAmount += payment.getAmount();
    }

    mainLayout->addWidget(m_paymentList);

    // Total amount
    m_totalLabel = new QLabel(QString("📊 Total Amount: $%1").arg(totalAmount, 0, 'f', 2));
    m_totalLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #27ae60; padding: 10px;");
    mainLayout->addWidget(m_totalLabel);

    // Instructions
    QLabel* instructionLabel = new QLabel("Please transfer the money through your bank app, then select a payment and click 'Mark as Paid'");
    instructionLabel->setStyleSheet("font-size: 12px; color: #7f8c8d; padding: 5px;");
    instructionLabel->setWordWrap(true);
    mainLayout->addWidget(instructionLabel);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_markPaidBtn = new QPushButton("✅ Mark Selected as Paid");
    m_markPaidBtn->setStyleSheet("QPushButton { background-color: #27ae60; color: white; padding: 10px 20px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #229954; }");
    m_markPaidBtn->setEnabled(false);

    m_remindLaterBtn = new QPushButton("⏰ Remind Me Later");
    m_remindLaterBtn->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; padding: 10px 20px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #7f8c8d; }");

    buttonLayout->addWidget(m_remindLaterBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_markPaidBtn);

    mainLayout->addLayout(buttonLayout);

    // Connections
    connect(m_paymentList, &QListWidget::itemSelectionChanged, this, [this]() {
        m_markPaidBtn->setEnabled(m_paymentList->currentItem() != nullptr);
    });
    connect(m_markPaidBtn, &QPushButton::clicked, this, &PaymentNotificationDialog::onMarkAsPaid);
    connect(m_remindLaterBtn, &QPushButton::clicked, this, &PaymentNotificationDialog::onRemindLater);
}

void PaymentNotificationDialog::onMarkAsPaid()
{
    QListWidgetItem* currentItem = m_paymentList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "Warning", "Please select a payment to mark as paid.");
        return;
    }

    QString instanceId = currentItem->data(Qt::UserRole).toString();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirm Payment",
        "Have you transferred the money through your bank app?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        try {
            m_service->markPaymentAsPaid(instanceId.toStdString());

            // Remove from list
            delete m_paymentList->takeItem(m_paymentList->row(currentItem));

            // Update total
            double totalAmount = 0.0;
            for (const auto& payment : m_duePayments) {
                if (QString::fromStdString(payment.getName()) != instanceId) {
                    totalAmount += payment.getAmount();
                }
            }
            m_totalLabel->setText(QString("📊 Total Amount: $%1").arg(totalAmount, 0, 'f', 2));

            QMessageBox::information(this, "Success", "Payment marked as paid!");

            emit paymentMarkedAsPaid();

            // Close if no more payments
            if (m_paymentList->count() == 0) {
                QMessageBox::information(this, "All Done", "All due payments have been marked as paid! 🎉");
                accept();
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", QString("Failed to mark payment as paid: %1").arg(e.what()));
        }
    }
}

void PaymentNotificationDialog::onRemindLater()
{
    reject();
}

