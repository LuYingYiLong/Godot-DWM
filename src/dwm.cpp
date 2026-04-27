#include "dwm.h"

#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#ifdef _WIN32
#include <algorithm>
#include <unordered_map>
#endif

namespace godot {

#ifdef _WIN32
	namespace {
		constexpr DWORD COLOR_DEFAULT = 0xFFFFFFFF;

		std::unordered_map<uint64_t, DWORD> attribute_cache;

		uint64_t _attribute_cache_key(HWND hwnd, DWORD attribute) {
			const uint64_t handle = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(hwnd));
			return (handle * 1315423911ull) ^ static_cast<uint64_t>(attribute);
		}

		void _cache_attribute(HWND hwnd, DWORD attribute, DWORD value) {
			attribute_cache[_attribute_cache_key(hwnd, attribute)] = value;
		}

		bool _get_cached_attribute(HWND hwnd, DWORD attribute, DWORD& value) {
			auto found = attribute_cache.find(_attribute_cache_key(hwnd, attribute));
			if (found == attribute_cache.end()) {
				return false;
			}
			value = found->second;
			return true;
		}

		void _erase_cached_attribute(HWND hwnd, DWORD attribute) {
			attribute_cache.erase(_attribute_cache_key(hwnd, attribute));
		}

		String _hresult_to_string(HRESULT hr) {
			return String("0x") + String::num_uint64(static_cast<uint32_t>(hr), 16);
		}

		void _push_hresult_error(const char* operation, HRESULT hr) {
			UtilityFunctions::push_error(String(operation) + " failed with HRESULT " + _hresult_to_string(hr));
		}

		bool _set_dwm_attribute(HWND hwnd, DWORD attribute, const void* value, DWORD size, const char* operation) {
			HRESULT hr = DwmSetWindowAttribute(hwnd, attribute, value, size);
			if (FAILED(hr)) {
				_push_hresult_error(operation, hr);
				return false;
			}
			return true;
		}

		bool _get_dwm_attribute(HWND hwnd, DWORD attribute, void* value, DWORD size, const char* operation, bool report_error = true) {
			HRESULT hr = DwmGetWindowAttribute(hwnd, attribute, value, size);
			if (FAILED(hr)) {
				if (report_error) {
					_push_hresult_error(operation, hr);
				}
				return false;
			}
			return true;
		}

		BYTE _color_component_to_byte(float value) {
			return static_cast<BYTE>(std::clamp(value, 0.0f, 1.0f) * 255.0f + 0.5f);
		}

		COLORREF _color_to_colorref(const Color& color) {
			return RGB(_color_component_to_byte(color.r), _color_component_to_byte(color.g), _color_component_to_byte(color.b));
		}

		Color _colorref_to_color(COLORREF color) {
			return Color(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f, 1.0f);
		}

		Rect2i _rect_to_rect2i(const RECT& rect) {
			return Rect2i(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
		}

		bool _is_windows_build_or_newer(DWORD major, DWORD minor, DWORD build) {
			using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
			HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
			if (!ntdll) {
				return false;
			}

			auto rtl_get_version = reinterpret_cast<RtlGetVersionFn>(GetProcAddress(ntdll, "RtlGetVersion"));
			if (!rtl_get_version) {
				return false;
			}

			OSVERSIONINFOW version = {};
			version.dwOSVersionInfoSize = sizeof(version);
			if (rtl_get_version(&version) != 0) {
				return false;
			}

			if (version.dwMajorVersion != major) {
				return version.dwMajorVersion > major;
			}
			if (version.dwMinorVersion != minor) {
				return version.dwMinorVersion > minor;
			}
			return version.dwBuildNumber >= build;
		}
	}
#endif

