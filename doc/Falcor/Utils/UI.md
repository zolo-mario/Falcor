# UI - User Interface Utilities

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **Font** - Font management
- [x] **Gui** - GUI system

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture, FBO)
- **Core/Enum** - Enum utilities
- **Utils/Math** - Mathematical utilities (Vector)
- **Utils/Color** - Color utilities (SampledSpectrum)
- External: **imgui.h** - Dear ImGui library

## Module Overview

The UI module provides comprehensive user interface utilities for the Falcor rendering framework. It includes a complete GUI system wrapping Dear ImGui, font management for text rendering, and various UI widgets (dropdowns, buttons, sliders, graphs, etc.). These utilities are essential for creating interactive rendering applications, debug tools, and parameter tuning interfaces.

## Component Specifications

### Font

**Files**:
- [`Font.h`](Source/Falcor/Utils/UI/Font.h:1) - Font header
- [`Font.cpp`](Source/Falcor/Utils/UI/Font.cpp:1) - Font implementation

**Purpose**: Font management for text rendering in GUI.

**Key Features**:
- Monospaced font support
- Character texture atlas
- Character descriptor management
- Font height and width calculations
- Tab width and letter spacing
- Efficient text rendering

**Core Methods**:

**Constructor**:
- [`Font(ref<Device> pDevice, const std::filesystem::path& path)`](Source/Falcor/Utils/UI/Font.h:50) - Constructor
  - Parameters:
    - `pDevice` - GPU device
    - `path` - File path without extension
  - Throws exception if creation failed

**Texture Access**:
- [`getTexture() const`](Source/Falcor/Utils/UI/Font.h:66) - Get character texture
  - Returns: Texture containing character atlas

**Character Queries**:
- [`getCharDesc(char c) const`](Source/Falcor/Utils/UI/Font.h:71) - Get character descriptor
  - Parameters:
    - `c` - Character to query
  - Returns: CharTexCrdDesc with texture coordinates and size

**Font Metrics**:
- [`getFontHeight() const`](Source/Falcor/Utils/UI/Font.h:80) - Get font height in pixels
- [`getTabWidth() const`](Source/Falcor/Utils/UI/Font.h:85) - Get tab width in pixels
- [`getLettersSpacing() const`](Source/Falcor/Utils/UI/Font.h:90) - Get spacing between letters in pixels

**Data Structures**:

**CharTexCrdDesc**:
- [`topLeft`](Source/Falcor/Utils/UI/Font.h:59) - Non-normalized origin of character in texture
- [`size`](Source/Falcor/Utils/UI/Font.h:60) - Size in pixels of character
- Used to initialize texture-coordinate when rendering

**Font Class**:
- [`mpTexture`](Source/Falcor/Utils/UI/Font.h:103) - Character texture reference
- [`mCharDesc[]`](Source/Falcor/Utils/UI/Font.h:104) - Character descriptor array
- [`mFontHeight`](Source/Falcor/Utils/UI/Font.h:105) - Font height in pixels
- [`mTabWidth`](Source/Falcor/Utils/UI/Font.h:106) - Tab width in pixels
- [`mLetterSpacing`](Source/Falcor/Utils/UI/Font.h:107) - Letter spacing in pixels

**Character Range**:
- [`mFirstChar`](Source/Falcor/Utils/UI/Font.h:98) - First character ('!')
- [`mLastChar`](Source/Falcor/Utils/UI/Font.h:99) - Last character ('~')
- [`mCharCount`](Source/Falcor/Utils/UI/Font.h:100) - Number of characters (mLastChar - mFirstChar + 1)
- [`mTexWidth`](Source/Falcor/Utils/UI/Font.h:101) - Texture width in pixels

**Technical Details**:

**Character Atlas**:
- Single texture containing all characters
- Non-normalized texture coordinates
- Character-specific size information
- Efficient text rendering

