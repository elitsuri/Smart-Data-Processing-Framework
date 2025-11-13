#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <vector>

#include "ProcessingSystem.h"
#include "ProcessorFactory.h"

void printDivider(const std::string& title = "")
{
    std::cout << "\n" << std::string(60, '=') << std::endl;
    if (!title.empty()) 
    {
        std::cout << "  " << title << std::endl;
        std::cout << std::string(60, '=') << std::endl;
    }
}

// ============ TEST 1: Integer Processing ============
void testIntegerProcessing() {
    printDivider("TEST 1: Integer Processing with NumericProcessor");
    
    ProcessingSystem<int> system(4, 1000);
    system.setProcessorByType(ProcessorType::NUMERIC, {{"multiplier", 5.0}});
    system.start();

    LOG_INFO("Adding 10 integer values for processing...");
    
    for (int i = 1; i <= 10; ++i) {
        system.addData(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Collect results
    std::vector<int> results;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    auto collected = system.getResults(10);
    for (const auto& res : collected) {
        std::cout << "Result: " << res << std::endl;
    }

    system.printStatistics();
    system.stop();
}

// ============ TEST 2: Float Processing with Filtering ============
void testFilteringProcessor() {
    printDivider("TEST 2: Float Processing with FilteringProcessor");
    
    ProcessingSystem<float> system(3, 1000);
    system.setProcessorByType(ProcessorType::FILTERING, {{"threshold", 5.0}});
    system.start();

    LOG_INFO("Adding float values (filter passes only >= 5.0)...");
    
    std::vector<float> values = {1.5f, 3.2f, 5.5f, 4.1f, 8.9f, 2.3f, 10.0f};
    for (float val : values) {
        system.addData(val);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    auto results = system.getResults(10);
    for (const auto& res : results) {
        std::cout << "Filtered Result: " << res << std::endl;
    }

    system.printStatistics();
    system.stop();
}

// ============ TEST 3: String Processing ============
void testStringProcessing() {
    printDivider("TEST 3: String Processing");
    
    ProcessingSystem<std::string> system(2, 100);
    system.setProcessorByType(ProcessorType::NUMERIC, {{"repetitions", 3.0}});
    system.start();

    LOG_INFO("Adding strings for processing...");
    
    std::vector<std::string> strings = {"Hello", "C++", "Templates"};
    for (const auto& str : strings) {
        system.addData(str);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    auto results = system.getResults(10);
    for (const auto& res : results) {
        std::cout << "String Result: " << res << std::endl;
    }

    system.printStatistics();
    system.stop();
}

// ============ TEST 4: Amplification Processor ============
void testAmplificationProcessor() {
    printDivider("TEST 4: Amplification Processor (gain = 2.5)");
    
    ProcessingSystem<double> system(4, 1000);
    system.setProcessorByType(ProcessorType::AMPLIFICATION, {{"gain", 2.5}});
    system.start();

    LOG_INFO("Adding double values for amplification...");
    
    for (int i = 1; i <= 8; ++i) {
        double value = i * 1.5;
        system.addData(value);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    auto results = system.getResults(10);
    for (const auto& res : results) {
        std::cout << "Amplified Result: " << res << std::endl;
    }

    system.printStatistics();
    system.stop();
}

// ============ TEST 5: Statistical Processor ============
void testStatisticalProcessor() {
    printDivider("TEST 5: Statistical Processor (running average)");
    
    ProcessingSystem<int> system(1, 1000);
    system.setProcessorByType(ProcessorType::STATISTICAL);
    system.start();

    LOG_INFO("Adding integer values for statistical processing...");
    
    std::vector<int> values = {10, 20, 30, 40, 50};
    for (int val : values) {
        system.addData(val);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    auto results = system.getResults(10);
    for (const auto& res : results) {
        std::cout << "Statistical Result (Average): " << res << std::endl;
    }

    system.printStatistics();
    system.stop();
}

// ============ TEST 6: Stress Test - High Throughput ============
void stressTest() {
    printDivider("TEST 6: Stress Test - High Throughput");
    
    ProcessingSystem<int> system(8, 5000);
    system.setProcessorByType(ProcessorType::NUMERIC, {{"multiplier", 2.0}});
    system.start();

    LOG_INFO("Starting high-throughput stress test...");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Producer thread - add 1000 items rapidly
    std::thread producer([&system]() {
        for (int i = 0; i < 1000; ++i) {
            system.addData(i, 5000);
            if (i % 100 == 0) {
                LOG_DEBUG("Producer: added " + std::to_string(i) + " items");
            }
        }
    });

    // Collector thread - collect results
    std::thread collector([&system]() {
        int collected = 0;
        while (collected < 1000) {
            auto result = system.getResult(100);
            if (result) {
                collected++;
                if (collected % 100 == 0) {
                    LOG_DEBUG("Collector: collected " + std::to_string(collected) + " items");
                }
            }
        }
    });

    producer.join();
    collector.join();

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime);

    std::cout << "\nStress Test Results:" << std::endl;
    std::cout << "  Time elapsed: " << duration.count() << " ms" << std::endl;
    std::cout << "  Throughput: " << (1000000.0 / duration.count()) << " items/sec" << std::endl;

    system.printStatistics();
    system.stop();
}

// ============ TEST 7: Multiple Processors (Factory Pattern) ============
void testProcessorFactory() {
    printDivider("TEST 7: Factory Pattern - Dynamic Processor Creation");
    
    LOG_INFO("Creating different processors using Factory...");

    // Integer processors
    auto& factory_int = ProcessorFactory<int>::getInstance();
    
    auto numeric = factory_int.createProcessor(
        ProcessorType::NUMERIC, {{"multiplier", 3.0}});
    auto filtering = factory_int.createProcessor(
        ProcessorType::FILTERING, {{"threshold", 5.0}});
    auto amplification = factory_int.createProcessor(
        ProcessorType::AMPLIFICATION, {{"gain", 1.5}});

    LOG_INFO("Created processors:");
    LOG_INFO("  - " + numeric->getName());
    LOG_INFO("  - " + filtering->getName());
    LOG_INFO("  - " + amplification->getName());

    // Test each processor
    int testValue = 5;
    LOG_INFO("Testing with value: " + std::to_string(testValue));
    
    std::cout << "  Numeric result: " << numeric->process(testValue) << std::endl;
    std::cout << "  Filtering result: " << filtering->process(testValue) << std::endl;
    std::cout << "  Amplification result: " << amplification->process(testValue) << std::endl;
}

// ============ Main ============
int main() {
    try {
        Logger::getInstance().setLogLevel(Logger::Level::INFO);

        printDivider("SMART DATA PROCESSING FRAMEWORK - COMPREHENSIVE TESTS");
        std::cout << "\n✅ Framework initialized with Templates, Multithreading, RAII, and Design Patterns\n";

        // Run all tests
        testIntegerProcessing();
        
        testFilteringProcessor();
        
        testStringProcessing();
        
        testAmplificationProcessor();
        
        testStatisticalProcessor();
        
        testProcessorFactory();
        
        stressTest();

        printDivider("ALL TESTS COMPLETED SUCCESSFULLY ✅");
        
        std::cout << "\n📊 Framework Features Demonstrated:" << std::endl;
        std::cout << "  ✓ Templates (generic processing for any data type)" << std::endl;
        std::cout << "  ✓ OOP & Polymorphism (Processor base class + derived classes)" << std::endl;
        std::cout << "  ✓ Design Patterns (Factory for processor creation)" << std::endl;
        std::cout << "  ✓ STL Containers (deque, unordered_map, vector)" << std::endl;
        std::cout << "  ✓ Multithreading (producer-consumer pattern)" << std::endl;
        std::cout << "  ✓ Synchronization (Mutex + Condition Variables)" << std::endl;
        std::cout << "  ✓ Smart Pointers (shared_ptr for RAII)" << std::endl;
        std::cout << "  ✓ Logging System (thread-safe logging)" << std::endl << std::endl;

    } catch (const std::exception& e) {
        LOG_CRITICAL("Application exception: " + std::string(e.what()));
        return EXIT_FAILURE;
    }
    std::cin.get();
    return EXIT_SUCCESS;
}
