// SPDX-License-Identifier: MulanPSL-2.0

#pragma once

#include <exception>
#include <optional>
#include <vector>
#include <functional>


namespace vega {


enum class PromiseStatus { Pending, Fulfilled, Rejected };


template <typename T>
struct PromiseState {
    PromiseStatus status = PromiseStatus::Pending;
    
    std::optional<T> value;
    std::exception_ptr exception;
    std::vector<std::function<void()>> continuations;


    void resolve(T v) {
        if (status != PromiseStatus::Pending) {
            return;
        }

        value = std::move(v);
        status = PromiseStatus::Fulfilled;
        
        resumeContinuations();
    }


    void reject(std::exception_ptr e) {
        if (status != PromiseStatus::Pending) {
            return;
        }

        exception = e;
        status = PromiseStatus::Rejected;
        
        resumeContinuations();
    }


    void addContinuation(std::function<void()> cont) {
        if (status != PromiseStatus::Pending)
            cont();
        else
            continuations.push_back(std::move(cont));
    }


    void resumeContinuations() {
        for (auto& cont : continuations) {
            cont();
        }
        continuations.clear();
    }
};


template<>
struct PromiseState<void> {
    PromiseStatus status = PromiseStatus::Pending;
    
    std::exception_ptr exception;
    std::vector<std::function<void()>> continuations;


    void resolve() {
        if (status != PromiseStatus::Pending) {
            return;
        }

        status = PromiseStatus::Fulfilled;
        
        resumeContinuations();
    }


    void reject(std::exception_ptr e) {
        if (status != PromiseStatus::Pending) {
            return;
        }

        exception = e;
        status = PromiseStatus::Rejected;
        
        resumeContinuations();
    }


    void addContinuation(std::function<void()> cont) {
        if (status != PromiseStatus::Pending)
            cont();
        else
            continuations.push_back(std::move(cont));
    }


    void resumeContinuations() {
        for (auto& cont : continuations) {
            cont();
        }
        continuations.clear();
    }
};



}  // namespace vega

