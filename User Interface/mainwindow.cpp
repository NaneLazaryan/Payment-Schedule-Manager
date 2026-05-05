#include "MainWindow.h"
#include "AddRecurringPaymentDialog.h"
#include "EditRecurringPaymentDialog.h"
#include "PaymentDetailsDialog.h"
#include "NotificationCenter.h"
#include "PaymentNotificationDialog.h"
#include "PaymentReminderDialog.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <algorithm>
#include <vector>
#include <QGridLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QFrame>
#include <QPalette>
#include <QFileDialog>
#include <QThread>
#include "SQLiteRepository.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Initialize service with in-memory repository
    auto repo = std::make_unique<InMemoryRepository>();
    m_paymentService = std::make_unique<PaymentService>(std::move(repo));
    m_notificationCenter = std::make_unique<NotificationCenter>();
    m_notificationCenter->addObserver(this);

    setupUI();
    setupMenuBar();
    applyModernStyle();

    // Setup auto-refresh timer (every 60 seconds)
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &MainWindow::refreshDashboard);
    m_refreshTimer->start(60000);

    // Initial load
    refreshDashboard();
    refreshRecurringPayments();

    setWindowTitle("Recurring Payment Schedule Manager");
    resize(1200, 800);

    // Setup reminder check timer (check every 5 minutes)
    m_reminderCheckTimer = new QTimer(this);
    connect(m_reminderCheckTimer, &QTimer::timeout, this, &MainWindow::showPaymentReminders);
    m_reminderCheckTimer->start(300000); // 5 minutes in milliseconds

    // Show reminders on startup after a short delay (5 seconds)
    QTimer::singleShot(5000, this, &MainWindow::showPaymentReminders);
}

MainWindow::~MainWindow()
{
    if (m_notificationCenter) {
        m_notificationCenter->removeObserver(this);
    }
}

void MainWindow::setupUI()
{
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setTabPosition(QTabWidget::North);

    setupDashboard();
    setupRecurringPaymentsTab();
    setupPaymentInstancesTab();
    setupUpcomingPaymentsTab();
    setupOverduePaymentsTab();
    setupPaymentHistoryTab();

    m_tabWidget->addTab(m_dashboardWidget, "📊 Dashboard");
    m_tabWidget->addTab(m_recurringWidget, "🔄 Recurring Payments");
    m_tabWidget->addTab(m_instancesWidget, "💳 Payment Instances");
    m_tabWidget->addTab(m_upcomingWidget, "📅 Upcoming");
    m_tabWidget->addTab(m_overdueWidget, "⚠️ Overdue");
    m_tabWidget->addTab(m_historyWidget, "📜 Payment History");

    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    setCentralWidget(m_tabWidget);
}

void MainWindow::setupDashboard()
{
    m_dashboardWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(m_dashboardWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel *titleLabel = new QLabel("Payment Schedule Dashboard");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    mainLayout->addWidget(titleLabel);

    // Statistics cards
    QGridLayout *statsLayout = new QGridLayout();
    statsLayout->setSpacing(15);

    // Card 1: Total Recurring Payments
    QFrame *card1 = new QFrame();
    card1->setFrameShape(QFrame::StyledPanel);
    card1->setStyleSheet("QFrame { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #667eea, stop:1 #764ba2); border-radius: 10px; padding: 20px; }");
    QVBoxLayout *card1Layout = new QVBoxLayout(card1);
    QLabel *card1Title = new QLabel("Total Recurring");
    card1Title->setStyleSheet("color: white; font-size: 14px; font-weight: 500;");
    m_totalRecurringLabel = new QLabel("0");
    m_totalRecurringLabel->setStyleSheet("color: white; font-size: 36px; font-weight: bold;");
    card1Layout->addWidget(card1Title);
    card1Layout->addWidget(m_totalRecurringLabel);
    card1Layout->addStretch();
    statsLayout->addWidget(card1, 0, 0);

    // Card 2: Active Payments
    QFrame *card2 = new QFrame();
    card2->setFrameShape(QFrame::StyledPanel);
    card2->setStyleSheet("QFrame { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #f093fb, stop:1 #f5576c); border-radius: 10px; padding: 20px; }");
    QVBoxLayout *card2Layout = new QVBoxLayout(card2);
    QLabel *card2Title = new QLabel("Active Payments");
    card2Title->setStyleSheet("color: white; font-size: 14px; font-weight: 500;");
    m_activePaymentsLabel = new QLabel("0");
    m_activePaymentsLabel->setStyleSheet("color: white; font-size: 36px; font-weight: bold;");
    card2Layout->addWidget(card2Title);
    card2Layout->addWidget(m_activePaymentsLabel);
    card2Layout->addStretch();
    statsLayout->addWidget(card2, 0, 1);

    // Card 3: Upcoming This Month
    QFrame *card3 = new QFrame();
    card3->setFrameShape(QFrame::StyledPanel);
    card3->setStyleSheet("QFrame { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4facfe, stop:1 #00f2fe); border-radius: 10px; padding: 20px; }");
    QVBoxLayout *card3Layout = new QVBoxLayout(card3);
    QLabel *card3Title = new QLabel("Upcoming (30 days)");
    card3Title->setStyleSheet("color: white; font-size: 14px; font-weight: 500;");
    m_upcomingCountLabel = new QLabel("0");
    m_upcomingCountLabel->setStyleSheet("color: white; font-size: 36px; font-weight: bold;");
    card3Layout->addWidget(card3Title);
    card3Layout->addWidget(m_upcomingCountLabel);
    card3Layout->addStretch();
    statsLayout->addWidget(card3, 0, 2);

    // Card 4: Overdue
    QFrame *card4 = new QFrame();
    card4->setFrameShape(QFrame::StyledPanel);
    card4->setStyleSheet("QFrame { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #fa709a, stop:1 #fee140); border-radius: 10px; padding: 20px; }");
    QVBoxLayout *card4Layout = new QVBoxLayout(card4);
    QLabel *card4Title = new QLabel("Overdue");
    card4Title->setStyleSheet("color: white; font-size: 14px; font-weight: 500;");
    m_overdueCountLabel = new QLabel("0");
    m_overdueCountLabel->setStyleSheet("color: white; font-size: 36px; font-weight: bold;");
    card4Layout->addWidget(card4Title);
    card4Layout->addWidget(m_overdueCountLabel);
    card4Layout->addStretch();
    statsLayout->addWidget(card4, 0, 3);

    mainLayout->addLayout(statsLayout);

    // Monthly total section
    QFrame *monthlyFrame = new QFrame();
    monthlyFrame->setFrameShape(QFrame::StyledPanel);
    monthlyFrame->setStyleSheet("QFrame { background: white; border: 2px solid #e0e0e0; border-radius: 10px; padding: 15px; }");
    QHBoxLayout *monthlyLayout = new QHBoxLayout(monthlyFrame);
    QLabel *monthlyTitle = new QLabel("Estimated Monthly Total:");
    monthlyTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #2c3e50;");
    m_monthlyTotalLabel = new QLabel("$0.00");
    m_monthlyTotalLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #27ae60;");
    monthlyLayout->addWidget(monthlyTitle);
    monthlyLayout->addWidget(m_monthlyTotalLabel);
    monthlyLayout->addStretch();
    mainLayout->addWidget(monthlyFrame);

    mainLayout->addStretch();
}

void MainWindow::setupRecurringPaymentsTab()
{
    m_recurringWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_recurringWidget);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);

    // Toolbar
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    m_addRecurringBtn = new QPushButton("➕ Add Payment");
    m_editRecurringBtn = new QPushButton("✏️ Edit");
    m_deleteRecurringBtn = new QPushButton("🗑️ Delete");
    m_toggleStatusBtn = new QPushButton("⏸️ Toggle Status");
    QPushButton *refreshBtn = new QPushButton("🔄 Refresh");

    m_addRecurringBtn->setStyleSheet("QPushButton { background-color: #27ae60; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #229954; }");

    toolbarLayout->addWidget(m_addRecurringBtn);
    toolbarLayout->addWidget(m_editRecurringBtn);
    toolbarLayout->addWidget(m_deleteRecurringBtn);
    toolbarLayout->addWidget(m_toggleStatusBtn);
    toolbarLayout->addWidget(refreshBtn);
    toolbarLayout->addStretch();

    layout->addLayout(toolbarLayout);

    // Table
    m_recurringTable = new QTableWidget();
    m_recurringTable->setColumnCount(8);
    m_recurringTable->setHorizontalHeaderLabels({"ID", "Name", "Amount", "Bank Account", "Category", "Schedule", "Status", "Next Payment"});
    m_recurringTable->horizontalHeader()->setStretchLastSection(true);
    m_recurringTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_recurringTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_recurringTable->setAlternatingRowColors(true);
    m_recurringTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_recurringTable->verticalHeader()->setVisible(false);

    layout->addWidget(m_recurringTable);

    // Connect signals
    connect(m_addRecurringBtn, &QPushButton::clicked, this, &MainWindow::onAddRecurringPayment);
    connect(m_editRecurringBtn, &QPushButton::clicked, this, &MainWindow::onEditRecurringPayment);
    connect(m_deleteRecurringBtn, &QPushButton::clicked, this, &MainWindow::onDeleteRecurringPayment);
    connect(m_toggleStatusBtn, &QPushButton::clicked, this, &MainWindow::onTogglePaymentStatus);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshRecurringPayments);
    connect(m_recurringTable, &QTableWidget::doubleClicked, this, &MainWindow::onViewPaymentDetails);
}