	void DWM::_bind_methods() {
		BIND_ENUM_CONSTANT(AUTO);
		BIND_ENUM_CONSTANT(NONE);
		BIND_ENUM_CONSTANT(MAINWINDOW);
		BIND_ENUM_CONSTANT(TRANSIENTWINDOW);
		BIND_ENUM_CONSTANT(TABBEDWINDOW);
		BIND_ENUM_CONSTANT(SYSTEMBACKDROP_TYPE_AUTO);
		BIND_ENUM_CONSTANT(SYSTEMBACKDROP_TYPE_NONE);
		BIND_ENUM_CONSTANT(SYSTEMBACKDROP_TYPE_MAINWINDOW);
		BIND_ENUM_CONSTANT(SYSTEMBACKDROP_TYPE_TRANSIENTWINDOW);
		BIND_ENUM_CONSTANT(SYSTEMBACKDROP_TYPE_TABBEDWINDOW);

		BIND_ENUM_CONSTANT(CORNER_DEFAULT);
		BIND_ENUM_CONSTANT(CORNER_DO_NOT_ROUND);
		BIND_ENUM_CONSTANT(CORNER_ROUND);
		BIND_ENUM_CONSTANT(CORNER_ROUND_SMALL);

		BIND_ENUM_CONSTANT(NCRENDERING_USE_WINDOW_STYLE);
		BIND_ENUM_CONSTANT(NCRENDERING_DISABLED);
		BIND_ENUM_CONSTANT(NCRENDERING_ENABLED);

		BIND_ENUM_CONSTANT(FLIP3D_DEFAULT);
		BIND_ENUM_CONSTANT(FLIP3D_EXCLUDE_BELOW);
		BIND_ENUM_CONSTANT(FLIP3D_EXCLUDE_ABOVE);

		BIND_ENUM_CONSTANT(ATTR_NCRENDERING_ENABLED);
		BIND_ENUM_CONSTANT(ATTR_NCRENDERING_POLICY);
		BIND_ENUM_CONSTANT(ATTR_TRANSITIONS_FORCEDISABLED);
		BIND_ENUM_CONSTANT(ATTR_ALLOW_NCPAINT);
		BIND_ENUM_CONSTANT(ATTR_CAPTION_BUTTON_BOUNDS);
		BIND_ENUM_CONSTANT(ATTR_NONCLIENT_RTL_LAYOUT);
		BIND_ENUM_CONSTANT(ATTR_FORCE_ICONIC_REPRESENTATION);
		BIND_ENUM_CONSTANT(ATTR_FLIP3D_POLICY);
		BIND_ENUM_CONSTANT(ATTR_EXTENDED_FRAME_BOUNDS);
		BIND_ENUM_CONSTANT(ATTR_HAS_ICONIC_BITMAP);
		BIND_ENUM_CONSTANT(ATTR_DISALLOW_PEEK);
		BIND_ENUM_CONSTANT(ATTR_EXCLUDED_FROM_PEEK);
		BIND_ENUM_CONSTANT(ATTR_CLOAK);
		BIND_ENUM_CONSTANT(ATTR_CLOAKED);
		BIND_ENUM_CONSTANT(ATTR_FREEZE_REPRESENTATION);
		BIND_ENUM_CONSTANT(ATTR_PASSIVE_UPDATE_MODE);
		BIND_ENUM_CONSTANT(ATTR_USE_HOSTBACKDROPBRUSH);
		BIND_ENUM_CONSTANT(ATTR_USE_IMMERSIVE_DARK_MODE);
		BIND_ENUM_CONSTANT(ATTR_WINDOW_CORNER_PREFERENCE);
		BIND_ENUM_CONSTANT(ATTR_BORDER_COLOR);
		BIND_ENUM_CONSTANT(ATTR_CAPTION_COLOR);
		BIND_ENUM_CONSTANT(ATTR_TEXT_COLOR);
		BIND_ENUM_CONSTANT(ATTR_VISIBLE_FRAME_BORDER_THICKNESS);
		BIND_ENUM_CONSTANT(ATTR_SYSTEMBACKDROP_TYPE);

		ClassDB::bind_static_method("DWM", D_METHOD("is_composition_enabled"), &DWM::is_composition_enabled);
		ClassDB::bind_static_method("DWM", D_METHOD("is_title_bar_coloring_supported"), &DWM::is_title_bar_coloring_supported);
		ClassDB::bind_static_method("DWM", D_METHOD("is_window_valid", "target_window"), &DWM::is_window_valid);
		ClassDB::bind_static_method("DWM", D_METHOD("get_native_window_handle", "target_window"), &DWM::get_native_window_handle);

		ClassDB::bind_static_method("DWM", D_METHOD("set_title_bar_color", "target_window", "color", "sync_border"), &DWM::set_title_bar_color, DEFVAL(true));
		ClassDB::bind_static_method("DWM", D_METHOD("set_border_color", "target_window", "color"), &DWM::set_border_color);
		ClassDB::bind_static_method("DWM", D_METHOD("set_title_bar_text_color", "target_window", "color"), &DWM::set_title_bar_text_color);
		ClassDB::bind_static_method("DWM", D_METHOD("get_title_bar_color", "target_window"), &DWM::get_title_bar_color);
		ClassDB::bind_static_method("DWM", D_METHOD("get_border_color", "target_window"), &DWM::get_border_color);
		ClassDB::bind_static_method("DWM", D_METHOD("get_title_bar_text_color", "target_window"), &DWM::get_title_bar_text_color);
		ClassDB::bind_static_method("DWM", D_METHOD("reset_to_default_colors", "target_window"), &DWM::reset_to_default_colors);

		ClassDB::bind_static_method("DWM", D_METHOD("enable_dark_mode", "target_window", "enable"), &DWM::enable_dark_mode);
		ClassDB::bind_static_method("DWM", D_METHOD("extend_frame_into_client_area", "target_window", "left", "right", "top", "bottom"), &DWM::extend_frame_into_client_area);
		ClassDB::bind_static_method("DWM", D_METHOD("get_colorization_color"), &DWM::get_colorization_color);
		ClassDB::bind_static_method("DWM", D_METHOD("get_composition_timing_info", "target_window"), &DWM::get_composition_timing_info);
		ClassDB::bind_static_method("DWM", D_METHOD("enable_hostbackdropbrush", "target_window", "enable"), &DWM::enable_hostbackdropbrush);
		ClassDB::bind_static_method("DWM", D_METHOD("set_systembackdrop_type", "target_window", "type"), &DWM::set_systembackdrop_type, DEFVAL(AUTO));
		ClassDB::bind_static_method("DWM", D_METHOD("enable_mmcss", "enable"), &DWM::enable_mmcss);

		ClassDB::bind_static_method("DWM", D_METHOD("set_window_corner_preference", "target_window", "preference"), &DWM::set_window_corner_preference);
		ClassDB::bind_static_method("DWM", D_METHOD("get_window_corner_preference", "target_window"), &DWM::get_window_corner_preference);
		ClassDB::bind_static_method("DWM", D_METHOD("set_non_client_rendering_policy", "target_window", "policy"), &DWM::set_non_client_rendering_policy);
		ClassDB::bind_static_method("DWM", D_METHOD("set_flip3d_policy", "target_window", "policy"), &DWM::set_flip3d_policy);
		ClassDB::bind_static_method("DWM", D_METHOD("force_disable_transitions", "target_window", "disable"), &DWM::force_disable_transitions);
		ClassDB::bind_static_method("DWM", D_METHOD("set_peek_disallowed", "target_window", "disallow"), &DWM::set_peek_disallowed);
		ClassDB::bind_static_method("DWM", D_METHOD("set_excluded_from_peek", "target_window", "exclude"), &DWM::set_excluded_from_peek);
		ClassDB::bind_static_method("DWM", D_METHOD("get_visible_frame_border_thickness", "target_window"), &DWM::get_visible_frame_border_thickness);
		ClassDB::bind_static_method("DWM", D_METHOD("get_window_bounds", "target_window", "extended_frame"), &DWM::get_window_bounds, DEFVAL(true));
		ClassDB::bind_static_method("DWM", D_METHOD("get_caption_button_bounds", "target_window"), &DWM::get_caption_button_bounds);

		ClassDB::bind_static_method("DWM", D_METHOD("set_window_attribute_int", "target_window", "attribute", "value"), &DWM::set_window_attribute_int);
		ClassDB::bind_static_method("DWM", D_METHOD("get_window_attribute_int", "target_window", "attribute"), &DWM::get_window_attribute_int);
		ClassDB::bind_static_method("DWM", D_METHOD("set_window_attribute_bool", "target_window", "attribute", "value"), &DWM::set_window_attribute_bool);
		ClassDB::bind_static_method("DWM", D_METHOD("get_window_attribute_bool", "target_window", "attribute"), &DWM::get_window_attribute_bool);
		ClassDB::bind_static_method("DWM", D_METHOD("set_window_attribute_color", "target_window", "attribute", "color"), &DWM::set_window_attribute_color);
		ClassDB::bind_static_method("DWM", D_METHOD("get_window_attribute_color", "target_window", "attribute"), &DWM::get_window_attribute_color);
		ClassDB::bind_static_method("DWM", D_METHOD("get_window_attribute_rect", "target_window", "attribute"), &DWM::get_window_attribute_rect);

		ClassDB::bind_static_method("DWM", D_METHOD("get_window_style", "target_window", "extended"), &DWM::get_window_style, DEFVAL(false));
		ClassDB::bind_static_method("DWM", D_METHOD("set_window_style", "target_window", "style", "extended", "apply_frame_change"), &DWM::set_window_style, DEFVAL(false), DEFVAL(true));
		ClassDB::bind_static_method("DWM", D_METHOD("set_window_style_bits", "target_window", "mask", "enabled", "extended", "apply_frame_change"), &DWM::set_window_style_bits, DEFVAL(false), DEFVAL(true));
	}

#ifdef _WIN32
	HWND DWM::_get_godot_window_handle(Window* target_window) {
		ERR_FAIL_NULL_V_MSG(target_window, nullptr, "Target window is null.");

		DisplayServer* display_server = DisplayServer::get_singleton();
		ERR_FAIL_NULL_V_MSG(display_server, nullptr, "DisplayServer singleton is not available.");

		const int64_t hwnd_int = display_server->window_get_native_handle(DisplayServer::WINDOW_HANDLE, target_window->get_window_id());
		ERR_FAIL_COND_V_MSG(hwnd_int == 0, nullptr, "Godot returned a null native window handle.");

		HWND hwnd = reinterpret_cast<HWND>(hwnd_int);
		ERR_FAIL_COND_V_MSG(!IsWindow(hwnd), nullptr, "The native handle is not a valid HWND.");
		return hwnd;
	}
#endif

