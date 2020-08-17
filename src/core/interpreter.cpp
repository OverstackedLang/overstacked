#include <assert.h>

#include <core/interpreter.hpp>

Success::Success() {
    result = ExecutionResultType::Success;
    error = U"Success";
}

EvalStackUnderflow::EvalStackUnderflow() {
    result = ExecutionResultType::Error;
    error = U"Evaluation stack underflow";
}

CallStackOverflow::CallStackOverflow() {
    result = ExecutionResultType::Error;
    error = U"Call stack overflow";
}

TypeError::TypeError() {
    result = ExecutionResultType::Error;
    error = U"Type error";
}

Interpreter::Interpreter(size_t maxRecursionDepth)
    : callStack(maxRecursionDepth) {
    callStack.registerRootMarker(heap);
    evalStack.registerRootMarker(heap);
}

void Interpreter::defineNativeWord(const std::u32string& name,
                                   NativeWord word) {
    String* str = heap.makeStringObject(name);
    Value val;
    val.word = word;
    val.type = ValueType::NativeWord;
    symTable.declareVariable(str, val);
    breakString = heap.makeStringObject(std::u32string(U"break"));
    returnString = heap.makeStringObject(std::u32string(U"return"));
    callString = heap.makeStringObject(std::u32string(U"!"));
    commaString = heap.makeStringObject(std::u32string(U","));
}

ExecutionResult Interpreter::callInterpreter(Array* code, bool newScope) {
    callStack.addArrayCallFrame(code, newScope);
    if (newScope) {
        symTable.createScope();
    }
    while (true) {
        // nothing on the call stack => interpreter can be exited
        if (callStack.frames.empty()) {
            return Success();
        }
        StackFrame& topFrame = callStack.frames.back();
        // if native subroutine of the stack, aknowledge success
        // and return
        if (topFrame.native) {
            return Success();
        }
        // if ip is beyond frame size, perform automatic return
        if (topFrame.code->values.size() <= topFrame.ip) {
            if (callStack.removeTopCallFrame()) {
                symTable.leaveScope();
            }
            continue;
        }
        // fetch current instruction
        Value ins = topFrame.code->values[topFrame.ip];
        // execute instruction depending on its type
        switch (ins.type) {
            case ValueType::Nil:
            case ValueType::Boolean:
            case ValueType::Numeric:
            case ValueType::String:
            case ValueType::Placeholder:
            case ValueType::Array:
            case ValueType::NativeWord:
                evalStack.pushBack(ins);
                topFrame.ip++;
                break;
            case ValueType::Word:
                if (ins.str == callString || ins.str == commaString) {
                    std::optional<Value> newTraceOptional = evalStack.popBack();
                    if (!newTraceOptional) {
                        return EvalStackUnderflow();
                    }
                    Value newTrace = newTraceOptional.value();
                    if (newTrace.type != ValueType::Array) {
                        return TypeError();
                    }
                    topFrame.ip++;
                    if (!callStack.addArrayCallFrame(newTrace.arr,
                                                     ins.str == callString)) {
                        return CallStackOverflow();
                    }
                    if (ins.str->get() == U"!") {
                        symTable.createScope();
                    }
                    continue;
                } else {
                    Value val = symTable.getVariable(ins.str);
                    if (val.type == ValueType::NativeWord) {
                        if (!callStack.addNativeCallFrame()) {
                            return CallStackOverflow();
                        }
                        ExecutionResult result = val.word(*this);
                        if (result.result != ExecutionResultType::Success) {
                            return result;
                        }
                        callStack.removeTopCallFrame();
                    } else {
                        evalStack.pushBack(val);
                    }
                    topFrame.ip++;
                }
                break;
            case ValueType::WordAssign:
            case ValueType::WordDeclare:
                std::optional<Value> valOptional = evalStack.popBack();
                if (!valOptional) {
                    return EvalStackUnderflow();
                }
                Value val = valOptional.value();
                if (ins.type == ValueType::WordAssign) {
                    symTable.setVariable(ins.str, val);
                } else {
                    symTable.declareVariable(ins.str, val);
                }
                topFrame.ip++;
                break;
        }
    }
}