**Font Metrics**:
- Height: Vertical space per character
- Tab width: Horizontal space for tab character
- Letter spacing: Space between letters
- Measured in pixels

**Character Range**:
- ASCII range from '!' to '~'
- 95 printable characters
- Fixed character set

**Use Cases**:
- Text rendering in GUI
- Debug information display
- Parameter labels
- Status messages
- Console output

### Gui

**Files**:
- [`Gui.h`](Source/Falcor/Utils/UI/Gui.h:1) - GUI header
- [`Gui.cpp`](Source/Falcor/Utils/UI/Gui.cpp:1) - GUI implementation
- [`Gui.slang`](Source/Falcor/Utils/UI/Gui.slang:1) - GUI shader

**Purpose**: Complete GUI system wrapping Dear ImGui.

**Key Features**:
- Dear ImGui integration
- Comprehensive widget set
- Dropdown menus and radio buttons
- Text boxes and sliders
- Color pickers
- Graphs and plots
- Image display
- Drag and drop support
- Keyboard and mouse event handling
- Window management
- Menu system
- Font management

**Core Methods**:

**Lifecycle**:
- [`Gui(ref<Device> pDevice, uint32_t width, uint32_t height, float scaleFactor = 1.f)`](Source/Falcor/Utils/UI/Gui.h:687) - Constructor
  - Parameters:
    - `pDevice` - GPU device
    - `width` - GUI width in pixels
    - `height` - GUI height in pixels
    - `scaleFactor` - UI scale factor

- [`~Gui()`](Source/Falcor/Utils/UI/Gui.h:688) - Destructor

**Frame Management**:
- [`beginFrame()`](Source/Falcor/Utils/UI/Gui.h:706) - Begin new frame
  - Must be called at start of each frame

- [`render(RenderContext* pContext, const ref<Fbo>& pFbo, float elapsedTime)`](Source/Falcor/Utils/UI/Gui.h:712) - Render GUI
  - Parameters:
    - `pContext` - Render context
    - `pFbo` - Framebuffer object
    - `elapsedTime` - Time since last frame

**Event Handling**:
- [`onWindowResize(uint32_t width, uint32_t height)`](Source/Falcor/Utils/UI/Gui.h:717) - Handle window resize
  - Parameters:
    - `width` - New window width
    - `height` - New window height

- [`onMouseEvent(const MouseEvent& event)`](Source/Falcor/Utils/UI/Gui.h:722) - Handle mouse events
  - Returns: True if event was handled

- [`onKeyboardEvent(const KeyboardEvent& event)`](Source/Falcor/Utils/UI/Gui.h:727) - Handle keyboard events
  - Returns: True if event was handled

**Font Management**:
- [`addFont(const std::string& name, const std::filesystem::path& path)`](Source/Falcor/Utils/UI/Gui.h:695) - Add a font
  - Parameters:
    - `name` - Font name
    - `path` - Font file path

- [`setActiveFont(const std::string& font)`](Source/Falcor/Utils/UI/Gui.h:700) - Set active font
  - Parameters:
    - `font` - Font name

- [`getFont(std::string f = "")`](Source/Falcor/Utils/UI/Gui.h:702) - Get font by name
  - Parameters:
    - `f` - Font name
  - Returns: Font pointer

- [`pickUniqueColor(const std::string& key)`](Source/Falcor/Utils/UI/Gui.h:690) - Pick unique color for key
  - Parameters:
    - `key` - Color key string
  - Returns: Unique color value

**Data Structures**:

**DropdownValue**:
- [`value`](Source/Falcor/Utils/UI/Gui.h:77) - User-defined index
- [`label`](Source/Falcor/Utils/UI/Gui.h:78) - Label of dropdown option

**RadioButton**:
- [`buttonID`](Source/Falcor/Utils/UI/Gui.h:85) - User-defined index
- [`label`](Source/Falcor/Utils/UI/Gui.h:86) - Label of radio button
- [`sameLine`](Source/Falcor/Utils/UI/Gui.h:87) - Same line as previous widget