void MainWindow::setupPaymentInstancesTab()
{
    m_instancesWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_instancesWidget);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);

    // Toolbar
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    m_generateInstancesBtn = new QPushButton("🔄 Generate Instances (Next 90 Days)");
    m_generateInstancesBtn->setStyleSheet("QPushButton { background-color: #9b59b6; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #8e44ad; }");

    m_markPaidBtn = new QPushButton("✅ Mark as Paid");
    m_markFailedBtn = new QPushButton("❌ Mark as Failed");
    m_cancelInstanceBtn = new QPushButton("🚫 Cancel");
    QPushButton *refreshBtn = new QPushButton("🔄 Refresh");

    m_markPaidBtn->setStyleSheet("QPushButton { background-color: #27ae60; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #229954; }");

    toolbarLayout->addWidget(m_generateInstancesBtn);
    toolbarLayout->addWidget(m_markPaidBtn);
    toolbarLayout->addWidget(m_markFailedBtn);
    toolbarLayout->addWidget(m_cancelInstanceBtn);
    toolbarLayout->addWidget(refreshBtn);
    toolbarLayout->addStretch();

    layout->addLayout(toolbarLayout);

    // Table
    m_instancesTable = new QTableWidget();
    m_instancesTable->setColumnCount(7);
    m_instancesTable->setHorizontalHeaderLabels({"Instance ID", "Payment Name", "Amount", "Due Date", "Bank Account", "Status", "Category"});
    m_instancesTable->horizontalHeader()->setStretchLastSection(true);
    m_instancesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_instancesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_instancesTable->setAlternatingRowColors(true);
    m_instancesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_instancesTable->verticalHeader()->setVisible(false);

    layout->addWidget(m_instancesTable);

    // Connect signals
    connect(m_generateInstancesBtn, &QPushButton::clicked, this, &MainWindow::onGenerateInstances);
    connect(m_markPaidBtn, &QPushButton::clicked, this, &MainWindow::onMarkAsPaid);
    connect(m_markFailedBtn, &QPushButton::clicked, this, &MainWindow::onMarkAsFailed);
    connect(m_cancelInstanceBtn, &QPushButton::clicked, this, &MainWindow::onCancelInstance);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshPaymentInstances);
    connect(m_instancesTable, &QTableWidget::doubleClicked, this, &MainWindow::onViewInstanceDetails);
}

void MainWindow::setupUpcomingPaymentsTab()
{
    m_upcomingWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_upcomingWidget);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);

    QLabel *titleLabel = new QLabel("Upcoming Payments (Next 30 Days)");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; padding: 10px;");
    layout->addWidget(titleLabel);

    m_upcomingTable = new QTableWidget();
    m_upcomingTable->setColumnCount(7);
    m_upcomingTable->setHorizontalHeaderLabels({"Due Date", "Payment Name", "Amount", "Bank Account", "Category", "Days Until Due", "Status"});
    m_upcomingTable->horizontalHeader()->setStretchLastSection(true);
    m_upcomingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_upcomingTable->setAlternatingRowColors(true);
    m_upcomingTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_upcomingTable->verticalHeader()->setVisible(false);

    layout->addWidget(m_upcomingTable);
}

void MainWindow::setupOverduePaymentsTab()
{
    m_overdueWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_overdueWidget);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);

    QLabel *titleLabel = new QLabel("⚠️ Overdue Payments - Immediate Action Required");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #e74c3c; padding: 10px; background: #fadbd8; border-radius: 5px;");
    layout->addWidget(titleLabel);

    m_overdueTable = new QTableWidget();
    m_overdueTable->setColumnCount(7);
    m_overdueTable->setHorizontalHeaderLabels({"Due Date", "Payment Name", "Amount", "Bank Account", "Category", "Days Overdue", "Status"});
    m_overdueTable->horizontalHeader()->setStretchLastSection(true);
    m_overdueTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_overdueTable->setAlternatingRowColors(true);
    m_overdueTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_overdueTable->verticalHeader()->setVisible(false);

    layout->addWidget(m_overdueTable);
}

