#include <fstream>
#include <functional>
#include <iostream>
#include <windows.h>
#include <shellapi.h>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace Utils
{
    // Custom hash function for case-insensitive comparison
    struct CaseInsensitiveHash {
        size_t operator()(const std::string& s) const {
            std::string lowercase_str = s;
            std::transform(lowercase_str.begin(), lowercase_str.end(), lowercase_str.begin(), ::tolower);
            return std::hash<std::string>()(lowercase_str);
        }
    };

    // Custom comparison function for case-insensitive comparison
    struct CaseInsensitiveEqual {
        bool operator()(const std::string& s1, const std::string& s2) const {
            std::string lowercase_s1 = s1;
            std::string lowercase_s2 = s2;
            std::transform(lowercase_s1.begin(), lowercase_s1.end(), lowercase_s1.begin(), ::tolower);
            std::transform(lowercase_s2.begin(), lowercase_s2.end(), lowercase_s2.begin(), ::tolower);
            return lowercase_s1 == lowercase_s2;
        }
    };
}

namespace DragonUtils
{
    const char* CONFIG_FILE_PATH = "X:/user/data/other/dragon/config.json";

    enum ArgumentType
    {
        OPEN
    };

    std::unordered_map<std::string, DragonUtils::ArgumentType> arguments = {
        {"-o", DragonUtils::ArgumentType::OPEN}
    };

    std::unordered_map<std::string, std::function<void()>, Utils::CaseInsensitiveHash, Utils::CaseInsensitiveEqual> openArgs;

    const char* to_string(ArgumentType type)
    {
        switch (type)
        {
            case OPEN: return "OPEN";
            default: return "unknown type";
        }
    }

    DWORD openWithDefaultApp(const char* file, const char* params = nullptr)
    {
        HINSTANCE result =  ShellExecuteA(
            nullptr,
            "open",
            file,
            params,
            nullptr,
            SW_SHOW
        );

        if (reinterpret_cast<intptr_t>(result) <= 32)
        {
            DWORD errCode = GetLastError();
            WCHAR buffer[265];
            if (!FormatMessageW(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                errCode,
                0,
                buffer,
                sizeof(buffer),
                nullptr
                ))
            {
                std::cerr << "Cannot retrieve error message" << std::endl;
                return errCode;
            }

            wprintf(buffer);
            std::cerr << "file: " << file << std::endl;
        }
        return 0;
    }

    int parseConfig()
    {
        using namespace nlohmann;

        std::ifstream file(DragonUtils::CONFIG_FILE_PATH);

        if (!file.is_open())
        {
            std::cerr << "Could not load config file at: " << DragonUtils::CONFIG_FILE_PATH << std::endl;
            return -1;
        }

        json parsedJson;
        file >> parsedJson;

        auto openMap = parsedJson["-o"];
        for (auto& [key, value]: openMap.items())
        {
            DragonUtils::openArgs[key] = [value]()
            {
                DragonUtils::openWithDefaultApp(value.get<std::string>().c_str());
            };
        }

        file.close();
        return 0;
    }

    void openArg(int argc, char* argv[])
    {
        if (argc <= 2)
        {
            std::cout << "Insuffient argumemtes! Usage: dragon -o [arg]" << std::endl;
            return;
        }

        if (parseConfig() != 0) { return; }

        for (int i = 2; i <= argc - 1; i++)
        {
            auto it = openArgs.find(argv[i]);
            if (it != openArgs.end())
            {
                it->second();
            } else
            {
                openWithDefaultApp(argv[i]);
            }
        }
    }

    void executeArgument(ArgumentType type, int argc, char* argv[])
    {
        switch (type)
        {
            case OPEN:
            {
                openArg(argc, argv);
                break;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        std::cout << "No arguments passed!" << std::endl;
        return 0;
    }

    auto it = DragonUtils::arguments.find(argv[1]);
    if (it != DragonUtils::arguments.end())
    {
        DragonUtils::executeArgument(it->second, argc, argv);
    } else
    {
        std::cout << "Unknown command: " << argv[1] << std::endl;
    }

    return 0;
}