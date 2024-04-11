#include<stdio.h>
#include<stdlib.h>
#include<string.h>


// Define custormer,account and transaction structure for future references 
struct Customer {
    char id[9];
    char name[21];
    char address[51];
    char mobile[11];
};

struct Account {
    char id[9];
    char number[16];
    char type[3];
    float balance;
};

struct Transaction {
    char account_number[16];
    char date[11];
    float amount;
    char type[12];  // Adjusted size to accommodate "Withdrawal"
};

//different features involved
int login();
void staffMenu();
void addCustomer();
void addAccount();
void deposit();
void withdraw();
void balanceEnquiry();
void loanApprovals();
void logoff();

#define CUSTOMER_FILE "customer.dat"
#define ACCOUNT_FILE "account_details.dat"
#define TRANSACTION_FILE "Deposit_Withdraw.dat"

int main()
{
 int choice;
    do
    {
        choice = login();
        switch (choice)
        {
            case 1:
                staffMenu();
                break;
            case 0:
                printf("Exiting program.\n");
                break;
            default:
                printf("Invalid login credentials. Please try again.\n");
        }
    } while (choice != 0);

}

int login() {
    char username[50], password[50];
    printf("Enter username: ");
    scanf("%s", username);
    getchar(); 
    printf("Enter password: ");
    scanf("%s", password);
    getchar(); 

    // Assuming staff credentials are stored in a staff datasheet, it goes and checks login credentials with the details entered 
    return 0;
}

