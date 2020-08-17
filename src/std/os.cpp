#include <io/termio.hpp>
#include <std/os.hpp>

ExecutionResult printStrOp(Interpreter& interp) {
    std::optional<Value> top = interp.evalStack.popBack();
    if (!top) {
        return EvalStackUnderflow();
    }
    if (top.value().type != ValueType::String) {
        return TypeError();
    }
    print(top.value().str->get());
    return Success();
}

void addOSModuleNativeWords(Interpreter& interp) {
    interp.defineNativeWord(U"print_str", printStrOp);
}