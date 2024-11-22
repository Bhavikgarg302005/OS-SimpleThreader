#include <iostream>
#include <list>
#include <pthread.h>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <chrono>

// Struct for sending arguments for single parallel for
typedef struct {
    int low; 
    int high;
    std::function<void(int)> lambda;
} thread_args;

// Struct for sending arguments for double parallel for
typedef struct {
    int low1; 
    int high1;
    int low2;
    int high2;
    std::function<void(int,int)> lambda;
} thread_args2;

// Function for single variable in lambda function 
void *thread_fxn(void *ptr) {
    thread_args *t = (thread_args *)ptr;
    for (int i = t->low; i < t->high; i++) {
        t->lambda(i);
    }
    pthread_exit(nullptr);
}

// Function for double variable in lambda function
void* thread_fxn2(void *ptr) {
    thread_args2 *t = (thread_args2 *)ptr;
    for (int i = t->low1; i < t->high1; ++i) {
        for (int j = t->low2; j < t->high2; ++j) {
            t->lambda(i, j);
        }
    }
    pthread_exit(nullptr);
}

// Helper function to get the minimum of two integers
int min(int a, int b) {
    return (a <= b) ? a : b;
}

// Function for handling single valued lambda function
void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads) {
    auto start_time = std::chrono::high_resolution_clock::now();
    if (numThreads <= 0) {
        perror("Given wrong number of Threads");
        exit(EXIT_FAILURE);
    }

    thread_args Args[numThreads];
    pthread_t Threads[numThreads];

    int chuck_size = (high - low + numThreads - 1) / numThreads; // Ensure each thread gets at least one item

    for (int i = 0; i < numThreads; i++) {
        Args[i].low = low + i * chuck_size;
        Args[i].high = min(high, Args[i].low + chuck_size);
        Args[i].lambda = lambda;
        pthread_create(&Threads[i], nullptr, thread_fxn, (void *)&Args[i]);
    }

    for (int i = 0; i < numThreads; i++) {
        pthread_join(Threads[i], nullptr);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "Execution time: " << duration.count() << " microseconds\n";
}

// // Function for handling double valued lambda function
void parallel_for(int low1, int high1, int low2, int high2, std::function<void(int, int)> &&lambda, int numThreads) {
    auto start_time = std::chrono::high_resolution_clock::now();

    // Validate the number of threads
    if (numThreads <= 0) {
        std::cerr << "Error: Number of threads must be greater than zero." << std::endl;
        exit(EXIT_FAILURE);
    }

    pthread_t threads[numThreads];
    thread_args2 args[numThreads];
    int chunk = (high1 - low1) / numThreads;

    // Create threads
    for (int i = 0; i < numThreads; ++i) {
        int range = (high1 - low1) / numThreads;
        int thread_low = low1 + i * range;
        int thread_high = (i == numThreads - 1) ? high1 : thread_low + range;

        int range2 = (high2 - low2) / numThreads;
        int thread_low2 = low2 + i * range2;
        int thread_high2 = (i == numThreads - 1) ? high2 : thread_low2 + range2;

        args[i].low1 = thread_low;
        args[i].high1 = thread_high;
        args[i].low2 = low2;
        args[i].high2 = high2;
        args[i].lambda = lambda;

        // Create thread and check for errors
        pthread_create(&threads[i], nullptr, thread_fxn2, static_cast<void *>(&args[i]));

    }

    // Join threads
    for (int i = 0; i < numThreads; ++i) {
        // Join thread and check for errors
        pthread_join(threads[i], nullptr);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "Execution time: " << duration.count() << " microseconds\n";
}