void MainWindow::setupPaymentHistoryTab()
{
    m_historyWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_historyWidget);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);

    // Title and filter toolbar
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("📜 Payment History");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; padding: 10px;");

    QLabel *filterLabel = new QLabel("Filter by Status:");
    m_historyFilterCombo = new QComboBox();
    m_historyFilterCombo->addItem("All", -1);
    m_historyFilterCombo->addItem("Paid", (int)PaymentStatus::PAID);
    m_historyFilterCombo->addItem("Failed", (int)PaymentStatus::FAILED);
    m_historyFilterCombo->addItem("Cancelled", (int)PaymentStatus::CANCELLED);
    m_historyFilterCombo->addItem("Overdue", (int)PaymentStatus::OVERDUE);
    m_historyFilterCombo->setStyleSheet("QComboBox { padding: 5px; border-radius: 3px; }");

    m_refreshHistoryBtn = new QPushButton("🔄 Refresh");
    m_refreshHistoryBtn->setStyleSheet("QPushButton { background-color: #3498db; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #2980b9; }");

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(filterLabel);
    headerLayout->addWidget(m_historyFilterCombo);
    headerLayout->addWidget(m_refreshHistoryBtn);

    layout->addLayout(headerLayout);

    // History table
    m_historyTable = new QTableWidget();
    m_historyTable->setColumnCount(8);
    m_historyTable->setHorizontalHeaderLabels({"Date", "Payment Name", "Recurring Payment", "Amount", "Bank Account", "Status", "Payment Method", "Paid Date"});
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setAlternatingRowColors(true);
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyTable->verticalHeader()->setVisible(false);
    m_historyTable->setSortingEnabled(true);

    layout->addWidget(m_historyTable);

    // Connect signals
    connect(m_refreshHistoryBtn, &QPushButton::clicked, this, &MainWindow::refreshPaymentHistory);
    connect(m_historyFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::refreshPaymentHistory);
}

void MainWindow::refreshPaymentHistory()
{
    m_historyTable->setRowCount(0);

    auto allInstances = m_paymentService->getAllPaymentInstances();

    // Filter by status if a filter is selected
    int filterIndex = m_historyFilterCombo->currentData().toInt();
    std::vector<PaymentInstance> filteredInstances;

    for (const auto& instance : allInstances) {
        if (instance.getStatus() == PaymentStatus::PENDING) {
            continue;
        }

        if (filterIndex == -1 || static_cast<int>(instance.getStatus()) == filterIndex) {
            filteredInstances.push_back(instance);
        }
    }

    // Sort by due date (most recent first)
    std::sort(filteredInstances.begin(), filteredInstances.end(),
              [](const PaymentInstance& a, const PaymentInstance& b) {
                  return a.getDueDate() > b.getDueDate();
              });

    for (const auto& instance : filteredInstances) {
        int row = m_historyTable->rowCount();
        m_historyTable->insertRow(row);

        // Date (due date)
        QString dueDate = QString::fromStdString(DateTimeUtils::formatDateTime(instance.getDueDate(), "%Y-%m-%d"));
        m_historyTable->setItem(row, 0, new QTableWidgetItem(dueDate));

        QString recurringName = QString::fromStdString(instance.getRecurringPaymentId());
        RecurringPayment* recurring = m_paymentService->getRecurringPayment(instance.getRecurringPaymentId());
        if (recurring) {
            recurringName = QString::fromStdString(recurring->getName());
        }

        // Instance ID
        m_historyTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(instance.getName())));

        // Recurring payment name
        m_historyTable->setItem(row, 2, new QTableWidgetItem(recurringName));

        // Amount
        m_historyTable->setItem(row, 3, new QTableWidgetItem(QString("$%1").arg(instance.getAmount(), 0, 'f', 2)));

        // Bank Account
        m_historyTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(instance.getBankAccountNumber())));

        // Status
        QTableWidgetItem *statusItem = new QTableWidgetItem(QString::fromStdString(PaymentTypeUtils::statusToString(instance.getStatus())));
        statusItem->setForeground(QBrush(QColor(getStatusColor(instance.getStatus()))));
        statusItem->setFont(QFont("Arial", 10, QFont::Bold));
        m_historyTable->setItem(row, 5, statusItem);

        // Payment Method
        QString methodStr = "N/A";
        if (instance.getPaymentMethod().has_value()) {
            methodStr = QString::fromStdString(PaymentTypeUtils::paymentMethodString(instance.getPaymentMethod().value()));
        }
        m_historyTable->setItem(row, 6, new QTableWidgetItem(methodStr));

        // Paid Date
        QString paidDateStr = "N/A";
        if (instance.getPaidDate().has_value()) {
            paidDateStr = QString::fromStdString(DateTimeUtils::formatDateTime(instance.getPaidDate().value(), "%Y-%m-%d"));
        }
        m_historyTable->setItem(row, 7, new QTableWidgetItem(paidDateStr));
    }

    m_historyTable->resizeColumnsToContents();
}

void MainWindow::setupMenuBar()
{
    // QMenu *fileMenu = menuBar()->addMenu("&File");
    // QAction *exitAction = fileMenu->addAction("E&xit");
    // connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    // QMenu *viewMenu = menuBar()->addMenu("&View");
    // QAction *refreshAction = viewMenu->addAction("&Refresh All");
    // connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshDashboard);

    // QMenu *helpMenu = menuBar()->addMenu("&Help");
    // QAction *aboutAction = helpMenu->addAction("&About");
    // connect(aboutAction, &QAction::triggered, this, [this]() {
    //     QMessageBox::about(this, "About", "Recurring Payment Schedule Manager\nVersion 1.0\n\nManage your recurring payments efficiently.");
    // });

    QMenu *fileMenu = menuBar()->addMenu("&File");

    QAction *exportAction = fileMenu->addAction("&Export to CSV...");
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportData);

    QAction *importAction = fileMenu->addAction("&Import from CSV...");
    connect(importAction, &QAction::triggered, this, &MainWindow::onImportData);

    fileMenu->addSeparator();

    QAction *backupAction = fileMenu->addAction("&Backup Database...");
    connect(backupAction, &QAction::triggered, this, &MainWindow::onBackupDatabase);

    QAction *restoreAction = fileMenu->addAction("&Restore Database...");
    connect(restoreAction, &QAction::triggered, this, &MainWindow::onRestoreDatabase);

    fileMenu->addSeparator();

    QAction *exitAction = fileMenu->addAction("E&xit");
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    QMenu *viewMenu = menuBar()->addMenu("&View");
    QAction *refreshAction = viewMenu->addAction("&Refresh All");
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshDashboard);

    viewMenu->addSeparator();
    QAction *checkRemindersAction = viewMenu->addAction("Check Payment &Reminders");
    checkRemindersAction->setShortcut(QKeySequence("Ctrl+R"));
    connect(checkRemindersAction, &QAction::triggered, this, &MainWindow::showPaymentReminders);

    QAction *statsAction = viewMenu->addAction("&Database Statistics");
    connect(statsAction, &QAction::triggered, this, &MainWindow::onShowDatabaseStats);

    QMenu *helpMenu = menuBar()->addMenu("&Help");
    QAction *aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About",
                           "Recurring Payment Schedule Manager\n\n"
                           "Features:\n"
                           "• SQLite database storage\n"
                           "• Recurring payment schedules\n"
                           "• Payment tracking and history\n"
                           "• Export/Import capabilities\n"
                           "• Database backup and restore\n\n"
                           "Manage your recurring payments efficiently.");
    });
}

