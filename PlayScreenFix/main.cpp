#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <appmodel.h>
#include <shobjidl_core.h>
#include <string>

const std::string target = "mc-ab-new-play-screen-";
const std::string untarget = "justasimplestringabcde";
const std::wstring systemfolder = L"WindowsApps";

static int getMinecraftPID()
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W process;
    process.dwSize = sizeof(process);

    if (Process32FirstW(snapshot, &process))
    {
        do
        {
            if (wcscmp(process.szExeFile, L"Minecraft.Windows.exe") == 0)
            {
                CloseHandle(snapshot);
                return process.th32ProcessID;
            }
        } while (Process32NextW(snapshot, &process));
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

static bool wcontain(std::wstring str0, std::wstring str1) {
    int8_t find = str0.find(str1);
    if (find != std::string::npos) {
        return true;
    }
    else return false;
}

static void computeLPS(const std::string& pat, std::vector<std::size_t>& lps) {
    std::size_t m = pat.size();
    lps.assign(m, 0);
    std::size_t len = 0;
    for (std::size_t i = 1; i < m; ) {
        if (pat[i] == pat[len]) {
            lps[i++] = ++len;
        }
        else if (len > 0) {
            len = lps[len - 1];
        }
        else {
            lps[i++] = 0;
        }
    }
}

bool patch(const std::string& path, bool& wpatch)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::vector<char> data(file.tellg());
    file.seekg(0);
    file.read(data.data(), data.size());
    file.close();

    const std::string targetstr = wpatch ? target : untarget;
    const std::string replacement = wpatch ? untarget : target;
    std::size_t n = data.size();
    std::size_t m = targetstr.size();

    std::vector<std::size_t> lps;
    computeLPS(targetstr, lps);

    bool found = false;
    std::size_t i = 0;
    std::size_t j = 0;
    
    while (i < n) {
        if (data[i] == targetstr[j]) {
            ++i; ++j;
            if (j == m) {
                std::copy(
                    replacement.begin(), replacement.end(), data.begin() + (i - j)
                );
                found = true;
                j = lps[j - 1];
            }
        }
        else if (j > 0) {
            j = lps[j - 1];
        }
        else {
            ++i;
        }
    }


    if (!found) return false;

    std::ofstream outFile(path, std::ios::binary);
    outFile.write(data.data(), data.size());
    outFile.close();

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
        system("pause");
    }

    // first - we need to get minecraft path

    std::cout << "Determining location of minecraft exe file...\n";

    std::wstring minecraftPath = GetPackagePath((PackageFullNameFromFamilyName(L"Microsoft.MinecraftUWP_8wekyb3d8bbwe"))) + L"\\Minecraft.Windows.exe";
    if (minecraftPath.empty() || minecraftPath == L"\\Minecraft.Windows.exe")
    {
        std::cout << "Minecraft's exe file was not found!\n";
        system("pause");
        return -1;
    }

    std::wcout << "Minecraft exe found! Path: " << minecraftPath << std::endl;

    // If minecraft path contains "WindowsApps"
    if (wcontain(minecraftPath, systemfolder)) {
        std::wcout << L"\nMinecraft is installed in a system path: " << minecraftPath << L".\nGo to the download page of Bedrock Launcher? ";
        std::string Response;
        std::cout << "[y/N]: ";
        std::getline(std::cin, Response);
        if (Response[0] != 'y' && Response[0] != 'Y' && Response[0] != 'n' && Response[0] != 'N') {
            do {
                std::cout << "Enter a valid input: [y/N] or ENTER to exit: ";
                std::getline(std::cin, Response);
            } while (!Response.empty() && Response[0] != 'y' && Response[0] != 'Y' && Response[0] != 'n' && Response[0] != 'N');
    }

        if (Response.empty() || Response[0] == 'n' || Response[0] == 'N') {
            system("pause");
            return -5;
        }
        else {
            ShellExecuteW(0, 0, L"https://github.com/bedrockLauncher/BedrockLauncher/releases/latest/", 0, 0, SW_SHOW);
            system("pause");
            return -4;
        }
    }

    // Patch mode (patch, unpatch and exit)
    bool willpatch;
    std::cout << "\n1 = patch;\n2 = restore (if patched with this program);\n3 = exit\nYour choice: ";

    std::string response;
    std::getline(std::cin, response);
    if (response[0] != '1' && response[0] != '2' && response[0] != '3') {
        do {
            std::cout << "Enter a valid input: [1/2/3] or ENTER to exit: ";
            std::getline(std::cin, response);
        } while (!response.empty() && response[0] != '1' && response[0] != '2' && response[0] != '3');
    }

    if (response.empty() || response[0] == '3') {
        system("pause");
        return 0;
    }
    else if (response[0] == '2') willpatch = false;
    else willpatch = true;

    std::string buf(minecraftPath.begin(), minecraftPath.end());
    if (!patch(buf, willpatch))
    {
        std::cout << (willpatch ? "Patch is already applied!\n" : "Minecraft is already unpatched!\n");

        system("pause");
        return -3;
    }
    std::cout << (willpatch ? "Patch applied successfully.\n" : "Successfully unpached.\n");
    system("pause");
    return 0;
}
