// SPDX-License-Identifier: MulanPSL-2.0

#pragma once


#include <coroutine>
#include <exception>
#include <memory>

#include <vega/PromiseState.h>

template <typename T>
struct Promise {
    std::shared_ptr<PromiseState<T>> state;

    Promise() : state(std::make_shared<PromiseState<T>>()) {}
    Promise(std::shared_ptr<PromiseState<T>> state) : state(state) {}

    struct promise_type {
        std::shared_ptr<PromiseState<T>> state = std::make_shared<PromiseState<T>>();

        Promise get_return_object() { return Promise{state}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }

        auto return_value(T&& v) { state->resolve(v); }
        void unhandled_exception() { state->reject(std::current_exception()); }
    };
    

    struct Awaiter {
        std::shared_ptr<PromiseState<T>> state;

        bool await_ready() { return state->status != PromiseStatus::Pending; }

        void await_suspend(std::coroutine_handle<> h) {
            state->addContinuation( [h] () { h.resume(); } );
        }
        
        T&& await_resume() {
            if (state->exception)
                std::rethrow_exception(state->exception);
            return std::move(*state->value);
        }
    };

    Awaiter operator co_await() { return Awaiter{state}; }
};


template<>
struct Promise<void> {

    std::shared_ptr<PromiseState<void>> state;

    Promise() : state(std::make_shared<PromiseState<void>>()) {}
    Promise(std::shared_ptr<PromiseState<void>> state) : state(state) {}

    struct promise_type {
        std::shared_ptr<PromiseState<void>> state = std::make_shared<PromiseState<void>>();
        
        Promise get_return_object() { return Promise{state}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        
        void return_void() { state->resolve(); }
        void unhandled_exception() { state->reject(std::current_exception()); }
    };
    

    struct Awaiter {
        std::shared_ptr<PromiseState<void>> state;

        bool await_ready() {
            return state->status != PromiseStatus::Pending;
        }

        void await_suspend(std::coroutine_handle<> h) {
            state->addContinuation( [h] () { h.resume(); } );
        }
        
        void await_resume() {
            if (state->exception)
                std::rethrow_exception(state->exception);
        }
    };

    Awaiter operator co_await() { return Awaiter{state}; }
};