	bool DWM::is_composition_enabled() {
#ifdef _WIN32
		BOOL enabled = FALSE;
		HRESULT hr = DwmIsCompositionEnabled(&enabled);
		if (FAILED(hr)) {
			_push_hresult_error("DwmIsCompositionEnabled", hr);
			return false;
		}
		return enabled != FALSE;
#else
		return false;
#endif
	}

	bool DWM::is_title_bar_coloring_supported() {
#ifdef _WIN32
		return _is_windows_build_or_newer(10, 0, 22000);
#else
		return false;
#endif
	}

	bool DWM::is_window_valid(Window* target_window) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		return hwnd && IsWindow(hwnd);
#else
		return false;
#endif
	}

	int64_t DWM::get_native_window_handle(Window* target_window) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		return reinterpret_cast<int64_t>(hwnd);
#else
		return 0;
#endif
	}

	void DWM::set_title_bar_color(Window* target_window, const Color& color, const bool sync_border) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		ERR_FAIL_COND_MSG(!hwnd, "HWND is invalid.");

		COLORREF caption_color = _color_to_colorref(color);
		if (_set_dwm_attribute(hwnd, DWMWA_CAPTION_COLOR, &caption_color, sizeof(caption_color), "DwmSetWindowAttribute(DWMWA_CAPTION_COLOR)")) {
			_cache_attribute(hwnd, DWMWA_CAPTION_COLOR, caption_color);
		}

		const float brightness = color.r * 0.299f + color.g * 0.587f + color.b * 0.114f;
		COLORREF text_color = brightness > 0.6f ? RGB(0, 0, 0) : RGB(255, 255, 255);
		if (_set_dwm_attribute(hwnd, DWMWA_TEXT_COLOR, &text_color, sizeof(text_color), "DwmSetWindowAttribute(DWMWA_TEXT_COLOR)")) {
			_cache_attribute(hwnd, DWMWA_TEXT_COLOR, text_color);
		}

		if (sync_border) {
			COLORREF border_color = caption_color;
			if (_set_dwm_attribute(hwnd, DWMWA_BORDER_COLOR, &border_color, sizeof(border_color), "DwmSetWindowAttribute(DWMWA_BORDER_COLOR)")) {
				_cache_attribute(hwnd, DWMWA_BORDER_COLOR, border_color);
			}
		}
