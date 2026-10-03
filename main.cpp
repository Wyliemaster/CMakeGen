// CMakeGen - generates CMakeLists.txt for new projects
// Copyright (C) 2026 Wyliemaster
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <iostream>
#include <cstdlib>
#include <memory>
#include <string_view>
#include <fstream>
#include <optional>
#include <cctype>
#include <filesystem>

#define LOG_ERR(ERR) std::cerr << "ERROR: " << ERR << std::endl

enum class Kind
{
    C,
    CXX,
};
std::string_view kind_to_string(Kind k)
{
    switch (k)
    {
    case Kind::C:
        return "C";
    case Kind::CXX:
        return "CXX";
    default:
        return "";
    }
}

struct LanguageInfo
{
    std::string_view name;
    Kind kind;
    int standard;
    std::string_view cmake_version;
};

constexpr LanguageInfo kLanguages[] = {
    // C++
    {"C++:98", Kind::CXX, 98, "3.10"},
    {"C++:11", Kind::CXX, 11, "3.10"},
    {"C++:14", Kind::CXX, 14, "3.10"},
    {"C++:17", Kind::CXX, 17, "3.10"},
    {"C++:20", Kind::CXX, 20, "3.12"},
    {"C++:23", Kind::CXX, 23, "3.20"},
    {"C++:26", Kind::CXX, 26, "3.25"},
    {"C++", Kind::CXX, 20, "3.12"},        // alias for C++:20
    {"C++:latest", Kind::CXX, 26, "3.25"}, // alias for C++:26

    // C
    {"C:90", Kind::C, 90, "3.10"},
    {"C:99", Kind::C, 99, "3.10"},
    {"C:11", Kind::C, 11, "3.10"},
    {"C:17", Kind::C, 17, "3.21"},
    {"C:23", Kind::C, 23, "3.21"},
    {"C", Kind::C, 17, "3.21"},        // alias for C:17
    {"C:latest", Kind::C, 23, "3.21"}, // alias for C:23
};

struct ArgumentData
{
    int count = 0;
    std::unique_ptr<std::string_view[]> argv;
    std::string_view project_name;
    LanguageInfo lang = {"C++:17", Kind::CXX, 17, "3.10"};
};

bool iequals(std::string_view a, std::string_view b)
{
    if (a.size() != b.size())
        return false;

    for (size_t i = 0; i < a.size(); i++)
    {
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
        {
            return false;
        }
    }

    return true;
}
void check_help(std::string_view arg)
{
    if (iequals(arg, "--help") || iequals(arg, "-h"))
    {
        std::cout << "-p [Language]: Describe which language CMake should be set up with\n";
        std::exit(EXIT_SUCCESS);
    }
}
std::optional<LanguageInfo> process_language(std::string_view str)
{
    for (const LanguageInfo &lang : kLanguages)
    {
        if (iequals(str, lang.name))
        {
            return lang;
        }
    }

    return std::nullopt;
}

void process_arguments(ArgumentData &data)
{


    bool p_processed = false;
    for (int i = 2; i < data.count; i++)
    {
        std::string_view argument = data.argv[i];


        if (iequals(argument, "-p"))
        {
            if (i + 1 >= data.count)
            {
                LOG_ERR("No argument supplied with -p");
                std::exit(EXIT_FAILURE);
            }

            if (p_processed == true)
            {
                LOG_ERR("Duplicate Flag `-p`");
                std::exit(EXIT_FAILURE);
            }

            std::string_view language = data.argv[i + 1];

            std::optional<LanguageInfo> opt = process_language(language);
            if (!opt)
            {
                LOG_ERR("Unrecognised Language: " << language);
                std::exit(EXIT_FAILURE);
            }

            data.lang = *opt;
            p_processed = true;
            i++;
        }
        else
        {
            LOG_ERR("Unrecognised argument (" << argument << ")");
            std::exit(EXIT_FAILURE);
        }
    }
}

void generate_cmake(const ArgumentData &data)
{
    std::ofstream out("CMakeLists.txt");

    if (!out)
    {
        LOG_ERR("Could Not open CMakeLists.txt");
        std::exit(EXIT_FAILURE);
    }

    std::string_view lang = kind_to_string(data.lang.kind);

    out << "cmake_minimum_required(VERSION " << data.lang.cmake_version << ")\n\n";

    out << "project(" << data.project_name;
    out << " LANGUAGES " << lang;

    out << ")\n\n";

    out << "set(CMAKE_" << lang << "_STANDARD " << data.lang.standard << ")\n";
    out << "set(CMAKE_" << lang << "_STANDARD_REQUIRED ON)\n";
    out << "set(CMAKE_" << lang << "_EXTENSIONS OFF)\n\n";

    out << "add_executable(" << data.project_name;

    if (data.lang.kind == Kind::C)
    {
        out << " main.c";
    }

    else if (data.lang.kind == Kind::CXX)
    {
        out << " main.cpp";
    }

    out << ")\n";
}

void generate_main(const ArgumentData &data)
{
    const char *filename = data.lang.kind == Kind::C ? "main.c" : "main.cpp";

    if (std::filesystem::exists(filename))
    {
        return;
    }

    std::ofstream out(filename);
    if (!out)
    {
        LOG_ERR("Could not open " << filename);
        std::exit(EXIT_FAILURE);
    }

    if (data.lang.kind == Kind::C)
    {
        out << "#include <stdio.h>\n\nint main(void)\n{\n\tprintf(\"Hello World\\n\");\n\n\treturn 0;\n}\n";
    }
    else
    {
        out << "#include <iostream>\n\nint main()\n{\n\tstd::cout << \"Hello World\\n\";\n\treturn 0;\n}\n";
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: \"" << argv[0] << " ProjectName [Options]\"\n";
        std::cout << "Help: \"" << argv[0] << " --help\"" << std::endl;

        return EXIT_FAILURE;
    }

    for (int i = 0; i < argc; i++)
    {
        check_help(argv[i]);
    }

    ArgumentData data;
    data.count = argc;

    data.argv = std::make_unique<std::string_view[]>(argc);
    for (int i = 0; i < argc; ++i)
    {
        data.argv[i] = argv[i];
    }

    // Make sure they've not accidentally started with a flag
    // Should always be a project name
    if (data.argv[1].empty() || data.argv[1].front() == '-')
    {
        LOG_ERR("First argument must be the project name");
        return EXIT_FAILURE;
    }
    data.project_name = data.argv[1];

    process_arguments(data);
    generate_cmake(data);
    generate_main(data);
    return EXIT_SUCCESS;
}