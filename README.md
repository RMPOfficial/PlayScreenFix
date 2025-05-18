# Minecraft Old Play Screen Patcher

## Description
This tool reverts Minecraft Bedrock Edition to the old play screen by patching the executable file. It automatically finds the Minecraft installation, checks version compatibility, and applies the necessary changes.

## Features
- Automatic detection of Minecraft installation location
- Version compatibility check (requires Minecraft 1.21.50 or newer)
- Safety checks to prevent patching while Minecraft is running
- Verification if patch has already been applied

## Requirements
- Minecraft Bedrock Edition (Windows) version 1.21.50 or newer
- Minecraft must be installed not in system-restricted folder aka WindowsApps

## Alternative Launchers
To work with this patch, it is recommended to use alternative launchers that install Minecraft in custom folders:
- [Jiayi Launcher](https://github.com/JiayiSoftware/JiayiLauncher)
- [Bedrock Launcher](https://github.com/BedrockLauncher/BedrockLauncher)

## How It Works
1. The tool finds the Minecraft executable location
2. Verifies the game isn't currently running
3. Checks if the installed version is compatible (1.21.50+)
4. Searches for the identifier string "mc-ab-new-play-screen-" in the executable
5. Replaces this string with spaces to disable the new play screen feature

## Usage
1. Close Minecraft if it's running
2. The tool will automatically detect and patch your Minecraft installation
3. Wait for the "Patch applied successfully" message
5. Launch Minecraft and enjoy the old play screen!

## Acknowledgements
Special thanks to [@inotflying](https://github.com/inotflying) for discovering the original bypass method. This tool is an improved version with automation and additional safety checks.
