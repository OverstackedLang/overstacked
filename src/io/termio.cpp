#include <codecvt>
#include <io/termio.hpp>
#include <iostream>
#include <linenoise.hpp>
#include <locale>

std::string fromUTF32(const std::u32string &s);
std::u32string toUTF32(const std::string &s);

std::u32string readLine(const std::u32string &prompt) {
    std::string promptUtf8 = fromUTF32(prompt);
    std::string result;
    linenoise::Readline(promptUtf8.c_str(), result);
    linenoise::AddHistory(result.c_str());
    return toUTF32(result);
}

void print(const std::u32string &str) { std::cout << fromUTF32(str); }