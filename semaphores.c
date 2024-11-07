/*
 * Bank Account Simulation with Synchronization
 * Created by: Brandon Radford
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int account_balance = 0;       // Shared bank account balance
pthread_mutex_t balance_mutex; // Mutex for synchronizing balance operations
pthread_cond_t withdraw_cond;  // Condition variable to handle withdrawals
int withdraw_active = 0;       // Flag to indicate if a withdrawal is active

// Function to check the current balance
void read_balance(int account_number) {
    pthread_mutex_lock(&balance_mutex);  // Lock the mutex
    printf("Account Holder %d reads balance: $%d\n", account_number, account_balance);
    pthread_mutex_unlock(&balance_mutex); // Unlock the mutex
}

// Function to deposit a specified amount into the account
void deposit_amount(int account_number, int amount) {
    pthread_mutex_lock(&balance_mutex);  // Lock the mutex
    account_balance += amount;
    printf("Account Holder %d deposits: $%d, New Balance: $%d\n", account_number, amount, account_balance);
    pthread_mutex_unlock(&balance_mutex); // Unlock the mutex
}

// Function to withdraw a specified amount from the account
void withdraw_amount(int account_number, int amount) {
    pthread_mutex_lock(&balance_mutex);      // Lock the mutex

    // Wait if a withdrawal is currently active
    while (withdraw_active) {
        pthread_cond_wait(&withdraw_cond, &balance_mutex);
    }

    // Set the withdrawal as active
    withdraw_active = 1;

    // Check if there are sufficient funds to withdraw
    if (account_balance >= amount) {
        account_balance -= amount;
        printf("Account Holder %d withdraws: $%d, New Balance: $%d\n", account_number, amount, account_balance);
    } else {
        printf("Account Holder %d attempted to withdraw $%d but insufficient balance. Current Balance: $%d\n", account_number, amount, account_balance);
    }

    // Reset withdrawal flag and signal other threads
    withdraw_active = 0;
    pthread_cond_signal(&withdraw_cond);
    pthread_mutex_unlock(&balance_mutex);    // Unlock the mutex
}

// Account holder function to simulate random actions
void* account_holder(void* arg) {
    int account_number = *((int*)arg);
    free(arg);

    // Simulate random operations
    for (int i = 0; i < 5; i++) {
        int action = rand() % 3;  // Randomly choose an action: read (0), deposit (1), or withdraw (2)
        int amount = (rand() % 100) + 1;  // Random amount between 1 and 100

        if (action == 0) {
            read_balance(account_number);
        } else if (action == 1) {
            deposit_amount(account_number, amount);
        } else if (action == 2) {
            withdraw_amount(account_number, amount);
        }

        sleep(1);  // Short delay between actions
    }

    return NULL;
}

int main() {
    pthread_t account_threads[3];

    // Initialize the mutex and condition variable
    pthread_mutex_init(&balance_mutex, NULL);
    pthread_cond_init(&withdraw_cond, NULL);

    // Create three account holder threads
    for (int i = 0; i < 3; i++) {
        int* account_number = malloc(sizeof(int));
        *account_number = i + 1;  // Assign unique account numbers (1, 2, 3)
        pthread_create(&account_threads[i], NULL, account_holder, account_number);
    }

    // Wait for all threads to complete
    for (int i = 0; i < 3; i++) {
        pthread_join(account_threads[i], NULL);
    }

    // Destroy the mutex and condition variable
    pthread_mutex_destroy(&balance_mutex);
    pthread_cond_destroy(&withdraw_cond);

    return 0;
}
