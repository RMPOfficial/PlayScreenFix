#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include <fstream>
#include <vector>
#include <algorithm>

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

static std::string executeCommand(const std::string& command)
{
    std::string result;
    FILE* pipe = _popen(command.c_str(), "r");
    char buffer[128];

    if (!pipe) return "Error";

    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }

    _pclose(pipe);

    // this is needed cuz PowerShell command result has \n at the end
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());

    return result;
}

std::string getMinecraftVersion(const std::string& exePath)
{
    std::string command = "powershell -Command \"(Get-Command '" + exePath + "').Version.ToString()\"";
    std::string version = executeCommand(command);
    if (version == "Error" || version.empty()) {
        return "Unknown";
    }
    return version;
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
    std::cout << "Welcome to improved version of OldPlayScreen patcher by @inotflying\nThanks to him for the bypass method\n\n";

    // making sure minecraft is not running rn, patching stuff at runtime is not cool and may cause ub

    if (getMinecraftPID() != 0)
    {
        std::cout << "Mincraft is currently running!\nPlease close the game and press any key to continue...\n";
        std::cin.get();
    }

    // first - we need to get minecraft path

    std::cout << "Determining location of minecraft exe file...\n";

    std::string minecraftPath = executeCommand("powershell -Command \"Get-AppxPackage Microsoft.MinecraftUWP | Select-Object -ExpandProperty InstallLocation\"") + "\\Minecraft.Windows.exe";

    if (minecraftPath.empty())
    {
        std::cout << "Minecraft's exe file was not found!\nPress any key to exit...\n";
        std::cin.get();
        return -1;
    }

    std::cout << "Minecraft exe found! Path: " << minecraftPath << std::endl;

    // now we have to make sure mc ver is over or same as 1.21.50 since its a ver were first time was added new play screen

    std::string minecraftVersion = getMinecraftVersion(minecraftPath);
    std::string minVersion = "1.21.50";

    if (minecraftVersion == "Unknown")
    {
        std::cout << "Unable to determine Minecraft version. Proceed with caution.\n";
        std::cout << "Press any key to exit...\n";
        std::cin.get();
        return -1;
    }
    else if (minecraftVersion < minVersion)
    {
        std::cout << "Minecraft version " << minecraftVersion << " is lower than the required " << minVersion << "!\nPlease update Minecraft.\n";
        std::cout << "Press any key to exit...\n";
        std::cin.get();
        return -2;
    }

    std::cout << "Minecraft version " << minecraftVersion << " is supported!\n";

    if (!patch(minecraftPath))
    {
        std::cout << "Patch is already applied!\nPress any key to exit...\n";
        std::cin.get();
        return -3;
    }

    std::cout << "Patch applied successfully.\n";
    std::cout << "Press any key to exit...\n";
    std::cin.get();
    return 0;
}