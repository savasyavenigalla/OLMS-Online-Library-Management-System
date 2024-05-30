#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#define PORT 7667 // Define the port number you want to use
#define MAX_CLIENTS 10 // Define the maximum number of clients your server can handle
#define MAX_REQUEST_LENGTH 100

#define MAX_USERS 100
#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50
#define DATABASE_FILE_user "users.txt"
pthread_mutex_t users_file_mutex = PTHREAD_MUTEX_INITIALIZER;
// Structure to represent a user
typedef struct
{
    int id; // Unique identifier for the user
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
} User;
pthread_mutex_t users_mutex = PTHREAD_MUTEX_INITIALIZER;
// Array to store users
User users[MAX_USERS];
int numUsers = 0;
int nextUserID = 101;

#define MAX_BOOKS 100
#define MAX_TITLE_LENGTH 100
#define MAX_AUTHOR_LENGTH 100
#define DATABASE_FILE_books "books.txt"
pthread_mutex_t books_file_mutex = PTHREAD_MUTEX_INITIALIZER;
// Structure to represent a book
typedef struct
{
    int id;
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    int quantity;
} Book;
// Array to store books
Book inventory[MAX_BOOKS];
int numBooks = 0;
pthread_mutex_t inventory_mutex = PTHREAD_MUTEX_INITIALIZER;

#define USER_COLLECTION_FILE "user_collections.txt"
pthread_mutex_t usercollections_file_mutex = PTHREAD_MUTEX_INITIALIZER;
// Structure to represent a user's book collection
typedef struct
{
    int userID;
    int bookID;
    int quantity;
} UserCollection;
// Array to store user collections
UserCollection userCollections[MAX_USERS * MAX_BOOKS];
int numUserCollections = 0;
pthread_mutex_t userCollections_mutex = PTHREAD_MUTEX_INITIALIZER;

#define ADMIN_USERNAME "niveda"
#define ADMIN_PASSWORD "password"

// Function prototypes

void loadBooks();
void loadUserCollections();
void saveBooks();
void NsaveBooks();
void addBook(int nsd);
void deleteBook(int nsd);
void modifyBook(int nsd);
Book getBookDetails(int bookID);

int authenticateUser(char *username, char *password);
void registerNewUser(int nsd,char *username, char *password);
int authenticateAdmin(char *username, char *password);
void loadUsersAndSetNextUserID();
void addToCollection(int nsd, int userID, int bookID, int quantity);
void saveUserCollections();
void NsaveUserCollections() ;
void returnUserBook(int nsd, int userID);

void loadBooks()
{
    pthread_mutex_lock(&books_file_mutex);
    printf("mutex-lock for books.txt\n");
    FILE *file = fopen(DATABASE_FILE_books, "r");
    if (file == NULL)
    {
        printf("Error opening books file.\n");
        pthread_mutex_unlock(&books_file_mutex);
        printf("mutex_unlock for books.txt\n");
        return;
    }

    pthread_mutex_lock(&inventory_mutex);
    printf("mutex-lock for inventory\n");
    // Clear the current inventory
    numBooks = 0;

    while (fscanf(file, "%d;%[^;];%[^;];%d\n", &inventory[numBooks].id, inventory[numBooks].title,
                  inventory[numBooks].author, &inventory[numBooks].quantity) == 4)
    {
        numBooks++;
        if (numBooks >= MAX_BOOKS)
        {
            printf("Maximum number of books reached. Some books may not have been loaded.\n");
            break;
        }
    }
    pthread_mutex_unlock(&inventory_mutex);
    printf("mutex_unlock for inventory\n");
    fclose(file);
    pthread_mutex_unlock(&books_file_mutex);
    printf("mutex_unlock for books.txt\n");
}

