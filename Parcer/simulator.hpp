#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <charconv>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "../Fixed/Fixed.h"
#include "../Fixed/FastFixed.h"
#include "../LiquidSim/LiquidSim.h"
#ifdef FLOAT
#error "FLOAT is already defined"
#endif
#ifdef DOUBLE
#error "DOUBLE is already defined"
#endif
#ifdef FIXED
#error "FIXED is already defined"
#endif
#ifdef FAST_FIXED
#error "FAST_FIXED is already defined"
#endif
#ifdef S
#error "S is defined"
#endif

#define FLOAT            float
#define DOUBLE           double
#define FAST_FIXED(N, K) FastFixed<N, K>
#define FIXED(N, K)      Fixed<N, K>

#define S(N, M)                  \
    types::SizePair {            \
        .rows = N, .columns = M, \
    }

void* LS;

namespace types {

struct SizePair {
    std::size_t rows{};
    std::size_t columns{};
};

struct Context {
    std::vector<std::vector<char>> field;
};

template <class... Types>
struct TypesList;

namespace detail {

template <std::size_t I, class Type, class... Types>
struct TypesListGetAtHelper1 {
    static_assert(I < 1 + sizeof...(Types));
    using type = typename TypesListGetAtHelper1<I - 1, Types...>::type;
};

template <class Type, class... Types>
struct TypesListGetAtHelper1<0, Type, Types...> {
    using type = Type;
};

template <std::size_t I, class TypesListType>
struct TypesListGetAtHelper2;

template <std::size_t I, class... Types>
struct TypesListGetAtHelper2<I, TypesList<Types...>> {
    using type = typename TypesListGetAtHelper1<I, Types...>::type;
};

}  // namespace detail

template <std::size_t I, class TypesListType>
using TypesListGetAt = typename detail::TypesListGetAtHelper2<I, TypesListType>::type;

template <std::size_t I, class... Types>
using TypesPackGetAt = typename detail::TypesListGetAtHelper1<I, Types...>::type;

namespace detail {

inline constexpr std::size_t kDynamicExtent = std::numeric_limits<std::size_t>::max();

template <class PElementType,
          class VelocityElementType,
          class VelocityFlowElementType,
          std::size_t Rows    = kDynamicExtent,
          std::size_t Columns = kDynamicExtent>
void start_simulation(const Context& ctx) {
    if constexpr (Rows == kDynamicExtent && Columns == kDynamicExtent) {
        auto LS = LiquidSimDynamic<10000, PElementType, VelocityElementType, VelocityFlowElementType>(ctx.field.size(), ctx.field.front().size() - 1, ctx.field);//field, rho, p, p_old, velocity, velocity_flow, last_use, UT);
        LS.save("FIXED_DYN");
        LS.simulate();
    } else {
        auto LS = LiquidSim<Rows, Columns + 1, 10000, PElementType,VelocityElementType, VelocityFlowElementType>(ctx.field);
        LS.save("FIXED_ST");
        LS.simulate();
    }
}

template <class PElementType,
          class VelocityElementType,
          class VelocityFlowElementType,
          SizePair StaticSize,
          SizePair... StaticSizes>
void select_size_and_start_simulation_impl(const Context& ctx) {
    static_assert(StaticSize.rows > 0);
    static_assert(StaticSize.columns > 0);

    if (StaticSize.rows == ctx.field.size() && StaticSize.columns == ctx.field.front().size() - 1) {
        start_simulation<PElementType, VelocityElementType, VelocityFlowElementType,
                         StaticSize.rows, StaticSize.columns>(ctx);
    } else if constexpr (sizeof...(StaticSizes) == 0) {
        start_simulation<PElementType, VelocityElementType, VelocityFlowElementType>(ctx);
    } else {
        select_size_and_start_simulation_impl<PElementType, VelocityElementType,
                                              VelocityFlowElementType, StaticSizes...>(ctx);
    }
}

template <class PElementType,
          class VelocityElementType,
          class VelocityFlowElementType,
          SizePair... StaticSizes>
void select_size_and_start_simulation(const Context& ctx) {
    if constexpr (sizeof...(StaticSizes) > 0) {
        select_size_and_start_simulation_impl<PElementType, VelocityElementType,
                                              VelocityFlowElementType, StaticSizes...>(ctx);
    } else {
        start_simulation<PElementType, VelocityElementType, VelocityFlowElementType>(ctx);
    }
}

#define STRINGIFY_EXACT_NO_EVAL(expr) #expr

inline constexpr std::string_view kFloatTypeName   = STRINGIFY_EXACT_NO_EVAL(FLOAT);
inline constexpr std::string_view kDoubleTypeName  = STRINGIFY_EXACT_NO_EVAL(DOUBLE);
inline constexpr std::string_view kFastFixedPrefix = "FAST_FIXED(";
inline constexpr std::string_view kFastFixedSuffix = ")";
inline constexpr std::string_view kFixedPrefix     = "FIXED(";
inline constexpr std::string_view kFixedSuffix     = ")";

#undef STRINGIFY_EXACT_NO_EVAL

template <class AllowedFloatTypesList, class SelectedFloatTypesList, SizePair... StaticSizes>
class FloatTypesSelector;

template <class... AllowedFloatTypes, class... SelectedFloatTypes, SizePair... StaticSizes>
class FloatTypesSelector<TypesList<AllowedFloatTypes...>,
                         TypesList<SelectedFloatTypes...>,
                         StaticSizes...> {
    using FieldVec = std::vector<std::vector<char>>;

    static constexpr bool kCanUseFloatType =
        std::disjunction_v<std::is_same<AllowedFloatTypes, FLOAT>...>;

    static constexpr bool kCanUseDoubleType =
        std::disjunction_v<std::is_same<AllowedFloatTypes, DOUBLE>...>;

public:
    template <class... Args>
        requires std::conjunction_v<std::is_same<std::string_view, Args>...>
    static void select_type_and_size_and_start_simulation(const Context& ctx,
                                                          std::string_view type_name,
                                                          Args... type_names) {
        if (kCanUseFloatType && type_name == kFloatTypeName) {
            next_call_with_selected_type<FLOAT, Args...>(ctx, type_names...);
            return;
        }
        if (kCanUseDoubleType && type_name == kDoubleTypeName) {
            next_call_with_selected_type<DOUBLE, Args...>(ctx, type_names...);
            return;
        }

        std::size_t n{};
        std::size_t k{};
        if (try_parse_fixed_params(type_name, kFastFixedPrefix, kFastFixedSuffix, n, k)) {
            if (select_fixed_and_size_and_start_simulation<true, Args...>(n, k, ctx,
                                                                          type_names...)) {
                return;
            }
        }
        if (try_parse_fixed_params(type_name, kFixedPrefix, kFixedSuffix, n, k)) {
            if (select_fixed_and_size_and_start_simulation<false, Args...>(n, k, ctx,
                                                                           type_names...)) {
                return;
            }
        }

        throw_on_bad_type_name(type_name);
    }

private:
    static bool try_parse_fixed_params(std::string_view type_name,
                                       std::string_view prefix,
                                       std::string_view suffix,
                                       std::size_t& n,
                                       std::size_t& k) {
        if (!type_name.starts_with(prefix)) {
            return false;
        }
        type_name.remove_prefix(prefix.size());

        if (!type_name.ends_with(suffix)) {
            return false;
        }
        type_name.remove_suffix(suffix.size());

        const std::size_t sep_char_pos = type_name.find(',');
        if (sep_char_pos >= type_name.size()) {
            return false;
        }

        auto strip_sv = [](std::string_view s) noexcept {
            while (!s.empty() && std::isspace(s.front())) {
                s.remove_prefix(1);
            }
            while (!s.empty() && std::isspace(s.back())) {
                s.remove_suffix(1);
            }
            return s;
        };
        const std::string_view n_str = strip_sv(type_name.substr(0, sep_char_pos));
        const std::string_view k_str = strip_sv(type_name.substr(sep_char_pos + 1));

        if (std::from_chars(n_str.data(), n_str.data() + n_str.size(), n).ec != std::errc{}) {
            return false;
        }

        if (std::from_chars(k_str.data(), k_str.data() + k_str.size(), k).ec != std::errc{}) {
            return false;
        }

        return n > 0 && k > 0;
    }

    template <bool Fast, class... Args>
    static bool select_fixed_and_size_and_start_simulation(std::size_t n,
                                                           std::size_t k,
                                                           const Context& ctx,
                                                           Args... type_names) {
        return select_fixed_and_size_and_start_simulation_impl<0, Fast, Args...>(n, k, ctx,
                                                                                 type_names...);
    }

    template <std::size_t I, bool Fast, class... Args>
    static bool select_fixed_and_size_and_start_simulation_impl(std::size_t n,
                                                                std::size_t k,
                                                                const Context& ctx,
                                                                Args... type_names) {
        using FloatType = TypesPackGetAt<I, AllowedFloatTypes...>;

        if constexpr (requires {
                          {
                              FloatType::kNValue == std::size_t {}
                          } -> std::same_as<bool>;
                          {
                              FloatType::kKValue == std::size_t {}
                          } -> std::same_as<bool>;
                          {
                              FloatType::kFast == bool {}
                          } -> std::same_as<bool>;
                      }) {
            if constexpr (FloatType::kFast == Fast) {
                if (FloatType::kNValue == n && FloatType::kKValue == k) {
                    next_call_with_selected_type<FloatType, Args...>(ctx, type_names...);
                    return true;
                }
            }
        }

        if constexpr (I + 1 < sizeof...(AllowedFloatTypes)) {
            return select_fixed_and_size_and_start_simulation_impl<I + 1, Fast, Args...>(
                n, k, ctx, type_names...);
        }

        return false;
    }

    [[noreturn]] static void throw_on_bad_type_name(std::string_view type_name) {
        throw std::invalid_argument("Could not parse type " + std::string{type_name});
    }

    template <class FloatType, class... Args>
    static void next_call_with_selected_type(const Context& ctx, const Args&... type_names) {
        if constexpr (sizeof...(type_names) > 0) {
            using AllowedFloatTypesList  = TypesList<AllowedFloatTypes...>;
            using SelectedFloatTypesList = TypesList<SelectedFloatTypes..., FloatType>;

            FloatTypesSelector<AllowedFloatTypesList, SelectedFloatTypesList, StaticSizes...>::
                select_type_and_size_and_start_simulation(ctx, type_names...);
        } else {
            select_size_and_start_simulation<SelectedFloatTypes..., FloatType, StaticSizes...>(ctx);
        }
    }
};

}  // namespace detail

struct SimulationParams {
    std::string_view p_type_name{};
    std::string_view v_type_name{};
    std::string_view v_flow_type_name{};
};

template <class TypesList, SizePair... StaticSizes>
class Simulator;

template <class... FloatTypes, SizePair... StaticSizes>
class [[nodiscard]] Simulator<TypesList<FloatTypes...>, StaticSizes...> {
public:
    static Simulator from_params(SimulationParams params) {
        return Simulator{std::move(params)};
    }

    void start_on_field(const Context& ctx) const {
        assert(!ctx.field.empty());
        assert(!ctx.field.front().empty());
        detail::FloatTypesSelector<TypesList<FloatTypes...>, TypesList<>, StaticSizes...>::
            select_type_and_size_and_start_simulation(ctx, params_.p_type_name, params_.v_type_name,
                                                      params_.v_flow_type_name);
    }

private:
    explicit constexpr Simulator(SimulationParams&& params) noexcept : params_{std::move(params)} {}

    SimulationParams params_;
};

}  // namespace types