void MainWindow::onGenerateInstances()
{
    auto now = DateTimeUtils::now();
    auto endDate = now + std::chrono::hours(24 * 90);

    try {
        size_t countBefore = m_paymentService->getAllPaymentInstances().size();
        m_paymentService->generatePaymentsUpTo(endDate);

        size_t countAfter = m_paymentService->getAllPaymentInstances().size();
        int generatedCount = countAfter - countBefore;

        refreshPaymentInstances();
        refreshDashboard();

        m_notifiedInstances.clear();

        // Check for due payments after generating instances
        checkDuePaymentsAndNotify();

        if (generatedCount > 0) {
            QMessageBox::information(this, "Success",
                                     QString("Generated %1 new payment instances for the next 90 days!").arg(generatedCount));
        } else {
            QMessageBox::information(this, "Info",
                                     "No new instances to generate. All upcoming payments are already created.");
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error",
                              QString("Failed to generate instances: %1").arg(e.what()));
    }
}



void MainWindow::applyModernStyle()
{
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f5f7fa;
        }
        QTabWidget::pane {
            border: 1px solid #d0d0d0;
            background: white;
            border-radius: 5px;
        }
        QTabBar::tab {
            background: #e8e8e8;
            color: #333;
            padding: 10px 20px;
            margin-right: 2px;
            border-top-left-radius: 5px;
            border-top-right-radius: 5px;
            font-weight: 500;
        }
        QTabBar::tab:selected {
            background: white;
            color: #2c3e50;
            font-weight: bold;
        }
        QTabBar::tab:hover {
            background: #d5d5d5;
        }
        QTableWidget {
            border: 1px solid #ddd;
            border-radius: 5px;
            gridline-color: #e0e0e0;
            background-color: white;
            selection-background-color: #3498db;
            font-size: 13px;
        }
        QTableWidget::item {
            padding: 8px;
        }
        QHeaderView::section {
            background-color: #34495e;
            color: white;
            padding: 10px;
            border: none;
            font-weight: bold;
            font-size: 13px;
        }
        QPushButton {
            background-color: #3498db;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 5px;
            font-weight: 500;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #2980b9;
        }
        QPushButton:pressed {
            background-color: #21618c;
        }
        QPushButton:disabled {
            background-color: #bdc3c7;
        }
    )");
}

QString MainWindow::getCategoryColor(PaymentCategory category)
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

QString MainWindow::getStatusColor(PaymentStatus status)
{
    switch (status) {
    case PaymentStatus::PENDING: return "#f39c12";
    case PaymentStatus::PAID: return "#27ae60";
    case PaymentStatus::OVERDUE: return "#e74c3c";
    case PaymentStatus::FAILED: return "#c0392b";
    case PaymentStatus::CANCELLED: return "#95a5a6";
    case PaymentStatus::PROCESSING: return "#3498db";
    default: return "#000000";
    }
}

void MainWindow::refreshRecurringPayments()
{
    m_recurringTable->setRowCount(0);
    auto payments = m_paymentService->getAllRecurringPayments();

    for (const auto& payment : payments) {
        int row = m_recurringTable->rowCount();
        m_recurringTable->insertRow(row);

        m_recurringTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(payment.getId())));
        m_recurringTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(payment.getName())));
        m_recurringTable->setItem(row, 2, new QTableWidgetItem(QString("$%1").arg(payment.getAmount(), 0, 'f', 2)));
        m_recurringTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(payment.getBankAccountNumber())));

        QTableWidgetItem *categoryItem = new QTableWidgetItem(QString::fromStdString(PaymentTypeUtils::categoryToString(payment.getCategory())));
        categoryItem->setForeground(QBrush(QColor(getCategoryColor(payment.getCategory()))));
        categoryItem->setFont(QFont("Arial", 10, QFont::Bold));
        m_recurringTable->setItem(row, 4, categoryItem);

        m_recurringTable->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(payment.getScheduleDescription())));

        QTableWidgetItem *statusItem = new QTableWidgetItem(payment.isActive() ? "✅ Active" : "⏸️ Inactive");
        statusItem->setForeground(QBrush(QColor(payment.isActive() ? "#27ae60" : "#95a5a6")));
        statusItem->setFont(QFont("Arial", 10, QFont::Bold));
        m_recurringTable->setItem(row, 6, statusItem);

        auto nextPayment = m_paymentService->getNextPayment(payment.getId());
        if (nextPayment) {
            QString nextDate = QString::fromStdString(DateTimeUtils::formatDateTime(nextPayment->getDueDate(), "%Y-%m-%d"));
            m_recurringTable->setItem(row, 7, new QTableWidgetItem(nextDate));
        } else {
            m_recurringTable->setItem(row, 7, new QTableWidgetItem("N/A"));
        }
    }

    m_recurringTable->resizeColumnsToContents();
}

void MainWindow::refreshPaymentInstances()
{
    m_instancesTable->setRowCount(0);
    auto instances = m_paymentService->getAllPaymentInstances();

    for (const auto& instance : instances) {
        int row = m_instancesTable->rowCount();
        m_instancesTable->insertRow(row);

        m_instancesTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(instance.getName())));
        m_instancesTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(instance.getRecurringPaymentId())));
        m_instancesTable->setItem(row, 2, new QTableWidgetItem(QString("$%1").arg(instance.getAmount(), 0, 'f', 2)));
        m_instancesTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(DateTimeUtils::formatDateTime(instance.getDueDate(), "%Y-%m-%d"))));
        m_instancesTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(instance.getBankAccountNumber())));

        QTableWidgetItem *statusItem = new QTableWidgetItem(QString::fromStdString(PaymentTypeUtils::statusToString(instance.getStatus())));
        statusItem->setForeground(QBrush(QColor(getStatusColor(instance.getStatus()))));
        statusItem->setFont(QFont("Arial", 10, QFont::Bold));
        m_instancesTable->setItem(row, 5, statusItem);

        QTableWidgetItem *categoryItem = new QTableWidgetItem(QString::fromStdString(PaymentTypeUtils::categoryToString(instance.getCategory())));
        categoryItem->setForeground(QBrush(QColor(getCategoryColor(instance.getCategory()))));
        m_instancesTable->setItem(row, 6, categoryItem);
    }

    m_instancesTable->resizeColumnsToContents();
}

