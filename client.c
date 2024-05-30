#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#define MAX_USERS 100
#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50
#define MAX_BOOKS 100
#define MAX_TITLE_LENGTH 100
#define MAX_AUTHOR_LENGTH 100
typedef struct
{
    int id; // Unique identifier for the user
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
} User;

User users[MAX_USERS];
int numUsers = 0;
int nextUserID = 101;
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
// Function prototypes
void login(int sd);
void registerUser(int sd);
void adminAuth(int sd);
void addBook(int sd);
void userActions(int nsd,int id);
void deleteBook(int sd)
{
    int id;

    // Receive book ID from the user
    printf("Enter the ID of the book to delete: ");
    scanf("%d", &id);

    // Send the book ID to the server
    write(sd, &id, sizeof(id));

    // Receive response from the server
    char response[100];
    read(sd, response, sizeof(response));
    printf("Server response: %s\n", response);
}
void modifyBook(int sd)
{
    int id, change;

    // Enter the book ID and quantity change
    printf("Enter ID of the book to modify: ");
    scanf("%d", &id);
    printf("Enter quantity change (+/-): ");
    scanf("%d", &change);

    // Send book ID and quantity change to the server
    write(sd, &id, sizeof(id));
    write(sd, &change, sizeof(change));

    // Receive response from the server
    char response[100];
    read(sd, response, sizeof(response));
    printf("Server response: %s\n", response);
}

int main()
{
    struct sockaddr_in serv;
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1)
    {
        perror("Error: ");
        return -1;
    }

    int portno = 7667;
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(portno);

    if (connect(sd, (struct sockaddr *)&serv, sizeof(serv)) != -1)
    {
        int choice;

        printf("\n╔═══════════════════════════════════════════════════════╗\n");
        printf("║    Welcome to the Online Library Management System    ║\n");
        printf("╚═══════════════════════════════════════════════════════╝\n");
        printf("Please choose an option to continue:\n");
        printf("--------------------------------------------------\n");
        printf("1. Login\n");
        printf("   - Access your account and manage your collection.\n");
        printf("2. Register\n");
        printf("   - Create a new account to start using the system.\n");
        printf("3. Admin Authentication\n");
        printf("   - Admin access for managing the system.\n");
        printf("--------------------------------------------------\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            login(sd);
            break;
        case 2:
            registerUser(sd);
            break;
        case 3:
            adminAuth(sd);
            break;
        default:
            printf("Invalid choice.\n");
        }
    }
    else
    {
        printf("Unsuccessful connection.\n");
        perror("");
    }

    close(sd);
    return 0;
}