#else
		ERR_FAIL_MSG("Title bar color modification is only supported on Windows.");
#endif
	}

	void DWM::set_border_color(Window* target_window, const Color& color) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		ERR_FAIL_COND_MSG(!hwnd, "HWND is invalid.");

		COLORREF border_color = _color_to_colorref(color);
		if (_set_dwm_attribute(hwnd, DWMWA_BORDER_COLOR, &border_color, sizeof(border_color), "DwmSetWindowAttribute(DWMWA_BORDER_COLOR)")) {
			_cache_attribute(hwnd, DWMWA_BORDER_COLOR, border_color);
		}
#else
		ERR_FAIL_MSG("Border color modification is only supported on Windows.");
#endif
	}

	void DWM::set_title_bar_text_color(Window* target_window, const Color& color) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		ERR_FAIL_COND_MSG(!hwnd, "HWND is invalid.");

		COLORREF text_color = _color_to_colorref(color);
		if (_set_dwm_attribute(hwnd, DWMWA_TEXT_COLOR, &text_color, sizeof(text_color), "DwmSetWindowAttribute(DWMWA_TEXT_COLOR)")) {
			_cache_attribute(hwnd, DWMWA_TEXT_COLOR, text_color);
		}
#else
		ERR_FAIL_MSG("Title bar text color modification is only supported on Windows.");
