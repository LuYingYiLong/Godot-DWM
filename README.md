<p align="center">
   <img width="3400" height="920" alt="dwm_banner" src="https://github.com/user-attachments/assets/7aa6b1fd-0e71-4577-9b04-574e37249d8a" />
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

## Requirements

- Windows 10 or later (some features require Windows 11 Build 22000+)
- Godot 4.0+
- OpenGL or D3D12 rendering backend (Vulkan not supported for backdrop effects)

## Installation

1. Clone this repository:
   ```bash
   git clone --recursive https://github.com/LuYingYiLong/GodotDWM.git
   ```

2. Copy the `addons/dwm/` folder to your project's `addons/` directory

## Usage

```gdscript
func _ready():
    if OS.get_name() != "Windows":
        return
    
    var window = get_window()
    
    # Enable Mica effect
    DWM.set_systembackdrop_type(window, DWM.MAINWINDOW)
    
    # Dark mode
    DWM.enable_dark_mode(window, true)
    
    # Custom title bar color
    DWM.set_title_bar_color(window, Color(0.2, 0.3, 0.8))
    
    # Extend frame for custom title bar (32px at top)
    DWM.extend_frame_into_client_area(window, 0, 0, 32, 0)
```

## System Backdrop Types

| Type | Description |
|------|-------------|
| `AUTO` | Auto-select |
| `NONE` | No effect |
| `MAINWINDOW` | Mica effect (Windows 11) |
| `TRANSIENTWINDOW` | Acrylic effect |
| `TABBEDWINDOW` | Mica Alt effect (Windows 11) |

## Building from Source

```bash
scons platform=windows target=template_debug arch=x86_64
```

## Notes

- Windows only - check `OS.get_name() == "Windows"` before use
- Some features require `is_title_bar_coloring_supported()` check
- Most features require native window decorations (`WINDOW_FLAG_BORDERLESS` must be `false`)
