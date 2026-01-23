// SPDX-License-Identifier: MulanPSL-2.0

#pragma once

#include <exception>
#include <optional>
#include <vector>
#include <functional>


namespace vega {


enum class PromiseStatus { Pending, Fulfilled, Rejected };


class PromiseStateBase {
public:

    virtual ~PromiseStateBase() = default;

public:

    PromiseStatus status = PromiseStatus::Pending;
    std::exception_ptr exception;
    
protected:

    std::vector<std::function<void()>> continuations;

public:

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


    void reject(std::exception_ptr e) {
        if (status != PromiseStatus::Pending) {
            return;
        }

        exception = e;
        status = PromiseStatus::Rejected;
        
        resumeContinuations();
    }
};


template <typename T>
class PromiseState : public PromiseStateBase {
public:
    std::optional<T> value;


    void resolve(T v) {
        if (status != PromiseStatus::Pending) {
            return;
        }

        value = std::move(v);
        status = PromiseStatus::Fulfilled;
        
        resumeContinuations();
    }
};


template<>
struct PromiseState<void> : public PromiseStateBase {
public:
    void resolve() {
        if (status != PromiseStatus::Pending) {
            return;
        }

        status = PromiseStatus::Fulfilled;
        
        resumeContinuations();
    }
};



}  // namespace vega