void MainWindow::refreshUpcomingPayments()
{
    m_upcomingTable->setRowCount(0);
    auto now = DateTimeUtils::now();
    auto endDate = now + std::chrono::hours(24 * 30); // 30 days
    auto upcoming = m_paymentService->getUpcomingPayments(now, endDate);

    for (const auto& payment : upcoming) {
        int row = m_upcomingTable->rowCount();
        m_upcomingTable->insertRow(row);

        QString dueDate = QString::fromStdString(DateTimeUtils::formatDateTime(payment.getDueDate(), "%Y-%m-%d"));
        m_upcomingTable->setItem(row, 0, new QTableWidgetItem(dueDate));
        m_upcomingTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(payment.getRecurringPaymentId())));
        m_upcomingTable->setItem(row, 2, new QTableWidgetItem(QString("$%1").arg(payment.getAmount(), 0, 'f', 2)));
        m_upcomingTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(payment.getBankAccountNumber())));

        QTableWidgetItem *categoryItem = new QTableWidgetItem(QString::fromStdString(PaymentTypeUtils::categoryToString(payment.getCategory())));
        categoryItem->setForeground(QBrush(QColor(getCategoryColor(payment.getCategory()))));
        m_upcomingTable->setItem(row, 4, categoryItem);

        auto duration = std::chrono::duration_cast<std::chrono::hours>(payment.getDueDate() - now);
        int daysUntil = duration.count() / 24;
        m_upcomingTable->setItem(row, 5, new QTableWidgetItem(QString::number(daysUntil)));

        QTableWidgetItem *statusItem = new QTableWidgetItem(QString::fromStdString(PaymentTypeUtils::statusToString(payment.getStatus())));
        statusItem->setForeground(QBrush(QColor(getStatusColor(payment.getStatus()))));
        m_upcomingTable->setItem(row, 6, statusItem);
    }

    m_upcomingTable->resizeColumnsToContents();
}

void MainWindow::refreshOverduePayments()
{
    m_overdueTable->setRowCount(0);
    auto overdue = m_paymentService->getOverduePayments();
    auto now = DateTimeUtils::now();

    for (const auto& payment : overdue) {
        int row = m_overdueTable->rowCount();
        m_overdueTable->insertRow(row);

        QString dueDate = QString::fromStdString(DateTimeUtils::formatDateTime(payment.getDueDate(), "%Y-%m-%d"));
        QTableWidgetItem *dueDateItem = new QTableWidgetItem(dueDate);
        dueDateItem->setBackground(QBrush(QColor("#fadbd8")));
        m_overdueTable->setItem(row, 0, dueDateItem);

        m_overdueTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(payment.getRecurringPaymentId())));
        m_overdueTable->setItem(row, 2, new QTableWidgetItem(QString("$%1").arg(payment.getAmount(), 0, 'f', 2)));
        m_overdueTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(payment.getBankAccountNumber())));

        QTableWidgetItem *categoryItem = new QTableWidgetItem(QString::fromStdString(PaymentTypeUtils::categoryToString(payment.getCategory())));
        categoryItem->setForeground(QBrush(QColor(getCategoryColor(payment.getCategory()))));
        m_overdueTable->setItem(row, 4, categoryItem);

        auto duration = std::chrono::duration_cast<std::chrono::hours>(now - payment.getDueDate());
        int daysOverdue = duration.count() / 24;
        QTableWidgetItem *overdueItem = new QTableWidgetItem(QString::number(daysOverdue));
        overdueItem->setForeground(QBrush(QColor("#e74c3c")));
        overdueItem->setFont(QFont("Arial", 10, QFont::Bold));
        m_overdueTable->setItem(row, 5, overdueItem);

        QTableWidgetItem *statusItem = new QTableWidgetItem(QString::fromStdString(PaymentTypeUtils::statusToString(payment.getStatus())));
        statusItem->setForeground(QBrush(QColor("#e74c3c")));
        statusItem->setFont(QFont("Arial", 10, QFont::Bold));
        m_overdueTable->setItem(row, 6, statusItem);
    }

    m_overdueTable->resizeColumnsToContents();
}

void MainWindow::checkDuePaymentsAndNotify()
{
    auto pending = m_paymentService->getPendingPayments();
    QSet<QString> currentPending;
    std::vector<PaymentInstance> duePayments;
    //auto now = DateTimeUtils::now();

    for (const auto& payment : pending) {
        QString instanceId = QString::fromStdString(payment.getName());
        currentPending.insert(instanceId);

        bool isDue = payment.isDueToday() || payment.isOverdue();
        bool alreadyNotified = m_notifiedInstances.contains(instanceId);

        if (!isDue) {
            continue;
        }

        if(alreadyNotified){
            continue;
        }

        duePayments.push_back(payment);
        m_notifiedInstances.insert(instanceId);
    }

    // Drop notifications for items that are no longer pending
    m_notifiedInstances = m_notifiedInstances.intersect(currentPending);

    if (!duePayments.empty() && m_notificationCenter) {
        QMessageBox::warning(this, "Warning", "Please work.");
        m_notificationCenter->notifyPaymentsDue(duePayments);
    }
}

void MainWindow::onPaymentsDue(const std::vector<PaymentInstance>& duePayments)
{
    if (duePayments.empty()) return;

    PaymentNotificationDialog dialog(duePayments, m_paymentService.get(), this);
    connect(&dialog, &PaymentNotificationDialog::paymentMarkedAsPaid,
            this, &MainWindow::refreshDashboard);

    dialog.exec();
    // if (duePayments.empty()) return;

    // QStringList messages;
    // for (const auto& payment : duePayments) {
    //     QString message = QString("Payment: %1\nAmount: $%2\nDue: %3\nBank Account: %4")
    //                           .arg(QString::fromStdString(payment.getRecurringPaymentId()))
    //                           .arg(payment.getAmount(), 0, 'f', 2)
    //                           .arg(QString::fromStdString(DateTimeUtils::formatDateTime(payment.getDueDate(), "%Y-%m-%d")))
    //                           .arg(QString::fromStdString(payment.getBankAccountNumber()));
    //     messages << message;
    // }

    // QMessageBox::information(this, "Payments Due", messages.join("\n\n"));
}