void loadUserCollections()
{
    pthread_mutex_lock(&usercollections_file_mutex);
    printf("mutex-lock for user_collections.txt\n");
    FILE *file = fopen(USER_COLLECTION_FILE, "r");
    if (file == NULL)
    {
        printf("Error opening user collections file.\n");
        pthread_mutex_unlock(&usercollections_file_mutex);
        printf("mutex_unlock for user_collections.txt\n");
        return;
    }

    pthread_mutex_lock(&userCollections_mutex);
    printf("mutex-lock for userCollections\n");
    // Clear the current user collections
    numUserCollections = 0;

    while (fscanf(file, "%d %d %d\n", &userCollections[numUserCollections].userID, 
                  &userCollections[numUserCollections].bookID, 
                  &userCollections[numUserCollections].quantity) == 3)
    {
        numUserCollections++;
        if (numUserCollections >= MAX_USERS * MAX_BOOKS)
        {
            printf("Maximum number of user collections reached. Some collections may not have been loaded.\n");
            break;
        }
    }
    pthread_mutex_unlock(&userCollections_mutex);
    printf("mutex_unlock for userCollections\n");
    fclose(file);
    pthread_mutex_unlock(&usercollections_file_mutex);
    printf("mutex_unlock for user_collections.txt\n");


}


void saveBooks()
{
    pthread_mutex_lock(&books_file_mutex);
    printf("mutex-lock for books.txt\n");
    FILE *file = fopen(DATABASE_FILE_books, "w");
    if (file == NULL)
    {
        printf("Error opening books file for writing.\n");
        pthread_mutex_unlock(&books_file_mutex);
        printf("mutex_unlock for books.txt\n");
        return;
    }
    pthread_mutex_lock(&inventory_mutex);
    printf("mutex-lock inventory\n");
    for (int i = 0; i < numBooks; i++)
    {
        fprintf(file, "%d;%s;%s;%d\n", inventory[i].id, inventory[i].title, inventory[i].author, inventory[i].quantity);
    }
    pthread_mutex_unlock(&inventory_mutex);
    printf("mutex_unlock inventory\n");
    fclose(file);
    pthread_mutex_unlock(&books_file_mutex);
    printf("mutex_unlock books.txt\n");
}
void NsaveBooks()
{
    pthread_mutex_lock(&books_file_mutex);
    printf("mutex-lock for books.txt\n");
    FILE *file = fopen(DATABASE_FILE_books, "w");
    if (file == NULL)
    {
        printf("Error opening books file for writing.\n");
        pthread_mutex_unlock(&books_file_mutex);
        printf("mutex_unlock for books.txt\n");
        return;
    }

    for (int i = 0; i < numBooks; i++)
    {
        fprintf(file, "%d;%s;%s;%d\n", inventory[i].id, inventory[i].title, inventory[i].author, inventory[i].quantity);
    }

    fclose(file);
    pthread_mutex_unlock(&books_file_mutex);
    printf("mutex_unlock books.txt\n");
}

void addBook(int nsd)
{
    int id, quantity;
    char title[MAX_TITLE_LENGTH], author[MAX_AUTHOR_LENGTH];

    // Receive book details from the client
    read(nsd, &id, sizeof(id));
    read(nsd, title, sizeof(title));
    read(nsd, author, sizeof(author));
    read(nsd, &quantity, sizeof(quantity));
    pthread_mutex_lock(&inventory_mutex);
    printf("mutex-lock for inventory\n");
    // Check if the book ID already exists
    for (int i = 0; i < numBooks; i++)
    {
        if (inventory[i].id == id)
        {
            char *message = "Book ID already exists. Please use a different ID.";
            write(nsd, message, strlen(message) + 1);
            pthread_mutex_unlock(&inventory_mutex);
            printf("mutex_unlock for inventory\n");
            return;
        }
    }

    // Add the received book to the inventory
    if (numBooks >= MAX_BOOKS)
    {
        char *message = "Cannot add more books. Maximum limit reached.";
        write(nsd, message, strlen(message) + 1);
        pthread_mutex_unlock(&inventory_mutex);
        printf("mutex_unlock for inventory\n");
        return;
    }

    inventory[numBooks].id = id;
    strncpy(inventory[numBooks].title, title, MAX_TITLE_LENGTH - 1);
    inventory[numBooks].title[MAX_TITLE_LENGTH - 1] = '\0'; // Ensure null-terminated
    strncpy(inventory[numBooks].author, author, MAX_AUTHOR_LENGTH - 1);
    inventory[numBooks].author[MAX_AUTHOR_LENGTH - 1] = '\0'; // Ensure null-terminated
    inventory[numBooks].quantity = quantity;

    numBooks++;
    NsaveBooks();
    
    char *message = "Book added successfully.";
    write(nsd, message, strlen(message) + 1);
    
    pthread_mutex_unlock(&inventory_mutex);
    printf("mutex_unlock for inventory\n");
}

