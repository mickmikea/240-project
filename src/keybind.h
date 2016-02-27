#ifndef KEYBIND_H
#define KEYBIND_H

#include <functional>
#include <string>

typedef std::function<void(std::string&, int)> keybindCallback;

struct Keybind
{
    int pressedKey;
    keybindCallback callback;
};

#endif // KEYBIND_H
