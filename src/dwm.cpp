#include "dwm.h"

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/display_server.hpp>

namespace godot {

	void DWM::_bind_methods() {
		BIND_ENUM_CONSTANT(AUTO);
		BIND_ENUM_CONSTANT(NONE);
		BIND_ENUM_CONSTANT(MAINWINDOW);
		BIND_ENUM_CONSTANT(TRANSIENTWINDOW);
		BIND_ENUM_CONSTANT(TABBEDWINDOW);
		ClassDB::bind_static_method("DWM", D_METHOD("is_composition_enabled"), &DWM::is_composition_enabled);
		ClassDB::bind_static_method("DWM", D_METHOD("set_title_bar_color", "target_window", "color", "sync_border"), &DWM::set_title_bar_color, DEFVAL(true));
		ClassDB::bind_static_method("DWM", D_METHOD("get_title_bar_color", "target_window"), &DWM::get_title_bar_color);
		ClassDB::bind_static_method("DWM", D_METHOD("set_border_color", "target_window", "color"), &DWM::set_border_color);
		ClassDB::bind_static_method("DWM", D_METHOD("get_border_color", "target_window"), &DWM::get_border_color);
		ClassDB::bind_static_method("DWM", D_METHOD("reset_to_default_colors", "target_window"), &DWM::reset_to_default_colors);
		ClassDB::bind_static_method("DWM", D_METHOD("is_title_bar_coloring_supported"), &DWM::is_title_bar_coloring_supported);
		ClassDB::bind_static_method("DWM", D_METHOD("enable_dark_mode", "target_window", "enable"), &DWM::enable_dark_mode);
		ClassDB::bind_static_method("DWM", D_METHOD("extend_frame_into_client_area", "target_window", "left", "right", "top", "bottom"), &DWM::extend_frame_into_client_area);
		ClassDB::bind_static_method("DWM", D_METHOD("get_colorization_color"), &DWM::get_colorization_color);
		ClassDB::bind_static_method("DWM", D_METHOD("get_composition_timing_info", "target_window"), &DWM::get_composition_timing_info);
		ClassDB::bind_static_method("DWM", D_METHOD("enable_hostbackdropbrush", "target_window", "enable"), &DWM::enable_hostbackdropbrush);
		ClassDB::bind_static_method("DWM", D_METHOD("set_systembackdrop_type", "target_window", "type"), &DWM::set_systembackdrop_type);
		ClassDB::bind_static_method("DWM", D_METHOD("enable_mmcss", "enable"), &DWM::enable_mmcss);
	}

	HWND DWM::_get_godot_window_handle(Window* target_window) {
#ifdef _WIN32
		if (!target_window) {
			UtilityFunctions::push_error("Target window is null.");
			return nullptr;
		}
		Window* window_node = Object::cast_to<Window>(target_window);
		if (!window_node) {
			UtilityFunctions::push_error("The provided object is not a Window node.");
			return nullptr;
		}
		DisplayServer* display_server = DisplayServer::get_singleton();
		if (display_server == nullptr) {
			UtilityFunctions::push_error("Failed to get the DisplayServer singleton.");
			return nullptr;
		}
		// 获取本机窗口句柄
		int64_t hwnd_int = display_server->window_get_native_handle(DisplayServer::WINDOW_HANDLE, window_node->get_window_id());

		if (!hwnd_int) {
			UtilityFunctions::push_error("Got null native handle.");
			return nullptr;
		}
		// 转换为HWND类型
		HWND hwnd = reinterpret_cast<HWND>(hwnd_int);

		if (!IsWindow(hwnd)) {
			UtilityFunctions::push_error("The obtained handle is not a valid window.");
			return nullptr;
		}
		return hwnd;
#else
		return nullptr;
#endif
	}

	bool DWM::is_composition_enabled() {
#ifdef _WIN32
		BOOL is_enabled = FALSE;
		HRESULT hr = DwmIsCompositionEnabled(&is_enabled);
		if (FAILED(hr)) return false;
		return is_enabled != FALSE;
#else
		return false;
#endif // !_WIN32
	}

