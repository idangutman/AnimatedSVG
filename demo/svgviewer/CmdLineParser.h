/*
 * Copyright (c) 2025 Idan Gutman
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * This demo project contains a viewer for SVG files that uses ArduinoSVG.
 * It utilizes SDL3 for graphics (https://github.com/libsdl-org/SDL).
 */

#include <stdlib.h>
#include <string.h>
#include <string>

#ifndef CMDLINE_PARSER_H
#define CMDLINE_PARSER_H

class CmdLineParser
{
private:
    typedef enum
    {
        OPTION_TYPE_ARGUMENT,
        OPTION_TYPE_FLAG,
        OPTION_TYPE_BOOL,
        OPTION_TYPE_INT,
        OPTION_TYPE_COLOR,
    } OptionType;

    typedef struct Option
    {
        OptionType type;
        std::string shortOpt;
        std::string longOpt;
        std::string name;
        std::string description;
        bool isOptional;
        bool hasValue;
        union {
            void* ptrValue;
            bool* boolValue;
            int* intValue;
            const char** argValue;
        };
        struct Option* next;
    } Option;

public:
    CmdLineParser()
    {
        _options = NULL;
        _lastError = "";
    }

    ~CmdLineParser()
    {
        while (_options != NULL)
        {
            Option* next = _options->next;
            delete _options;
            _options = next;
        }
    }

public:
    bool Parse(int argc, const char** argv)
    {
        bool success = true;
        Option* nextArgOpt;
        Option* opt;

        _lastError = "";

        // Reset the has value flags.
        for (opt = _options; opt != NULL; opt = opt->next)
        {
            opt->hasValue = false;
        }

        // Find the next argument option.
        for (nextArgOpt = _options;
             nextArgOpt != NULL && nextArgOpt->type != OPTION_TYPE_ARGUMENT;
             nextArgOpt = nextArgOpt->next);

        for (int i = 1; success && i < argc; i++)
        {
            // Check if this is an argument or flag.
            if (*argv[i] != '-')
            {
                if (nextArgOpt == NULL)
                {
                    _lastError = "Too many arguments";
                    success = false;
                }
                else
                {
                    *nextArgOpt->argValue = argv[i];
                    nextArgOpt->hasValue = true;

                    // Move to next argument option.
                    for (;nextArgOpt != NULL && nextArgOpt->type != OPTION_TYPE_ARGUMENT;
                         nextArgOpt = nextArgOpt->next);
                }
            }
            else
            {
                for (opt = _options; success && opt != NULL; opt = success ? opt->next : opt)
                {
                    if (opt->type == OPTION_TYPE_ARGUMENT)
                    {
                        continue;
                    }

                    if ((opt->shortOpt == &argv[i][1]) || ((argv[i][1] == '-') && (opt->longOpt == &argv[i][2])))
                    {
                        if (opt->type == OPTION_TYPE_FLAG)
                        {
                            *opt->boolValue = true;
                            opt->hasValue = true;
                        }
                        else
                        {
                            i++;
                            if (i >= argc)
                            {
                                _lastError = "Missing value for option ";
                                _lastError += opt->name;
                                success = false;
                            }
                            else
                            {
                                switch (opt->type)
                                {
                                    case OPTION_TYPE_BOOL:
                                    {
                                        if ((strcasecmp(argv[i], "true") == 0) || (strcmp(argv[i], "1") == 0))
                                        {
                                            *opt->boolValue = true;
                                            opt->hasValue = true;
                                        }
                                        else if ((strcasecmp(argv[i], "false") == 0) || (strcmp(argv[i], "0") == 0))
                                        {
                                            *opt->boolValue = false;
                                            opt->hasValue = true;
                                        }
                                        else
                                        {
                                            success = false;
                                        }
                                        break;
                                    }
                                    case OPTION_TYPE_INT:
                                    {
                                        bool isHex = (strncmp(argv[i], "0x", 2) == 0) ? true : false;
                                        int start = isHex ? 2 : 0;
                                        int radix = isHex ? 16 : 10;
                                        char* end = NULL;
                                        *opt->intValue = strtol(&argv[i][start], &end, radix);
                                        if (end == &argv[i][start])
                                        {
                                            success = false;
                                        }
                                        else
                                        {
                                            opt->hasValue = true;
                                        }
                                        break;
                                    }
                                    case OPTION_TYPE_COLOR:
                                    {
                                        if (*argv[i] == '#')
                                        {
                                            char* end = NULL;
                                            *opt->intValue = strtol(&argv[i][1], &end, 16);
                                            if (end == &argv[i][1])
                                            {
                                                success = false;
                                            }
                                            else
                                            {
                                                if (strlen(argv[i]) == 4) // #ccc
                                                {
                                                    *opt->intValue = 
                                                        (*opt->intValue & 0xF) | (*opt->intValue & 0xF) << 4 |
                                                        (*opt->intValue & 0xF0) << 4 | (*opt->intValue & 0xF0) << 8 |
                                                        (*opt->intValue & 0xF00) << 8 | (*opt->intValue & 0xF00) << 12;
                                                }
                                                opt->hasValue = true;
                                            }
                                        }
                                        else
                                        {
                                            success = false;
                                        }
                                        break;
                                    }
                                    default: // do nothing
                                    {
                                        break;
                                    }
                                }

                                if (!success)
                                {
                                    _lastError = "Invalid value for option ";
                                    _lastError += opt->name + ": ";
                                    _lastError += argv[i];
                                }
                            }
                        }

                        break;
                    }
                }

                if (opt == NULL)
                {
                    _lastError = "Unrecognized option: ";
                    _lastError += argv[i];
                    success = false;
                }
            }
        }

        // Check for all required options.
        for (opt = _options; opt != NULL; opt = opt->next)
        {
            if (!opt->hasValue && !opt->isOptional)
            {
                _lastError = "Missing required ";
                _lastError += (opt->type == OPTION_TYPE_ARGUMENT) ? "argument" : "option";
                _lastError += ": " + opt->name;
                success = false;
            }
        }

        return success;
    }

