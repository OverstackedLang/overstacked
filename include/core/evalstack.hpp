#pragma once

#include <core/heap.hpp>
#include <core/values.hpp>
#include <optional>
#include <vector>

class EvaluationStack {
    std::vector<Value> stack;

   public:
    void registerRootMarker(Heap &heap);
    std::optional<Value> popBack();
    void pushBack(Value val);
    bool assertDepth(size_t count);
    void clear();

    inline const std::vector<Value> getStack() { return stack; }
};