#ifndef DWM_H
#define DWM_H

#include <godot_cpp/classes/ref_counted.hpp>

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

namespace godot {
	class Window;

	class DWM : public RefCounted {
		GDCLASS(DWM, RefCounted)

	private:
		static HWND _get_godot_window_handle(Window* target_window);

	protected:
		static void _bind_methods();

	public:
		enum SystembackdropType {
			SYSTEMBACKDROP_TYPE_AUTO,
			SYSTEMBACKDROP_TYPE_NONE,
			SYSTEMBACKDROP_TYPE_MAINWINDOW,
			SYSTEMBACKDROP_TYPE_TRANSIENTWINDOW,
			SYSTEMBACKDROP_TYPE_TABBEDWINDOW
		};

		static bool is_composition_enabled();								// 检查 DWM 合成是否启用
		static void set_title_bar_color(
			Window* target_window,
			const Color& color,
			const bool sync_border
		);																	// 设置标题栏颜色
		static void set_border_color(
			Window* target_window,
			const Color& color
		);																	// 设置边框颜色
		static void set_title_bar_text_color(
			Window* target_window,
			const Color& color
		);																	// 设置标题栏文本颜色
		static Color get_title_bar_color(Window* target_window);			// 获取标题栏颜色
		static Color get_border_color(Window* target_window);				// 获取边框颜色
		static Color get_title_bar_text_color(Window* target_window);		// 获取标题栏文本颜色
		static void reset_to_default_colors(Window* target_window);         // 重置为默认颜色
		static bool is_title_bar_coloring_supported();						// 检查是否支持标题栏着色
		static void enable_dark_mode(
			Window* target_winow,
			const bool enable
		);																	// 启用或禁用暗模式
		static void extend_frame_into_client_area(
			Window* target_window,
			const int left,
			const int right,
			const int top,
			const int bottom
		);																	// 扩展窗口边框到客户区
		static Color get_colorization_color();								// 检索 DWM 玻璃合成使用的当前颜色
		static Dictionary get_composition_timing_info(
			Window* target_window
		);																	// 获取 DWM 合成的时间信息
		static void enable_hostbackdropbrush(
			Window* target_window,
			const bool enable
		);																	// 启用或禁用主机背景画笔
		static void set_systembackdrop_type(
			Window* target_window,
			SystembackdropType p_type = SYSTEMBACKDROP_TYPE_AUTO
		);																	// 指定窗口的系统绘制背景材料
		static void enable_mmcss(bool enable);								// 启动 MMCSS
	};
}

VARIANT_ENUM_CAST(DWM::SystembackdropType);

#endif // !DWM_H