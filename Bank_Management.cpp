#include<iostream>
#include<string>
#include<unordered_map>
#include<cstdlib>  
#include<ctime>
#include<sqlite3.h>

using namespace std;

class BankAccount{
    private:
    string name;
    int accountNum;
    double balance;
    string password;
    string email;
    
    public:

    BankAccount() {
        name = "";
        accountNum = 0;
        balance = 0.0;
        password = "";
        email = "";
    }

    BankAccount(string name,int accountNum, double balance, string password, string email){
        this -> name = name;
        this -> accountNum = accountNum;
        this -> balance = balance;
        this -> password = password;
        this -> email = email;
    }
    
    string getName()const{
        return name;
    }

    int getAccountNum()const{
        return accountNum;
    }

    double getBalance()const{
        return balance;
    }

    void deposit(double amount){
        balance+=amount;
    }

    string getPassword()const{
        return password;
    }

    string getEmail()const{
        return email;
    }

    void withdraw(double amount){
        if(balance>=amount){
            balance-=amount;
            cout<<"₹"<<amount<<" Withdraw Successfully.\n";
        }
        else{
            cout<<"Insufficient Balance!\n";
        }     
    }

    string toFileString()const{
        return name + " " + to_string(accountNum)+" "+to_string(balance)+" "+ password+" "+email;
    }
};

class BankManagement{
    private:
    unordered_map<int, BankAccount> accounts;
    sqlite3* db;

    public:

    BankManagement(){
        connectDB();
        createTables();
        loadFromDB();
    }

    ~BankManagement(){
        sqlite3_close(db);
    }

    void connectDB(){
        int dbStatus = sqlite3_open("Bank.db", &db);
        if (dbStatus != SQLITE_OK) {
            cerr << "Error opening database!\n";
            ::exit(1);  // Use ::exit() to avoid confusion
        }
        else{
            cout << "Database connected successfully.\n";
        }
    }    