void MainWindow::refreshDashboard()
{
    auto allRecurring = m_paymentService->getAllRecurringPayments();
    auto now = DateTimeUtils::now();
    auto endDate = now + std::chrono::hours(24 * 30);
    auto upcoming = m_paymentService->getUpcomingPayments(now, endDate);
    auto overdue = m_paymentService->getOverduePayments();

    m_totalRecurringLabel->setText(QString::number(allRecurring.size()));

    int activeCount = 0;
    double monthlyTotal = 0.0;
    for (const auto& payment : allRecurring) {
        if (payment.isActive()) {
            activeCount++;
            monthlyTotal += payment.getAmount(); // Simplified monthly estimate
        }
    }

    m_activePaymentsLabel->setText(QString::number(activeCount));
    m_upcomingCountLabel->setText(QString::number(upcoming.size()));
    m_overdueCountLabel->setText(QString::number(overdue.size()));
    m_monthlyTotalLabel->setText(QString("$%1").arg(monthlyTotal, 0, 'f', 2));

    refreshRecurringPayments();
    refreshUpcomingPayments();
    refreshOverduePayments();
    checkDuePaymentsAndNotify();
}

void MainWindow::onTabChanged(int index)
{
    switch (index) {
    case 0: refreshDashboard(); break;
    case 1: refreshRecurringPayments(); break;
    case 2: refreshPaymentInstances(); break;
    case 3: refreshUpcomingPayments(); break;
    case 4: refreshOverduePayments(); break;
    case 5: refreshPaymentHistory(); break;
    }
}

void MainWindow::onAddRecurringPayment()
{
    AddRecurringPaymentDialog dialog(m_paymentService.get(), this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshRecurringPayments();
        refreshDashboard();
        QMessageBox::information(this, "Success", "Recurring payment added successfully!");
    }
}

void MainWindow::onEditRecurringPayment()
{
    int row = m_recurringTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Warning", "Please select a payment to edit.");
        return;
    }

    QString id = m_recurringTable->item(row, 0)->text();
    RecurringPayment* payment = m_paymentService->getRecurringPayment(id.toStdString());

    if (payment) {
        EditRecurringPaymentDialog dialog(m_paymentService.get(), payment, this);
        if (dialog.exec() == QDialog::Accepted) {
            refreshRecurringPayments();
            refreshDashboard();
            QMessageBox::information(this, "Success", "Payment updated successfully!");
        }
    }
}

void MainWindow::onDeleteRecurringPayment()
{
    int row = m_recurringTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Warning", "Please select a payment to delete.");
        return;
    }

    QString id = m_recurringTable->item(row, 0)->text();
    QString name = m_recurringTable->item(row, 1)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete",
                                                              QString("Are you sure you want to delete '%1'?").arg(name),
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_paymentService->deleteRecurringPayment(id.toStdString())) {
            refreshRecurringPayments();
            refreshDashboard();
            QMessageBox::information(this, "Success", "Payment deleted successfully!");
        }
    }
}

void MainWindow::onTogglePaymentStatus()
{
    int row = m_recurringTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Warning", "Please select a payment.");
        return;
    }

    QString id = m_recurringTable->item(row, 0)->text();
    RecurringPayment* payment = m_paymentService->getRecurringPayment(id.toStdString());

    if (payment) {
        if (payment->isActive()) {
            m_paymentService->deactivateRecurringPayment(id.toStdString());
        } else {
            m_paymentService->activateRecurringPayment(id.toStdString());
        }
        refreshRecurringPayments();
        refreshDashboard();
    }
}

void MainWindow::onViewPaymentDetails()
{
    int row = m_recurringTable->currentRow();
    if (row < 0) return;

    QString id = m_recurringTable->item(row, 0)->text();
    RecurringPayment* payment = m_paymentService->getRecurringPayment(id.toStdString());

    if (payment) {
        PaymentDetailsDialog dialog(payment, this);
        dialog.exec();
    }
}

void MainWindow::onMarkAsPaid()
{
    int row = m_instancesTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Warning", "Please select a payment instance.");
        return;
    }

    QString id = m_instancesTable->item(row, 0)->text();

    try {
        m_paymentService->markPaymentAsPaid(id.toStdString());
        m_notifiedInstances.remove(id);
        refreshPaymentInstances();
        refreshDashboard();
        QMessageBox::information(this, "Success", "Payment marked as paid!");
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void MainWindow::onMarkAsFailed()
{
    int row = m_instancesTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Warning", "Please select a payment instance.");
        return;
    }

    QString id = m_instancesTable->item(row, 0)->text();

    try {
        m_paymentService->markPaymentAsFailed(id.toStdString(), "Failed by user");
        m_notifiedInstances.remove(id);
        refreshPaymentInstances();
        refreshDashboard();
        QMessageBox::information(this, "Success", "Payment marked as failed!");
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void MainWindow::onCancelInstance()
{
    int row = m_instancesTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Warning", "Please select a payment instance.");
        return;
    }

    QString id = m_instancesTable->item(row, 0)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Cancel",
                                                              "Are you sure you want to cancel this payment instance?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        try {
            m_paymentService->cancelPaymentInstance(id.toStdString());
            m_notifiedInstances.remove(id);
            refreshPaymentInstances();
            refreshDashboard();
            QMessageBox::information(this, "Success", "Payment cancelled!");
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", e.what());
        }
    }
}

void MainWindow::onViewInstanceDetails()
{
    int row = m_instancesTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Info", "Select a payment instance to view details.");
        return;
    }

    QString id = m_instancesTable->item(row, 0)->text();
    auto instances = m_paymentService->getAllPaymentInstances();
    auto it = std::find_if(instances.begin(), instances.end(), [&](const PaymentInstance& inst) {
        return inst.getName() == id.toStdString();
    });

    if (it == instances.end()) {
        QMessageBox::information(this, "Instance Details",
                                 QString("Instance ID: %1\nBank Account: %2")
                                     .arg(id)
                                     .arg(m_instancesTable->item(row, 4)->text()));
        return;
    }

    const PaymentInstance& instance = *it;

    QString details = QString("Instance ID: %1\nPayment: %2\nAmount: $%3\nDue: %4\nStatus: %5\nBank Account: %6")
                          .arg(QString::fromStdString(instance.getName()))
                          .arg(QString::fromStdString(instance.getRecurringPaymentId()))
                          .arg(instance.getAmount(), 0, 'f', 2)
                          .arg(QString::fromStdString(DateTimeUtils::formatDateTime(instance.getDueDate(), "%Y-%m-%d")))
                          .arg(QString::fromStdString(PaymentTypeUtils::statusToString(instance.getStatus())))
                          .arg(QString::fromStdString(instance.getBankAccountNumber()));

    QMessageBox::information(this, "Instance Details", details);
}