    const char* GetLastError()
    {
        return _lastError.length() > 0 ? _lastError.c_str() : NULL;
    }

    const char* GetSyntax()
    {
        _syntax = "[options]";
        std::string args;

        int nonArgExtraChars = 5; //"-" + ", --"
        int maxWidth = 0;
        for (Option* opt = _options; opt != NULL; opt = opt->next)
        {
            if (opt->type == OPTION_TYPE_ARGUMENT)
            {
                std::string arg = (opt->isOptional ? "[" : "<") + opt->name + (opt->isOptional ? "]" : ">");
                int width = arg.length();
                maxWidth = (maxWidth > width) ? maxWidth : width;
                _syntax += " " + arg;
            }
            else
            {
                int width = opt->shortOpt.length() + opt->longOpt.length() +
                            nonArgExtraChars + ((opt->type != OPTION_TYPE_FLAG) ? 8 : 0);
                maxWidth = (maxWidth > width) ? maxWidth : width;
            }
        }
        _syntax += "\n";

        // Spacing to description.
        maxWidth += 3;

        for (Option* opt = _options; opt != NULL; opt = opt->next)
        {
            if (opt->type == OPTION_TYPE_ARGUMENT)
            {
                std::string arg = (opt->isOptional ? "[" : "<") + opt->name + (opt->isOptional ? "]" : ">");
                int width = arg.length();
                _syntax += "  " + arg;
                for (int i = 0; i < maxWidth-width; i++)
                {
                    _syntax += " ";
                }
                _syntax += opt->description + "\n";
            }
        }
        _syntax += "Options:\n";
        for (Option* opt = _options; opt != NULL; opt = opt->next)
        {
            if (opt->type != OPTION_TYPE_ARGUMENT)
            {
                int width = opt->shortOpt.length() + opt->longOpt.length() + 
                            nonArgExtraChars + ((opt->type != OPTION_TYPE_FLAG) ? 8 : 0);
                _syntax += "  -" + opt->shortOpt + ", --" + opt->longOpt;
                _syntax += (opt->type != OPTION_TYPE_FLAG) ? " <value>" : "";
                for (int i = 0; i < maxWidth-width; i++)
                {
                    _syntax += " ";
                }
                _syntax += opt->description + (!opt->isOptional ? "(required)\n" : "\n");
            }
        }

        return _syntax.c_str();
    }

    void AddFlagOption(const char* shortOpt, const char* longOpt, const char* name, const char* description,
                       bool* value, bool isOptional = true)
    {
        Option* option = AddOption(shortOpt, longOpt, name, description, isOptional);
        option->type = OPTION_TYPE_FLAG;
        option->boolValue = value;
    }

    void AddBoolOption(const char* shortOpt, const char* longOpt, const char* name, const char* description,
                       bool* value, bool isOptional = true)
    {
        Option* option = AddOption(shortOpt, longOpt, name, description, isOptional);
        option->type = OPTION_TYPE_BOOL;
        option->boolValue = value;
    }

    void AddIntOption(const char* shortOpt, const char* longOpt, const char* name, const char* description,
                      int* value, bool isOptional = true)
    {
        Option* option = AddOption(shortOpt, longOpt, name, description, isOptional);
        option->type = OPTION_TYPE_INT;
        option->intValue = value;
    }

    void AddColorOption(const char* shortOpt, const char* longOpt, const char* name, const char* description,
                        int* value, bool isOptional = true)
    {
        Option* option = AddOption(shortOpt, longOpt, name, description, isOptional);
        option->type = OPTION_TYPE_COLOR;
        option->intValue = value;
    }

    void AddArgument(const char* name, const char* description, const char** value, bool isOptional = false)
    {
        Option* option = AddOption(NULL, NULL, name, description, isOptional);
        option->type = OPTION_TYPE_ARGUMENT;
        option->argValue = value;
    }

private:
    Option* AddOption(const char* shortOpt, const char* longOpt, const char* name, const char* description, bool isOptional)
    {
        Option* option = new Option;
        option->shortOpt = shortOpt != NULL ? shortOpt : "";
        option->longOpt = longOpt != NULL ? longOpt : "";
        option->name = name != NULL ? name : "";
        option->description = description != NULL ? description : "";
        option->isOptional = isOptional;
        option->ptrValue = NULL;
        option->next = NULL;

        Option* tail;
        for (tail = _options; tail != NULL && tail->next != NULL; tail = tail->next);
        if (tail == NULL)
        {
            _options = option;
        }
        else
        {
            tail->next = option;
        }

        return option;
    }

    Option* _options;
    std::string _lastError;
    std::string _syntax;
};

#endif //CMDLINE_PARSER_H