    void createTables() {
        string createAccountTable = "CREATE TABLE IF NOT EXISTS Accounts ("
                                    "accountNum INTEGER PRIMARY KEY, "
                                    "name TEXT NOT NULL, "
                                    "balance REAL, "
                                    "password TEXT, "
                                    "email TEXT);";
    
        string createTransactionTable = "CREATE TABLE IF NOT EXISTS Transactions ("
                                        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                        "accountNum INTEGER, "
                                        "transactionType TEXT, "
                                        "amount REAL, "
                                        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
                                        "FOREIGN KEY(accountNum) REFERENCES Accounts(accountNum));";
    
        char* errorMsg;
        int exit1 = sqlite3_exec(db, createAccountTable.c_str(), NULL, 0, &errorMsg);
        if (exit1 != SQLITE_OK) {
            cerr << "Error creating Accounts table: " << errorMsg << "\n";
            sqlite3_free(errorMsg);
        }
    
        int exit2 = sqlite3_exec(db, createTransactionTable.c_str(), NULL, 0, &errorMsg);
        if (exit2 != SQLITE_OK) {
            cerr << "Error creating Transactions table: " << errorMsg << "\n";
            sqlite3_free(errorMsg);
        }
    }
    
    
    void loadFromDB(){
        string selectQuery = "SELECT * FROM Accounts;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, selectQuery.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                int accountNum = sqlite3_column_int(stmt, 0);
                string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                double balance = sqlite3_column_double(stmt, 2);
                string password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                string email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

                accounts[accountNum] = BankAccount(name, accountNum, balance, password, email);
            }
            sqlite3_finalize(stmt);
        } else {
            cerr << "Error loading accounts from database.\n";
        }
    }

    void saveAccountToDB(const BankAccount& account){
        string insertQuery = "INSERT OR REPLACE INTO Accounts (accountNum, name, balance, password, email) VALUES (" +
        to_string(account.getAccountNum()) + ", '" + account.getName() + "', " +
        to_string(account.getBalance()) + ", '" + account.getPassword() + "', '" +
        account.getEmail() + "');";

        char* errorMsg;
        int exit = sqlite3_exec(db, insertQuery.c_str(), NULL, 0, &errorMsg);

        if (exit != SQLITE_OK) {
            cerr << "Error inserting account: " << errorMsg << "\n";
            sqlite3_free(errorMsg);
        }
    }

    void saveTransactionToDB(int accountNum, string transactionType, double amount) {
        string insertQuery = "INSERT INTO Transactions (accountNum, transactionType, amount) VALUES (" + to_string(accountNum) + ", '" + transactionType + "', " + to_string(amount) + ");";
        char* errorMsg;
        int exit = sqlite3_exec(db, insertQuery.c_str(), NULL, 0, &errorMsg);
    
        if (exit != SQLITE_OK) {
            cerr << "Error inserting transaction: " << errorMsg << "\n";
            sqlite3_free(errorMsg);
        }
    }
    
    void showTransactionHistory(int accountNum) {
        string selectQuery = "SELECT transactionType, amount, timestamp FROM Transactions WHERE accountNum = " + to_string(accountNum) + " ORDER BY timestamp DESC;";
        sqlite3_stmt* stmt;
    
        if (sqlite3_prepare_v2(db, selectQuery.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
            cout << "\nTransaction History for Account " << accountNum << ":\n";
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                string transactionType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                double amount = sqlite3_column_double(stmt, 1);
                string timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    
                cout << transactionType << " ₹" << amount << " on " << timestamp << "\n";
            }
            sqlite3_finalize(stmt);
        } else {
            cerr << "Error retrieving transaction history.\n";
        }
    }
    

    void deleteAccountFromDB(int accountNum) {
        string deleteQuery = "DELETE FROM Accounts WHERE accountNum = " + to_string(accountNum) + ";";
        char* errorMsg;
        int exit = sqlite3_exec(db, deleteQuery.c_str(), NULL, 0, &errorMsg);

        if (exit != SQLITE_OK) {
            cerr << "Error deleting account: " << errorMsg << "\n";
            sqlite3_free(errorMsg);
        }
    }

    void AddAccount(string name,double balance, string password, string email){
        int accountNum;
        bool unique;
        do {
            unique = true;
            accountNum = 1000 + (rand() % 9000); // Generate a random account number
        }
        while(accounts.find(accountNum)!= accounts.end());

        BankAccount newAccount(name, accountNum, balance, password, email);
        accounts[accountNum] = newAccount; 
        saveAccountToDB(newAccount);       
        
        cout<<"Account Number: "<<accountNum<<endl;
        cout<<"Account Created Successfully.\n";

    }

    void showAllAccounts(){
        cout<<"All Accounts\n";
        for (const auto& acc : accounts) {
            cout << "Name: " << acc.second.getName() << " Account Number: " << acc.first<< " Balance: ₹" << acc.second.getBalance() << "\n";
        }
        
    }

    void showTotalBalance() {
        double totalBalance = 0;
        
        for (const auto& acc : accounts) {
            totalBalance += acc.second.getBalance();
        }
        
    
        cout << "Total Amount Present in the Bank: ₹" << totalBalance << "\n";
    }
    
    
    void searchAccount(int accountNum){
        if(accounts.find(accountNum)!=accounts.end()){
                string enteredPassword;
                cout<<"Enter Password: ";
                cin>>enteredPassword;

                if(enteredPassword==accounts[accountNum].getPassword()){
                    cout<<"Name: "<<accounts[accountNum].getName()<<" Account Number: "<<accounts[accountNum].getAccountNum()<<" Balance: "<<accounts[accountNum].getBalance()<<"\n";
                }else{
                    cout<<"Incorrect Password! Access Denied.\n";
                }
            }
        else{
            cout<<"Account not found!\n";
        }
        
    }

    BankAccount* findAccount(int accountNum, string password){
        auto it = accounts.find(accountNum);
        if(it != accounts.end()){
            if(it->second.getPassword() == password){
                return &(it->second);
            }
            else{
                cout<<"Incorrect Password! Access Denied.\n";
                return nullptr;
            }
        }

        cout<<"Account not found!\n";
        return nullptr;
    }

    void deleteAccount(int accountNum, string password){
        auto it = accounts.find(accountNum);
        if (it != accounts.end()) {
            if(it->second.getPassword() == password){
                accounts.erase(it);
                deleteAccountFromDB(accountNum);
                cout << "Account Deleted Successfully\n";
            }
            else{
                cout<<"Incorrect password!\n";
                return;
            }    
        }
        else{
            cout<<"Account not found!\n";
        }
    }
};

bool adminLogin(){                              // admin login pass
    string correctEmail = "admin@bank.in";
    string correctPassword = "admin123";

    string email, password;

    cout<<"Enter E-Mail: ";
    cin>>email;
    if(email!=correctEmail){
        cout<<"Invalid email! Access Denied.\n";
        return false;
    }

    cout<<"Enter Admin Password: ";
    cin>>password;
    if(password!=correctPassword){
        cout<<"Invalid Password! Access Denied.\n";
        return false;
    }
    return true;    
}

