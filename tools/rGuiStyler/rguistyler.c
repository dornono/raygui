/*******************************************************************************************
*
*   rGuiStyler v2.0 - raygui Style Editor
*
*   Compile this program using:
*       gcc -o $(NAME_PART).exe $(FILE_NAME) external/tinyfiledialogs.c -I..\.. \ 
*       -lraylib -lglfw3 -lopengl32 -lgdi32 -lcomdlg32 -lole32 -std=c99 -Wall
*
*   CONTRIBUTORS:
*       Adria Arranz    - 2018 (v2.0)
*       Jordi Jorba     - 2018 (v2.0)
*       Sergio Martinez - 2015..2017 (v1.0)
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2016 Ramon Santamaria and Co.
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#define RAYGUI_STYLE_SAVE_LOAD
#include "raygui.h"

#include "external/tinyfiledialogs.h"   // Open/Save file dialogs

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define FONT_SIZE           10
#define COLOR_REC           GuiLinesColor()
#define CONTROL_LIST_HEIGHT      38
#define STATUS_BAR_HEIGHT   25

#define NUM_CONTROLS        15
#define NUM_STYLES_A         4
#define NUM_STYLES_B         8
#define NUM_STYLES_C        12

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { 
    LABEL = 0, 
    LABELBUTTON,
    BUTTON, 
    IMAGEBUTTON,
    TOGGLE, 
    TOGGLEGROUP, 
    SLIDER, 
    SLIDERBAR, 
    PROGRESSBAR, 
    CHECKBOX, 
    SPINNER, 
    COMBOBOX, 
    TEXTBOX,
    LISTVIEW,
    COLORPICKER
} GuiControlType;

typedef enum { 
    BORDER_COLOR_NORMAL = 0,
    BASE_COLOR_NORMAL,
    TEXT_COLOR_NORMAL,
    BORDER_COLOR_FOCUSED,
    BASE_COLOR_FOCUSED,
    TEXT_COLOR_FOCUSED,
    BORDER_COLOR_PRESSED,
    BASE_COLOR_PRESSED,
    TEXT_COLOR_PRESSED,
    BORDER_COLOR_DISABLED,
    BASE_COLOR_DISABLED,
    TEXT_COLOR_DISABLED
} GuiStyleType;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static char currentPath[256];       // Path to current working folder

const char *guiControlText[NUM_CONTROLS] = { 
    "LABEL", 
    "LABELBUTTON",
    "BUTTON", 
    "IMAGEBUTTON",
    "TOGGLE", 
    "TOGGLEGROUP", 
    "SLIDER", 
    "SLIDERBAR", 
    "PROGRESSBAR", 
    "CHECKBOX", 
    "SPINNER", 
    "COMBOBOX", 
    "TEXTBOX",
    "LISTVIEW",
    "COLORPICKER"
};

const char *guiStylesTextA[NUM_STYLES_A] = { 
    "TEXT_COLOR_NORMAL",
    "TEXT_COLOR_FOCUSED",
    "TEXT_COLOR_PRESSED",
    "TEXT_COLOR_DISABLED"
};

const char *guiStylesTextB[NUM_STYLES_B] = { 
    "BORDER_COLOR_NORMAL",
    "BASE_COLOR_NORMAL",
    "BORDER_COLOR_FOCUSED",
    "BASE_COLOR_FOCUSED",
    "BORDER_COLOR_PRESSED",
    "BASE_COLOR_PRESSED",
    "BORDER_COLOR_DISABLED",
    "BASE_COLOR_DISABLED",
};

const char *guiStylesTextC[NUM_STYLES_C] = { 
    "BORDER_COLOR_NORMAL",
    "BASE_COLOR_NORMAL",
    "TEXT_COLOR_NORMAL",
    "BORDER_COLOR_FOCUSED",
    "BASE_COLOR_FOCUSED",
    "TEXT_COLOR_FOCUSED",
    "BORDER_COLOR_PRESSED",
    "BASE_COLOR_PRESSED",
    "TEXT_COLOR_PRESSED",
    "BORDER_COLOR_DISABLED",
    "BASE_COLOR_DISABLED",
    "TEXT_COLOR_DISABLED"
};

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void BtnLoadStyle(void);     // Button load style function
static void BtnSaveStyle(void);     // Button save style function

static int GetGuiStylePropertyIndex(int control, int property);
static Color ColorBox(Rectangle bounds, Color *colorPicker, Color color);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 720;
    const int screenHeight = 640;
    
    //SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "rGuiStyler - raygui style editor");
    //SetWindowMinSize(700, 700);
    
    int dropsCount = 0;
    char **droppedFiles;
    
    int guiPosX = 336;
    int guiPosY = 36;
    bool saveColor = false;

    // Define gui controls rectangles
    Rectangle bounds[NUM_CONTROLS] = {
        (Rectangle){ guiPosX + 23, guiPosY + 18, 50, 10 },          // LABEL
        (Rectangle){ guiPosX + 98, guiPosY + 18, 140, 10 },         // LABELBUTTON
        (Rectangle){ guiPosX + 184, guiPosY + 250, 180, 30 },       // BUTTON
        (Rectangle){ guiPosX + 251, guiPosY + 5, 113, 32 },         // IMAGEBUTTON
        (Rectangle){ guiPosX + 20, guiPosY + 54, 60, 30 },          // TOGGLE
        (Rectangle){ guiPosX + 98, guiPosY + 54, 65, 30 },          // TOGGLEGROUP
        (Rectangle){ guiPosX + 20, guiPosY + 104, 344, 20 },        // SLIDER
        (Rectangle){ guiPosX + 20, guiPosY + 134, 344, 20 },        // SLIDERBAR
        (Rectangle){ guiPosX + 20, guiPosY + 164, 320, 20 },        // PROGRESSBAR    
        (Rectangle){ guiPosX + 344, guiPosY + 164, 20, 20 },        // CHECKBOX
        (Rectangle){ guiPosX + 20, guiPosY + 204, 150, 30 },        // SPINNER
        (Rectangle){ guiPosX + 20, guiPosY + 250, 150, 30 },        // COMBOBOX
        (Rectangle){ guiPosX + 184, guiPosY + 204, 180, 30 },       // TEXTBOX
        (Rectangle){ 10, guiPosY + 3, 140, 563 },                   // LISTVIEW
        (Rectangle){ guiPosX + 20, guiPosY + 295, 240, 240 },       // COLORPICKER
    };

    // Get current directory
    // NOTE: Current working directory could not match current executable directory
    GetCurrentDir(currentPath, sizeof(currentPath));
    currentPath[strlen(currentPath)] = '\\';
    currentPath[strlen(currentPath) + 1] = '\0';      // Not really required
    
    GuiLoadStyleImage("resources/rguistyle_default_light.png");
    
    Texture2D texIcons = LoadTexture("resources/icons.png");
    
    SetTargetFPS(60);
    //------------------------------------------------------------

    // Gui controls data
    //-----------------------------------------------------------
    bool toggle = false;
    bool toggleValue = false;
    const char *toggleGuiText[4] = { "toggle", "group", "selection", "options" };

    float sliderValue = 50.0f;
    float sliderBarValue = 20.0f;
    float progressValue = 0.0f;
    
    bool checked = false;
    
    int spinnerValue = 28;

    int comboNum = 4;
    const char *comboText[4] = { "Image Style (.png)", "Image Style (.raw)", "Text Style (.rgst)", "Binary Style (.rgsb)" };
    int comboActive = 0;
    
    char guiText[32] =  "raygui_default_style.png";
    
    Color colorPickerValue = RED;
    
    int currentSelectedControl = -1;
    int currentSelectedProperty = -1;
    int previousSelectedProperty = -1;
    int previousSelectedControl = -1;
    
    Color colorBoxValue[12];
    
    for (int i = 0; i < 12; i++) colorBoxValue[i] = WHITE;
    
    char colorHex[9] = "00000000";
    
    bool toggleBackgroundColor = false;
    bool toggleLinesColor = false;
    bool toggleDefaultColor = false;
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsFileDropped())
        {
            droppedFiles = GetDroppedFiles(&dropsCount);
            GuiLoadStyle(droppedFiles[0]);
            ClearDroppedFiles();
        }
        
        if (toggleBackgroundColor)
        {
            styleGeneric[DEFAULT_BACKGROUND_COLOR] = GetHexValue(colorPickerValue);
        }
        else
        {
        if ((currentSelectedControl != -1) && (currentSelectedProperty != -1))
        {
            if ((previousSelectedProperty != currentSelectedProperty) || (previousSelectedControl != currentSelectedControl)) saveColor = false;
            
            if (!saveColor)
            {
                colorPickerValue = GetColor(GuiGetStyleProperty(GetGuiStylePropertyIndex(currentSelectedControl, currentSelectedProperty)));
                saveColor = true;
            }

            GuiSetStyleProperty(GetGuiStylePropertyIndex(currentSelectedControl, currentSelectedProperty), GetHexValue(colorPickerValue));
        }
        }
        
        previousSelectedProperty = currentSelectedProperty;
        previousSelectedControl = currentSelectedControl;
        
        // Update progress bar automatically
        progressValue += 0.0005f;
        if (progressValue > 1.0f) progressValue = 0.0f;

        // TODO: Support text editing on GuiTextBox()
        // NOTE: Editing mode should be detected (status = MOUSE_HOVER) and update colorPicker properly...
        sprintf(colorHex, "%02X%02X%02X%02X", colorPickerValue.r, colorPickerValue.g, colorPickerValue.b, colorPickerValue.a);
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            // Draw background rectangle
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), GuiBackgroundColor());
            
            // Draw info bar top
            DrawRectangle(0, 0, GetScreenWidth(), 24, GRAY);
            DrawText("CHOOSE CONTROL", 35, 8, styleGeneric[DEFAULT_TEXT_SIZE], LIGHTGRAY);
            DrawText(">      CHOOSE PROPERTY STYLE", 152, 8, styleGeneric[DEFAULT_TEXT_SIZE], LIGHTGRAY);
            DrawText(">                            STYLE VIEWER", guiPosX + 10, 8, styleGeneric[DEFAULT_TEXT_SIZE], LIGHTGRAY);
            
            // Draw status bar bottom
            DrawRectangle(0, GetScreenHeight() - 24, GetScreenWidth(), 24, LIGHTGRAY);
            GuiLabel((Rectangle){20, GetScreenHeight() - 16, 100, 20}, FormatText("CURRENT SELECTION: %s_%s", guiControlText[currentSelectedControl], guiStylesTextC[currentSelectedProperty]));
            GuiLabel((Rectangle){guiPosX + 100, GetScreenHeight() - 16, 100, 20}, FormatText("SAVE STATUS: %s", guiText));
            
            // Draw Gui controls
            currentSelectedControl = GuiListView(bounds[LISTVIEW], guiControlText, NUM_CONTROLS, currentSelectedControl);
            
            if (currentSelectedControl < 0) GuiDisable();
            
            switch (currentSelectedControl)
            {
                case LABEL:
                case LABELBUTTON: currentSelectedProperty = GuiListView((Rectangle){ 156, guiPosY + 3, 180, 485 }, guiStylesTextA, NUM_STYLES_A, currentSelectedProperty); break;
                case SLIDER: 
                case SLIDERBAR:
                case PROGRESSBAR:
                case CHECKBOX:
                case COLORPICKER: currentSelectedProperty = GuiListView((Rectangle){ 156, guiPosY + 3, 180, 485 }, guiStylesTextB, NUM_STYLES_B, currentSelectedProperty); break;
                case BUTTON:
                case IMAGEBUTTON:
                case TOGGLE: 
                case TOGGLEGROUP: 
                case COMBOBOX: 
                case TEXTBOX: 
                case SPINNER:
                case LISTVIEW:
                default: currentSelectedProperty = GuiListView((Rectangle){ 156, guiPosY + 3, 180, 485 }, guiStylesTextC, NUM_STYLES_C, currentSelectedProperty); break;
            }

            GuiEnable();
            
            checked = GuiCheckBox(bounds[CHECKBOX], checked);
            
            if (checked) GuiDisable();

            GuiLabel(bounds[LABEL], "rGuiStyler");

            if (GuiLabelButton(bounds[LABELBUTTON], "github.com/raysan5/raygui")) {}
            
            if (GuiImageButtonEx(bounds[IMAGEBUTTON], texIcons , (Rectangle){ 0, 0, texIcons.width/3, texIcons.height/6 }, "Load Style")) { }
            
            toggle = GuiToggleButton(bounds[TOGGLE], "toggle", toggle);
            
            toggleValue = GuiToggleGroup(bounds[TOGGLEGROUP], toggleGuiText, 4, toggleValue);
            
            sliderValue = GuiSlider(bounds[SLIDER], sliderValue, 0, 100);
            
            sliderBarValue = GuiSliderBar(bounds[SLIDERBAR], sliderBarValue, -10.0f, 40.0f);
            
            progressValue = GuiProgressBar(bounds[PROGRESSBAR], progressValue, 0.0f, 1.0f);
            
            spinnerValue = GuiSpinner(bounds[SPINNER], spinnerValue, 12, 31);
            
            comboActive = GuiComboBox(bounds[COMBOBOX], comboText, comboNum, comboActive);

            GuiTextBox(bounds[TEXTBOX], guiText, spinnerValue);
            
            colorPickerValue = GuiColorPicker(bounds[COLORPICKER], colorPickerValue);

            // Draw default styles toggles
            toggleBackgroundColor = GuiToggleButton((Rectangle){ 156, guiPosY + 496, 180, 20 }, "BACKGROUND_COLOR", toggleBackgroundColor);
            toggleLinesColor = GuiToggleButton((Rectangle){ 156, guiPosY + 521, 180, 20 }, "LINES_COLOR", toggleLinesColor);
            toggleDefaultColor = GuiToggleButton((Rectangle){ 156, guiPosY + 546, 180, 20 }, "DEFAULT_STYLE_MODE", toggleDefaultColor);
            
            // Draw labels for GuiColorPicker information
            GuiGroupBox((Rectangle){ guiPosX + 303, guiPosY + 299, 60, 74 }, "RGBA");
            
            GuiLabel((Rectangle){ guiPosX + 313, guiPosY + 310, 60, 70 }, FormatText("R:   %03i", colorPickerValue.r));
            GuiLabel((Rectangle){ guiPosX + 313, guiPosY + 325, 60, 70 }, FormatText("G:   %03i", colorPickerValue.g));
            GuiLabel((Rectangle){ guiPosX + 313, guiPosY + 340, 60, 70 }, FormatText("B:   %03i", colorPickerValue.b));
            GuiLabel((Rectangle){ guiPosX + 313, guiPosY + 355, 60, 70 }, FormatText("A:   %03i", colorPickerValue.a));
            
            GuiGroupBox((Rectangle){ guiPosX + 303, guiPosY + 385, 60, 60 }, "HSV");
            GuiTextBox((Rectangle){ guiPosX + 303, guiPosY + 545, 60, 20 }, colorHex, 8);
            
            for(int i = 0; i < 12; i++) colorBoxValue[i] = ColorBox((Rectangle){ guiPosX + 303 + 20*(i%3), guiPosY +  455 + 20*(i/3), 20, 20 }, &colorPickerValue, colorBoxValue[i]);
            
            DrawRectangleLinesEx((Rectangle){ guiPosX + 303, guiPosY +  455, 60, 80 }, 2, GetColor(styleGeneric[DEFAULT_BORDER_COLOR_NORMAL]));
            
            GuiEnable();

            // Draw Load and Save buttons
            if (GuiButton(bounds[BUTTON], "Save Style")) BtnSaveStyle();
            
            // Draw selected control rectangles
            if (currentSelectedControl >= 0) DrawRectangleLinesEx((Rectangle){ bounds[currentSelectedControl].x - 2, bounds[currentSelectedControl].y -2, bounds[currentSelectedControl].width + 4, bounds[currentSelectedControl].height + 4 }, 1, RED);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    // De-Initialization
    //--------------------------------------------------------------------------------------    
    UnloadTexture(texIcons);

    ClearDroppedFiles();        // Clear internal buffers
    
    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}

//--------------------------------------------------------------------------------------------
// Module functions
//--------------------------------------------------------------------------------------------

// Button load style function
static void BtnLoadStyle(void)
{
    // Open file dialog
    const char *filters[] = { "*.rstyle" };

    const char *fileName; // = tinyfd_openFileDialog("Load raygui style file", currentPath, 1, filters, "raygui Style Files (*.rstyle)", 0);

    // TODO: Load style file
    
    if (fileName != NULL) GuiLoadStyle(fileName);
}

// Button save style function
static void BtnSaveStyle(void)
{
    char currrentPathFile[256];

    // Add sample file name to currentPath
    strcpy(currrentPathFile, currentPath);
    strcat(currrentPathFile, "mystyle.rstyle\0");

    // Save file dialog
    const char *filters[] = { "*.rstyle" };
    const char *fileName; // = tinyfd_saveFileDialog("Save raygui style file", currrentPathFile, 1, filters, "raygui Style Files (*.rstyle)");

    if (fileName != NULL)
    {
        // TODO: Save style file (image or text or binary)
        
        GuiSaveStyle(fileName);
        fileName = "";
    }
}

static int GetGuiStylePropertyIndex(int control, int property)
{
    int guiProp = -1;
    
    switch (control)
    {
        case LABEL: guiProp = LABEL_TEXT_COLOR_NORMAL + property; break;
        case LABELBUTTON: guiProp = LABEL_TEXT_COLOR_NORMAL + property; break;
        case BUTTON: guiProp = BUTTON_BORDER_COLOR_NORMAL + property; break;
        case IMAGEBUTTON: guiProp = BUTTON_BORDER_COLOR_NORMAL + property; break;
        case TOGGLE: guiProp = TOGGLE_BORDER_COLOR_NORMAL + property; break; 
        case TOGGLEGROUP: guiProp = TOGGLE_BORDER_COLOR_NORMAL + property; break;
        case SLIDER: guiProp = SLIDER_BORDER_COLOR_NORMAL + property; break;
        case SLIDERBAR: guiProp = SLIDERBAR_BORDER_COLOR_NORMAL + property; break;
        case PROGRESSBAR: guiProp = PROGRESSBAR_BORDER_COLOR_NORMAL + property; break;
        case CHECKBOX: guiProp = CHECKBOX_BORDER_COLOR_NORMAL + property; break;
        case SPINNER: guiProp = SPINNER_BORDER_COLOR_NORMAL + property; break;
        case COMBOBOX: guiProp = COMBOBOX_BORDER_COLOR_NORMAL + property; break;
        case TEXTBOX: guiProp = TEXTBOX_BORDER_COLOR_NORMAL + property; break;
        case LISTVIEW: guiProp = LISTVIEW_BORDER_COLOR_NORMAL + property; break;
        case COLORPICKER: guiProp = COLORPICKER_BORDER_COLOR_NORMAL + property; break;
        default: break;
    }

    //guiProp = LABEL_TEXT_COLOR_NORMAL + property/3;                   // type A
    //guiProp = SLIDER_BORDER_COLOR_NORMAL + property + property/2;     // type B
    //guiProp = TOGGLE_BORDER_COLOR_NORMAL + property;                  // type C

    return guiProp;
}

// Color box control to save color samples from color picker
// NOTE: It requires colorPicker pointer for updating in case of selection
static Color ColorBox(Rectangle bounds, Color *colorPicker, Color color)
{
    Vector2 mousePoint = GetMousePosition();
    
    // Update color box
    if (CheckCollisionPointRec(mousePoint, bounds))
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) *colorPicker = (Color){ color.r, color.g, color.b, color.a };
        else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) color = *colorPicker;
    }
    
    // Draw color box
    DrawRectangleRec(bounds, color);
    DrawRectangleLinesEx(bounds, 1, GetColor(styleGeneric[DEFAULT_BORDER_COLOR_NORMAL]));
    
    return color;
}