	void DWM::set_title_bar_color(Window* target_window, const Color& color, const bool sync_border) {
#ifdef _WIN32
		// 获取Godot窗口句柄
		HWND hwnd = _get_godot_window_handle(target_window);
		if (hwnd == nullptr) return;
		// 将Color转换为COLORREF ([0.0, 1.0]范围转换为[0, 255]范围)
		int r = CLAMP(color.r * 255, 0, 255);
		int g = CLAMP(color.g * 255, 0, 255);
		int b = CLAMP(color.b * 255, 0, 255);
		// 启用暗模式标题栏
		BOOL dark_mode = TRUE;
		HRESULT hr_dark = DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark_mode, sizeof(dark_mode));
		if (FAILED(hr_dark)) UtilityFunctions::push_warning("Failed to set dark mode, results may vary");
		// 设置标题栏颜色
		COLORREF caption_color = RGB(r, g, b);
		HRESULT hr_caption = DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &caption_color, sizeof(caption_color));
		if (FAILED(hr_caption)) UtilityFunctions::push_error("Failed to set caption color - feature may not be supported");
		// 根据亮度自动选择文本颜色（黑色或白色）
		float brightness = (r * 0.299f + g * 0.587f + b * 0.114f) / 255.0f;
		COLORREF text_color = (brightness > 0.6f) ? RGB(0, 0, 0) : RGB(255, 255, 255);
		HRESULT hr_text = DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &text_color, sizeof(text_color));
		// 如果同步边框颜色，则设置边框颜色 (与标题栏颜色相同)
		if (sync_border) {
			COLORREF border_color = RGB(r, g, b);
			DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &border_color, sizeof(border_color));
		}
#else
		UtilityFunctions::push_error("Title bar color modification is only supported on Windows");
#endif
	}

	void DWM::set_border_color(Window* target_window, const Color& color) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		if (hwnd == nullptr) return;
		// 将Color转换为COLORREF ([0.0, 1.0]范围转换为[0, 255]范围)
		int r = CLAMP(color.r * 255, 0, 255);
		int g = CLAMP(color.g * 255, 0, 255);
		int b = CLAMP(color.b * 255, 0, 255);
		// 设置边框颜色
		COLORREF border_color = RGB(r, g, b);
		DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &border_color, sizeof(border_color));
#endif
	}

	void DWM::set_title_bar_text_color(Window* target_window, const Color& color) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		if (hwnd == nullptr) return;

		// 将Color转换为COLORREF ([0.0, 1.0]范围转换为[0, 255]范围)
		int r = CLAMP(color.r * 255, 0, 255);
		int g = CLAMP(color.g * 255, 0, 255);
		int b = CLAMP(color.b * 255, 0, 255);

		// 设置标题栏文本颜色
		COLORREF text_color = RGB(r, g, b);
		DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &text_color, sizeof(text_color));
#endif
	}

	Color DWM::get_title_bar_color(Window* target_window) {
#ifdef _WIN32
		HWND hwnd = DWM::_get_godot_window_handle(target_window);
		if (hwnd == nullptr) return Color(1, 1, 1, 1); // 默认白色
		COLORREF caption_color = 0;
		HRESULT hr = DwmGetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &caption_color, sizeof(caption_color));
		if (SUCCEEDED(hr)) {
			// 提取RGB分量
			uint8_t r = (caption_color >> 16) & 0xFF;
			uint8_t g = (caption_color >> 8) & 0xFF;
			uint8_t b = caption_color & 0xFF;
			// 转换为Godot的Color格式 ([0, 255]范围转换为[0.0, 1.0]范围)
			return Color(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
		}
		else {
			UtilityFunctions::push_error("Failed to get caption color - feature may not be supported");
			return Color(1, 1, 1, 1); // 默认白色
		}
#endif
	}

	Color DWM::get_title_bar_text_color(Window* target_window) {
#ifdef _WIN32
		HWND hwnd = DWM::_get_godot_window_handle(target_window);
		if (hwnd == nullptr) return Color(1, 1, 1, 1); // 默认白色
		COLORREF text_color = 0;
		HRESULT hr = DwmGetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &text_color, sizeof(text_color));

		if (SUCCEEDED(hr)) {
			// 提取RGB分量
			uint8_t r = (text_color >> 16) & 0xFF;
			uint8_t g = (text_color >> 8) & 0xFF;
			uint8_t b = text_color & 0xFF;
			// 转换为Godot的Color格式 ([0, 255]范围转换为[0.0, 1.0]范围)
			return Color(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
		}
		else {
			UtilityFunctions::push_error("Failed to get text color - feature may not be supported");
			return Color(1, 1, 1, 1); // 默认白色
		}
#endif
	}

	Color DWM::get_border_color(Window* target_window) {
#ifdef _WIN32
		HWND hwnd = DWM::_get_godot_window_handle(target_window);
		if (hwnd == nullptr) return Color(1, 1, 1, 1);	// 默认白色
		COLORREF border_color = 0;
		HRESULT hr = DwmGetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &border_color, sizeof(border_color));
		if (SUCCEEDED(hr)) {
			// 提取RGB分量
			uint8_t r = (border_color >> 16) & 0xFF;
			uint8_t g = (border_color >> 8) & 0xFF;
			uint8_t b = border_color & 0xFF;
			// 转换为Godot的Color格式 ([0, 255]范围转换为[0.0, 1.0]范围)
			return Color(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
		}
		else {
			UtilityFunctions::push_error("Failed to get border color - feature may not be supported");
			return Color(1, 1, 1, 1); // 默认白色
		}
#endif
	}

	void DWM::reset_to_default_colors(Window* target_window) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		if (hwnd == nullptr) return;
		COLORREF default_color = 0xFFFFFFFF;
		DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &default_color, sizeof(default_color));
		DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &default_color, sizeof(default_color));
		DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &default_color, sizeof(default_color));
		BOOL system_dark_mode = FALSE;
		DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &system_dark_mode, sizeof(system_dark_mode));