**TextFlags**:
- [`Empty`](Source/Falcor/Utils/UI/Gui.h:94) - No flags
- [`FitWindow`](Source/Falcor/Utils/UI/Gui.h:95) - Also hides label

**WindowFlags**:
- [`Empty`](Source/Falcor/Utils/UI/Gui.h:100) - No flags
- [`ShowTitleBar`](Source/Falcor/Utils/UI/Gui.h:101) - Show title bar
- [`AllowMove`](Source/Falcor/Utils/UI/Gui.h:102) - Allow window move
- [`SetFocus`](Source/Falcor/Utils/UI/Gui.h:103) - Take focus when window appears
- [`CloseButton`](Source/Falcor/Utils/UI/Gui.h:104) - Add close button
- [`NoResize`](Source/Falcor/Utils/UI/Gui.h:105) - Disable manual resizing
- [`AutoResize`](Source/Falcor/Utils/UI/Gui.h:106) - Auto resize window to fit content

**WidgetFlags**:
- [`Empty`](Source/Falcor/Utils/UI/Gui.h:113) - No flags
- [`SameLine`](Source/Falcor/Utils/UI/Gui.h:114) - Show title bar
- [`Inactive`](Source/Falcor/Utils/UI/Gui.h:115) - Inactive widget, disallow edits