void MainWindow::onExportData()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Export Payments to CSV",
                                                    "payments_export.csv",
                                                    "CSV Files (*.csv)");

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Export Error",
                              "Failed to open file for writing: " + file.errorString());
        return;
    }

    QTextStream out(&file);

    // Export recurring payments
    out << "=== RECURRING PAYMENTS ===\n";
    out << "ID,Name,Amount,Bank Account,Category,Schedule,Active,Start Date,End Date\n";

    auto payments = m_paymentService->getAllRecurringPayments();
    for (const auto& payment : payments) {
        out << QString::fromStdString(payment.getId()) << ","
            << QString::fromStdString(payment.getName()) << ","
            << payment.getAmount() << ","
            << QString::fromStdString(payment.getBankAccountNumber()) << ","
            << QString::fromStdString(PaymentTypeUtils::categoryToString(payment.getCategory())) << ","
            << QString::fromStdString(payment.getScheduleDescription()) << ","
            << (payment.isActive() ? "Active" : "Inactive") << ","
            << QString::fromStdString(DateTimeUtils::formatDateTime(payment.getStartDate())) << ",";

        if (payment.getEndDate().has_value()) {
            out << QString::fromStdString(DateTimeUtils::formatDateTime(payment.getEndDate().value()));
        }
        out << "\n";
    }

    // Export payment instances
    out << "\n=== PAYMENT INSTANCES ===\n";
    out << "ID,Recurring Payment ID,Amount,Due Date,Bank Account,Status,Category,Payment Method,Paid Date\n";

    auto instances = m_paymentService->getAllPaymentInstances();
    for (const auto& instance : instances) {
        out << QString::fromStdString(instance.getName()) << ","
            << QString::fromStdString(instance.getRecurringPaymentId()) << ","
            << instance.getAmount() << ","
            << QString::fromStdString(DateTimeUtils::formatDateTime(instance.getDueDate())) << ","
            << QString::fromStdString(instance.getBankAccountNumber()) << ","
            << QString::fromStdString(PaymentTypeUtils::statusToString(instance.getStatus())) << ","
            << QString::fromStdString(PaymentTypeUtils::categoryToString(instance.getCategory())) << ",";

        if (instance.getPaymentMethod().has_value()) {
            out << QString::fromStdString(PaymentTypeUtils::paymentMethodString(instance.getPaymentMethod().value()));
        }
        out << ",";

        if (instance.getPaidDate().has_value()) {
            out << QString::fromStdString(DateTimeUtils::formatDateTime(instance.getPaidDate().value()));
        }
        out << "\n";
    }

    // Export payment history (completed payments only)
    out << "\n=== PAYMENT HISTORY (COMPLETED) ===\n";
    out << "Instance ID,Payment Name,Recurring Payment ID,Amount,Due Date,Bank Account,Status,Category,Payment Method,Paid Date,Completion Date\n";

    std::vector<PaymentInstance> historyInstances;
    for (const auto& instance : instances) {
        if (instance.getStatus() != PaymentStatus::PENDING) {
            historyInstances.push_back(instance);
        }
    }

    // Sort by due date (most recent first)
    std::sort(historyInstances.begin(), historyInstances.end(),
              [](const PaymentInstance& a, const PaymentInstance& b) {
                  return a.getDueDate() > b.getDueDate();
              });

    for (const auto& instance : historyInstances) {
        QString recurringName = QString::fromStdString(instance.getRecurringPaymentId());
        RecurringPayment* recurring = m_paymentService->getRecurringPayment(instance.getRecurringPaymentId());
        if (recurring) {
            recurringName = QString::fromStdString(recurring->getName());
        }

        out << QString::fromStdString(instance.getName()) << ","
            << recurringName << ","
            << QString::fromStdString(instance.getRecurringPaymentId()) << ","
            << instance.getAmount() << ","
            << QString::fromStdString(DateTimeUtils::formatDateTime(instance.getDueDate(), "%Y-%m-%d")) << ","
            << QString::fromStdString(instance.getBankAccountNumber()) << ","
            << QString::fromStdString(PaymentTypeUtils::statusToString(instance.getStatus())) << ","
            << QString::fromStdString(PaymentTypeUtils::categoryToString(instance.getCategory())) << ",";

        if (instance.getPaymentMethod().has_value()) {
            out << QString::fromStdString(PaymentTypeUtils::paymentMethodString(instance.getPaymentMethod().value()));
        }
        out << ",";

        if (instance.getPaidDate().has_value()) {
            out << QString::fromStdString(DateTimeUtils::formatDateTime(instance.getPaidDate().value(), "%Y-%m-%d"));
        }
        out << ",";

        // Use paid date as completion date if available, otherwise use current date for other statuses
        if (instance.isPaid() && instance.getPaidDate().has_value()) {
            out << QString::fromStdString(DateTimeUtils::formatDateTime(instance.getPaidDate().value(), "%Y-%m-%d"));
        } else if (!instance.isPending()) {
            out << QString::fromStdString(DateTimeUtils::formatDateTime(DateTimeUtils::now(), "%Y-%m-%d"));
        }
        out << "\n";
    }

    file.close();

    int historyCount = historyInstances.size();
    int pendingCount = instances.size() - historyCount;

    QMessageBox::information(this, "Export Successful",
                             QString("Data exported successfully to:\n%1\n\n"
                                     "Recurring Payments: %2\n"
                                     "All Payment Instances: %3\n"
                                     "  - Pending: %4\n"
                                     "  - History (Completed): %5")
                                 .arg(fileName)
                                 .arg(payments.size())
                                 .arg(instances.size())
                                 .arg(pendingCount)
                                 .arg(historyCount));
}

void MainWindow::onImportData()
{
    QMessageBox::information(this, "Import Data",
                             "Import functionality is available but requires careful CSV formatting.\n\n"
                             "For safety, please use the 'Add Payment' button to create new payments manually.");
}

void MainWindow::onBackupDatabase()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Backup Database",
                                                    QString("payments_backup_%1.db").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
                                                    "Database Files (*.db)");

    if (fileName.isEmpty()) {
        return;
    }

    try
    {
        QFile dbFile("payments.db");
        if(!dbFile.exists()){
            QMessageBox::warning(this, "Backup Warning",
                                 "No database file found. You might be using in-memory storage.\n"
                                 "Use Export to CSV instead for backing up in-memory data.");
            return;
        }

        m_paymentService.reset();
        QThread::msleep(100);

        if(QFile::exists(fileName)){
            QFile::remove(fileName);
        }

        // Copy the database file
        if (QFile::copy("payments.db", fileName)) {
            // Reinitialize the service with SQLite
            auto repo = std::make_unique<SQLiteRepository>("payments.db");
            m_paymentService = std::make_unique<PaymentService>(std::move(repo));

            refreshDashboard();

            QMessageBox::information(this, "Backup Successful",
                                     QString("Database backed up successfully to:\n%1").arg(fileName));
        } else {
            // Restore the connection even if backup failed
            auto repo = std::make_unique<SQLiteRepository>("payments.db");
            m_paymentService = std::make_unique<PaymentService>(std::move(repo));

            QMessageBox::critical(this, "Backup Failed",
                                  QString("Failed to create database backup.\nError: %1")
                                      .arg(dbFile.errorString()));
        }
    }
    catch(const std::exception& e){
        // Try to restore the connection
        try {
            auto repo = std::make_unique<SQLiteRepository>("payments.db");
            m_paymentService = std::make_unique<PaymentService>(std::move(repo));
        } catch (...) {
            // If can't restore, use in-memory
            auto repo = std::make_unique<InMemoryRepository>();
            m_paymentService = std::make_unique<PaymentService>(std::move(repo));
        }

        QMessageBox::critical(this, "Backup Error",
                              QString("An error occurred during backup: %1").arg(e.what()));
    }
}

