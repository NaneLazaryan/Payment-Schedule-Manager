#include "PaymentDetailsDialog.h"
#include <QPushButton>
#include <QHBoxLayout>

PaymentDetailsDialog::PaymentDetailsDialog(RecurringPayment* payment, QWidget *parent)
    : QDialog(parent), m_payment(payment)
{
    setupUI();
    displayPaymentInfo();
    setWindowTitle("Payment Details");
    resize(500, 450);
}

void PaymentDetailsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("Recurring Payment Details");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; padding: 10px;");
    mainLayout->addWidget(titleLabel);

    m_detailsText = new QTextEdit();
    m_detailsText->setReadOnly(true);
    m_detailsText->setStyleSheet(R"(
        QTextEdit {
            background-color: #ecf0f1;
            border: 2px solid #bdc3c7;
            border-radius: 5px;
            padding: 10px;
            font-family: 'Courier New', monospace;
            font-size: 12px;
        }
    )");
    mainLayout->addWidget(m_detailsText);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *closeBtn = new QPushButton("Close");
    closeBtn->setStyleSheet("QPushButton { background-color: #3498db; color: white; padding: 10px 20px; font-weight: bold; }");
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeBtn);

    mainLayout->addLayout(buttonLayout);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void PaymentDetailsDialog::displayPaymentInfo()
{
    QString details = QString::fromStdString(m_payment->toString());

    // Format with HTML for better presentation
    QString htmlDetails = "<pre style='font-family: Arial; font-size: 13px;'>";
    htmlDetails += details;
    htmlDetails += "</pre>";

    m_detailsText->setHtml(htmlDetails);
}
