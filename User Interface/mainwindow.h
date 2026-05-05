#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTimer>
#include <QComboBox>
#include <memory>
#include <QSet>
#include <vector>
#include "NotificationCenter.h"
#include "PaymentService.h"
#include "InMemoryRepository.h"

class MainWindow : public QMainWindow, public NotificationObserver
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddRecurringPayment();
    void onEditRecurringPayment();
    void onDeleteRecurringPayment();
    void onTogglePaymentStatus();
    void onViewPaymentDetails();

    void onMarkAsPaid();
    void onMarkAsFailed();
    void onCancelInstance();
    void onViewInstanceDetails();
    void onGenerateInstances();

    void refreshRecurringPayments();
    void refreshPaymentInstances();
    void refreshUpcomingPayments();
    void refreshOverduePayments();
    void refreshPaymentHistory();
    void refreshDashboard();

    void onTabChanged(int index);

    // Database operations
    void onExportData();
    void onImportData();
    void onBackupDatabase();
    void onRestoreDatabase();
    void onShowDatabaseStats();

    // NotificationObserver
    void onPaymentsDue(const std::vector<PaymentInstance>& duePayments) override;

    void onReminderMarkAsPaid(const std::string& instanceId);
private:
    void setupUI();
    void setupMenuBar();
    void setupDashboard();
    void setupRecurringPaymentsTab();
    void setupPaymentInstancesTab();
    void setupUpcomingPaymentsTab();
    void setupOverduePaymentsTab();
    void setupPaymentHistoryTab();
    void checkDuePaymentsAndNotify();

    void applyModernStyle();
    QString getCategoryColor(PaymentCategory category);
    QString getStatusColor(PaymentStatus status);

    // Core service
    std::unique_ptr<PaymentService> m_paymentService;

    // Main widgets
    QTabWidget *m_tabWidget;

    // Dashboard widgets
    QWidget *m_dashboardWidget;
    QLabel *m_totalRecurringLabel;
    QLabel *m_activePaymentsLabel;
    QLabel *m_upcomingCountLabel;
    QLabel *m_overdueCountLabel;
    QLabel *m_monthlyTotalLabel;

    // Recurring payments tab
    QWidget *m_recurringWidget;
    QTableWidget *m_recurringTable;
    QPushButton *m_addRecurringBtn;
    QPushButton *m_editRecurringBtn;
    QPushButton *m_deleteRecurringBtn;
    QPushButton *m_toggleStatusBtn;
    QPushButton* m_generateInstancesBtn;

    // Payment instances tab
    QWidget *m_instancesWidget;
    QTableWidget *m_instancesTable;
    QPushButton *m_markPaidBtn;
    QPushButton *m_markFailedBtn;
    QPushButton *m_cancelInstanceBtn;

    // Upcoming payments tab
    QWidget *m_upcomingWidget;
    QTableWidget *m_upcomingTable;

    // Overdue payments tab
    QWidget *m_overdueWidget;
    QTableWidget *m_overdueTable;

    // Payment history tab
    QWidget *m_historyWidget;
    QTableWidget *m_historyTable;
    QPushButton *m_refreshHistoryBtn;
    QComboBox *m_historyFilterCombo;

    // Auto-refresh timer
    QTimer *m_refreshTimer;
    QSet<QString> m_notifiedInstances;
    std::unique_ptr<NotificationCenter> m_notificationCenter;

    QTimer *m_reminderCheckTimer;  // New timer for checking reminders
    void showPaymentReminders();   // New method to show reminder dialog
};

#endif // MAINWINDOW_H
