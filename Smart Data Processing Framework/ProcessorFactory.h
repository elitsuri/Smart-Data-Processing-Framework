#pragma once

#include <memory>
#include <map>
#include <functional>
#include <stdexcept>
#include "Processor.h"

enum class ProcessorType {
    NUMERIC,
    STATISTICAL,
    FILTERING,
    AMPLIFICATION
};

template<typename T>
class ProcessorFactory {
public:
    static ProcessorFactory& getInstance() {
        static ProcessorFactory instance;
        return instance;
    }

    std::shared_ptr<Processor<T>> createProcessor(
        ProcessorType type,
        const std::map<std::string, double>& params = {}) {
        
        switch (type) {
            case ProcessorType::NUMERIC: {
                double multiplier = params.count("multiplier") ? 
                    params.at("multiplier") : 2.0;
                return std::make_shared<NumericProcessor<T>>(
                    static_cast<T>(multiplier));
            }
            case ProcessorType::STATISTICAL: {
                return std::make_shared<StatisticalProcessor<T>>();
            }
            case ProcessorType::FILTERING: {
                double threshold = params.count("threshold") ? 
                    params.at("threshold") : 0.0;
                return std::make_shared<FilteringProcessor<T>>(
                    static_cast<T>(threshold));
            }
            case ProcessorType::AMPLIFICATION: {
                double gain = params.count("gain") ? 
                    params.at("gain") : 1.5;
                return std::make_shared<AmplificationProcessor<T>>(gain);
            }
            default:
                throw std::invalid_argument("Unknown processor type");
        }
    }

private:
    ProcessorFactory() = default;
    ProcessorFactory(const ProcessorFactory&) = delete;
    ProcessorFactory& operator=(const ProcessorFactory&) = delete;
};

// Specialization for string type
template<>
class ProcessorFactory<std::string> {
public:
    static ProcessorFactory& getInstance() {
        static ProcessorFactory instance;
        return instance;
    }

    std::shared_ptr<Processor<std::string>> createProcessor(
        ProcessorType type,
        const std::map<std::string, double>& params = {}) {
        
        switch (type) {
            case ProcessorType::NUMERIC: {
                int repetitions = params.count("repetitions") ? 
                    static_cast<int>(params.at("repetitions")) : 2;
                return std::make_shared<NumericProcessor<std::string>>(repetitions);
            }
            case ProcessorType::STATISTICAL:
            case ProcessorType::FILTERING:
            case ProcessorType::AMPLIFICATION:
                throw std::invalid_argument("This processor type is not supported for strings");
            default:
                throw std::invalid_argument("Unknown processor type");
        }
    }

private:
    ProcessorFactory() = default;
    ProcessorFactory(const ProcessorFactory&) = delete;
    ProcessorFactory& operator=(const ProcessorFactory&) = delete;
};
