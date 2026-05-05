# Recurring Payment Schedule Manager
The Recurring Payment Schedule Manager is a Qt-based desktop application designed to manage recurring payments with automatic scheduling, due date tracking, notifications, and payment history. 
The application follows a layered architecture with clear separation of concerns and employs several design patterns to ensure maintainability, extensibility, and testability.

### Component Architecture

#### Core Domain Models
- **RecurringPayment**: Represents a recurring payment template with schedule, amount, category, and bank account
- **PaymentInstance**: Represents a specific payment occurrence with due date, status, and payment details
- **RecurringSchedule**: Abstract base for different schedule types (Daily, Weekly, Monthly, Yearly)
- **PaymentTypes**: Enumerations for PaymentStatus, PaymentCategory, PaymentMethod

#### Service Layer
- **PaymentService**: Central business logic service that orchestrates payment operations
- **PaymentGenerator**: Generates payment instances based on recurring payment schedules
- **NotificationCenter**: Manages payment due notifications using Observer pattern

#### Data Layer
- **IPaymentRepository**: Abstract interface defining data access operations
- **InMemoryRepository**: In-memory implementation for testing/development
- **SQLiteRepository**: Persistent storage using SQLite database

#### Presentation Layer
- **MainWindow**: Main application window with tabbed interface
- **AddRecurringPaymentDialog**: Dialog for creating new recurring payments
- **EditRecurringPaymentDialog**: Dialog for editing existing payments
- **PaymentDetailsDialog**: Dialog for viewing payment details

## Used Design Patterns 
1. _Observer Pattern_. For notification mechanisms
2. _Factory Pattern (Abstract Factory)_. Create different types of schedule objects
3. _Facade Pattern_. Provide a simplified interface to a complex subsystem
4. _Strategy Pattern_. Allow different schedule algorithms to be used interchangeably

## File Structure
```
RecurringPaymentSchedule/
├── Core Domain Models
│   ├── RecurringPayment.h/cpp
│   ├── PaymentInstance.h/cpp
│   ├── PaymentTypes.h/cpp
│   └── DateTime.h/cpp
│
├── Schedule System
│   ├── RecurringSchedule.h/cpp
│   ├── DailySchedule.h/cpp
│   ├── WeeklySchedule.h/cpp
│   ├── MonthlySchedule.h/cpp
│   ├── YearlySchedule.h/cpp
│   └── ScheduleFactory.h
│
├── Business Logic
│   ├── PaymentService.h/cpp
│   ├── PaymentGenerator.h/cpp
│   └── NotificationCenter.h
│
├── Data Access
│   ├── IPaymentRepository.h
│   ├── InMemoryRepository.h/cpp
│   └── SQLiteRepository.h/cpp
│
├── User Interface
│   ├── mainwindow.h/cpp
│   ├── AddRecurringPaymentDialog.h/cpp
│   ├── EditRecurringPaymentDialog.h/cpp
│   └── PaymentDetailsDialog.h/cpp
│
└── Entry Point
    └── main.cpp
```
## Technology Stack

- **Framework**: Qt 6.5.3
- **Language**: C++17
- **Database**: SQLite (via Qt SQL)
- **Build System**: qmake
- **Platform**: Windows (MinGW 64-bit)