void userActions(int sd,int userID)
{
    int userChoice;
    do
    {
        
        printf("\n╔═════════════════════════════════╗\n");
        printf("║           User Actions          ║\n");
        printf("╚═════════════════════════════════╝\n");
        printf(" 1. View Books\n");
        printf("    - Browse available books in the inventory.\n");
        printf(" 2. Add Book to Collection\n");
        printf("    - Add a book from the inventory to your collection.\n");
        printf(" 3. View My Collection\n");
        printf("    - View the books in your personal collection.\n");
        printf(" 4. Return Book\n");
        printf("    - Return a book from your collection to the inventory.\n");
        printf(" 5. Logout\n");
        printf("    - Logout from your account.\n");
        // printf("╔═════════════════════════════════╗\n");
        printf("Enter your choice: ");

        scanf("%d", &userChoice); // Read user's choice

        // Send user's choice to the server
        write(sd, &userChoice, sizeof(userChoice));

        switch (userChoice)
        {
        case 1:
            // Handle view books
            printf("\nOption Selected: View Books\n");

            // Read the inventory sent by the server
            // struct Book inventory[MAX_BOOKS];
            read(sd, inventory, sizeof(inventory));
            printf("Inventory loaded successfully.\n");

            printf("┌────┬──────────────────────────────┬──────────────────────────────┬─────────┐\n");
            printf("│ ID │ Title                        │ Author                       │ Quantity│\n");
            printf("├────┼──────────────────────────────┼──────────────────────────────┼─────────┤\n");
            for (int i = 0; i < MAX_BOOKS; i++)
            {
                if (inventory[i].id != 0 && inventory[i].quantity != 0)
                {
                    printf("│ %-2d │ %-28s │ %-28s │ %-7d │\n",
                           inventory[i].id, inventory[i].title, inventory[i].author, inventory[i].quantity);
                }
            }
            printf("└────┴──────────────────────────────┴──────────────────────────────┴─────────┘\n");
            
            
            break;

        case 2: // Add Book to Collection
            //printf("Add Book to Collection option selected. Enter book ID: ");
            printf("\nOption Selected: Add Book to Collection\n");
            printf("Enter the ID of the book you wish to add: ");
            int bookID;
            scanf("%d", &bookID);
            printf("Enter the quantity: ");
            int quantity;
            scanf("%d", &quantity);

            // Send book ID and quantity to the server
            write(sd, &bookID, sizeof(bookID));
            write(sd, &quantity, sizeof(quantity));

            // Receive response from the server
            char addResponse[100];
            read(sd, addResponse, sizeof(addResponse));
            printf("%s\n", addResponse);
            
            break;


        case 3: // View My Collection
        {
            
            printf("\nOption Selected: View My Collection\n");
            printf("\nYour Collection:\n");
            printf("┌────┬──────────────────────────────┬──────────────────────────────┬─────────┐\n");
            printf("│ ID │ Title                        │ Author                       │ Quantity│\n");
            printf("├────┼──────────────────────────────┼──────────────────────────────┼─────────┤\n");

            // Receive the user's collection with book details from the server
            while (1)
            {
                Book bookDetails;
                int quantity;

                // Read book details from server
                if (read(sd, &bookDetails, sizeof(bookDetails)) <= 0)
                {
                    perror("Error reading book details");
                    break;
                }

                if (bookDetails.id == -1)
                { // Check for end signal
                    break;
                }

                // Read quantity from server
                if (read(sd, &quantity, sizeof(quantity)) <= 0)
                {
                    perror("Error reading quantity");
                    break;
                }
                if(quantity!=0){
                    printf("│ %-2d │ %-28s │ %-28s │ %-7d │\n",
                       bookDetails.id, bookDetails.title, bookDetails.author, quantity);
                }
                
            }
            printf("└────┴──────────────────────────────┴──────────────────────────────┴─────────┘\n");
            
            break;
        }

        case 4: // Return Book
        {
            printf("\nOption Selected: Return Book\n");
            printf("Enter the ID of the book you wish to return: ");
            int bookID, quantity;
            // printf("Enter Book ID to return: ");
            scanf("%d", &bookID);
            printf("Enter quantity to return: ");
            scanf("%d", &quantity);
            write(sd, &bookID, sizeof(bookID));
            write(sd, &quantity, sizeof(quantity));
            char returnResponse[100];
            read(sd, returnResponse, sizeof(returnResponse));
            printf("%s\n", returnResponse);
            
            break;
        }

        case 5:
            printf("\nLogging out. Thank you for using the Online Library Management System!\n");

            // Implement logout functionality
            break;
        default:
            printf("Invalid choice. Please try again.\n");
        }
    } while (userChoice != 5); // Exit loop when user chooses to logout
}
// Function to register a new user
void registerNewUser(int sd)
{
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];

    // Get the username and password from the admin
    printf("Enter new username: ");
    printf("SPACES IN THE USERNAME ARE INVALID");
    scanf("%s", username);
    printf("Enter new password: ");
    scanf("%s", password);

    // Send the username and password to the server
    write(sd, username, sizeof(username));
    write(sd, password, sizeof(password));

    // Read the server's response
    char response[256];
    read(sd, response, sizeof(response));
    printf("%s\n", response);
}

void deleteUser(int sd){
    char user_name[MAX_USERNAME_LENGTH];
    printf("Enter the name of the user you want to delete: ");
    printf("SPACES IN THE USERNAME ARE INVALID\n");
    scanf("%s", user_name);
    write(sd, user_name, sizeof(user_name));
    char response[256];
    read(sd, response, sizeof(response));
    printf("%s\n", response);
}
void registerUser(int sd)
{
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    printf("SPACES IN THE USERNAME ARE INVALID\n");
    printf("Enter new username: ");
    scanf("%s", username);
    printf("Enter new password: ");
    scanf("%s", password);

    int option = 2;
    write(sd, &option, sizeof(option));
    write(sd, username, sizeof(username));
    write(sd, password, sizeof(password));

    char buf[100];
    read(sd, buf, sizeof(buf));
    printf("Server response: %s\n", buf);
}
void adminViewUser(int sd)
{
    int numUsers;
    read(sd, &numUsers, sizeof(numUsers)); // Read the number of users

    printf("\n╔═════════════════════════════════╗\n");
    printf("║           User List   s         ║\n");
    printf("╚═════════════════════════════════╝\n");
    // Print the user list in a formatted table
    printf("┌──────┬────────────────────────────────────────┐\n");
    printf("│ ID   │ Username                               │\n");
    printf("├──────┼────────────────────────────────────────┤\n");
    User user;
    for (int i = 0; i < numUsers; i++)
    {
        
        read(sd, &user, sizeof(user)); // Read each user's details

        printf("│ %-2d │ %-38s │\n", user.id, user.username);
    }

    printf("└──────┴────────────────────────────────────────┘\n");
    
}

