# Godot DWM Plugin

**Windows Desktop Window Manager (DWM) Integration for Godot 4**

Godot 的 Windows 桌面窗口管理器 (DWM) 集成插件

---

## 📋 简介 (Overview)

This Godot plugin provides direct access to Windows Desktop Window Manager (DWM) APIs, enabling modern Windows 11/10 visual effects and advanced window customization for your Godot applications.

此 Godot 插件提供对 Windows 桌面窗口管理器 (DWM) API 的直接访问，使您的 Godot 应用程序能够使用现代 Windows 11/10 视觉效果和高级窗口自定义功能。

---

## ✨ 功能特性 (Features)

### 视觉特效 (Visual Effects)
- **Mica Material** - Windows 11's native backdrop material (Windows 11 Build 22000+)
- **Acrylic Effect** - Semi-transparent blur effect for transient windows
- **Custom Title Bar Colors** - Personalized window title bar and border colors
- **Dark Mode Support** - Native dark mode for window decorations

### 窗口控制 (Window Control)
- **Frame Extension** - Extend window frame into client area for custom title bars
- **Border Customization** - Individual control over border appearance
- **System Backdrop Types** - AUTO, NONE, MAINWINDOW, TRANSIENTWINDOW, TABBEDWINDOW

### 性能优化 (Performance)
- **MMCSS Support** - Multimedia Class Scheduler Service for better performance
- **Composition Timing** - Access to DWM composition metrics

---

## 💻 系统要求 (System Requirements)

| 功能 | Windows 版本 | Godot 版本 |
|------|-------------|-----------|
| 基础 DWM 功能 | Windows 10+ | Godot 4.0+ |
| Mica / Mica Alt | Windows 11 Build 22000+ | Godot 4.0+ |
| 自定义标题栏颜色 | Windows 10+ (部分功能需 11) | Godot 4.0+ |

---

## 📦 安装 (Installation)

1. **下载插件** (Download the plugin)
   ```bash
   git clone https://github.com/yourusername/godot-dwm-plugin.git
   ```

2. **安装到项目** (Install to project)
   - Copy the `dwm` folder to your project's `addons/` directory
   - 将 `dwm` 文件夹复制到项目的 `addons/` 目录

3. **启用插件** (Enable the plugin)
   - Open Project → Project Settings → Plugins
   - Enable the "DWM" plugin
   - 打开 项目 → 项目设置 → 插件
   - 启用 "DWM" 插件

---

## 🚀 快速开始 (Quick Start)

### 基础设置 (Basic Setup)
```gdscript
# 为主窗口启用 Mica 效果 (Enable Mica effect for main window)
func _ready():
    if OS.get_name() == "Windows":
        DWM.set_systembackdrop_type(get_window(), DWM.MAINWINDOW)
```

### 自定义标题栏 (Custom Title Bar)
```gdscript
# 扩展窗口框架以创建自定义标题栏 (Extend frame for custom title bar)
func _ready():
    # 在顶部扩展 32 像素用于自定义标题栏
    # Extend 32 pixels at top for custom title bar
    DWM.extend_frame_into_client_area(get_window(), 0, 0, 32, 0)
    
    # 设置标题栏颜色 (Set title bar color)
    DWM.set_title_bar_color(get_window(), Color(0.2, 0.3, 0.8))
```

### 深色模式 (Dark Mode)
```gdscript
# 检测系统主题并应用 (Detect system theme and apply)
func _ready():
    var is_dark_mode = DisplayServer.screen_is_kept_on() # 示例检测逻辑
    DWM.enable_dark_mode(get_window(), is_dark_mode)
```

---

## 📚 API 概览 (API Overview)

### 核心方法 (Core Methods)

| 方法 | 描述 (EN) | 描述 (ZH) |
|------|-----------|-----------|
| `set_systembackdrop_type()` | Set backdrop material (Mica/Acrylic) | 设置背景材质（Mica/亚克力）|
| `extend_frame_into_client_area()` | Extend window frame | 扩展窗口框架 |
| `set_title_bar_color()` | Customize title bar color | 自定义标题栏颜色 |
| `enable_dark_mode()` | Toggle dark mode | 切换深色模式 |
| `enable_mmcss()` | Enable performance boost | 启用性能优化 |

### 系统背景类型 (System Backdrop Types)

```gdscript
enum SystembackdropType {
    AUTO = 0,           # 自动选择 / Auto-select
    NONE = 1,           # 无效果 / No effect
    MAINWINDOW = 2,     # Mica 效果 / Mica effect
    TRANSIENTWINDOW = 3, # 亚克力效果 / Acrylic effect
    TABBEDWINDOW = 4    # Mica Alt 效果 / Mica Alt effect
}
```

---

## ⚠️ 注意事项 (Important Notes)

- **平台限制** (Platform Limitation): This plugin is **Windows-only** and will not work on other platforms.
- **版本兼容** (Version Compatibility): Some features require specific Windows builds. Always check `is_title_bar_coloring_supported()` before using color functions.
- **渲染管线兼容性** (Rendering Backend Compatibility): `set_systembackdrop_type()` method is **not supported** in Vulkan rendering backend. Switch to OpenGL or D3D12 in Project Settings → Rendering → Driver to use backdrop effects.
- **性能影响** (Performance Impact): Complex backdrop effects may impact performance on older hardware.
- **Godot 窗口标志** (Godot Window Flags): Most features require native window decorations. Ensure `WINDOW_FLAG_BORDERLESS` is `false`.

---

## 🐛 故障排除 (Troubleshooting)

**问题**: 方法调用无效果 (Methods have no effect)
- **解决方案**: 检查 Windows 版本是否支持，确认插件已正确启用
- **Solution**: Check Windows version support and ensure plugin is enabled

**问题**: 背景效果不显示 (Backdrop effects not showing)
- **解决方案**: 确保窗口未设置为无边框模式，并检查渲染管线是否为 OpenGL 或 D3D12（Vulkan 不支持）
- **Solution**: Ensure window is not in borderless mode and rendering backend is OpenGL or D3D12 (Vulkan not supported)

---

## 🤝 贡献 (Contribution)

欢迎提交 Issue 和 Pull Request！ (Issues and PRs are welcome!)

---

## 📄 许可证 (License)

MIT License - see [LICENSE](LICENSE) file for details

---

## 🔗 相关资源 (Related Resources)

- [Godot Documentation](https://docs.godotengine.org/)
- [Microsoft DWM Documentation](https://docs.microsoft.com/en-us/windows/win32/dwm/dwm-overview)
- [Windows 11 Design Guidelines](https://docs.microsoft.com/en-us/windows/apps/design/signature-experiences/design-principles)

---