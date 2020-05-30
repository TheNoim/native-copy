#include "native-copy.h"

std::tuple<Array, bool> processArray(const Env& env, const Array& rulesArray, const Value& values) {
    if (!values.IsArray()) {
        return std::make_tuple(Array::New(env, 0), true);
    }

    Array tempArray = Array::New(env);
    int nextIndex = 0;
    for (int i2 = 0; i2 < rulesArray.Length(); i2++) {
        Value ruleValue = rulesArray[i2];
        if (ruleValue.IsArray()) {
            auto [newArray, skip] = processArray(env, ruleValue.As<Array>(), values.As<Array>()[i2]);
            if (skip) continue;
            tempArray[nextIndex] = newArray;
            nextIndex++;
        } else {
            if (ruleValue.IsBoolean()) {
                if (!ruleValue.ToBoolean()) continue;
                tempArray[nextIndex] = values.As<Array>().Get(i2);
                nextIndex++;
            } else if (ruleValue.IsObject()) {
                auto targetAsArray = values.As<Array>();
                auto target = targetAsArray[i2];
                tempArray[nextIndex] = CopyObject(env, target, ruleValue);
                nextIndex++;
            }
        }
    }
    return std::make_tuple(tempArray, false);
}

Object CopyObject(const Env& env, const Value& target, const Value& rules) {
    auto targetObject = target.As<Object>();
    auto rulesObject = rules.As<Object>();
    Object returnObject = Object::New(env);

    Array properties = rulesObject.GetPropertyNames();

    for (int i = 0; i < properties.Length(); i++) {
        auto property = properties[i];

        if (!rulesObject.Has(property) || !targetObject.Has(property)) continue;

        if (rulesObject.Get(property).IsBoolean()) {
            if (!rulesObject.Get(property).ToBoolean()) continue;
            returnObject.Set(property, targetObject.Get(property));
        } else if (rulesObject.Get(property).IsObject() && !rulesObject.Get(property).IsArray()) {
            if (targetObject.Get(property).IsArray()) {
                auto targetArray = targetObject.Get(property).As<Array>();
                Array tempArray = Array::New(env);
                for (int i2 = 0; i2 < targetArray.Length(); i2++) {
                    tempArray[i2] = CopyObject(env, targetArray[i2], rulesObject.Get(property));
                }
                returnObject.Set(property, tempArray);
            } else {
                returnObject.Set(property, CopyObject(env, targetObject.Get(property), rulesObject.Get(property)));
            }
        } else if (rulesObject.Get(property).IsArray()) {
            auto [toSet, skip] = processArray(env, rulesObject.Get(property).As<Array>(), targetObject.Get(property));
            if (skip) continue;
            returnObject.Set(property, toSet);
        }
    }

    return returnObject;
}

Array CopyArray(const Env& env, const Value& target, const Value& rules) {
    auto targetArray = target.As<Array>();
    Array returnArray = Array::New(env);

    for (int i = 0; i < targetArray.Length(); i++) {
        returnArray[i] = CopyObject(env, targetArray[i], rules);
    }

    return returnArray;
}

Value Copy(const CallbackInfo& info) {
	Env env = info.Env();

	if (info.Length() < 2) {
        TypeError::New(env, "You need to pass at least 2 arguments").ThrowAsJavaScriptException();
        return env.Null();
	}

    Value targetToCopy = info[0];
	Value copyRule = info[1];

	if (!targetToCopy.IsObject() && !targetToCopy.IsArray()) {
        TypeError::New(env, "First argument needs to be an object or an array.").ThrowAsJavaScriptException();
        return env.Null();
	}

	if (!copyRule.IsObject()) {
        TypeError::New(env, "Second argument needs to be an object.").ThrowAsJavaScriptException();
        return env.Null();
	}

	if (targetToCopy.IsArray()) {
	    return CopyArray(env, targetToCopy, copyRule);
	} else {
	    return CopyObject(env, targetToCopy, copyRule);
	}
}

Object Init(Env env, Object exports) {
  exports.Set(String::New(env, "NativeCopy"), Function::New(env, Copy));
  return exports;
}

NODE_API_MODULE(NativeCopy, Init);