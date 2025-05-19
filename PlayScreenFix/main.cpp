#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <appmodel.h>
#include <shobjidl_core.h>

const std::string target = "mc-ab-new-play-screen-";

static int getMinecraftPID()
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 process;
    process.dwSize = sizeof(process);

    if (Process32First(snapshot, &process))
    {
        do
        {
            if (strcmp(process.szExeFile, "Minecraft.Windows.exe") == 0)
            {
                CloseHandle(snapshot);
                return process.th32ProcessID;
            }
        } while (Process32Next(snapshot, &process));
    }

    CloseHandle(snapshot);
    return 0;
}

std::wstring PackageFullNameFromFamilyName(std::wstring familyName)
{
    std::wstring fullName;
    UINT32 count = 0;
    UINT32 length = 0;

    // First call gets the count and length; PACKAGE_FILTER_HEAD tells Windows to query Application Packages
    LONG status = FindPackagesByPackageFamily(familyName.c_str(), PACKAGE_FILTER_HEAD, &count, nullptr, &length, nullptr, nullptr);
    if (status == ERROR_SUCCESS || status != ERROR_INSUFFICIENT_BUFFER)
        return fullName;

    PWSTR* fullNames = (PWSTR*)malloc(count * sizeof(*fullNames));
    PWSTR buffer = (PWSTR)malloc(length * sizeof(WCHAR));
    UINT32* properties = (UINT32*)malloc(count * sizeof(*properties));


    if (buffer == nullptr || fullNames == nullptr || properties == nullptr)
        goto Cleanup;


    // Second call gets all fullNames
    // buffer and properties are needed even though they're never used
    status = FindPackagesByPackageFamily(familyName.c_str(), PACKAGE_FILTER_HEAD, &count, fullNames, &length, buffer, properties);
    if (status != ERROR_SUCCESS)
        goto Cleanup;
    else
        fullName = std::wstring(fullNames[0]); // Get the first activatable package found; usually there is only one anyway

Cleanup:
    if (properties != nullptr)
        free(properties);
    if (buffer != nullptr)
        free(buffer);
    if (fullNames != nullptr)
        free(fullNames);


    return fullName;
}

std::wstring GetPackagePath(const std::wstring& packageFullName)
{
    UINT32 pathLength = 0;
    LONG rc = GetPackagePathByFullName(packageFullName.c_str(), &pathLength, NULL);
    if (rc != ERROR_INSUFFICIENT_BUFFER && rc != ERROR_MORE_DATA)
        return L"";
    wchar_t* pathBuffer = new wchar_t[pathLength];
    rc = GetPackagePathByFullName(packageFullName.c_str(), &pathLength, pathBuffer);
    if (rc != ERROR_SUCCESS)
        return L"";

    return pathBuffer;
}

bool patch(const std::string& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::vector<char> data(file.tellg());
    file.seekg(0);
    file.read(data.data(), data.size());
    file.close();

    size_t targetLength = target.size();
    bool found = false;

    for (size_t i = 0; i <= data.size() - targetLength; i++)
    {
        if (std::equal(data.begin() + i, data.begin() + i + targetLength, target.begin()))
        {
            std::fill(data.begin() + i, data.begin() + i + targetLength, ' ');
            found = true;
            i += targetLength - 1;
        }
    }

    if (!found) return false;

    std::ofstream outFile(path, std::ios::binary);
    outFile.write(data.data(), data.size());

    return true;
}

int main()
{
    setlocale(LC_CTYPE, "");

    std::cout << "Welcome to improved version of OldPlayScreen patcher by @inotflying\nThanks to him for the bypass method\n\n";

    // making sure minecraft is not running rn, patching stuff at runtime is not cool and may cause ub

    if (getMinecraftPID() != 0)
    {
        std::cout << "Mincraft is currently running!\nPlease close the game and press any key to continue...\n";
        std::cin.get();
    }

    // first - we need to get minecraft path

    std::cout << "Determining location of minecraft exe file...\n";

    std::wstring minecraftPath = GetPackagePath((PackageFullNameFromFamilyName(L"Microsoft.MinecraftUWP_8wekyb3d8bbwe"))) + L"\\Minecraft.Windows.exe";
    if (minecraftPath.empty())
    {
        std::cout << "Minecraft's exe file was not found!\nPress any key to exit...\n";
        std::cin.get();
        return -1;
    }

    std::wcout << "Minecraft exe found! Path: " << minecraftPath << std::endl;
    std::string buf(minecraftPath.begin(), minecraftPath.end());
    if (!patch(buf))
    {
        std::cout << "Patch is already applied!\nPress any key to exit...\n";
        std::cin.get();
        return -3;
    }

    std::cout << "Patch applied successfully.\n";
    system("pause");
    return 0;
}