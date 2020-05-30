//
// Created by Nils Bergmann on 30.05.20.
//

#ifndef NATIVECOPY_NATIVE_COPY_H
#define NATIVECOPY_NATIVE_COPY_H

#include <napi.h>
#include <tuple>
#include <iostream>
#include <string>
#include <sstream>

using namespace Napi;

Object CopyObject(const Env& env, const Value& target, const Value& rules);

std::tuple<Array, bool> processArray(const Env& env, const Array& rulesArray, const Value& values);

Array CopyArray(const Env& env, const Value& target, const Value& rules);

Value Copy(const CallbackInfo& info);

Object Init(Env env, Object exports);

#endif //NATIVECOPY_NATIVE_COPY_H