**Widgets Class**:
- [`group(const std::string& label, bool beginExpanded = false)`](Source/Falcor/Utils/UI/Gui.h:128) - Begin new group
- [`indent(float i)`](Source/Falcor/Utils/UI/Gui.h:133) - Indent next item
- [`separator(uint32_t count = 1)`](Source/Falcor/Utils/UI/Gui.h:138) - Add separator
- [`dummy(const char label[], const float2& size, bool sameLine = false)`](Source/Falcor/Utils/UI/Gui.h:146) - Dummy object for spacing
- [`rect(const float2& size, const float4& color = float4(1.0f, 1.0f, 1.0f, 1.0f), bool filled = false, bool sameLine = false)`](Source/Falcor/Utils/UI/Gui.h:155) - Display rectangle
- [`dropdown(const char label[], const DropdownList& values, uint32_t& var, bool sameLine = false)`](Source/Falcor/Utils/UI/Gui.h:167) - Add dropdown menu
- [`dropdown<T>(const char label[], T& var, bool sameLine = false)`](Source/Falcor/Utils/UI/Gui.h:177) - Add dropdown menu for enum
- [`button(const char label[], bool sameLine = false)`](Source/Falcor/Utils/UI/Gui.h:205) - Add button
- [`radioButtons(const RadioButtonGroup& buttons, uint32_t& activeID)`](Source/Falcor/Utils/UI/Gui.h:213) - Add radio button group
- [`direction(const char label[], float3& direction)`](Source/Falcor/Utils/UI/Gui.h:221) - Add direction widget
- [`checkbox<T>(const char label[], T& var, bool sameLine = false)`](Source/Falcor/Utils/UI/Gui.h:232) - Add checkbox
- [`dragDropSource(const char label[], const char dataLabel[], const std::string& payloadString)`](Source/Falcor/Utils/UI/Gui.h:241) - Drag and drop source
- [`dragDropDest(const char dataLabel[], std::string& payloadString)`](Source/Falcor/Utils/UI/Gui.h:249) - Drag and drop destination
- [`text(const std::string& text, bool sameLine = false)`](Source/Falcor/Utils/UI/Gui.h:257) - Static text
- [`textWrapped(const std::string& text)`](Source/Falcor/Utils/UI/Gui.h:263) - Static text wrapped to window
- [`textbox(const std::string& label, std::string& text, TextFlags flags = TextFlags::Empty)`](Source/Falcor/Utils/UI/Gui.h:272) - Add text box
- [`textbox(const char label[], char buf[], size_t bufSize, uint32_t lineCount = 1, TextFlags flags = TextFlags::Empty)`](Source/Falcor/Utils/UI/Gui.h:282) - Add text box with buffer
- [`multiTextbox(const char label[], const std::vector<std::string>& textLabels, std::vector<std::string>& textEntries)`](Source/Falcor/Utils/UI/Gui.h:287) - Add multiple text boxes
- [`tooltip(const std::string& text, bool sameLine = true)`](Source/Falcor/Utils/UI/Gui.h:295) - Render tooltip
- [`rgbColor(const char label[], float3& var, bool sameLine = false)`](Source/Falcor/Utils/UI/Gui.h:305) - Add RGB color widget
- [`rgbaColor(const char label[], float4& var, bool sameLine = false)`](Source/Falcor/Utils/UI/Gui.h:314) - Add RGBA color widget
- [`loadImage(const std::filesystem::path& path)`](Source/Falcor/Utils/UI/Gui.h:322) - Load image as texture
- [`image(const char label[], const Texture* pTex, float2 size = float2(0), bool maintainRatio = true, bool sameLine = false)`](Source/Falcor/Utils/UI/Gui.h:332) - Display image
- [`imageButton(const char label[], const Texture* pTex, float2 size, bool maintainRatio = true, bool sameLine = false)`](Source/Falcor/Utils/UI/Gui.h:333) - Add image button
- [`var<T>(const char label[], T& var, T minVal, T maxVal, float step, bool sameLine, const char* displayFormat)`](Source/Falcor/Utils/UI/Gui.h:349) - Add scalar variable widget
- [`slider<T>(const char label[], T& var, T minVal, T maxVal, float step, bool sameLine, const char* displayFormat)`](Source/Falcor/Utils/UI/Gui.h:359) - Add scalar slider widget
- [`var<T>(const char label[], T& var, typename T::value_type minVal, typename T::value_type maxVal, float step, bool sameLine, const char* displayFormat)`](Source/Falcor/Utils/UI/Gui.h:380) - Add vector variable widget
- [`slider<T>(const char label[], T& var, typename T::value_type minVal, typename T::value_type maxVal, float step, bool sameLine, const char* displayFormat)`](Source/Falcor/Utils/UI/Gui.h:392) - Add vector slider widget
- [`matrix<T>(const char label[], MatrixType& var, float minVal, float maxVal, bool sameLine)`](Source/Falcor/Utils/UI/Gui.h:412) - Add matrix widget
- [`graph(const char label[], GraphCallback func, void* pUserData, uint32_t sampleCount, int32_t sampleOffset, float yMin, float yMax, uint32_t width, uint32_t height)`](Source/Falcor/Utils/UI/Gui.h:429) - Add graph widget
- [`spectrum<T>(const char label[], SampledSpectrum<T>& spectrum)`](Source/Falcor/Utils/UI/Gui.h:449) - Add spectrum widget
- [`spectrum<T>(const char label[], SampledSpectrum<T>& spectrum, SpectrumUI<T>& spectrumUI)`](Source/Falcor/Utils/UI/Gui.h:458) - Add spectrum widget with UI

**Group Class**:
- [`Group()`](Source/Falcor/Utils/UI/Gui.h:552) - Default constructor
- [`Group(Gui* pGui, const std::string& label, bool beginExpanded = false)`](Source/Falcor/Utils/UI/Gui.h:560) - Constructor with label and expansion state
- [`open() const`](Source/Falcor/Utils/UI/Gui.h:573) - Check if group is open
- [`operator bool() const`](Source/Falcor/Utils/UI/Gui.h:578) - Bool operator to check if group is open
- [`release()`](Source/Falcor/Utils/UI/Gui.h:580) - End collapsible group block

