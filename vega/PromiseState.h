// SPDX-License-Identifier: MulanPSL-2.0

#pragma once

#include <exception>
#include <optional>
#include <vector>
#include <functional>
#include <memory>


namespace vega {


// forward declaration
class Scheduler;


enum class PromiseStatus { Pending, Fulfilled, Rejected };


class PromiseStateBase : public std::enable_shared_from_this<PromiseStateBase> {
protected:

    PromiseStateBase() = default;


public:

    static std::shared_ptr<PromiseStateBase> create() {
        return std::make_shared<PromiseStateBase>(PromiseStateBase{});
    }

    template <typename T = PromiseStateBase>
    std::shared_ptr<T> getPtr() {
        return shared_from_this();
    }


    std::shared_ptr<PromiseStateBase> getPtr() {
        return this->getPtr<>();
    }

    virtual ~PromiseStateBase() = default;

public:

    PromiseStatus status = PromiseStatus::Pending;
    std::exception_ptr exception;

    /**
     * It is user's responsibility to ensure this is not destroyed before promise ends.
     *
     * If set, the promise will be scheduled on the given scheduler.
     * If null, the promise will be scheduled on the current thread.
     */
    Scheduler* scheduler = nullptr;
    
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


    void resumeContinuationsOnScheduler(Scheduler* scheduler = nullptr);


    void reject(std::exception_ptr e) {
        if (status != PromiseStatus::Pending) {
            return;
        }

        exception = e;
        status = PromiseStatus::Rejected;
        
        resumeContinuationsOnScheduler();
    }
};


template <typename T>
class PromiseState : public PromiseStateBase {
protected:
    PromiseState() = default;

public:
    static std::shared_ptr<PromiseState<T>> create() {
        return std::make_shared<PromiseState<T>>(PromiseState<T>{});
    }


    std::optional<T> value;


    void resolve(T v) {
        if (status != PromiseStatus::Pending) {
            return;
        }

        value = std::move(v);
        status = PromiseStatus::Fulfilled;
        
        resumeContinuationsOnScheduler();
    }
};


template<>
struct PromiseState<void> : public PromiseStateBase {
protected:
    
    PromiseState() = default;

public:
    static std::shared_ptr<PromiseState<void>> create() {
        return std::make_shared<PromiseState<void>>(PromiseState<void>{});
    }

    void resolve() {
        if (status != PromiseStatus::Pending) {
            return;
        }

        status = PromiseStatus::Fulfilled;
        
        resumeContinuationsOnScheduler();
    }
};



}  // namespace vega