#endif
	}

	Color DWM::get_title_bar_color(Window* target_window) {
#ifdef _WIN32
		return get_window_attribute_color(target_window, ATTR_CAPTION_COLOR);
#else
		return Color();
#endif
	}

	Color DWM::get_border_color(Window* target_window) {
#ifdef _WIN32
		return get_window_attribute_color(target_window, ATTR_BORDER_COLOR);
#else
		return Color();
#endif
	}

	Color DWM::get_title_bar_text_color(Window* target_window) {
#ifdef _WIN32
		return get_window_attribute_color(target_window, ATTR_TEXT_COLOR);
#else
		return Color();
#endif
	}

	void DWM::reset_to_default_colors(Window* target_window) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		ERR_FAIL_COND_MSG(!hwnd, "HWND is invalid.");

		DWORD default_color = COLOR_DEFAULT;
		_set_dwm_attribute(hwnd, DWMWA_CAPTION_COLOR, &default_color, sizeof(default_color), "DwmSetWindowAttribute(DWMWA_CAPTION_COLOR)");
		_set_dwm_attribute(hwnd, DWMWA_TEXT_COLOR, &default_color, sizeof(default_color), "DwmSetWindowAttribute(DWMWA_TEXT_COLOR)");
		_set_dwm_attribute(hwnd, DWMWA_BORDER_COLOR, &default_color, sizeof(default_color), "DwmSetWindowAttribute(DWMWA_BORDER_COLOR)");
		_erase_cached_attribute(hwnd, DWMWA_CAPTION_COLOR);
		_erase_cached_attribute(hwnd, DWMWA_TEXT_COLOR);
		_erase_cached_attribute(hwnd, DWMWA_BORDER_COLOR);
#else
		ERR_FAIL_MSG("Color reset is only supported on Windows.");
#endif
	}

	void DWM::enable_dark_mode(Window* target_window, const bool enable) {
#ifdef _WIN32
		set_window_attribute_bool(target_window, ATTR_USE_IMMERSIVE_DARK_MODE, enable);
#else
		ERR_FAIL_MSG("Dark mode is only supported on Windows.");
#endif
	}

	void DWM::extend_frame_into_client_area(Window* target_window, const int left, const int right, const int top, const int bottom) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		ERR_FAIL_COND_MSG(!hwnd, "HWND is invalid.");

		MARGINS margins = {};
		margins.cxLeftWidth = left;
		margins.cxRightWidth = right;
		margins.cyTopHeight = top;
		margins.cyBottomHeight = bottom;

		HRESULT hr = DwmExtendFrameIntoClientArea(hwnd, &margins);
		if (FAILED(hr)) {
			_push_hresult_error("DwmExtendFrameIntoClientArea", hr);
		}
#else
		ERR_FAIL_MSG("Frame extension is only supported on Windows.");