#else
		UtilityFunctions::push_error("Not supported on this platform");
#endif
	}

	bool DWM::is_title_bar_coloring_supported() {
#ifdef _WIN32
		OSVERSIONINFOEXW osvi = { sizeof(osvi) };
		DWORDLONG condition_mask = 0;
		VER_SET_CONDITION(condition_mask, VER_MAJORVERSION, VER_GREATER_EQUAL);
		VER_SET_CONDITION(condition_mask, VER_MINORVERSION, VER_GREATER_EQUAL);
		VER_SET_CONDITION(condition_mask, VER_BUILDNUMBER, VER_GREATER_EQUAL);
		// Windows 11 的内核版本依然是 10.0，但构建号起始于 22000
		osvi.dwMajorVersion = 10;
		osvi.dwMinorVersion = 0;
		osvi.dwBuildNumber = 22000;
		return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, condition_mask) != FALSE;
#else
		return false;
#endif
	}

	void DWM::enable_dark_mode(Window* target_window, bool enable) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		if (hwnd == nullptr) return;
		BOOL dark_mode = enable ? TRUE : FALSE;
		HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark_mode, sizeof(dark_mode));
		if (FAILED(hr)) UtilityFunctions::push_error("Failed to set dark mode - feature may not be supported");
#endif
	}

	void DWM::extend_frame_into_client_area(Window* target_window, int left, int right, int top, int bottom) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		if (hwnd == nullptr) return;
		MARGINS margins;
		margins.cxLeftWidth = left;
		margins.cxRightWidth = right;
		margins.cyTopHeight = top;
		margins.cyBottomHeight = bottom;
		HRESULT hr = DwmExtendFrameIntoClientArea(hwnd, &margins);
		if (FAILED(hr)) UtilityFunctions::push_error("Failed to extend frame into client area - feature may not be supported");
