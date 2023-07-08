
/*
This header only exists to help make C++ syntax less goofy
*/

#pragma once

#include <memory>
#include <unordered_map>

template<typename T>
using ptr = T*;

template<typename T>
using ref = T&;

//I'll be honest: these names are too short and not super legible.
//
//But they're better than the types they're aliasing

template<typename T>
using sptr = std::shared_ptr<T>;

template<typename T>
using uptr = std::unique_ptr<T>;

template<typename T>
using wptr = std::weak_ptr<T>;

/*
STORY TIME!

Back when I was writing this header, I wanted to alias the std::make_... functions. But that didn't work.

Turns out, templates are really goofy. So I sacked it. But the length of the existing functions still bugged me.

It's June 13th and I'm tired. This is the closest I can get to truly aliasing them.
*/

#define new_sptr std::make_shared
#define new_uptr std::make_unique

template<typename In, typename Out, typename std::enable_if<std::is_base_of<Out, In>::value>::type* = nullptr>
sptr<Out> inline constexpr sptr_demote(sptr<In> in) noexcept
{
	return *(sptr<Out>*)&in;
}

//Great, now we're doing containers too.

template<typename K, typename V, typename H = std::hash<K>>
using HashMap = std::unordered_map<K, V, H>;

#define pass [[fallthrough]]

//Gotta make a way to make lambdas legible too smh my head

#define lambda(...) [&](__VA_ARGS__)
#define lambda_v(...) [=](__VA_ARGS__)
