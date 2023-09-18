#pragma once
#include <fce/codec/types.hpp>
#include <type_traits>
FCE_CODEC_NS_BEGIN

template<typename T, typename = void>
struct can_encode : public std::false_type
{};

template<typename T>
struct can_encode<T, std::void_t<decltype(T::encode)>> : public std::true_type
{};

template<typename T, typename = void>
struct can_decode : public std::false_type
{};

template<typename T>
struct can_decode<T, std::void_t<decltype(T::decode)>> : public std::true_type
{};

template<typename T>
constexpr bool is_codec = can_encode<T>::value && can_decode<T>::value;

FCE_CODEC_NS_END