void MainWindow::onRestoreDatabase()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Restore Database",
                                                    "",
                                                    "Database Files (*.db)");

    if (fileName.isEmpty()) {
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              "Confirm Restore",
                                                              "Restoring will replace all current data. Are you sure?\n\n"
                                                              "It's recommended to create a backup first.",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    m_paymentService.reset();

    // Remove old database
    QFile::remove("payments.db");

    // Copy backup to current database
    if (QFile::copy(fileName, "payments.db")) {
        try {
            auto repo = std::make_unique<SQLiteRepository>("payments.db");
            m_paymentService = std::make_unique<PaymentService>(std::move(repo));

            refreshDashboard();

            QMessageBox::information(this, "Restore Successful",
                                     "Database restored successfully from:\n" + fileName);
        }
        catch (const std::exception& e) {
            QMessageBox::critical(this, "Restore Failed",
                                  QString("Failed to restore database: %1").arg(e.what()));
        }
    } else {
        QMessageBox::critical(this, "Restore Failed",
                              "Failed to copy database file.");
    }
}

void MainWindow::onShowDatabaseStats()
{
    auto allPayments = m_paymentService->getAllRecurringPayments();
    auto allInstances = m_paymentService->getAllPaymentInstances();

    int activeCount = 0;
    int inactiveCount = 0;
    for (const auto& payment : allPayments) {
        if (payment.isActive()) activeCount++;
        else inactiveCount++;
    }

    int pendingCount = 0;
    int paidCount = 0;
    int overdueCount = 0;
    int failedCount = 0;

    for (const auto& instance : allInstances) {
        switch (instance.getStatus()) {
        case PaymentStatus::PENDING: pendingCount++; break;
        case PaymentStatus::PAID: paidCount++; break;
        case PaymentStatus::OVERDUE: overdueCount++; break;
        case PaymentStatus::FAILED: failedCount++; break;
        default: break;
        }
    }

    QString stats = QString(
                        "📊 Database Statistics\n\n"
                        "Recurring Payments:\n"
                        "  • Total: %1\n"
                        "  • Active: %2\n"
                        "  • Inactive: %3\n\n"
                        "Payment Instances:\n"
                        "  • Total: %4\n"
                        "  • Pending: %5\n"
                        "  • Paid: %6\n"
                        "  • Overdue: %7\n"
                        "  • Failed: %8\n\n"
                        "Database: payments.db\n"
                        "Storage: SQLite"
                        ).arg(allPayments.size())
                        .arg(activeCount)
                        .arg(inactiveCount)
                        .arg(allInstances.size())
                        .arg(pendingCount)
                        .arg(paidCount)
                        .arg(overdueCount)
                        .arg(failedCount);

    QMessageBox::information(this, "Database Statistics", stats);
}

void MainWindow::showPaymentReminders()
{
    // First, ensure we have generated instances for today
    auto now = DateTimeUtils::now();
    auto todayEnd = now + std::chrono::hours(24);

    try {
        m_paymentService->generatePaymentsUpTo(todayEnd);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error",
                              QString("Failed to generate payment instances: %1").arg(e.what()));
        return;
    }

    auto pending = m_paymentService->getPendingPayments();
    auto due = m_paymentService->getPaymentInstancesByStatus(PaymentStatus::OVERDUE);

    for(const auto& d : due){
        pending.push_back(d);
    }

    std::vector<PaymentInstance> duePayments;

    for (const auto& payment : pending) {
        if (payment.isDueToday() || payment.isOverdue()) {
            duePayments.push_back(payment);
        }
    }

    if (!duePayments.empty()) {
        PaymentReminderDialog *dialog = new PaymentReminderDialog(duePayments, this);

        connect(dialog, &PaymentReminderDialog::markPaymentAsPaid,
                this, &MainWindow::onReminderMarkAsPaid);

        connect(dialog, &PaymentReminderDialog::snoozeReminder, this, [this]() {
            m_reminderCheckTimer->stop();
            m_reminderCheckTimer->start(1800000); // 30 minutes
        });

        dialog->exec();
        delete dialog;

        // Restart normal timer after dialog closes
        m_reminderCheckTimer->stop();
        m_reminderCheckTimer->start(300000); // Back to 5 minutes
    } else {
        QMessageBox::information(this, "No Reminders",
                                 "You have no payments due today or overdue.");
    }

    // auto now = DateTimeUtils::now();
    // auto pending = m_paymentService->getPendingPayments();

    // std::vector<PaymentInstance> duePayments;

    // for (const auto& payment : pending) {
    //     if (payment.isDueToday() || payment.isOverdue()) {
    //         duePayments.push_back(payment);
    //     }
    // }

    // if (!duePayments.empty()) {
    //     PaymentReminderDialog *dialog = new PaymentReminderDialog(duePayments, this);

    //     connect(dialog, &PaymentReminderDialog::markPaymentAsPaid,
    //             this, &MainWindow::onReminderMarkAsPaid);

    //     connect(dialog, &PaymentReminderDialog::snoozeReminder, this, [this]() {
    //         m_reminderCheckTimer->stop();
    //         m_reminderCheckTimer->start(1800000); // 30 minutes
    //     });

    //     dialog->exec();
    //     delete dialog;

    //     // Restart normal timer after dialog closes
    //     m_reminderCheckTimer->stop();
    //     m_reminderCheckTimer->start(300000); // Back to 5 minutes
    // }
}

void MainWindow::onReminderMarkAsPaid(const std::string& instanceId)
{
    try {
        m_paymentService->markPaymentAsPaid(instanceId);
        m_notifiedInstances.remove(QString::fromStdString(instanceId));

        // Refresh all views
        refreshPaymentInstances();
        refreshDashboard();
        refreshPaymentHistory();

        QMessageBox::information(this, "Success", "Payment marked as paid!");
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error",
                              QString("Failed to mark payment as paid: %1").arg(e.what()));
    }
}
