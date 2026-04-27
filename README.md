<p align="center">
   <img alt="dwm_banner" src="https://github.com/user-attachments/assets/7aa6b1fd-0e71-4577-9b04-574e37249d8a" />
</p>

<h1 align="center">Godot-DWM</h1>

<p align="center">
Windows Desktop Window Manager (DWM) integration for Godot 4. Provides access to Windows 11/10 visual effects like Mica, Acrylic, and custom title bars.
</p>

## Features

- **Mica/Acrylic Effects** - Native Windows 11 backdrop materials
- **Custom Title Bar** - Colors and frame extension for custom UI
- **Dark Mode** - Native window decoration theming
- **MMCSS Support** - Multimedia Class Scheduler for better performance
- **Low-level DWM Access** - Raw DWM attributes, frame bounds, corner preferences, and Win32 style bits

## Requirements

- Windows 10 or later (some features require Windows 11 Build 22000+)
- Godot 4.0+
- OpenGL or D3D12 rendering backend (Vulkan not supported for backdrop effects)

## Installation

Recommended: install Godot-DWM from the Godot Asset Library:

**[Godot Asset Library - Godot-DWM](https://godotengine.org/asset-library/asset/4476)**

You can also download a release from GitHub or build the GDExtension from source if you need the latest development version.

## Usage

```gdscript
func _ready() -> void:
    if OS.get_name() != "Windows":
        return
    
    var window: Window = get_window()
    
    # Enable Mica effect
    DWM.set_systembackdrop_type(window, DWM.MAINWINDOW)
    
    # Dark mode
    DWM.enable_dark_mode(window, true)
    
    # Custom title bar color
    DWM.set_title_bar_color(window, Color(0.2, 0.3, 0.8))
    
    # Extend frame for custom title bar (32px at top)
    DWM.extend_frame_into_client_area(window, 0, 0, 32, 0)
```

## Low-level Usage

```gdscript
func _ready() -> void:
    if OS.get_name() != "Windows":
        return

    var window: Window = get_window()

    # Rounded corner control (Windows 11)
    DWM.set_window_corner_preference(window, DWM.CORNER_ROUND_SMALL)

    # Raw DWM attributes, useful when Microsoft adds new supported values
    DWM.set_window_attribute_bool(window, DWM.ATTR_TRANSITIONS_FORCEDISABLED, true)
    var frame_rect: Rect2i = DWM.get_window_bounds(window, true)

    # Win32 style bits. Example: WS_THICKFRAME = 0x00040000
    DWM.set_window_style_bits(window, 0x00040000, false)
```

## System Backdrop Types

| Type | Description |
|------|-------------|
| `AUTO` | Auto-select |
| `NONE` | No effect |
| `MAINWINDOW` | Mica effect (Windows 11) |
| `TRANSIENTWINDOW` | Acrylic effect |
| `TABBEDWINDOW` | Mica Alt effect (Windows 11) |

The previous names (`SYSTEMBACKDROP_TYPE_MAINWINDOW`, etc.) are still available as aliases.

## Building from Source

```bash
scons platform=windows target=template_debug arch=x86_64
```

## Notes

- Windows only - check `OS.get_name() == "Windows"` before use
- Some features require `is_title_bar_coloring_supported()` check
- Most features require native window decorations (`WINDOW_FLAG_BORDERLESS` must be `false`)
