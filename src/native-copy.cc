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
                if (targetObject.Get(property).IsNull() || targetObject.Get(property).IsUndefined() || targetObject.Get(property).IsSymbol()) {
                    returnObject.Set(property, targetObject.Get(property));
                } else {
                    returnObject.Set(property, CopyObject(env, targetObject.Get(property), rulesObject.Get(property)));
                }
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

// https://stackoverflow.com/questions/44973435/stdptr-fun-replacement-for-c17
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c);}));
    return s;
}

void SetTrueRecursive(const Env& env, Reference<Object>& targetReference, std::vector<std::string>& fragments) {
    std::string currentKey = fragments[0];
    Object target = targetReference.Value();
    currentKey = ltrim(currentKey);
    if (currentKey.empty()) return;
    if (fragments.size() == 1) {
        if (!target.Has(fragments[0])) {
            target.Set(currentKey.c_str(), Boolean::New(env, true));
        }
        return;
    } else {
        if (!target.Has(currentKey)) target.Set(currentKey, Object::New(env));
        Reference<Object> setNext = Reference<Object>::New(target.Get(currentKey).As<Object>());
        fragments.erase(fragments.begin());
        return SetTrueRecursive(env, setNext, fragments);
    }
}

// https://thispointer.com/how-to-split-a-string-in-c/
std::vector<std::string> split(std::string str, char delimeter)
{
    std::stringstream ss(str);
    std::string item;
    std::vector<std::string> splittedStrings;
    while (std::getline(ss, item, delimeter))
    {
        splittedStrings.push_back(item);
    }
    return splittedStrings;
}

Object ConvertRule(const CallbackInfo& info) {
    Env env = info.Env();

    if (info.Length() < 1) {
        TypeError::New(env, "You need to pass at least 1 arguments").ThrowAsJavaScriptException();
    }

    auto convertValue = info[0];

    if (convertValue.IsArray()) {
        auto array = convertValue.As<Array>();

        Object returnObject = Object::New(env);
        Reference<Object> reference = Reference<Object>::New(returnObject);

        for (int i = 0; i < array.Length(); i++) {
            Value pathValue = array[i];
            if (!pathValue.IsString()) continue;
            std::string delimiter = ".";
            std::string path = pathValue.ToString().Utf8Value();
            std::vector<std::string> pathFragments = split(path, '.');

            if (pathFragments.empty()) continue;

            SetTrueRecursive(env, reference, pathFragments);
            std::vector<std::string>().swap(pathFragments);
        }

        return returnObject;
    } else if (convertValue.IsObject()) {
        return convertValue.ToObject();
    }

    TypeError::New(env, "Argument is not an array and also not an object.").ThrowAsJavaScriptException();
    return Object::New(env);
}

Object Init(Env env, Object exports) {
  exports.Set(String::New(env, "NativeCopy"), Function::New(env, Copy));
  exports.Set(String::New(env, "ConvertRule"), Function::New(env, ConvertRule));
  return exports;
}

NODE_API_MODULE(NativeCopy, Init);