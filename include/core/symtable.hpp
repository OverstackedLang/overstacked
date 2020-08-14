#pragma once

#include <core/heap.hpp>
#include <core/values.hpp>
#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>

class SymbolTable {
    std::deque<std::unordered_set<std::u32string>> declaredInScope;
    std::unordered_map<std::u32string, std::deque<Value>> values;

   public:
    void createScope();
    void leaveScope();
    void declareVariable(const std::u32string& name, Value val);
    void setVariable(const std::u32string& name, Value val);
    Value getVariable(const std::u32string& name);
    void registerRootMarker(Heap& heap);
};