void deleteBook(int nsd)
{
    int id;

    // Receive book ID from the client
    read(nsd, &id, sizeof(id));

    int found = 0;
    pthread_mutex_lock(&inventory_mutex);
    printf("mutex-lock for inventory\n");
    for (int i = 0; i < numBooks; i++)
    {
        if (inventory[i].id == id)
        {
            found = 1;

            // Mark the book as deleted
            inventory[i].id = 0;
            inventory[i].quantity = 0; // You may also set other fields to indicate it's not available

            printf("Book with ID %d deleted successfully.\n", id);
            char *message = "Book deleted successfully.";
            write(nsd, message, strlen(message) + 1);
            break;
        }
    }

    if (!found)
    {
        printf("Book with ID %d not found.\n", id);
    }

    // Save the changes to the file
    NsaveBooks();
    pthread_mutex_unlock(&inventory_mutex);
    printf("mutex_unlock for inventory\n");

    // Send response message to the client
    if (!found)
    {
        char *message = "Book not found.";
        write(nsd, message, strlen(message) + 1);
        
    }
    
}
void modifyBook(int nsd)
{
    int id, change;

    // Receive book ID and quantity change from the client
    read(nsd, &id, sizeof(id));
    read(nsd, &change, sizeof(change));

    int found = 0;
    pthread_mutex_lock(&inventory_mutex);
    printf("mutex-lock\n");
    for (int i = 0; i < numBooks; i++)
    {
        if (inventory[i].id == id)
        {
            found = 1;
            inventory[i].quantity += change;
            printf("Quantity for Book ID %d updated to %d.\n", id, inventory[i].quantity);
            break;
        }
    }

    if (!found)
    {
        printf("Book with ID %d not found.\n", id);
    }

    // Save the changes to the file
    NsaveBooks();
    pthread_mutex_unlock(&inventory_mutex);
    printf("mutex_unlock\n");

    // Send response to the client
    if (found)
    {
        char *message = "Book modified successfully.";
        write(nsd, message, strlen(message) + 1);
    }
    else
    {
        char *message = "Book not found.";
        write(nsd, message, strlen(message) + 1);
    }
}
void registerNewuser(int nsd,char *username, char *password)
{
    pthread_mutex_lock(&users_mutex);
    // Check if the username already exists
    for (int i = 0; i < numUsers; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            char *message = "Username already exists. Please choose a different username.";
            write(nsd, message, strlen(message) + 1);
            pthread_mutex_unlock(&users_mutex);
            return;
        }
    }

    if (numUsers >= MAX_USERS)
    {
        char *message = "Maximum number of users reached.";
        write(nsd, message, strlen(message) + 1);
        pthread_mutex_unlock(&users_mutex);
        return;
    }
    if (numUsers >= MAX_USERS)
    {
        printf("Maximum number of users reached.\n");
        pthread_mutex_unlock(&users_mutex);
        return;
    }
    users[numUsers].id = nextUserID++; // Assign the next available ID and then increment
    strcpy(users[numUsers].username, username);
    strcpy(users[numUsers].password, password);
    numUsers++;
    pthread_mutex_unlock(&users_mutex);

    pthread_mutex_lock(&users_file_mutex);
    FILE *file = fopen(DATABASE_FILE_user, "a");
    if (file == NULL)
    {
        printf("Error opening database file.\n");
        pthread_mutex_unlock(&users_file_mutex);
        exit(1);
    }
    fprintf(file, "%s %s %d\n", username, password, users[numUsers - 1].id); // Include user ID in the database file
    fclose(file);
    pthread_mutex_unlock(&users_file_mutex);

    printf("User added successfully.\n");
    printf("Welcome, %s! Your user ID is: %d\n", username, users[numUsers - 1].id); // Print welcome message with user ID
}

