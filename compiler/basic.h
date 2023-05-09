
/*
This header only exists to help make C++ syntax less goofy
*/

#pragma once

#include <memory>

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

#define pass [[fallthrough]]

//Gotta make a way to make lambdas legible too smh my head

#define lambda(...) [&](__VA_ARGS__)
#define lambda_v(...) [=](__VA_ARGS__)
