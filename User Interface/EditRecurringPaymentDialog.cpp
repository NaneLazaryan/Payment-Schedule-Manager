#include "EditRecurringPaymentDialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

EditRecurringPaymentDialog::EditRecurringPaymentDialog(PaymentService* service, RecurringPayment* payment, QWidget *parent)
    : QDialog(parent), m_service(service), m_payment(payment)
{
    setupUI();
    loadPaymentData();
    setWindowTitle("Edit Recurring Payment");
    resize(450, 400);
}

void EditRecurringPaymentDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *infoGroup = new QGroupBox("Payment Information");
    QFormLayout *formLayout = new QFormLayout(infoGroup);

    m_idLabel = new QLabel();
    m_idLabel->setStyleSheet("color: #7f8c8d; font-weight: bold;");
    formLayout->addRow("ID:", m_idLabel);

    m_nameEdit = new QLineEdit();
    formLayout->addRow("Name*:", m_nameEdit);

    m_amountSpin = new QDoubleSpinBox();
    m_amountSpin->setRange(0.01, 999999.99);
    m_amountSpin->setDecimals(2);
    m_amountSpin->setPrefix("$ ");
    formLayout->addRow("Amount*:", m_amountSpin);

    m_bankAccountEdit = new QLineEdit();
    formLayout->addRow("Bank Account*:", m_bankAccountEdit);

    m_categoryCombo = new QComboBox();
    m_categoryCombo->addItem("Rent", (int)PaymentCategory::RENT);
    m_categoryCombo->addItem("Subscription", (int)PaymentCategory::SUBSCRIPTION);
    m_categoryCombo->addItem("Insurance", (int)PaymentCategory::INSURANCE);
    m_categoryCombo->addItem("Entertainment", (int)PaymentCategory::ENTERTAINMENT);
    m_categoryCombo->addItem("Other", (int)PaymentCategory::OTHER);
    formLayout->addRow("Category*:", m_categoryCombo);

    m_scheduleLabel = new QLabel();
    m_scheduleLabel->setStyleSheet("color: #34495e; font-style: italic;");
    m_scheduleLabel->setWordWrap(true);
    formLayout->addRow("Schedule:", m_scheduleLabel);

    mainLayout->addWidget(infoGroup);

    QLabel *noteLabel = new QLabel("Note: Schedule cannot be modified. Create a new payment to change schedule.");
    noteLabel->setStyleSheet("color: #e67e22; font-size: 11px; font-style: italic;");
    noteLabel->setWordWrap(true);
    mainLayout->addWidget(noteLabel);

    mainLayout->addStretch();

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *cancelBtn = new QPushButton("Cancel");
    QPushButton *saveBtn = new QPushButton("Save Changes");
    saveBtn->setStyleSheet("QPushButton { background-color: #27ae60; color: white; padding: 10px 20px; font-weight: bold; }");

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(saveBtn);

    mainLayout->addLayout(buttonLayout);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &EditRecurringPaymentDialog::onAccept);

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

void EditRecurringPaymentDialog::loadPaymentData()
{
    m_idLabel->setText(QString::fromStdString(m_payment->getId()));
    m_nameEdit->setText(QString::fromStdString(m_payment->getName()));
    m_amountSpin->setValue(m_payment->getAmount());
    m_bankAccountEdit->setText(QString::fromStdString(m_payment->getBankAccountNumber()));

    for (int i = 0; i < m_categoryCombo->count(); ++i) {
        if (m_categoryCombo->itemData(i).toInt() == (int)m_payment->getCategory()) {
            m_categoryCombo->setCurrentIndex(i);
            break;
        }
    }

    m_scheduleLabel->setText(QString::fromStdString(m_payment->getScheduleDescription()));
}

void EditRecurringPaymentDialog::onAccept()
{
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
        m_payment->setName(m_nameEdit->text().trimmed().toStdString());
        m_payment->setAmount(m_amountSpin->value());
        m_payment->setCategory(static_cast<PaymentCategory>(m_categoryCombo->currentData().toInt()));
        m_payment->setBankAccountNumber(m_bankAccountEdit->text().trimmed().toStdString());

        m_service->updateRecurringPayment(*m_payment);

        accept();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to update payment: %1").arg(e.what()));
    }
}