**Window Class**:
- [`Window(Gui* pGui, const char* name, uint2 size = {0, 0}, uint2 pos = {0, 0}, Gui::WindowFlags flags = Gui::WindowFlags::Default)`](Source/Falcor/Utils/UI/Gui.h:599) - Constructor
- [`Window(const Widgets& w, const char* name, bool& open, uint2 size, uint2 pos, Gui::WindowFlags flags)`](Source/Falcor/Utils/UI/Gui.h:615) - Constructor with open flag
- [`Window(const Widgets& w, const char* name, uint2 size, uint2 pos, Gui::WindowFlags flags)`](Source/Falcor/Utils/UI/Gui.h:622) - Constructor with Widgets
- [`~Window()`](Source/Falcor/Utils/UI/Gui.h:635) - Destructor
- [`release()`](Source/Falcor/Utils/UI/Gui.h:640) - End window
- [`columns(uint32_t numColumns)`](Source/Falcor/Utils/UI/Gui.h:646) - Begin column within current window
- [`nextColumn()`](Source/Falcor/Utils/UI/Gui.h:651) - Proceed to next column within window
- [`windowPos(uint32_t x, uint32_t y)`](Source/Falcor/Utils/UI/Gui.h:658) - Set window position in pixels
- [`windowSize(uint32_t width, uint32_t height)`](Source/Falcor/Utils/UI/Gui.h:665) - Set window size in pixels

**Menu Class**:
- [`Menu(Gui* pGui, const char* name)`](Source/Falcor/Utils/UI/Gui.h:475) - Constructor
- [`~Menu()`](Source/Falcor/Utils/UI/Gui.h:477) - Destructor
- [`release()`](Source/Falcor/Utils/UI/Gui.h:482) - End menu of items

**Dropdown Class**:
- [`Dropdown(Gui* pGui, const char label[])`](Source/Falcor/Utils/UI/Gui.h:489) - Constructor
- [`~Dropdown()`](Source/Falcor/Utils/UI/Gui.h:495) - Destructor
- [`release()`](Source/Falcor/Utils/UI/Gui.h:499) - End drop down menu list of items
- [`item(const std::string& label, bool& var, const std::string& shortcut = "")`](Source/Falcor/Utils/UI/Gui.h:508) - Item for main menu bar
- [`item(const std::string& label, const std::string& shortcut = "")`](Source/Falcor/Utils/UI/Gui.h:516) - Item for dropdown menu
- [`separator()`](Source/Falcor/Utils/UI/Gui.h:521) - Add separator between menu items
- [`menu(const char* name)`](Source/Falcor/Utils/UI/Gui.h:527) - Add sub-menu within current dropdown menu
- [`item(const std::string& label)`](Source/Falcor/Utils/UI/Gui.h:542) - Add item to menu

**MainMenu Class**:
- [`MainMenu(Gui* pGui)`](Source/Falcor/Utils/UI/Gui.h:675) - Constructor
- [`~MainMenu()`](Source/Falcor/Utils/UI/Gui.h:677) - Destructor
- [`release()`](Source/Falcor/Utils/UI/Gui.h:682) - End main menu bar

**IDScope Class**:
- [`IDScope(const void* id)`](Source/Falcor/Utils/UI/Gui.h:739) - Constructor
- [`~IDScope()`](Source/Falcor/Utils/UI/Gui.h:741) - Destructor

**Technical Details**:

**Dear ImGui Integration**:
- Wraps external Dear ImGui library
- Provides Falcor-specific API
- Handles font management
- Manages window lifecycle

**Widget Types**:
- **Dropdowns**: Single-selection menus with user variables
- **Radio Buttons**: Mutually exclusive selection groups
- **Buttons**: Clickable action triggers
- **Checkboxes**: Boolean toggles
- **Text Boxes**: Single and multi-line text input
- **Sliders**: Scalar and vector value adjustment
- **Color Pickers**: RGB and RGBA color selection
- **Graphs**: Function-based visualization
- **Images**: Texture display and buttons
- **Spectrum**: Spectral curve visualization