void updateInventory(int sd)
{
    int updateChoice;
    do
    {
        printf("\nUpdate Inventory\n");
        printf("1. Add Book\n");
        printf("2. Delete Book\n");
        printf("3. Modify Book\n");
        printf("4. View User List\n");
        printf("5. Add New User\n");
        printf("6. Delete User\n");
        printf("7. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &updateChoice);

        write(sd, &updateChoice, sizeof(updateChoice)); // Send the choice to the server

        switch (updateChoice)
        {
        case 1:
            addBook(sd); // Call the function to add a new book
            break;
        case 2:
            printf("Delete Book option selected.\n");
            deleteBook(sd);
            break;
        case 3:
            printf("Modify Book option selected.\n");
            modifyBook(sd);
            break;
        case 4:
            printf("View User List option selected.\n");
            // Call function to view user list
            adminViewUser(sd);
            break;
        case 5:
            printf("Add New User option selected.\n");
            // Call function to add new user
            registerNewUser(sd);

                break;
        case 6:
            printf("Delete a user option selected.\n");
                deleteUser(sd);
                break;
        case 7:
            printf("Exiting update inventory.\n");
            break;    
        default:
            printf("Invalid choice. Please try again.\n");
        }
    } while (updateChoice != 7);
}

void addBook(int sd)
{
    int id, quantity;
    char title[MAX_TITLE_LENGTH], author[MAX_AUTHOR_LENGTH];

    printf("Enter book ID: ");
    scanf("%d", &id);
    printf("Enter book title: ");
    scanf(" %[^\n]", title);
    printf("Enter book author: ");
    scanf(" %[^\n]", author);
    printf("Enter book quantity: ");
    scanf("%d", &quantity);

    // Send book details to the server
    write(sd, &id, sizeof(id));
    write(sd, title, sizeof(title));
    write(sd, author, sizeof(author));
    write(sd, &quantity, sizeof(quantity));

    // Receive response from the server
    char response[100];
    read(sd, response, sizeof(response));
    printf("%s\n", response);
}

void adminAuth(int sd)
{
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    printf("Enter admin username: ");
    scanf("%s", username);
    printf("Enter admin password: ");
    scanf("%s", password);

    int option = 3;
    write(sd, &option, sizeof(option));
    write(sd, username, sizeof(username));
    write(sd, password, sizeof(password));

    char buf[100];
    read(sd, buf, sizeof(buf));
    printf("Server response: %s\n", buf);

    if (strcmp(buf, "Admin authenticated") == 0)
    {
        int adminChoice;
        do
        {
            printf("Admin authenticated. Choose an option:\n");
            printf("1. Load Inventory\n");
            printf("2. Update Inventory\n");
            printf("3. Exit\n");
            printf("Enter your choice: ");
            scanf("%d", &adminChoice);
            write(sd, &adminChoice, sizeof(adminChoice));
            switch (adminChoice)
            {
            case 1:
                // Read the inventory sent by the server
                
                read(sd, inventory, sizeof(inventory));
                printf("Inventory loaded.\n");
                // Display the inventory
                printf("┌────┬────────────────────────────────────────┬──────────────────────────┬─────────┐\n");
                printf("│ ID │ Title                                  │ Author                   │ Quantity│\n");
                printf("├────┼────────────────────────────────────────┼──────────────────────────┼─────────┤\n");

                for (int i = 0; i < MAX_BOOKS; i++)
                {
                    if (inventory[i].id != 0)
                    {
                        printf("│ %-2d │ %-38s │ %-24s │ %-7d │\n", inventory[i].id, inventory[i].title, inventory[i].author, inventory[i].quantity);
                    }
                }

                printf("└────┴────────────────────────────────────────┴──────────────────────────┴─────────┘\n");

                break;
            case 2:
                // Placeholder for update inventory functionality
                printf("Update Inventory option selected.\n");
                updateInventory(sd);
                // Implement update inventory functionality later
                break;
            case 3:
                printf("Exiting admin actions.\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
            }
        } while (adminChoice != 3);
    }
}

void login(int sd) {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    char credentials[MAX_USERNAME_LENGTH + MAX_PASSWORD_LENGTH + 2];
    int authenticated = 0;
    printf("===================================\n");
    printf("           User Login\n");
    printf("===================================\n");
    int option = 1;
    write(sd, &option, sizeof(option));
    while (!authenticated) {
        
        printf("Enter username: ");
        scanf("%s", username);
        printf("Enter password: ");
        scanf("%s", password);
        snprintf(credentials, sizeof(credentials), "%s %s", username, password);

        // Send the credentials string to the server
        if (write(sd, credentials, strlen(credentials) + 1) < 0) { // Send null-terminated string
            perror("Error writing credentials to server");
        }
        char buf[100];
        int n =read(sd, buf, sizeof(buf));
        if (n < 0) {
            perror("Error reading response from server");
        }
        buf[n] = '\0'; 
        printf("Server response: %s\n", buf);
        if (strcmp(buf, "Authenticated") == 0) {
            authenticated = 1;
            printf("\nLogin successful! Accessing your account...\n");
            write(sd, "RequestUserID", strlen("RequestUserID") + 1); // Send actual length of request
            int userID;
            read(sd, &userID, sizeof(userID));
            printf("Your User ID is: %d\n", userID);
            userActions(sd,userID);
        } else if (strcmp(buf, "Retry!") == 0) {
            authenticated = 0;
            printf("\nInvalid login credentials. Retry!\n");  
        }
    }
}