int main(){
    
    srand(time(0));

    BankManagement bank;
    char option;

    mainMenu:
    do{
    cout<<"\nBank Management System\n";
    cout<<"\tMain Menu\n";
    cout<<"1. Admin Login\n";
    cout<<"2. User Login\n";
    cout<<"3. Exit\n";
    cout<<"------------------------------------\n";

    int login;
    cout<<"Select Login: ";
    cin>>login;

    if (login==1){
        if (adminLogin()){
            int adminChoice;
            do{
                cout<<"\nAdmin Menu\n";
                cout<<"1. Show all accounts\n";
                cout<<"2. Show total available balance in bank\n";
                cout<<"3. Exit\n";
                cout<<"------------------------------------\n";
                cout<<"Enter your choice: ";
                cin>>adminChoice; 

                switch (adminChoice){
                    case 1:{
                        bank.showAllAccounts();
                        break;
                    }
                    case 2:{
                        bank.showTotalBalance();
                        break;
                    }
                    case 3:{
                        cout<<"Returning to Main Menu.\n";
                        goto mainMenu;
                    }
                }
            }while(adminChoice!=3);
        }
    }
    else if(login == 2){
        int choice;
        do{
        cout<<"\nUser Menu\n";
        cout<<"1. Create New Account\n";
        cout<<"2. Search Account\n";
        cout<<"3. Deposit Money\n";
        cout<<"4. Withdraw Money\n";
        cout<<"5. Delete Account\n";
        cout<<"6. Show Transaction History\n";
        cout<<"7. Exit\n";
        cout<<"------------------------------------\n";
        cout<<"Enter your choice: ";
        cin>>choice;

        switch(choice){
            case 1:{    // Create New Account
                string name;
                double balance;
                string password;
                string email;

                cout<<"Enter Name: ";
                cin.ignore();
                getline(cin,name);

                cout<<"Enter E-Mail: ";
                cin>>email;

                cout<<"Enter Initial Balance: ₹";
                cin>>balance;

                cout<<"Create password: ";
                cin>>password;

                bank.AddAccount(name,balance,password,email);
                break;
            }

            case 2:{    // Search Account
                int accountNum;
                cout<<"Enter Account Number: ";
                cin>>accountNum;
                bank.searchAccount(accountNum);
                break;
            }

            case 3:{    // Deposit Money
                int accountNum;
                double amount;
                string password;

                cout<<"Enter Account Number: ";
                cin>>accountNum;

                cout<<"Enter Password: ";
                cin>>password;
                BankAccount* account = bank.findAccount(accountNum,password);
                if(account != NULL){
                    cout<<"Enter amount to deposit: ₹";
                    cin>>amount;
                    account->deposit(amount);
                    cout << "₹" << amount << " Deposited Successfully.\n";
                    cout << "Updated Balance: ₹" << account->getBalance() << "\n";
                    bank.saveAccountToDB(*account);
                    bank.saveTransactionToDB(accountNum, "Deposit", amount);

                }
                else{
                    cout<<"Account not found!\n";
                }
                break;
            }

            case 4:{    // Withdraw Money
                int accountNum;
                double amount;
                string password;
                cout<<"Enter Account Number: ";
                cin>>accountNum;
                cout<<"Enter password:";
                cin>>password;
                BankAccount* account = bank.findAccount(accountNum, password);
                if(account != NULL){
                    cout<<"Enter amount to withdraw: ₹";
                    cin>>amount;
                    account->withdraw(amount);
                    cout << "Updated Balance: ₹" << account->getBalance() << "\n";
                    bank.saveAccountToDB(*account);
                    bank.saveTransactionToDB(accountNum, "Withdraw", amount);

                }
                else{
                    cout<<"Account not found!\n";
                }

                break;
            }

            case 5:{
                int accountNum;
                string password;

                cout<<"Enter Account Number: ";
                cin>>accountNum;
                cout<<"Enter password: ";
                cin>>password;

                bank.deleteAccount(accountNum,password);
                break;
            }

            case 6: {    // Show Transaction History
                int accountNum;
                string password;
                cout<<"Enter Account Number: ";
                cin>>accountNum;
                cout<<"Enter Password: ";
                cin>>password;
            
                bank.showTransactionHistory(accountNum);
                break;
            }
            
            
            case 7:{
                cout<<"Returning to Main Menu.\n";
                goto mainMenu;
            }
        }
        
        cout<<"Do you want to continue or exit?[y/n]\n";
        cin>>option;
        }
        while (option=='y' || option=='Y');
        cout<<"Thanks for your transaction.\n";
    }
    else if (login == 3) {
        break;
    } 
    else {
        cout << "Invalid choice! Try again.\n";
    }
    
    } while (true);

    cout << "Thanks for using the Bank Management System.\n";
    return 0;
}