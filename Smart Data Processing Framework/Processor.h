#pragma once

#include <string>
#include <memory>
#include <typeinfo>
#include "Logger.h"

template<typename T>
class Processor {
public:
    virtual ~Processor() = default;

    virtual T process(const T& input) = 0;

    virtual std::string getName() const = 0;

    std::string getDataType() const {
        return typeid(T).name();
    }

    virtual void reset() {
        LOG_INFO("Processor reset: " + getName());
    }
};

// ============ CONCRETE IMPLEMENTATIONS ============

// Numeric Processor - for arithmetic operations
template<typename T>
class NumericProcessor : public Processor<T> {
public:
    NumericProcessor(T multiplier = 2) : multiplier_(multiplier) {}

    T process(const T& input) override {
        T result = input * multiplier_;
        LOG_DEBUG("NumericProcessor: " + std::to_string(input) + " -> " + std::to_string(result));
        return result;
    }

    std::string getName() const override {
        return "NumericProcessor";
    }

private:
    T multiplier_;
};

// String Processor - for string operations
template<>
class NumericProcessor<std::string> : public Processor<std::string> {
public:
    NumericProcessor(int repetitions = 2) : repetitions_(repetitions) {}

    std::string process(const std::string& input) override {
        std::string result;
        for (int i = 0; i < repetitions_; ++i) {
            result += input;
        }
        LOG_DEBUG("StringProcessor: " + input + " repeated " + std::to_string(repetitions_) + " times");
        return result;
    }

    std::string getName() const override {
        return "StringProcessor";
    }

private:
    int repetitions_;
};

// Statistical Processor - calculates statistics
template<typename T>
class StatisticalProcessor : public Processor<T> {
public:
    T process(const T& input) override {
        // For numeric types: apply smoothing/averaging
        total_ += input;
        count_++;
        T average = total_ / count_;
        LOG_DEBUG("StatisticalProcessor: average = " + std::to_string(average));
        return average;
    }

    std::string getName() const override {
        return "StatisticalProcessor";
    }

    void reset() override {
        total_ = 0;
        count_ = 0;
        Processor<T>::reset();
    }

private:
    T total_ = 0;
    int count_ = 0;
};

// Filtering Processor - filters data based on threshold
template<typename T>
class FilteringProcessor : public Processor<T> {
public:
    FilteringProcessor(T threshold = 0) : threshold_(threshold) {}

    T process(const T& input) override {
        if (input >= threshold_) {
            LOG_DEBUG("FilteringProcessor: value " + std::to_string(input) + " passed filter");
            return input;
        }
        LOG_DEBUG("FilteringProcessor: value " + std::to_string(input) + " filtered out");
        return T();
    }

    std::string getName() const override {
        return "FilteringProcessor";
    }

private:
    T threshold_;
};

// Amplification Processor - amplifies signals
template<typename T>
class AmplificationProcessor : public Processor<T> {
public:
    AmplificationProcessor(double gain = 1.5) : gain_(gain) {}

    T process(const T& input) override {
        T result = static_cast<T>(input * gain_);
        LOG_DEBUG("AmplificationProcessor: gain = " + std::to_string(gain_));
        return result;
    }

    std::string getName() const override {
        return "AmplificationProcessor";
    }

private:
    double gain_;
};