void deleteUser(int nsd, char *username) {
    pthread_mutex_lock(&users_mutex);
    int userIndex = -1;

    // Find the index of the user with the given username
    for (int i = 0; i < numUsers; i++) {
        if (strcmp(users[i].username, username) == 0) {
            userIndex = i;
            break;
        }
    }

    if (userIndex == -1) {
        // User not found
        char *message = "User not found.";
        write(nsd, message, strlen(message) + 1);
        pthread_mutex_unlock(&users_mutex);
        return;
    }

    // Shift remaining users to fill the gap
    for (int i = userIndex; i < numUsers - 1; i++) {
        users[i] = users[i + 1];
    }

    numUsers--;

    pthread_mutex_unlock(&users_mutex);

    pthread_mutex_lock(&users_file_mutex);
    FILE *file = fopen(DATABASE_FILE_user, "w");
    if (file == NULL) {
        printf("Error opening database file.\n");
        pthread_mutex_unlock(&users_file_mutex);
        exit(1);
    }

    // Rewrite user data to the file excluding the deleted user
    for (int i = 0; i < numUsers; i++) {
        fprintf(file, "%s %s %d\n", users[i].username, users[i].password, users[i].id);
    }

    fclose(file);
    pthread_mutex_unlock(&users_file_mutex);

    char *message = "User deleted successfully.";
    write(nsd, message, strlen(message) + 1);

    printf("User '%s' deleted successfully.\n", username);
}

// Function to retrieve book details based on book ID
Book getBookDetails(int bookID)
{
    pthread_mutex_lock(&inventory_mutex);
    for (int i = 0; i < numBooks; i++)
    {
        if (inventory[i].id == bookID)
        {
            pthread_mutex_unlock(&inventory_mutex);
            return inventory[i];
        }
    }
    pthread_mutex_unlock(&inventory_mutex);
    
    // Return a book with id -1 to indicate not found
    Book notFound = {.id = -1};
    return notFound;
}
void returnUserBook(int nsd, int userID)
{
    int bookID, quantity;
    read(nsd, &bookID, sizeof(bookID));
    read(nsd, &quantity, sizeof(quantity));


    pthread_mutex_lock(&usercollections_file_mutex);
    printf("mutex-lock user collection\n");

    int foundInCollection = 0;
    for (int i = 0; i < numUserCollections; i++)
    {
        if (userCollections[i].userID == userID && userCollections[i].bookID == bookID)
        {
            foundInCollection = 1;
            if (userCollections[i].quantity < quantity)
            {
                char *message = "You cannot return more books than you have.";
                write(nsd, message, strlen(message) + 1);
                pthread_mutex_unlock(&usercollections_file_mutex);
                printf("mutex_unlock\n");
                return;
            }

            userCollections[i].quantity -= quantity;

            // // If quantity becomes zero, remove the book from the collection
            // if (userCollections[i].quantity == 0)
            // {
            //     for (int j = i; j < numUserCollections - 1; j++)
            //     {
            //         userCollections[j] = userCollections[j + 1];
            //     }
            //     numUserCollections--;
            // }

            // Update inventory
            pthread_mutex_lock(&inventory_mutex);
            printf("mutex-lock inventory\n");
            for (int k = 0; k < numBooks; k++)
            {
                if (inventory[k].id == bookID)
                {
                    inventory[k].quantity += quantity;
                    break;
                }
            }

            NsaveBooks();           // Save the updated inventory to file
            NsaveUserCollections(); // Save the updated user collections to file
            char *message = "Book returned successfully.";
            write(nsd, message, strlen(message) + 1);
            // Unlock the mutexes before returning
            pthread_mutex_unlock(&inventory_mutex);
            printf("mutex_unlock inventory\n");
            pthread_mutex_unlock(&usercollections_file_mutex);
            printf("mutex_unlock collections\n");
            return;
        }
    }

    if (!foundInCollection)
    {
        char *message = "Book not found in your collection.";
        write(nsd, message, strlen(message) + 1);
    }
    // Unlock the mutexes before returning
    // pthread_mutex_unlock(&inventory_mutex);
    // printf("mutex_unlock\n");
    pthread_mutex_unlock(&usercollections_file_mutex);
    printf("mutex_unlock\n");
}
void viewUserList(int nsd)
{
    
    pthread_mutex_lock(&users_mutex);
    write(nsd, &numUsers, sizeof(numUsers)); // Send the number of users to the client

    for (int i = 0; i < numUsers; i++)
    {
        write(nsd, &users[i], sizeof(users[i])); // Send each user's details to the client
    }

    pthread_mutex_unlock(&users_mutex);
}

