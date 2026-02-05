@echo off
REM Start OpenCode interactively in the HLSFactory Native directory
REM Usage: start.bat

setlocal

REM Get script directory
set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

echo ==============================================
echo HLSFactory OpenCode Native - Interactive Mode
echo ==============================================
echo.
echo Available agents:
echo   @hlsfactory-orchestrator - Run the full pipeline
echo   @hlsfactory-file-writer  - Batch file operations
echo.
echo Example commands:
echo   @hlsfactory-orchestrator Process /path/to/repo to ./output
echo   Analyze the HLS designs in /path/to/repo
echo.
echo ==============================================
echo.

REM Start OpenCode interactively
opencode

endlocal
