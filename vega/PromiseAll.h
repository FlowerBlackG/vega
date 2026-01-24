// SPDX-License-Identifier: MulanPSL-2.0
// Mostly by Google Gemini 3.0 Pro.

#pragma once

#include <tuple>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <type_traits>

#include <vega/Promise.h> 


/**
 * namespace vega::__promise_all_details is NOT meant to be used directly. It should only be used inside this file.
 */
namespace vega::__promise_all_details {

// Trait to detect if T is a Promise<U> and extract U
template <typename T> 
struct promise_traits {
    static constexpr bool is_promise = false;
    using value_type = T;
};

template <typename T> 
struct promise_traits<Promise<T>> {
    static constexpr bool is_promise = true;
    using value_type = T;
};

// Trait to deduce the "Unwrapped" type from any argument
// (Works for T, Promise<T>, Callable->T, Callable->Promise<T>)
template <typename T>
auto get_unwrapped_type() {
    using T_decay = std::remove_cvref_t<T>;
    
    if constexpr (promise_traits<T_decay>::is_promise) {
        return typename promise_traits<T_decay>::value_type{};
    }
    else if constexpr (std::is_invocable_v<T_decay>) {
        using Ret = std::invoke_result_t<T_decay>;
        using Ret_decay = std::remove_cvref_t<Ret>;
        
        if constexpr (promise_traits<Ret_decay>::is_promise) {
            return typename promise_traits<Ret_decay>::value_type{};
        } else {
            return Ret{};
        }
    }
    else {
        return T_decay{};
    }
}

template <typename T>
using unwrap_t = decltype(get_unwrapped_type<T>());

// Normalizer: Converts Value/Callable/Promise -> Promise<T>
template <typename T>
auto to_promise(T&& arg) {
    using T_decay = std::remove_cvref_t<T>;
    using ValT = unwrap_t<T>;

    // Already a Promise?
    if constexpr (promise_traits<T_decay>::is_promise) {
        return std::forward<T>(arg);
    }
    // Is Callable?
    else if constexpr (std::is_invocable_v<T_decay>) {
        using Ret = std::invoke_result_t<T_decay>;
        // Callable returns Promise?
        if constexpr (promise_traits<std::remove_cvref_t<Ret>>::is_promise) {
            return arg();
        } else {
            // Callable returns Value (or void)
            if constexpr (std::is_void_v<Ret>) {
                arg();
                return Promise<void>::resolve();
            } else {
                return Promise<ValT>::resolve(arg());
            }
        }
    }
    // Plain Value
    else {
        return Promise<ValT>::resolve(std::forward<T>(arg));
    }
}


} // namespace vega::__promise_all_details


namespace vega {

template <typename... Args>
auto promiseAll(Args&&... args) {
    using namespace vega;
    using namespace vega::__promise_all_details;
    constexpr size_t N = sizeof...(Args);

    // --- Determine Result Type ---
    
    // Fallback for N=0 case to prevent tuple access errors
    using FirstT = std::conditional_t<(N>0), 
                   std::tuple_element_t<0, std::tuple<unwrap_t<Args>..., void>>, 
                   void>;

    // Check if all types match FirstT
    constexpr bool AllSame = (std::is_same_v<FirstT, unwrap_t<Args>> && ...);
    
    // We return vector<T> ONLY if: We have args, they are all same type, and type is not void.
    constexpr bool IsVector = (N > 0) && AllSame && !std::is_void_v<FirstT>;
    
    using ResultT = std::conditional_t<IsVector, std::vector<FirstT>, void>;
    
    // --- Setup Result Promise ---
    Promise<ResultT> result;
    result.state->scheduler = getCurrentScheduler();

    // Handle empty case
    if constexpr (N == 0) {
        result.state->resolve();
        return result;
    }

    // --- Shared State Management ---
    // We use a struct to hold the state shared across all async continuations
    struct State {
        std::atomic<size_t> remaining{N};
        std::atomic<bool> rejected{false};
        std::mutex mtx; // Protects vector writes
        
        // Storage: Use std::vector if homogeneous, else empty struct
        struct Empty {};
        std::conditional_t<IsVector, std::vector<FirstT>, Empty> values;
        
        State() {
            if constexpr (IsVector) values.resize(N);
        }
    };

    auto state = std::make_shared<State>();

    // --- Attachment Logic ---
    size_t idx = 0;

    auto attach = [&](auto&& arg, size_t i) {
        // Normalize input to Promise<T>
        auto p = to_promise(std::forward<decltype(arg)>(arg));
        
        // Attach continuation
        p.state->addContinuation([p_state = p.state, state, i, res_state = result.state]() {
            
            // Handle Rejection
            if (p_state->exception) {
                bool expected = false;
                // Only the first rejection triggers the result rejection
                if (state->rejected.compare_exchange_strong(expected, true)) {
                    res_state->reject(p_state->exception);
                }
                return;
            }

            // If already rejected by another promise, strictly exit
            if (state->rejected.load(std::memory_order_relaxed)) return;

            // Handle Success Value
            if constexpr (IsVector) {
                // p is guaranteed to be Promise<FirstT> here, so .value exists
                std::lock_guard lock(state->mtx);
                state->values[i] = std::move(*p_state->value);
            }

            // Decrement Counter
            // fetch_sub returns the value BEFORE decrement. So if it returns 1, it is now 0.
            if (state->remaining.fetch_sub(1) == 1) {
                if (!state->rejected) {
                    if constexpr (IsVector) {
                        res_state->resolve(std::move(state->values));
                    } else {
                        res_state->resolve();
                    }
                }
            }
        });
    };

    // Fold expression to run attach for every argument
    (attach(std::forward<Args>(args), idx++), ...);

    return result;
}


}  // namespace vega


/*

1. Prompt (to Claude 4.5 Opus):

** here is the same prompt you can find in test/promiseAll.cc. **

--------------------------------

2.1. Prompt (to Google Gemini 3.0 Pro):

learn this code

** here we pasted vega/PromiseAll.h generated by Claude. **

too long? maybe you have some more smart way to implement this elegantly without modifying the api (last function promiseAll).

note: the original code is by claude 4.5 opus, but i think you are smarter than it.

--------------------------------

2.2. Prompt (to Google Gemini 3.0 Pro):

your code should be able to pass the test

** here we pasted test/promiseAll.cc generated by Claude. **

and i give you other structs but you should do no modification to them:


** here we pasted vega/Promise.h. **
** here we pasted vega/PromiseState.h. **
** here we pasted vega/PromiseState.cc. **

--------------------------------

3. Some tiny modifications to the code by human.

*/

