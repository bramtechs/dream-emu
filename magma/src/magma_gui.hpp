#include <iostream>
#include <functional>

#include "magma.h"

static PopMenuConfig GetDefaultPopMenuConfig() {
    PopMenuConfig c;
    c.backColor = BLACK;
    c.lineColor = WHITE;
    c.textColor = WHITE;
    c.fontSize = 16;
    c.padding = 10.f;
    c.arrowPadding = 8.f;
    c.arrowScale = 10.f;
    c.arrowOscil = 0.f;
    c.arrowTumbleMode = false;
    return c;
}

enum ButtonBehavior {
    BUTTON_ACTIVE,
    BUTTON_INACTIVE,
    BUTTON_SPACER,
};

typedef std::function<void()> ButtonAction;
struct Button {
    const char* text;
    ButtonBehavior behavior;
    ButtonAction onClick;
    ButtonAction onHover;
};

struct ButtonTable : std::vector<Button> {
    void AddButton(const char* text, ButtonBehavior behavior, ButtonAction func, ButtonAction hover = NULL);
    void AddButton(const char* text, ButtonAction func, ButtonAction hover = NULL);
    void AddSpacer(const char* text = "");
};

struct ButtonGroup { // always assign as 'static'
    int count; // only known after one frame
    int selected;
    int index;
    bool goingUp; // dumb hack

    ButtonGroup();

    void reset(); // call right after static constructor
    void poll(bool useWASD = true); // navigate menu with keyboard
    void pollGrid(uint cols, bool useWASD = true); // navigate menu with keyboard (in four directions)
    bool next();
    bool skip(); // used to skip stuff, like labels

    bool IsButtonSelected(int* index); // WARN: does not account for inactive buttons,
    // you need to check these yourself
};

struct PopMenu {
    uint id;

    Color actualTextColor;
    PopMenuConfig config;
    ButtonGroup group;
    Vector2 topLeft;
    Vector2 size;
    int buttonCount;
    bool initialized;

    PopMenu(PopMenuConfig config, int priority=FOCUS_NORMAL);
    PopMenu(int priority=FOCUS_NORMAL);
    ~PopMenu();

    void RenderPanel(Color overrideColor=BLANK);
    int DrawPopButton(const char* text, bool selectable=true, bool isBlank=false);
    void DrawPopButtons(ButtonTable& table);

    void EndButtons(Vector2 panelPos);
    void EndButtons();

    bool IsInFocus();
    bool IsButtonSelected(int* index);
    void ProcessSelectedButton(ButtonTable& table);
};

void OpenSystemConsole();
void CloseSystemConsole();

void UpdateAndRenderEditor();
void UpdateAndRenderEditorGUI();

void UpdateAndDrawLog();