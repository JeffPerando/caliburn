
/*
This header only exists to help make C++ syntax less goofy

and some handy functions
*/

#pragma once

#include <atomic>
#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <variant>

/*
It's very handy in a project like this, to have a simple way to check for powers of 2.

According to my research, this is the best way to do this.
*/
#define IS_POW_2(x) ((x & (x - 1)) == 0)

template<typename T>
using ptr = T*;

template<typename T>
using ref = T&;

template<typename T>
using in = const T&;

template<typename T>
using out = T&;

template<typename T>
using sptr = std::shared_ptr<T>;

template<typename T>
using uptr = std::unique_ptr<T>;

#if __cplusplus >= 201703L
template<typename T>
using wptr = std::weak_ptr<T>;
#endif

/*
STORY TIME!

Back when I was writing this header, I wanted to alias the std::make_... functions. But that didn't work.

Turns out, templates are really goofy. So I sacked it. But the length of the existing functions still bugged me.

It's 13 June 2023 and I'm tired. This is the closest I can get to truly aliasing them.
*/

#define new_sptr std::make_shared
#define new_uptr std::make_unique

//Great, now we're doing containers too.

template<typename K, typename V, typename H = std::hash<K>>
using HashMap = std::unordered_map<K, V, H>;

template<typename T>
using atom = std::atomic<T>;

#define PASS [[fallthrough]]

/*
Lambda syntax is so goofy, this was inevitable.

The good news is, this is quite possibly the best part of this header.
*/

#define LAMBDA(...) [&](__VA_ARGS__)
#define LAMBDA_V(...) [=](__VA_ARGS__)

#define MATCH(VALUE, TYPE, NAME) if (auto NAME = std::get_if<TYPE>(&VALUE))
#define MATCH_EMPTY(VALUE) if (std::holds_alternative<std::monostate>(VALUE))
#define MATCH_WHILE(VALUE, TYPE, NAME) while (auto NAME = std::get_if<TYPE>(&VALUE))

#define DCAST dynamic_cast
#define RCAST reinterpret_cast
#define SCAST static_cast
