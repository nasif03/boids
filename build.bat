@echo off
setlocal enabledelayedexpansion

echo Building boids...

set LIBS=-lglfw3 -lopengl32 -lgdi32
set IMGUI_OBJS=imgui imgui_draw imgui_tables imgui_widgets imgui_impl_glfw imgui_impl_opengl3
set OUTPUT=boids.exe

if not exist obj mkdir obj

echo Compiling include\glad.c...
if not exist obj\glad.o (
    gcc -Iinclude -c include\glad.c -o obj\glad.o
    if !ERRORLEVEL! NEQ 0 exit /b !ERRORLEVEL!
)

echo Compiling ImGui source files...
for %%f in (%IMGUI_OBJS%) do (
    if not exist obj\%%f.o (
        g++ -std=c++17 -Iinclude -c include\imgui\%%f.cpp -o obj\%%f.o
        if !ERRORLEVEL! NEQ 0 exit /b !ERRORLEVEL!
    )
)

if exist obj\main.o del obj\main.o

echo Compiling src/main.cpp...
g++ -std=c++17 -Wall -Iinclude -c src\main.cpp -o obj\main.o
if %ERRORLEVEL% NEQ 0 (
    echo Error: Failed to compile src/main.cpp
    exit /b %ERRORLEVEL%
)

set ALL_OBJS=
for %%f in (obj\*.o) do (
    set ALL_OBJS=!ALL_OBJS! %%f
)

echo Linking %OUTPUT%...
g++ -Wall !ALL_OBJS! -o %OUTPUT% %LIBS%
if %ERRORLEVEL% NEQ 0 (
    echo Error: Linking failed.
    exit /b %ERRORLEVEL%
)

echo.
echo Build successful! Output: %OUTPUT%
endlocal