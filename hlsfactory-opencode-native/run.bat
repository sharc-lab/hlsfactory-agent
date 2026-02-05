@echo off
REM HLSFactory OpenCode Native - Run Script for Windows
REM Usage: run.bat [SOURCE_REPO] [OUTPUT_DIR]

setlocal EnableDelayedExpansion

REM Get script directory
set "SCRIPT_DIR=%~dp0"
set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

REM Default values or use arguments
if "%~1"=="" (
    set "SOURCE_REPO=C:/Users/tanma/OneDrive/Documents/GitHub/S2CBench"
) else (
    set "SOURCE_REPO=%~1"
)

if "%~2"=="" (
    set "OUTPUT_DIR=C:/Users/tanma/OneDrive/Documents/GitHub/S2CBench/_hlsfactory_output_native"
) else (
    set "OUTPUT_DIR=%~2"
)

echo ==============================================
echo HLSFactory OpenCode Native
echo ==============================================
echo Source Repository: %SOURCE_REPO%
echo Output Directory:  %OUTPUT_DIR%
echo ==============================================
echo.

REM Check if opencode is available
where opencode >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: opencode is not installed or not in PATH
    echo Install with: npm i -g opencode-ai@latest
    exit /b 1
)

REM Create output directory if it doesn't exist
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

REM Change to script directory
cd /d "%SCRIPT_DIR%"

echo Starting OpenCode orchestrator...
echo.

REM Run OpenCode with the orchestrator agent
opencode run --agent hlsfactory-orchestrator "Process the HLS repository at '%SOURCE_REPO%' and extract all HLS designs to '%OUTPUT_DIR%'. Execute the complete HLSFactory pipeline: 1. Analyze the repository and identify all HLS designs 2. Extract each design into its own folder 3. Find or generate testbenches for each design 4. Generate documentation for each design 5. Compile with clang++ and fix any errors 6. Generate TCL synthesis scripts 7. Create the final manifest. The HLS stub headers are available at: %SCRIPT_DIR%/stubs. Work through each stage systematically and process ALL designs found."

echo.
echo ==============================================
echo Pipeline complete!
echo Check %OUTPUT_DIR% for results
echo ==============================================

endlocal
