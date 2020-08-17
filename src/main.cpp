#include <codecvt>
#include <core/interpreter.hpp>
#include <io/fileio.hpp>
#include <io/parser.hpp>
#include <io/prettyprint.hpp>
#include <io/termio.hpp>
#include <iostream>
#include <locale>
#include <std/arith.hpp>
#include <std/comparisons.hpp>
#include <std/controlflow.hpp>
#include <std/os.hpp>
#include <std/stack.hpp>
#include <std/strings.hpp>

void initStd(Interpreter& interp) {
    addArithNativeWords(interp);
    addComparisonsNativeWords(interp);
    addStackManipNativeWords(interp);
    addControlFlowNativeWords(interp);
    addOSModuleNativeWords(interp);
    addStringManipulationNativeWords(interp);
}

void reportSyntaxError(ParseResult res) {
    if (res.status == ParseResult::Status::LexerError) {
        print(U"Lexer error at position ");
    } else {
        print(U"Parser error at position ");
    }
    std::cout << res.errorPos;
    print(U": ");
    print(res.description);
    print(U"\n");
}

void reportRuntimeError(ExecutionResult res,
                        [[maybe_unused]] Interpreter& interp) {
    print(U"Runtime error: ");
    print(std::u32string(res.error));
    print(U"\n");
}

void hostRepl() {
    Interpreter interp(1024);
    // initStd requires scope for adding native words
    interp.symTable.createScope();
    initStd(interp);
    while (true) {
        print(U"[");
        for (size_t i = 0; i < interp.evalStack.getStack().size(); ++i) {
            print(prettyprint(interp.evalStack.getStack()[i]));
            if (i != interp.evalStack.getStack().size() - 1) {
                print(U" ");
            }
        }
        print(U"]# ");
        std::u32string s = readLine();
        ParseResult result = parse(s, interp.heap);
        if (result.isError()) {
            reportSyntaxError(result);
            interp.heap.collectGarbage();
            continue;
        }
        ExecutionResult res = interp.callInterpreter(result.code, false);
        if (res.result != ExecutionResultType::Success) {
            reportRuntimeError(res, interp);
            interp.evalStack.clear();
            interp.callStack.frames.clear();
            interp.heap.collectGarbage();
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        hostRepl();
        return 0;
    }
    char* filename = argv[1];
    Interpreter interp(1024);
    // initStd requires scope for adding native words
    interp.symTable.createScope();
    initStd(interp);
    std::u32string source = readFile(filename);
    ParseResult result = parse(source, interp.heap);
    if (result.isError()) {
        reportSyntaxError(result);
        interp.heap.collectGarbage();
        return -1;
    }
    ExecutionResult res = interp.callInterpreter(result.code, false);
    if (res.result != ExecutionResultType::Success) {
        reportRuntimeError(res, interp);
        interp.evalStack.clear();
        interp.callStack.frames.clear();
        interp.heap.collectGarbage();
        return -1;
    }
    return 0;
}