#endif
	}

	Color DWM::get_colorization_color() {
#ifdef _WIN32
		DWORD color = 0;
		BOOL opaque = FALSE;

		HRESULT hr = DwmGetColorizationColor(&color, &opaque);
		ERR_FAIL_COND_V_MSG(FAILED(hr), Color(), "Failed to get DWM colorization color.");

		const uint8_t a = opaque ? 255 : static_cast<uint8_t>((color >> 24) & 0xFF);
		const uint8_t r = static_cast<uint8_t>((color >> 16) & 0xFF);
		const uint8_t g = static_cast<uint8_t>((color >> 8) & 0xFF);
		const uint8_t b = static_cast<uint8_t>(color & 0xFF);
		return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
#else
		return Color();
#endif
	}

	Dictionary DWM::get_composition_timing_info(Window* target_window) {
#ifdef _WIN32
		HWND hwnd = GetAncestor(_get_godot_window_handle(target_window), GA_ROOT);
		ERR_FAIL_COND_V_MSG(!hwnd || !IsWindow(hwnd), Dictionary(), "HWND is invalid.");

		DWM_TIMING_INFO timing_info = {};
		timing_info.cbSize = sizeof(DWM_TIMING_INFO);

		HRESULT hr = DwmGetCompositionTimingInfo(hwnd, &timing_info);
		if (FAILED(hr)) {
			String message = "DwmGetCompositionTimingInfo failed with HRESULT " + _hresult_to_string(hr);
			if (hr == DWM_E_COMPOSITIONDISABLED) {
				message += " (DWM composition is disabled)";
			}
			else if (hr == E_HANDLE) {
				message += " (invalid handle)";
			}
			else if (hr == E_INVALIDARG) {
				message += " (invalid argument)";
			}
			UtilityFunctions::push_warning(message);
			return Dictionary();
		}

		Dictionary result;
		result["rate_refresh"] = Vector2i(static_cast<int>(timing_info.rateRefresh.uiNumerator), static_cast<int>(timing_info.rateRefresh.uiDenominator));
		result["rate_compose"] = Vector2i(static_cast<int>(timing_info.rateCompose.uiNumerator), static_cast<int>(timing_info.rateCompose.uiDenominator));
		result["qpc_refresh_period"] = static_cast<int64_t>(timing_info.qpcRefreshPeriod);
		result["qpc_vblank"] = static_cast<int64_t>(timing_info.qpcVBlank);
		result["composition_refresh"] = static_cast<int64_t>(timing_info.cRefresh);
		result["composition_dx_refresh"] = static_cast<int64_t>(timing_info.cDXRefresh);
		result["qpc_compose"] = static_cast<int64_t>(timing_info.qpcCompose);
		result["composition_dx_present"] = static_cast<int64_t>(timing_info.cDXPresent);
		result["composition_refresh_frame"] = static_cast<int64_t>(timing_info.cRefreshFrame);
		result["composition_frame_submitted"] = static_cast<int64_t>(timing_info.cFrameSubmitted);
		result["composition_dx_present_submitted"] = static_cast<int64_t>(timing_info.cDXPresentSubmitted);
		result["composition_frame_confirmed"] = static_cast<int64_t>(timing_info.cFrameConfirmed);
		result["composition_dx_present_confirmed"] = static_cast<int64_t>(timing_info.cDXPresentConfirmed);
		result["composition_refresh_confirmed"] = static_cast<int64_t>(timing_info.cRefreshConfirmed);
		result["presentation_dx_refresh_confirmed"] = static_cast<int64_t>(timing_info.cDXRefreshConfirmed);
		result["composition_frames_late"] = static_cast<int64_t>(timing_info.cFramesLate);
		result["composition_frames_outstanding"] = static_cast<int64_t>(timing_info.cFramesOutstanding);
		result["composition_frames_displayed"] = static_cast<int64_t>(timing_info.cFramesDisplayed);
		result["qpc_frame_displayed"] = static_cast<int64_t>(timing_info.qpcFrameDisplayed);
		result["composition_refresh_frame_displayed"] = static_cast<int64_t>(timing_info.cRefreshFrameDisplayed);
		result["composition_frame_complete"] = static_cast<int64_t>(timing_info.cFrameComplete);
		result["qpc_frame_complete"] = static_cast<int64_t>(timing_info.qpcFrameComplete);
		result["composition_frame_pending"] = static_cast<int64_t>(timing_info.cFramePending);
		result["qpc_frame_pending"] = static_cast<int64_t>(timing_info.qpcFramePending);
		result["composition_frames_complete"] = static_cast<int64_t>(timing_info.cFramesComplete);
		result["composition_frames_pending"] = static_cast<int64_t>(timing_info.cFramesPending);
		result["composition_frames_available"] = static_cast<int64_t>(timing_info.cFramesAvailable);
		result["composition_frames_dropped"] = static_cast<int64_t>(timing_info.cFramesDropped);
		result["composition_frames_missed"] = static_cast<int64_t>(timing_info.cFramesMissed);
		result["composition_refresh_next_displayed"] = static_cast<int64_t>(timing_info.cRefreshNextDisplayed);
		result["composition_refresh_next_presented"] = static_cast<int64_t>(timing_info.cRefreshNextPresented);
		result["composition_refreshes_displayed"] = static_cast<int64_t>(timing_info.cRefreshesDisplayed);
		result["composition_refreshes_presented"] = static_cast<int64_t>(timing_info.cRefreshesPresented);
		result["composition_refresh_started"] = static_cast<int64_t>(timing_info.cRefreshStarted);
		result["composition_pixels_received"] = static_cast<int64_t>(timing_info.cPixelsReceived);
		result["composition_pixels_drawn"] = static_cast<int64_t>(timing_info.cPixelsDrawn);
		result["composition_buffers_empty"] = static_cast<int64_t>(timing_info.cBuffersEmpty);
		return result;
#else
		return Dictionary();
#endif
	}

	void DWM::enable_hostbackdropbrush(Window* target_window, const bool enable) {
#ifdef _WIN32
		set_window_attribute_bool(target_window, ATTR_USE_HOSTBACKDROPBRUSH, enable);
#else
		ERR_FAIL_MSG("Host backdrop brush is only supported on Windows.");
#endif
	}

	void DWM::set_systembackdrop_type(Window* target_window, SystembackdropType p_type) {
#ifdef _WIN32
		set_window_attribute_int(target_window, ATTR_SYSTEMBACKDROP_TYPE, static_cast<int>(p_type));
#else
		ERR_FAIL_MSG("System backdrop type is only supported on Windows.");
#endif
	}

	void DWM::enable_mmcss(bool enable) {
#ifdef _WIN32
		BOOL mmcss = enable ? TRUE : FALSE;
		HRESULT hr = DwmEnableMMCSS(mmcss);
		if (FAILED(hr)) {
			_push_hresult_error("DwmEnableMMCSS", hr);
		}
#else
		ERR_FAIL_MSG("MMCSS is only supported on Windows.");
#endif
	}

	bool DWM::set_window_corner_preference(Window* target_window, WindowCornerPreference preference) {
		return set_window_attribute_int(target_window, ATTR_WINDOW_CORNER_PREFERENCE, static_cast<int>(preference));
	}

	int DWM::get_window_corner_preference(Window* target_window) {
		return get_window_attribute_int(target_window, ATTR_WINDOW_CORNER_PREFERENCE);
	}

	bool DWM::set_non_client_rendering_policy(Window* target_window, NonClientRenderingPolicy policy) {
		return set_window_attribute_int(target_window, ATTR_NCRENDERING_POLICY, static_cast<int>(policy));
	}

	bool DWM::set_flip3d_policy(Window* target_window, Flip3DPolicy policy) {
		return set_window_attribute_int(target_window, ATTR_FLIP3D_POLICY, static_cast<int>(policy));
	}

	bool DWM::force_disable_transitions(Window* target_window, bool disable) {
		return set_window_attribute_bool(target_window, ATTR_TRANSITIONS_FORCEDISABLED, disable);
	}

	bool DWM::set_peek_disallowed(Window* target_window, bool disallow) {
		return set_window_attribute_bool(target_window, ATTR_DISALLOW_PEEK, disallow);
	}

	bool DWM::set_excluded_from_peek(Window* target_window, bool exclude) {
		return set_window_attribute_bool(target_window, ATTR_EXCLUDED_FROM_PEEK, exclude);
	}

	int DWM::get_visible_frame_border_thickness(Window* target_window) {
		return get_window_attribute_int(target_window, ATTR_VISIBLE_FRAME_BORDER_THICKNESS);
	}

	Rect2i DWM::get_window_bounds(Window* target_window, bool extended_frame) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		ERR_FAIL_COND_V_MSG(!hwnd, Rect2i(), "HWND is invalid.");

		RECT rect = {};
		if (extended_frame) {
			if (!_get_dwm_attribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect), "DwmGetWindowAttribute(DWMWA_EXTENDED_FRAME_BOUNDS)")) {
				return Rect2i();
			}
		}
		else if (!GetWindowRect(hwnd, &rect)) {
			UtilityFunctions::push_error("GetWindowRect failed.");
			return Rect2i();
		}
		return _rect_to_rect2i(rect);
