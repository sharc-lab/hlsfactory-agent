@echo off
REM HLSFactory OpenCode Native - Run Script for Windows
REM Usage: run.bat [SOURCE_REPO] [OUTPUT_DIR]

setlocal EnableDelayedExpansion

REM Get script directory
set "SCRIPT_DIR=%~dp0"
set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

REM Load .env file from project root (one level up from script dir)
if exist "%SCRIPT_DIR%\..\.env" (
    for /f "usebackq eol=# tokens=1,* delims==" %%A in ("%SCRIPT_DIR%\..\.env") do (
        set "%%A=%%B"
    )
)

REM Require SOURCE_REPO argument, OUTPUT_DIR is optional
if "%~1"=="" (
    echo Usage: run.bat SOURCE_REPO [OUTPUT_DIR]
    exit /b 1
    echo Usage: run.bat SOURCE_REPO [OUTPUT_DIR]
    exit /b 1
) else (
    set "SOURCE_REPO=%~1"
)

if "%~2"=="" (
    set "OUTPUT_DIR=%SOURCE_REPO%\_hlsfactory_output_native"
    set "OUTPUT_DIR=%SOURCE_REPO%\_hlsfactory_output_native"
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

REM Change to project root so OpenCode picks up opencode.json
cd /d "%SCRIPT_DIR%\.."

echo Starting OpenCode orchestrator...
echo.

REM Build the prompt
set "PROMPT=Process the HLS repository at '%SOURCE_REPO%' and extract all HLS designs to '%OUTPUT_DIR%'. "
set "PROMPT=%PROMPT%Execute the complete HLSFactory pipeline: "
set "PROMPT=%PROMPT%1. Analyze the repository and identify all HLS designs "
set "PROMPT=%PROMPT%2. Extract each design into its own folder "
set "PROMPT=%PROMPT%3. Find or generate testbenches for each design "
set "PROMPT=%PROMPT%4. Generate documentation for each design "
set "PROMPT=%PROMPT%5. Compile with clang++ and fix any errors "
set "PROMPT=%PROMPT%6. Generate TCL synthesis scripts "
set "PROMPT=%PROMPT%7. Create the final manifest. "
set "PROMPT=%PROMPT%The HLS stub headers are available at: %SCRIPT_DIR%/stubs. "
set "PROMPT=%PROMPT%Work through each stage systematically and process ALL designs found."

REM Build the prompt
set "PROMPT=Process the HLS repository at '%SOURCE_REPO%' and extract all HLS designs to '%OUTPUT_DIR%'. "
set "PROMPT=%PROMPT%Execute the complete HLSFactory pipeline: "
set "PROMPT=%PROMPT%1. Analyze the repository and identify all HLS designs "
set "PROMPT=%PROMPT%2. Extract each design into its own folder "
set "PROMPT=%PROMPT%3. Find or generate testbenches for each design "
set "PROMPT=%PROMPT%4. Generate documentation for each design "
set "PROMPT=%PROMPT%5. Compile with clang++ and fix any errors "
set "PROMPT=%PROMPT%6. Generate TCL synthesis scripts "
set "PROMPT=%PROMPT%7. Create the final manifest. "
set "PROMPT=%PROMPT%The HLS stub headers are available at: %SCRIPT_DIR%/stubs. "
set "PROMPT=%PROMPT%Work through each stage systematically and process ALL designs found."

REM Run OpenCode with the orchestrator agent
opencode run -m openrouter/moonshotai/kimi-k2.5 "%PROMPT%"

echo.
echo ==============================================
echo Pipeline complete!
echo Check %OUTPUT_DIR% for results
echo ==============================================

endlocal