void updateInventory(int nsd)
{
    int updateChoice;
    do
    {
        // Receive update choice from the client
        read(nsd, &updateChoice, sizeof(updateChoice));

        switch (updateChoice)
        {
        case 1:
            addBook(nsd); // Call the function to add a new book
            break;
        case 2:
            // Implement delete book functionality later
            printf("Delete Book option selected. \n");
            deleteBook(nsd);
            break;
        case 3:
            // Implement modify book functionality later
            printf("Modify Book option selected.\n");
            modifyBook(nsd);
            break;
        case 4:
            printf("View User List option selected.\n");
            viewUserList(nsd);
            break;
        case 5:
            printf("Add New User option selected. \n");
            // Implement add new user functionality
            char username[MAX_USERNAME_LENGTH];
            char password[MAX_PASSWORD_LENGTH];
            read(nsd, username, sizeof(username));
            read(nsd, password, sizeof(password));

            registerNewuser(nsd,username, password);

            char *message = "User registered successfully";
            write(nsd, message, strlen(message) + 1);

            break;
        case 6:
            printf("Delete User option selected.\n");
            char user_name[MAX_USERNAME_LENGTH];
            read(nsd, user_name, sizeof(user_name));
            deleteUser(nsd,user_name);
            break;
        case 7:
            printf("Exiting update inventory.\n");
            break;    
        default:
            printf("Invalid choice. Please try again.\n");
        }
    } while (updateChoice != 7);
}
// Function to save user collections to a file
void saveUserCollections()
{
    pthread_mutex_lock(&usercollections_file_mutex);
    printf("mutex-lock collections file\n");
    FILE *file = fopen(USER_COLLECTION_FILE, "w");
    if (file == NULL)
    {
        printf("Error opening user collections file for writing.\n");
        pthread_mutex_unlock(&usercollections_file_mutex);
        printf("mutex_unlock collection file\n");
        return;
    }

    for (int i = 0; i < numUserCollections; i++)
    {
        fprintf(file, "%d %d %d\n", userCollections[i].userID, userCollections[i].bookID, userCollections[i].quantity);
    }

    fclose(file);
    pthread_mutex_unlock(&usercollections_file_mutex);
    printf("mutex_unlock collections file\n");
}
void NsaveUserCollections()
{
    
    FILE *file = fopen(USER_COLLECTION_FILE, "w");
    if (file == NULL)
    {
        printf("Error opening user collections file for writing.\n");
        pthread_mutex_unlock(&usercollections_file_mutex);
        printf("mutex_unlock collection file\n");
        return;
    }

    for (int i = 0; i < numUserCollections; i++)
    {
        fprintf(file, "%d %d %d\n", userCollections[i].userID, userCollections[i].bookID, userCollections[i].quantity);
    }

    fclose(file);
   
}
void handleAdminChoice(int nsd, int adminChoice)
{
    switch (adminChoice)
    {
    case 1:          // Load Inventory
        loadBooks(); // Load inventory
        printf("Inventory loaded.\n");
        // Send inventory to client
        write(nsd, inventory, sizeof(inventory));
        break;
    case 2: // Update Inventory
        // Placeholder for update inventory functionality
        printf("Admin chose to update inventory.\n");
        updateInventory(nsd);
        // Implement update inventory functionality later
        break;
    case 3: // Exit
        printf("Admin chose to exit.\n");
        break;
    default:
        printf("Invalid admin choice.\n");
        break;
    }
}
void handleUserChoice(int nsd, int userID)
{
    int userChoice;
    do
    {
        // Receive user choice from the client
        read(nsd, &userChoice, sizeof(userChoice));

        switch (userChoice)
        {
        case 1:
            // Handle view books
            printf("View Books option selected by user ID %d.\n", userID);
            // Send the inventory to the client
            write(nsd, inventory, sizeof(inventory));
            break;

        case 2: // Add Book to Collection
            printf("Add Book to Collection option selected by user ID %d.\n", userID);
            int bookID, quantity;
            read(nsd, &bookID, sizeof(bookID));
            read(nsd, &quantity, sizeof(quantity));
            addToCollection(nsd, userID, bookID, quantity);
           // char *addMessage = "Book added to collection successfully.";
            //write(nsd, addMessage, strlen(addMessage) + 1);
            break;
       
        case 3: // View My Collection
        {
            printf("View My Collection option selected.\n");
            
            for (int i = 0; i < numUserCollections; i++)
            {
                if (userCollections[i].userID == userID)
                {
                    Book bookDetails = getBookDetails(userCollections[i].bookID);
                    write(nsd, &bookDetails, sizeof(bookDetails));
                    write(nsd, &userCollections[i].quantity, sizeof(userCollections[i].quantity));
                }
            }
            Book endSignal = {.id = -1};
            write(nsd, &endSignal, sizeof(endSignal));
            break;
        }

        case 4: // Return Book
        {
            printf("Return Book option selected.\n");
            returnUserBook(nsd, userID);
            break;
        }

        case 5:
            printf("Logout option selected by user ID %d.\n", userID);
            break;
        default:
            printf("Invalid choice received from user ID %d.\n", userID);
            break;
        }
    } while (userChoice != 5);
}