#else
		return Rect2i();
#endif
	}

	Rect2i DWM::get_caption_button_bounds(Window* target_window) {
		return get_window_attribute_rect(target_window, ATTR_CAPTION_BUTTON_BOUNDS);
	}

	bool DWM::set_window_attribute_int(Window* target_window, WindowAttribute attribute, int value) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		ERR_FAIL_COND_V_MSG(!hwnd, false, "HWND is invalid.");

		DWORD raw_value = static_cast<DWORD>(value);
		const DWORD raw_attribute = static_cast<DWORD>(attribute);
		if (!_set_dwm_attribute(hwnd, raw_attribute, &raw_value, sizeof(raw_value), "DwmSetWindowAttribute(int)")) {
			return false;
		}
		_cache_attribute(hwnd, raw_attribute, raw_value);
		return true;
#else
		return false;
#endif
	}

	int DWM::get_window_attribute_int(Window* target_window, WindowAttribute attribute) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		ERR_FAIL_COND_V_MSG(!hwnd, 0, "HWND is invalid.");

		DWORD value = 0;
		const DWORD raw_attribute = static_cast<DWORD>(attribute);
		if (!_get_dwm_attribute(hwnd, raw_attribute, &value, sizeof(value), "DwmGetWindowAttribute(int)", false)) {
			if (!_get_cached_attribute(hwnd, raw_attribute, value)) {
				_push_hresult_error("DwmGetWindowAttribute(int)", E_INVALIDARG);
				return 0;
			}
		}
		return static_cast<int>(value);
#else
		return 0;
#endif
	}

	bool DWM::set_window_attribute_bool(Window* target_window, WindowAttribute attribute, bool value) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		ERR_FAIL_COND_V_MSG(!hwnd, false, "HWND is invalid.");

		BOOL raw_value = value ? TRUE : FALSE;
		const DWORD raw_attribute = static_cast<DWORD>(attribute);
		if (!_set_dwm_attribute(hwnd, raw_attribute, &raw_value, sizeof(raw_value), "DwmSetWindowAttribute(bool)")) {
			return false;
		}
		_cache_attribute(hwnd, raw_attribute, raw_value);
		return true;
#else
		return false;
