#ifndef DWM_H
#define DWM_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#ifdef _WIN32
// 减少包含的头文件数量以加快编译速度
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dwmapi.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
// 启用沉浸式暗模式
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_CAPTION_COLOR
// 设置标题栏颜色
#define DWMWA_CAPTION_COLOR 35
#endif
#ifndef DWMWA_TEXT_COLOR
// 设置标题栏文本颜色
#define DWMWA_TEXT_COLOR 36
#endif
#ifndef DWMWA_BORDER_COLOR
// 设置窗口边框颜色
#define DWMWA_BORDER_COLOR 34
#endif
#endif

namespace godot
{
	/**
	 * 提供对Windows窗口管理器特性的控制，包括标题栏颜色修改、
	 * 主题切换和标题栏显示/隐藏等功能。
	**/
	class DWM : public RefCounted {
		GDCLASS(DWM, RefCounted)

	protected:
		static void _bind_methods();
		static HWND _get_godot_window_handle(Window* target_window);				// 获取Godot窗口句柄

	public:
		enum SystembackdropType {
			AUTO,
			NONE,
			MAINWINDOW,
			TRANSIENTWINDOW,
			TABBEDWINDOW
		};

		static bool is_composition_enabled();										// 检查DWM合成是否启用
		static void set_title_bar_color(Window* target_window, const Color& color, const bool sync_border); // 设置标题栏颜色
		static void set_border_color(Window* target_window, const Color& color);    // 设置边框颜色
		static void set_title_bar_text_color(Window* target_window, const Color& color); // 设置标题栏文本颜色
		static Color get_title_bar_color(Window* target_window);					// 获取标题栏颜色
		static Color get_border_color(Window* target_window);						// 获取边框颜色
		static Color get_title_bar_text_color(Window* target_window);				// 获取标题栏文本颜色
		static void reset_to_default_colors(Window* target_window);                 // 重置为默认颜色
		static bool is_title_bar_coloring_supported();								// 检查是否支持标题栏着色
		static void enable_dark_mode(Window* target_winow, bool enable);           // 启用或禁用暗模式
		static void extend_frame_into_client_area(Window* target_window, int left, int right, int top, int bottom); // 扩展窗口边框到客户区
		static Color get_colorization_color();										// 检索DWM玻璃合成使用的当前颜色
		static Dictionary get_composition_timing_info(Window* target_window);		// 获取DWM合成的时间信息
		static void enable_hostbackdropbrush(Window* target_window, bool enable);	// 启用或禁用主机背景画笔
		static void set_systembackdrop_type(Window* target_window, SystembackdropType p_type = AUTO); // 指定窗口的系统绘制背景材料
		static void enable_mmcss(bool enable);
	};
}

// 关键宏：必须放在类定义的大括号之外！
// 这告诉 Godot：DWM::SystembackdropType 这个枚举可以像 int 一样在 Variant 之间转换
VARIANT_ENUM_CAST(DWM::SystembackdropType);

#endif // !DWM_H