#pragma once

#include <variant>
#include <utility>
#include <type_traits>

constexpr bool debug_try = true;

#define TRY(expr)                                        \
    ({                                                   \
        auto _r = (expr);                                \
        if (!_r.ok()) {                                  \
            if (debug_try)                               \
                std::cout << "TRY failed at " << __FILE__ << ":" << __LINE__ << " in " << __func__ << std::endl; \
            return std::move(_r.error());                \
        }                                                \
        std::move(_r.value());                           \
    })

template <typename T, typename E>
class Result {
    std::variant<T, E> data_;
public:
    Result(T value)   : data_(std::move(value)) {}
    Result(E error)   : data_(std::move(error)) {}

    template <typename U,
              typename = std::enable_if_t<
                  std::is_convertible_v<U, T> &&
                  !std::is_same_v<U, T>>>
    Result(Result<U, E>&& other) {
        if (other.ok()) {
            data_.template emplace<0>(std::move(other.value()));
        } else {
            data_.template emplace<1>(std::move(other.error()));
        }
    }

    bool ok()    const noexcept { return data_.index() == 0; }
    bool failed() const noexcept { return data_.index() == 1; }
    explicit operator bool() const noexcept { return ok(); }

    T& value()        { return std::get<0>(data_); }
    const T& value() const { return std::get<0>(data_); }
    E& error()        { return std::get<1>(data_); }
    const E& error() const { return std::get<1>(data_); }

    T value_or(T fallback) const& {
        return ok() ? value() : std::move(fallback);
    }

    template <typename F>
    auto map(F&& f) -> Result<std::invoke_result_t<F, T>, E> {
        if (ok()) 
            return std::forward<F>(f)(std::move(value()));

        return std::move(error());
    }

    template <typename F>
    auto and_then(F&& f) -> std::invoke_result_t<F, T> {
        if (ok()) 
            return std::forward<F>(f)(std::move(value()));

        return std::move(error());
    }
};
