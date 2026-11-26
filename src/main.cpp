// Toolchain smoke test only: proves raylib links and a window opens.
// Milestone 1.1 replaces this with the RAII `Game` class and the fixed-step loop.
#include <raylib.h>

int main()
{
    constexpr int kVirtualWidth = 576;
    constexpr int kVirtualHeight = 324;
    constexpr int kScale = 2;

    InitWindow(kVirtualWidth * kScale, kVirtualHeight * kScale, "Hold the Harbor");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(SKYBLUE);
        DrawText("Hold the Harbor", 40, 40, 40, DARKBLUE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
