static bool IsRunning = false;

static HWND Window = NULL;
static BITMAPINFO BitmapInfo;
static void *BitmapMemory;
static HBITMAP BitmapHandle;
static HDC BitmapDeviceContext;
constexpr int BYTES_PER_PIXEL = 3;

static void RenderWeirdGradient(int BlueOffset, int GreenOffset, int Width, int Height)
{    
    int Pitch = Width*BYTES_PER_PIXEL;
    uint8 *Row = (uint8 *)BitmapMemory;    
    for(int Y = 0;
        Y < Height;
        ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int X = 0;
            X < Width;
            ++X)
        {
            uint8 Blue = (X + BlueOffset);
            uint8 Green = (Y + GreenOffset);
            
            *Pixel++ = ((Green << 8) | Blue);
        }

        Row += Pitch;
    }
}

static void ResizeDIBSection(int width, int height){

    if(BitmapHandle) DeleteObject(BitmapHandle);
    if(!BitmapDeviceContext) BitmapDeviceContext = CreateCompatibleDC(0);
    
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = width;
    BitmapInfo.bmiHeader.biHeight = height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    HBITMAP bitmapHandle = CreateDIBSection(
        BitmapDeviceContext,
        &BitmapInfo, //  [in]  const BITMAPINFO *pbmi,
        DIB_RGB_COLORS,//  [in]  UINT             usage,
        &BitmapMemory,//  [out] VOID             **ppvBits,
        NULL, //  [in]  HANDLE           hSection,
        NULL//  [in]  DWORD            offset
    );
}

static void DrawWindow(HDC context, int x, int y, int width, int height){
    StretchDIBits(
      context, ///[in] HDC              hdc,
      x, //[in] int              xDest,
      y, //[in] int              yDest,
      width, //[in] int              DestWidth,
      height, //[in] int              DestHeight,
      x, //[in] int              xSrc,
      y, //[in] int              ySrc,
      width, //[in] int              SrcWidth,
      height, //[in] int              SrcHeight,
      BitmapMemory, //[in] const VOID       *lpBits,
      &BitmapInfo, //[in] const BITMAPINFO *lpbmi,
      DIB_RGB_COLORS, //[in] UINT             iUsage,
      SRCCOPY //[in] DWORD            rop
    );
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        {
            PostQuitMessage(0);
            IsRunning = false;
        }
        return 0;
    case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int width = ClientRect.right - ClientRect.left;
            int height = ClientRect.bottom - ClientRect.top;

            std::cout << "Resized window to " << width << " x " << height << std::endl;

            ResizeDIBSection(width,height);
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC context = BeginPaint(hwnd, &ps);

            int x = ps.rcPaint.left;
            int y = ps.rcPaint.top;
            int width = ps.rcPaint.right - ps.rcPaint.left;
            int height = ps.rcPaint.right - ps.rcPaint.left;

            DrawWindow(context, x, y, width, height);

            // All painting occurs here, between BeginPaint and EndPaint.
            FillRect(context, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

            EndPaint(hwnd, &ps);
        }
        return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


bool InitWindow(int width, int height, const char* title, HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow){
    // Register the window class.
    LPCSTR CLASS_NAME = title;
    
    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    Window = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        title,    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );

    if (AllocConsole()) {
        freopen("CONIN$", "r", stdin);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr); 
    }

    if (Window == NULL)
    {
        std::cout << "Failed to create window!" << std::endl;
        std::system("pause");
        return false;
    }

    ShowWindow(Window, nCmdShow);
    IsRunning = true;
    return true;
}

bool WindowIsOpen() {
    static MSG msg = { };
    if (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);

    int WindowWidth = ClientRect.right - ClientRect.left;
    int WindowHeight = ClientRect.bottom - ClientRect.top;

    RenderWeirdGradient(10.f,20.f,WindowWidth, WindowHeight);

    HDC DeviceContext = GetDC(Window);
    DrawWindow(DeviceContext, 0, 0, WindowWidth, WindowHeight);
    ReleaseDC(Window, DeviceContext);
    
    return IsRunning;
}

void CloseWindow(){
    FreeConsole();
}
