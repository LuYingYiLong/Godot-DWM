#ifndef DWM_H
#define DWM_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/rect2i.hpp>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dwmapi.h>

#ifndef DWMWA_USE_HOSTBACKDROPBRUSH
#define DWMWA_USE_HOSTBACKDROPBRUSH 17
#endif
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif
#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34
#endif
#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif
#ifndef DWMWA_TEXT_COLOR
#define DWMWA_TEXT_COLOR 36
#endif
#ifndef DWMWA_VISIBLE_FRAME_BORDER_THICKNESS
#define DWMWA_VISIBLE_FRAME_BORDER_THICKNESS 37
#endif
#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif
#endif

namespace godot {
	class Window;

	class DWM : public RefCounted {
		GDCLASS(DWM, RefCounted)

	private:
#ifdef _WIN32
		static HWND _get_godot_window_handle(Window* target_window);
#endif

	protected:
		static void _bind_methods();

	public:
		enum SystembackdropType {
			AUTO = 0,
			NONE = 1,
			MAINWINDOW = 2,
			TRANSIENTWINDOW = 3,
			TABBEDWINDOW = 4,

			// Backwards-compatible aliases.
			SYSTEMBACKDROP_TYPE_AUTO = AUTO,
			SYSTEMBACKDROP_TYPE_NONE = NONE,
			SYSTEMBACKDROP_TYPE_MAINWINDOW = MAINWINDOW,
			SYSTEMBACKDROP_TYPE_TRANSIENTWINDOW = TRANSIENTWINDOW,
			SYSTEMBACKDROP_TYPE_TABBEDWINDOW = TABBEDWINDOW
		};

		enum WindowCornerPreference {
			CORNER_DEFAULT = 0,
			CORNER_DO_NOT_ROUND = 1,
			CORNER_ROUND = 2,
			CORNER_ROUND_SMALL = 3
		};

		enum NonClientRenderingPolicy {
			NCRENDERING_USE_WINDOW_STYLE = 0,
			NCRENDERING_DISABLED = 1,
			NCRENDERING_ENABLED = 2
		};

		enum Flip3DPolicy {
			FLIP3D_DEFAULT = 0,
			FLIP3D_EXCLUDE_BELOW = 1,
			FLIP3D_EXCLUDE_ABOVE = 2
		};

		enum WindowAttribute {
			ATTR_NCRENDERING_ENABLED = 1,
			ATTR_NCRENDERING_POLICY = 2,
			ATTR_TRANSITIONS_FORCEDISABLED = 3,
			ATTR_ALLOW_NCPAINT = 4,
			ATTR_CAPTION_BUTTON_BOUNDS = 5,
			ATTR_NONCLIENT_RTL_LAYOUT = 6,
			ATTR_FORCE_ICONIC_REPRESENTATION = 7,
			ATTR_FLIP3D_POLICY = 8,
			ATTR_EXTENDED_FRAME_BOUNDS = 9,
			ATTR_HAS_ICONIC_BITMAP = 10,
			ATTR_DISALLOW_PEEK = 11,
			ATTR_EXCLUDED_FROM_PEEK = 12,
			ATTR_CLOAK = 13,
			ATTR_CLOAKED = 14,
			ATTR_FREEZE_REPRESENTATION = 15,
			ATTR_PASSIVE_UPDATE_MODE = 16,
			ATTR_USE_HOSTBACKDROPBRUSH = 17,
			ATTR_USE_IMMERSIVE_DARK_MODE = 20,
			ATTR_WINDOW_CORNER_PREFERENCE = 33,
			ATTR_BORDER_COLOR = 34,
			ATTR_CAPTION_COLOR = 35,
			ATTR_TEXT_COLOR = 36,
			ATTR_VISIBLE_FRAME_BORDER_THICKNESS = 37,
			ATTR_SYSTEMBACKDROP_TYPE = 38
		};

		static bool is_composition_enabled();
		static bool is_title_bar_coloring_supported();
		static bool is_window_valid(Window* target_window);
		static int64_t get_native_window_handle(Window* target_window);

		static void set_title_bar_color(Window* target_window, const Color& color, const bool sync_border);
		static void set_border_color(Window* target_window, const Color& color);
		static void set_title_bar_text_color(Window* target_window, const Color& color);
		static Color get_title_bar_color(Window* target_window);
		static Color get_border_color(Window* target_window);
		static Color get_title_bar_text_color(Window* target_window);
		static void reset_to_default_colors(Window* target_window);

		static void enable_dark_mode(Window* target_window, const bool enable);
		static void extend_frame_into_client_area(Window* target_window, const int left, const int right, const int top, const int bottom);
		static Color get_colorization_color();
		static Dictionary get_composition_timing_info(Window* target_window);
		static void enable_hostbackdropbrush(Window* target_window, const bool enable);
		static void set_systembackdrop_type(Window* target_window, SystembackdropType p_type = AUTO);
		static void enable_mmcss(bool enable);

		static bool set_window_corner_preference(Window* target_window, WindowCornerPreference preference);
		static int get_window_corner_preference(Window* target_window);
		static bool set_non_client_rendering_policy(Window* target_window, NonClientRenderingPolicy policy);
		static bool set_flip3d_policy(Window* target_window, Flip3DPolicy policy);
		static bool force_disable_transitions(Window* target_window, bool disable);
		static bool set_peek_disallowed(Window* target_window, bool disallow);
		static bool set_excluded_from_peek(Window* target_window, bool exclude);
		static int get_visible_frame_border_thickness(Window* target_window);
		static Rect2i get_window_bounds(Window* target_window, bool extended_frame);
		static Rect2i get_caption_button_bounds(Window* target_window);

		static bool set_window_attribute_int(Window* target_window, WindowAttribute attribute, int value);
		static int get_window_attribute_int(Window* target_window, WindowAttribute attribute);
		static bool set_window_attribute_bool(Window* target_window, WindowAttribute attribute, bool value);
		static bool get_window_attribute_bool(Window* target_window, WindowAttribute attribute);
		static bool set_window_attribute_color(Window* target_window, WindowAttribute attribute, const Color& color);
		static Color get_window_attribute_color(Window* target_window, WindowAttribute attribute);
		static Rect2i get_window_attribute_rect(Window* target_window, WindowAttribute attribute);

		static int64_t get_window_style(Window* target_window, bool extended);
		static bool set_window_style(Window* target_window, int64_t style, bool extended, bool apply_frame_change);
		static bool set_window_style_bits(Window* target_window, int64_t mask, bool enabled, bool extended, bool apply_frame_change);
	};
}

VARIANT_ENUM_CAST(DWM::SystembackdropType);
VARIANT_ENUM_CAST(DWM::WindowCornerPreference);
VARIANT_ENUM_CAST(DWM::NonClientRenderingPolicy);
VARIANT_ENUM_CAST(DWM::Flip3DPolicy);
VARIANT_ENUM_CAST(DWM::WindowAttribute);

#endif // DWM_H