// Function to load users from a file
void loadUsersAndSetNextUserID()
{
    pthread_mutex_lock(&users_file_mutex);
    printf("mutex-lock\n");
    FILE *file = fopen(DATABASE_FILE_user, "r");
    if (file == NULL)
    {
        printf("Error opening database file.\n");
        exit(1);
    }
    while (fscanf(file, "%s %s %d", users[numUsers].username, users[numUsers].password, &users[numUsers].id) == 3)
    {
        numUsers++;
    }
    fclose(file);
    // Unlock the database file after reading
    pthread_mutex_unlock(&users_file_mutex);
    printf("mutex_unlock\n");
    if (numUsers > 0)
    {
        nextUserID = users[numUsers - 1].id + 1;
    }
}

// Function to authenticate a user
int authenticateUser(char *username, char *password)
{
    // Lock the users array before accessing
    pthread_mutex_lock(&users_mutex);
    printf("mutex-lock for users array\n");
    for (int i = 0; i < numUsers; i++)
    {   
        
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0)
        {   
            pthread_mutex_unlock(&users_mutex);
            printf("mutex_unlock from authentication\n");
            return users[i].id; // Return user ID if authenticated successfully
        }
    }
    pthread_mutex_unlock(&users_mutex);
    printf("mutex_unlock from authentication failed \n");
    return 0; // Authentication failed
}

// Function to add a new user
void registerNewUser(int nsd,char *username, char *password)
{
    pthread_mutex_lock(&users_mutex);
    printf("mutex-lock\n");
    // Check if the username already exists
    for (int i = 0; i < numUsers; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            char *message = "Username already exists. Please choose a different username.";
            write(nsd, message, strlen(message) + 1);
            pthread_mutex_unlock(&users_mutex);
            return;
        }
    }
    if (numUsers >= MAX_USERS)
    {
        printf("Maximum number of users reached.\n");
        pthread_mutex_unlock(&users_mutex);
        printf("mutex_unlock\n");
        return;
    }
    users[numUsers].id = nextUserID++; // Assign the next available ID and then increment
    strcpy(users[numUsers].username, username);
    strcpy(users[numUsers].password, password);
    numUsers++;
    pthread_mutex_lock(&users_file_mutex);
    printf("mutex-lock\n");
    FILE *file = fopen(DATABASE_FILE_user, "a");
    if (file == NULL)
    {
        printf("Error opening database file.\n");
        exit(1);
    }
    fprintf(file, "%s %s %d\n", username, password, users[numUsers - 1].id); // Include user ID in the database file
    fclose(file);
    pthread_mutex_unlock(&users_file_mutex);
    printf("mutex_unlock\n");
    pthread_mutex_unlock(&users_mutex);
    printf("mutex_unlock\n");
    printf("User added successfully.\n");
    printf("Welcome, %s! Your user ID is: %d\n", username, users[numUsers - 1].id); // Print welcome message with user ID
}

// Function to authenticate the admin
int authenticateAdmin(char *username, char *password)
{
    return (strcmp(username, ADMIN_USERNAME) == 0 && strcmp(password, ADMIN_PASSWORD) == 0);
}