**Window Management**:
- Multiple windows support
- Window flags for behavior control
- Position and size management
- Column layout support
- Title bar and close button

**Menu System**:
- Main menu bar
- Dropdown menus
- Sub-menus
- Keyboard shortcuts
- Separators

**Event Handling**:
- Mouse events (clicks, drags)
- Keyboard events
- Window resize events
- Drag and drop support

**Use Cases**:
- Debug UI for parameter tuning
- Render pass configuration
- Scene inspection tools
- Performance profiling interfaces
- Material editor
- Light editor
- Camera controls

## Architecture Patterns

### Wrapper Pattern
- Wraps external Dear ImGui library
- Provides consistent API
- Handles Falcor-specific features
- Font management integration

### Widget Pattern
- Type-safe widget creation
- Direct variable binding
- Event callback support
- Layout management

### Menu Pattern
- Hierarchical menu structure
- Keyboard shortcuts
- Dropdown and radio button groups

### Window Pattern
- Multiple window support
- Window flags for behavior
- Position and size management
- Column layout support

## Technical Details

### Font Implementation

**Character Atlas**:
- Single texture containing all characters
- Non-normalized texture coordinates (0-1 range)
- Character-specific size information
- Efficient GPU-based text rendering

**Font Metrics**:
- Height: Vertical space per character
- Tab width: Horizontal space for tab character
- Letter spacing: Space between letters
- All measured in pixels

**Character Range**:
- ASCII printable characters (95 characters)
- Fixed character set from '!' to '~'
- Efficient lookup by character code

### GUI Implementation

**Dear ImGui Integration**:
- External library dependency
- Falcor-specific extensions
- Font system integration
- Window management

**Widget System**:
- Type-safe API with templates
- Direct variable references
- Event-driven updates
- Layout control (sameLine, indentation)

**Event System**:
- Mouse event handling
- Keyboard event handling
- Window resize handling
- Drag and drop support

**Menu System**:
- Main menu bar
- Dropdown menus with sub-menus
- Keyboard shortcuts
- Separators for organization

**Window Management**:
- Multiple independent windows
- Window flags for behavior control
- Position and size management
- Column layout system
- Title bar and close button

### Color Widgets

**RGB Color Widget**:
- Direct float3 variable binding
- Min/max value support
- Step rate control
- Display format support

**RGBA Color Widget**:
- Direct float4 variable binding
- Min/max value support
- Step rate control
- Display format support

### Graph Widget

**Function-Based Visualization**:
- User-provided callback function
- Sample count control
- Sample offset for x-axis positioning
- Y-axis min/max for auto-range detection
- Width/height control
- User data pointer for callbacks

### Spectrum Widget

**Spectrum Visualization**:
- SampledSpectrum display and editing
- Modifiable parameters
- SpectrumUI for parameter control
- Direct variable binding

### Text Widgets

**Text Box**:
- Single and multi-line support
- Direct string variable binding
- Buffer-based input for large text
- Line count control

**Image Widgets**

**Image Display**:
- Texture-based rendering
- Aspect ratio maintenance
- Size control
- Cached image loading

**Slider Widgets**

**Scalar Sliders**:
- Min/max value support
- Step rate control
- Display format support
- Direct variable binding

**Vector Sliders**:
- Min/max value support
- Step rate control
- Display format support
- Direct variable binding

## Progress Log

- **2026-01-07T19:16:16Z**: UI sub-module analysis completed. Analyzed Font and Gui classes. Documented font management system with character atlas and metrics, comprehensive GUI system wrapping Dear ImGui with extensive widget set (dropdowns, buttons, sliders, graphs, colors, text boxes, images, menus, windows). Created comprehensive technical specification with detailed code patterns, data structures, and use cases.

## Next Steps

Proceed to update Utils module Folder Note to mark all sub-modules as Complete, then proceed to analyze RenderPasses module.
