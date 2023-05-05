
/*
This header only exists to help make C++ syntax less goofy
*/

#pragma once

#include <memory>

template<typename T>
using ptr = T*;

template<typename T>
using ref = T&;

/*
I'll be honest: these names are too short and not super legible.

But they're better than the types they're aliasing
*/

template<typename T>
using sptr = std::shared_ptr<T>;

template<typename T>
using uptr = std::unique_ptr<T>;

template<typename T>
using wptr = std::weak_ptr<T>;


#define pass [[fallthrough]]

//Gotta make a way to make lambdas legible too smh my head

#define lambda(...) [](__VA_ARGS__)

/*
Since we can't have multiple variadic parts, and I don't want clunky passthrough nonsense, I just hand-wrote these.
I doubt anyone will need more than 3 capture variables, but eh
*/
#define lambda_p1(v0, ...) [v0](__VA_ARGS__)
#define lambda_p2(v0, v1, ...) [v0, v1](__VA_ARGS__)
#define lambda_p3(v0, v1, v2, ...) [v0, v1, v2](__VA_ARGS__)