void staffMenu() 
{
    int choice;
    
    do 
    {
    printf("\nStaff Menu\n");
    printf("1. Add a Customer\n");
    printf("2. Add an Account\n");
    printf("3. Deposit\n");
    printf("4. Withdrawal\n");
    printf("5. Balance Enquiry\n");
    printf("6. Loan Approvals\n");
    printf("7. Logoff\n");
    printf("0. Return to Main Menu\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    getchar(); 

    switch (choice) {
        case 1:
            addCustomer();
            break;
        case 2:
            addAccount();
            break;
        case 3:
            deposit();
            break;
        case 4:
            withdraw();
            break;
        case 5:
            balanceEnquiry();
            break;
        case 6:
            loanApprovals();
            break;
        case 7:
            logoff();
            break;
        case 0:
            printf("Returning to main menu.\n");
            break; 
        default:
            printf("Invalid choice. Please try again.\n");
    }
} 
while (choice != 0);

}

void addCustomer() {
    //open the custormer file 

    struct Customer newCustomer;
    printf("Enter customer ID: ");
    scanf("%s", newCustomer.id);
    printf("Enter customer name: ");
    scanf("%s", newCustomer.name);
    printf("Enter customer address: ");
    scanf("%s", newCustomer.address);
    printf("Enter customer mobile: ");
    scanf("%s", newCustomer.mobile);

    //fprintf(file, "%s %s %s %s\n", newCustomer.id, newCustomer.name, newCustomer.address, newCustomer.mobile);
    //fclose(file);
    printf("Customer added successfully.\n");
}

void addAccount() {
    FILE *customerFile = fopen("customer.dat", "r");
    if (customerFile == NULL) {
        printf("Error: Unable to open customer file.\n");
        return;
    }

    char customerID[10]; // Increase by 1
    printf("Enter customer ID: ");
    fgets(customerID, sizeof(customerID), stdin);
    customerID[strcspn(customerID, "\n")] = '\0'; // Remove newline character

    // Search for the customer in the customer file
    struct Customer customer;
    int customerFound = 0;
    while (fscanf(customerFile, "%s %s %s %s", customer.id, customer.name, customer.address, customer.mobile) != EOF) {
        if (strcmp(customer.id, customerID) == 0) {
            customerFound = 1;
            break;
        }
    }
    fclose(customerFile);

    if (!customerFound) {
        printf("Customer with ID %s not found.\n", customerID);
        return;
    }

    FILE *accountFile = fopen("account_details.dat", "a");
    if (accountFile == NULL) {
        printf("Error: Unable to open account file.\n");
        return;
    }

    struct Account newAccount;
    strcpy(newAccount.id, customerID);
    printf("Enter account number: ");
    fgets(newAccount.number, sizeof(newAccount.number), stdin);
    newAccount.number[strcspn(newAccount.number, "\n")] = '\0'; // Remove newline character
    printf("Enter account type (SB, CA, RD, FD): ");
    fgets(newAccount.type, sizeof(newAccount.type), stdin);
    newAccount.type[strcspn(newAccount.type, "\n")] = '\0'; // Remove newline character
    printf("Enter opening balance: ");
    scanf("%f", &newAccount.balance);
    getchar(); // Consume newline character

    fprintf(accountFile, "%s %s %s %.2f\n", newAccount.id, newAccount.number, newAccount.type, newAccount.balance);
    fclose(accountFile);

    printf("Account added successfully for customer with ID %s.\n", customerID);
}


void deposit() {
    char account_number[17];
    float amount;
    
    printf("Enter account number: ");
    scanf("%s", account_number);
    printf("Enter deposit amount: ");
    scanf("%f", &amount);
    
    FILE *accountFile = fopen(ACCOUNT_FILE, "r+");
    if (accountFile == NULL) {
        printf("Error: Unable to open account file.\n");
        return;
    }
    
    struct Account account;
    int found = 0;
    while (fread(&account, sizeof(struct Account), 1, accountFile)) {
        if (strcmp(account.number, account_number) == 0) {
            found = 1;
            break;
        }
    }
    if (!found) {
        printf("Account not found.\n");
        fclose(accountFile);
        return;
    }
    
    // Update balance
    account.balance += amount;
    fseek(accountFile, -(long int)sizeof(struct Account), SEEK_CUR);
    fwrite(&account, sizeof(struct Account), 1, accountFile);
    fclose(accountFile);
    
    // Log transaction
    FILE *transactionFile = fopen(TRANSACTION_FILE, "a");
    if (transactionFile == NULL) {
        printf("Error: Unable to open transaction file.\n");
        return;
    }
    struct Transaction transaction;
    strcpy(transaction.account_number, account_number);
    strcpy(transaction.type, "Deposit");
    strcpy(transaction.date, "2024-03-10"); // You may need to replace this with the actual date
    transaction.amount = amount;
    fwrite(&transaction, sizeof(struct Transaction), 1, transactionFile);
    fclose(transactionFile);
    
    printf("Deposit successful.\n");
}

void withdraw() {
    char account_number[17];
    float amount;
    
    printf("Enter account number: ");
    scanf("%s", account_number);
    printf("Enter withdrawal amount: ");
    scanf("%f", &amount);
    
    FILE *accountFile = fopen(ACCOUNT_FILE, "r+");
    if (accountFile == NULL) {
        printf("Error: Unable to open account file.\n");
        return;
    }
    
    struct Account account;
    int found = 0;
    while (fread(&account, sizeof(struct Account), 1, accountFile)) {
        if (strcmp(account.number, account_number) == 0) {
            found = 1;
            break;
        }
    }
    if (!found) {
        printf("Account not found.\n");
        fclose(accountFile);
        return;
    }
    
    if (amount > account.balance) {
        printf("Insufficient balance.\n");
        fclose(accountFile);
        return;
    }
    
    // Update balance
    account.balance -= amount;
    fseek(accountFile, -(long int)sizeof(struct Account), SEEK_CUR);
    fwrite(&account, sizeof(struct Account), 1, accountFile);
    fclose(accountFile);
    
    // Log transaction
    FILE *transactionFile = fopen(TRANSACTION_FILE, "a");
    if (transactionFile == NULL) {
        printf("Error: Unable to open transaction file.\n");
        return;
    }
    struct Transaction transaction;
    strcpy(transaction.account_number, account_number);
    strcpy(transaction.type, "Withdrawal");
    strcpy(transaction.date, "2024-03-10"); // You may need to replace this with the actual date
    transaction.amount = amount;
    fwrite(&transaction, sizeof(struct Transaction), 1, transactionFile);
    fclose(transactionFile);
    
    printf("Withdrawal successful.\n");
}

void balanceEnquiry() {
    char account_number[17];
    
    printf("Enter account number: ");
    scanf("%s", account_number);
    
    FILE *accountFile = fopen(ACCOUNT_FILE, "r");
    if (accountFile == NULL) {
        printf("Error: Unable to open account file.\n");
        return;
    }
    
    struct Account account;
    int found = 0;
    while (fread(&account, sizeof(struct Account), 1, accountFile)) {
        if (strcmp(account.number, account_number) == 0) {
            found = 1;
            break;
        }
    }
    fclose(accountFile);
    if (!found) {
        printf("Account not found.\n");
        return;
    }
    
    printf("Account Balance: %.2f\n", account.balance);
}

void loanApprovals() {
    // Implement loan approval functionality here
    // This function could involve reading customer and account details,
    // assessing their eligibility for a loan based on certain criteria,
    // and updating a file/database to reflect the loan approval status.
    // It's a more complex operation and would require specific requirements
    // and criteria to be defined.
    printf("Loan approval functionality is not implemented yet.\n");
}

void logoff() {
    printf("Logged off successfully.\n");
}