#endif
	}

	Color DWM::get_colorization_color() {
#ifdef _WIN32
		DWORD color = 0;
		BOOL opaque = FALSE;

		HRESULT hr = DwmGetColorizationColor(&color, &opaque);
		if (SUCCEEDED(hr)) {
			// 提取RGB分量
			uint8_t r = (color >> 16) & 0xFF;
			uint8_t g = (color >> 8) & 0xFF;
			uint8_t b = color & 0xFF;
			// 转换为[0.0, 1.0]范围的Color
			return Color(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
		} else {
			UtilityFunctions::push_error("Failed to get colorization color - feature may not be supported");
			return Color(1.0f, 1.0f, 1.0f, 1.0f); // 默认白色
		}
#else
		return Color(1.0f, 1.0f, 1.0f, 1.0f); // 默认白色
#endif
	}

	Dictionary DWM::get_composition_timing_info(Window* target_window) {
#ifdef _WIN32
		HWND hwnd = GetAncestor(_get_godot_window_handle(target_window), GA_ROOT);	// 获取顶级窗口句柄
		if (!IsWindow(hwnd)) return Dictionary();

		DWM_TIMING_INFO timing_info = {};
		timing_info.cbSize = sizeof(DWM_TIMING_INFO);
		DWM_TIMING_INFO* p_timing_info = &timing_info;

		HRESULT hr = DwmGetCompositionTimingInfo(hwnd, p_timing_info);
		if (SUCCEEDED(hr)) {
			Dictionary result;
			result["rate_refresh"] = Vector2i(static_cast<int>(p_timing_info->rateCompose.uiNumerator), static_cast<int>(p_timing_info->rateCompose.uiDenominator)); // 监视器刷新率
			result["qpc_refresh_period"] = Variant(p_timing_info->qpcRefreshPeriod);					// 监视器刷新期
			result["rate_compose"] = Vector2i(static_cast<int>(p_timing_info->rateCompose.uiNumerator), static_cast<int>(p_timing_info->rateCompose.uiDenominator)); // 组合速率
			result["qpc_vblank"] = Variant(p_timing_info->qpcVBlank);									// 垂直空白前的查询性能计数器值
			result["composition_refresh"] = Variant(p_timing_info->cRefresh);							// DWM 刷新计数器
			result["composition_dx_refresh"] = Variant(p_timing_info->cDXRefresh);						// DirectX 刷新计数器
			result["qpc_compose"] = Variant(p_timing_info->qpcCompose);									// 在 qpcCompose 中构成的帧编号
			result["composition_dx_present"] = Variant(p_timing_info->cDXPresent);						// 用于标识呈现帧的 DirectX 当前编号
			result["composition_refresh_frame"] = Variant(p_timing_info->cRefreshFrame);				// 在 qpcCompose 中撰写的帧的刷新计数
			result["composition_frame_submitted"] = Variant(p_timing_info->cFrameSubmitted);			// 上次提交的 DWM 帧编号
			result["composition_dx_present_submitted"] = Variant(p_timing_info->cDXPresentSubmitted);	// 上次提交的 DirectX 当前编号
			result["composition_frame_confirmed"] = Variant(p_timing_info->cFrameConfirmed);			// 上次确认为显示的 DWM 帧编号
			result["composition_dx_present_confirmed"] = Variant(p_timing_info->cDXPresentConfirmed);	// 上次确认的 DirectX 当前编号
			result["composition_refresh_confirmed"] = Variant(p_timing_info->cRefreshConfirmed);		// GPU 确认为已完成的最后一帧的目标刷新计数
			result["presentation_dx_refresh_confirmed"] = Variant(p_timing_info->cDXRefreshConfirmed);	// 确认帧显示的 DirectX 刷新计数
			result["composition_frames_late"] = Variant(p_timing_info->cFramesLate);					// DWM 延迟显示的帧数
			result["composition_frames_out_standing"] = Variant(p_timing_info->cFramesOutstanding);		// 已发出但尚未确认为已完成的合成帧数
			result["composition_frame_displayed"] = Variant(p_timing_info->cFramesDisplayed);			// 显示的最后一帧
			result["qpc_frame_displayed"] = Variant(p_timing_info->qpcFrameDisplayed);					// 显示帧时合成的 QPC 时间通过
			result["composition_refresh_frame_displayed"] = Variant(p_timing_info->cRefreshFrameDisplayed); // 帧应变为可见时的垂直刷新计数
			result["composition_frame_complete"] = Variant(p_timing_info->cFrameComplete);				// 最后一个帧标记为已完成的 ID
			result["qpc_frame_complete"] = Variant(p_timing_info->qpcFrameComplete);					// 最后一个帧标记为已完成的 QPC 时间
			result["composition_frame_pending"] = Variant(p_timing_info->cFramePending);				// 最后一个帧标记为挂起的 ID
			result["qpc_frame_pending"] = Variant(p_timing_info->qpcFramePending);						// 最后一个帧标记为挂起的 QPC 时间
			result["composition_frames_displayed"] = Variant(p_timing_info->cFramesDisplayed);			// 显示的唯一帧数，此值仅在第二次调用 DwmGetCompositionTimingInfo 函数后有效
			result["composition_frames_complete"] = Variant(p_timing_info->cFramesComplete);			// 已接收的新已完成帧数
			result["composition_frames_pending"] = Variant(p_timing_info->cFramesPending);				// 提交到 DirectX 但尚未完成的新帧数
			result["composition_frames_available"] = Variant(p_timing_info->cFramesAvailable);			// 可用但未显示、已使用或已删除的帧数，此值仅在第二次调用 DwmGetCompositionTimingInfo 后有效
			result["composition_frames_dropped"] = Variant(p_timing_info->cFramesDropped);				// 由于合成发生太晚而从未显示的渲染帧数，此值仅在第二次调用 DwmGetCompositionTimingInfo 后有效
			result["composition_frames_missed"] = Variant(p_timing_info->cFramesMissed);				// 本应使用新帧但不可用时，旧框架被组合的次数
			result["composition_refresh_next_displayed"] = Variant(p_timing_info->cRefreshNextDisplayed); // 计划显示下一帧的帧计数
			result["composition_refresh_next_presented"] = Variant(p_timing_info->cRefreshNextPresented); // 计划显示下一个 DirectX 的帧计数
			result["composition_refreshes_displayed"] = Variant(p_timing_info->cRefreshesDisplayed);	// 自上次调用 DwmSetPresentParameters 函数以来为应用程序显示的刷新总数
			result["composition_refreshes_presented"] = Variant(p_timing_info->cRefreshesPresented);	// 自上次调用 DwmSetPresentParameters 以来，应用程序提供的刷新总数
			result["composition_refresh_started"] = Variant(p_timing_info->cRefreshStarted);			// 开始显示此窗口内容时的刷新数
			result["composition_pixels_received"] = Variant(p_timing_info->cPixelsReceived);			// DirectX 重定向到 DWM 的像素总数
			result["composition_pixels_drawn"] = Variant(p_timing_info->cPixelsDrawn);					// 绘制的像素数
			result["composition_buffers_empty"] = Variant(p_timing_info->cBuffersEmpty);				// 翻转链中的空缓冲区数
			return result;
		}
		else if (FAILED(hr)) {
			String error_msg = "DwmGetCompositionTimingInfo failed with HRESULT: 0x";
			error_msg += String::num_uint64(static_cast<uint32_t>(hr), 16);
			switch (hr) {
			case DWM_E_COMPOSITIONDISABLED:
				error_msg += " (DWM_E_COMPOSITIONDISABLED) - DWM composition is disabled";
				break;
			case E_HANDLE:
				error_msg += " (E_HANDLE) - Invalid handle";
				break;
			case E_INVALIDARG:
				error_msg += " (E_INVALIDARG) - Invalid Args";
				break;
			default:
				error_msg += "";
			}
			UtilityFunctions::push_error(error_msg);
			return Dictionary();
		}
		else {
			UtilityFunctions::push_error("Failed to get composition timing info - feature may not be supported");
			return Dictionary();
		}
#else
		return Dictionary();
#endif
	}

	void DWM::enable_hostbackdropbrush(Window * target_window, bool enable) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		if (hwnd == nullptr) return;
		BOOL hostbackdropbrush = enable ? TRUE : FALSE;
		HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_USE_HOSTBACKDROPBRUSH, &hostbackdropbrush, sizeof(hostbackdropbrush));
		if (FAILED(hr)) UtilityFunctions::push_error("Failed to use hostbackdropbrush");
#endif // _WIN32
	}

	void DWM::set_systembackdrop_type(Window* target_window, SystembackdropType p_type) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		if (hwnd == nullptr) return;
		DWM_SYSTEMBACKDROP_TYPE type = DWMSBT_AUTO;
		switch (p_type) {
		case AUTO: type = DWMSBT_AUTO; break;
		case NONE: type = DWMSBT_NONE; break;
		case MAINWINDOW: type = DWMSBT_MAINWINDOW; break;
		case TRANSIENTWINDOW: type = DWMSBT_TRANSIENTWINDOW; break;
		case TABBEDWINDOW: type = DWMSBT_TABBEDWINDOW; break;
		}
		HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &type, sizeof(type));
		if (FAILED(hr)) UtilityFunctions::push_error("Failed to set systembackdrop type");
#endif // _WIN32
	}

	void DWM::enable_mmcss(bool enable) {
#ifdef _WIN32
		BOOL mmcss = enable ? TRUE : FALSE;
		HRESULT hr = DwmEnableMMCSS(mmcss);
		if (FAILED(hr)) UtilityFunctions::push_error("Failed to enable MMCSS");
#endif // _WIN32
	}
}