#endif
	}

	bool DWM::get_window_attribute_bool(Window* target_window, WindowAttribute attribute) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		ERR_FAIL_COND_V_MSG(!hwnd, false, "HWND is invalid.");

		BOOL value = FALSE;
		const DWORD raw_attribute = static_cast<DWORD>(attribute);
		if (!_get_dwm_attribute(hwnd, raw_attribute, &value, sizeof(value), "DwmGetWindowAttribute(bool)", false)) {
			DWORD cached = 0;
			if (!_get_cached_attribute(hwnd, raw_attribute, cached)) {
				_push_hresult_error("DwmGetWindowAttribute(bool)", E_INVALIDARG);
				return false;
			}
			value = cached != 0 ? TRUE : FALSE;
		}
		return value != FALSE;
#else
		return false;
#endif
	}

	bool DWM::set_window_attribute_color(Window* target_window, WindowAttribute attribute, const Color& color) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		ERR_FAIL_COND_V_MSG(!hwnd, false, "HWND is invalid.");

		COLORREF raw_color = _color_to_colorref(color);
		const DWORD raw_attribute = static_cast<DWORD>(attribute);
		if (!_set_dwm_attribute(hwnd, raw_attribute, &raw_color, sizeof(raw_color), "DwmSetWindowAttribute(color)")) {
			return false;
		}
		_cache_attribute(hwnd, raw_attribute, raw_color);
		return true;
#else
		return false;
#endif
	}

	Color DWM::get_window_attribute_color(Window* target_window, WindowAttribute attribute) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		ERR_FAIL_COND_V_MSG(!hwnd, Color(), "HWND is invalid.");

		COLORREF color = 0;
		const DWORD raw_attribute = static_cast<DWORD>(attribute);
		if (!_get_dwm_attribute(hwnd, raw_attribute, &color, sizeof(color), "DwmGetWindowAttribute(color)", false)) {
			DWORD cached = 0;
			if (!_get_cached_attribute(hwnd, raw_attribute, cached)) {
				_push_hresult_error("DwmGetWindowAttribute(color)", E_INVALIDARG);
				return Color();
			}
			color = cached;
		}
		return _colorref_to_color(color);
#else
		return Color();
#endif
	}

	Rect2i DWM::get_window_attribute_rect(Window* target_window, WindowAttribute attribute) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		ERR_FAIL_COND_V_MSG(!hwnd, Rect2i(), "HWND is invalid.");

		RECT rect = {};
		if (!_get_dwm_attribute(hwnd, static_cast<DWORD>(attribute), &rect, sizeof(rect), "DwmGetWindowAttribute(rect)")) {
			return Rect2i();
		}
		return _rect_to_rect2i(rect);
#else
		return Rect2i();
#endif
	}

	int64_t DWM::get_window_style(Window* target_window, bool extended) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		ERR_FAIL_COND_V_MSG(!hwnd, 0, "HWND is invalid.");

		const int index = extended ? GWL_EXSTYLE : GWL_STYLE;
		SetLastError(0);
		LONG_PTR style = GetWindowLongPtrW(hwnd, index);
		if (style == 0 && GetLastError() != 0) {
			UtilityFunctions::push_error("GetWindowLongPtrW failed.");
			return 0;
		}
		return static_cast<int64_t>(style);
#else
		return 0;
#endif
	}

	bool DWM::set_window_style(Window* target_window, int64_t style, bool extended, bool apply_frame_change) {
#ifdef _WIN32
		HWND hwnd = _get_godot_window_handle(target_window);
		ERR_FAIL_COND_V_MSG(!hwnd, false, "HWND is invalid.");

		const int index = extended ? GWL_EXSTYLE : GWL_STYLE;
		SetLastError(0);
		LONG_PTR previous = SetWindowLongPtrW(hwnd, index, static_cast<LONG_PTR>(style));
		if (previous == 0 && GetLastError() != 0) {
			UtilityFunctions::push_error("SetWindowLongPtrW failed.");
			return false;
		}

		if (apply_frame_change) {
			const UINT flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED;
			if (!SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, flags)) {
				UtilityFunctions::push_error("SetWindowPos(SWP_FRAMECHANGED) failed.");
				return false;
			}
		}
		return true;
#else
		return false;
#endif
	}

	bool DWM::set_window_style_bits(Window* target_window, int64_t mask, bool enabled, bool extended, bool apply_frame_change) {
		int64_t style = get_window_style(target_window, extended);
		const int64_t next_style = enabled ? (style | mask) : (style & ~mask);
		if (next_style == style) {
			return true;
		}
		return set_window_style(target_window, next_style, extended, apply_frame_change);
	}
}
