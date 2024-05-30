# Online Library Management System (OLMS)

## Introduction

Welcome to the Online Library Management System (OLMS)! In the contemporary digital age, efficient management systems are crucial for the smooth functioning of libraries. This project aims to design and develop an OLMS that not only provides basic functionalities of a library but also ensures data security and concurrency control.

## Objective

The primary objective of this project is to develop an efficient and secure Online Library Management System that caters to the needs of both library administrators and users. Specific objectives include:

1. **User Authentication**: Implementation of user authentication mechanisms to ensure secure access to member accounts.
2. **Administrative Access**: Incorporation of password-protected administrative access to prevent unauthorized usage.
3. **Library Management**: Development of functionalities for adding, deleting, modifying book records, and managing member details for the admin. Users can register, log in, view books, and manage their personal cart.
4. **Concurrency Control**: Implementation of file-locking mechanisms using system calls like mutex locks to ensure data consistency and concurrency control.
5. **Socket Programming**: Utilization of socket programming to facilitate concurrent access to the library database by multiple clients.

## Features

### User Features
- **User Login**: Secure login mechanism for users.
- **User Registration**: New users can register for an account.
- **View Books**: Users can view the list of available books.
- **Manage Cart**: Users can add, remove, and modify books in their personal cart.
- **Borrow/Return Books**: Users can borrow and return books with protection mechanisms in place.

### Admin Features
- **Admin Login**: Secure login mechanism for administrators.
- **View Books**: Administrators can view the list of all books in the library.
- **Manage Books**: Administrators can add, delete, and modify book records.
- **Manage Users**: Administrators can add, delete, and view user details.
- **View Collection**: Administrators can view all user actions, including books accessed by users.

## Technical Details

### Concurrency and Data Consistency
- **Mutex Locks**: Ensures data consistency and concurrency control by using file-locking mechanisms.
- **Socket Programming**: Allows multiple clients (users and admin) to access the library concurrently.

### System Calls
- **File Handling**
- **File Locking**
- **Multithreading**
- **Interprocess Communication**

## System Requirements
- **Operating System**: Linux-based OS (for system call compatibility)
- **Compiler**: GCC
- **Libraries**: pthread, sockets

  ## Usage Instructions

### User Login and Registration
- Users need to register for an account.
- Registered users can log in using their credentials.

### User Actions
- After login, users can view books, manage their cart, and borrow/return books.

### Admin Actions
- Admins log in with their credentials.
- Admins can manage books, users, and view the entire collection of user actions.