void addToCollection(int nsd, int userID, int bookID, int quantity)
{
    pthread_mutex_lock(&inventory_mutex);
    printf("mutex-lock for inventory\n");
    int bookIndex = -1;
    // Find the index of the book in the inventory
    for (int i = 0; i < numBooks; i++)
    {
        if (inventory[i].id == bookID)
        {
            bookIndex = i;
            break;
        }
    }

    // Check if the book exists in the inventory
    if (bookIndex == -1)
    {
        char *message = "Book with specified ID not found in inventory.";
        write(nsd, message, strlen(message) + 1);
        // pthread_mutex_unlock(&usercollections_file_mutex);
        // printf("mutex_unlock\n");
        pthread_mutex_unlock(&inventory_mutex);
        printf("mutex_unlock inventory\n");
        return;
    }

    // Check if there are enough copies available in the inventory
    if (inventory[bookIndex].quantity < quantity)
    {
        char *message = "Not enough copies available in inventory.";
        write(nsd, message, strlen(message) + 1);
        pthread_mutex_unlock(&inventory_mutex);
        printf("mutex_unlock inventory\n");
        return;
    }

    // Update user's collection
    pthread_mutex_lock(&usercollections_file_mutex);
    printf("mutex-lock for collections.txt\n");
    for (int i = 0; i < numUserCollections; i++)
    {
        if (userCollections[i].userID == userID && userCollections[i].bookID == bookID)
        {
            userCollections[i].quantity += quantity;
            inventory[bookIndex].quantity -= quantity; // Decrease inventory quantity
            NsaveBooks();                               // Save the updated inventory to file
            NsaveUserCollections();                     // Save the updated user collections to file
            char *message = "Book quantity updated in user's collection.";
            write(nsd, message, strlen(message) + 1);
            pthread_mutex_unlock(&usercollections_file_mutex);
            printf("mutex_unlock collection_file\n");
            pthread_mutex_unlock(&inventory_mutex);
            printf("mutex_unlock inventory\n");
            return;
        }
    }

    // If the user's collection does not contain the book, add a new entry
    if (numUserCollections >= MAX_USERS * MAX_BOOKS)
    {
        char *message = "Cannot add more books to user's collection. Maximum limit reached.";
        write(nsd, message, strlen(message) + 1);
        pthread_mutex_unlock(&usercollections_file_mutex);
        printf("mutex_unlock collections\n");
        pthread_mutex_unlock(&inventory_mutex);
        printf("mutex_unlock inventory\n");
        return;
    }

    userCollections[numUserCollections].userID = userID;
    userCollections[numUserCollections].bookID = bookID;
    userCollections[numUserCollections].quantity = quantity;
    numUserCollections++;
    inventory[bookIndex].quantity -= quantity; // Decrease inventory quantity
    NsaveBooks();                               // Save the updated inventory to file
    NsaveUserCollections();                     // Save the updated user collections to file
    pthread_mutex_unlock(&usercollections_file_mutex);
    printf("mutex_unlock collections\n");
    pthread_mutex_unlock(&inventory_mutex);
    printf("mutex_unlock inventory\n");
    
    char *message = "Book added to user's collection.";
    write(nsd, message, strlen(message) + 1);
}
void *clientHandler(void *socket_desc)
{
    int nsd = *(int *)socket_desc;
    free(socket_desc);

    int option;
    read(nsd, &option, sizeof(option));

    switch (option)
    {
    case 1: // Login
    {
        char credentials[MAX_USERNAME_LENGTH + MAX_PASSWORD_LENGTH + 2];
        char username[MAX_USERNAME_LENGTH];
        char password[MAX_PASSWORD_LENGTH];
        int userID = 0;
        int retry = 1; // Flag to indicate whether to retry

        while (retry) {
            
            int n = read(nsd, credentials, sizeof(credentials));
            if (n < 0) {
                perror("Error reading credentials from client");
                continue; // Continue reading if there's an error
            } else if (n == 0) {
                printf("Connection closed by client\n");
                return NULL; // Exit function if client closes connection
            }
            credentials[n] = '\0'; 
            printf("Received credentials: %s\n", credentials);

            sscanf(credentials, "%s %s", username, password);
            printf("Parsed username: %s\n", username);
            printf("Parsed password: %s\n", password);

            if (username[0] != '\0' && password[0] != '\0') {
                
                userID = authenticateUser(username, password);

                if (userID != 0) {
                    printf("Authentication successful for user: %s\n", username);
                    char *message = "Authenticated";
                    if (write(nsd, message, strlen(message) + 1) < 0) {
                        perror("Error writing authentication response to client");
                        continue;
                    }

                    // Wait to receive the request for user ID from the client
                    char clientRequest[MAX_REQUEST_LENGTH];
                    n = read(nsd, clientRequest, sizeof(clientRequest));
                    if (n < 0) {
                        perror("Error reading client request");
                        continue;
                    }
                    clientRequest[n] = '\0'; // Ensure null-termination
                    if (strcmp(clientRequest, "RequestUserID") == 0) {
                        // Send the user ID to the client
                        if (write(nsd, &userID, sizeof(userID)) < 0) {
                            perror("Error writing userID to client");
                        }
                        
                        handleUserChoice(nsd, userID);
                    }
                    retry = 0; // Authentication successful, exit retry loop
                } else {
                    printf("Authentication failed for user: %s\n", username);
                    char *message = "Retry!";
                    if (write(nsd, message, strlen(message) + 1) < 0) {
                        perror("Error writing retry response to client");
                        continue;
                    }
                    memset(username, 0, sizeof(username));
                    memset(password, 0, sizeof(password));
                }
            } else {
                printf("Empty username or password\n");
                char *message = "Retry!";
                if (write(nsd, message, strlen(message) + 1) < 0) {
                    perror("Error writing retry response to client");
                    continue;
                }
            }
        }    
        break;
    }

    case 2: // Register
    {
        char username[MAX_USERNAME_LENGTH];
        char password[MAX_PASSWORD_LENGTH];
        read(nsd, username, sizeof(username));
        read(nsd, password, sizeof(password));

        registerNewUser(nsd,username, password);

        char *message = "User registered successfully";
        write(nsd, message, strlen(message) + 1);
        break;
    }
    case 3: // Admin authentication
    {
        char username[MAX_USERNAME_LENGTH];
        char password[MAX_PASSWORD_LENGTH];
        read(nsd, username, sizeof(username));
        read(nsd, password, sizeof(password));

        if (authenticateAdmin(username, password))
        {
            printf("Admin authentication successful for user: %s\n", username);
            char *message = "Admin authenticated";
            write(nsd, message, strlen(message) + 1);
            int adminChoice;
            while (1)
            {
                read(nsd, &adminChoice, sizeof(adminChoice)); // Read admin's choice

                if (adminChoice == 3) // Exit choice
                {
                    break;
                }

                handleAdminChoice(nsd, adminChoice); // Handle the admin's choice
            }
        }
        else
        {
            printf("Admin authentication failed for user: %s\n", username);
            char *message = "Admin authentication failed";
            write(nsd, message, strlen(message) + 1);
        }
        break;
    }
    default:
        printf("Invalid option.\n");
    }

    close(nsd);
    return NULL;
}


int main()
{
    loadUsersAndSetNextUserID(); // Load user data from file
    loadBooks();                 // Load the inventory data from file
    loadUserCollections();
    struct sockaddr_in serv, cli;
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1)
    {
        perror("Error: ");
        return -1;
    }
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(PORT);

    int opt = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("Error: ");
        return -1;
    }

    if (bind(sd, (struct sockaddr *)&serv, sizeof(serv)) == -1)
    {
        perror("Bind error:\n");
        return -1;
    }

    if (listen(sd, MAX_CLIENTS) == -1)
    {
        perror("Listen error:\n");
        return -1;
    }

    printf("Listening on port %d.\n", PORT);

    pthread_t thread_id;
    while (1)
    {
        int size = sizeof(cli);
        int *nsd = malloc(sizeof(int));
        *nsd = accept(sd, (struct sockaddr *)&cli, &size);
        if (*nsd < 0)
        {
            perror("Accept failed");
            free(nsd);
            continue;
        }

        printf("Client connected\n");

        if (pthread_create(&thread_id, NULL, clientHandler, (void *)nsd) < 0)
        {
            perror("Could not create thread");
            free(nsd);
            continue;
        }

        printf("Handler assigned\n");
    }

    close(sd);
    return 0;
}
