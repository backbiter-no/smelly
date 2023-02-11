/*
 * Copyright (C) 2017 anders Goyal <anders at backbiter-no.net>
 *
 * Distributed under terms of the GPL3 license.
 */

#pragma once
#include "data-types.h"
#include "screen.h"
#include "monotonic.h"
#include "window_logo.h"

#define OPT(name) global_state.opts.name

typedef enum { LEFT_EDGE, TOP_EDGE, RIGHT_EDGE, BOTTOM_EDGE } Edge;
typedef enum { RESIZE_DRAW_STATIC, RESIZE_DRAW_SCALED, RESIZE_DRAW_BLANK, RESIZE_DRAW_SIZE } ResizeDrawStrategy;
typedef enum { REPEAT_MIRROR, REPEAT_CLAMP, REPEAT_DEFAULT } RepeatStrategy;

typedef struct {
    char_type string[16];
    size_t len;
} UrlPrefix;

typedef enum AdjustmentUnit { POINT = 0, PERCENT = 1, PIXEL = 2 } AdjustmentUnit;

typedef struct {
    monotonic_t visual_bell_duration, cursor_blink_interval, cursor_stop_blinking_after, mouse_hide_wait, click_interval;
    double wheel_scroll_multiplier, touch_scroll_multiplier;
    int wheel_scroll_min_lines;
    bool enable_audio_bell;
    CursorShape cursor_shape;
    float cursor_beam_thickness;
    float cursor_underline_thickness;
    unsigned int url_style;
    unsigned int scrollback_pager_history_size;
    bool scrollback_fill_enlarged_window;
    char_type *select_by_word_characters;
    char_type *select_by_word_characters_forward;
    color_type url_color, background, foreground, active_border_color, inactive_border_color, bell_border_color, tab_bar_background, tab_bar_margin_color;
    color_type mark1_foreground, mark1_background, mark2_foreground, mark2_background, mark3_foreground, mark3_background;
    monotonic_t repaint_delay, input_delay;
    bool focus_follows_mouse;
    unsigned int hide_window_decorations;
    bool macos_hide_from_tasks, macos_quit_when_last_window_closed, macos_window_resizable, macos_traditional_fullscreen;
    unsigned int macos_option_as_alt;
    float macos_thicken_font;
    WindowTitleIn macos_show_window_title_in;
    char *bell_path;
    float background_opacity, dim_opacity;
    float text_contrast, text_gamma_adjustment;
    bool text_old_gamma;

    char *background_image, *default_window_logo;
    BackgroundImageLayout background_image_layout;
    ImageAnchorPosition window_logo_position;
    bool background_image_linear;
    float background_tint, background_tint_gaps, window_logo_alpha;

    bool dynamic_background_opacity;
    float inactive_text_alpha;
    Edge tab_bar_edge;
    unsigned long tab_bar_min_tabs;
    DisableLigature disable_ligatures;
    bool force_ltr;
    ResizeDrawStrategy resize_draw_strategy;
    bool resize_in_steps;
    bool sync_to_monitor;
    bool close_on_child_death;
    bool window_alert_on_bell;
    bool debug_keyboard;
    bool allow_hyperlinks;
    monotonic_t resize_debounce_time;
    MouseShape pointer_shape_when_grabbed;
    MouseShape default_pointer_shape;
    MouseShape pointer_shape_when_dragging;
    struct {
        UrlPrefix *values;
        size_t num, max_prefix_len;
    } url_prefixes;
    char_type *url_excluded_characters;
    bool detect_urls;
    bool tab_bar_hidden;
    double font_size;
    struct {
        double outer, inner;
    } tab_bar_margin_height;
    long macos_menubar_title_max_length;
    int macos_colorspace;
    struct {
        float val; AdjustmentUnit unit;
    } underline_position, underline_thickness, strikethrough_position, strikethrough_thickness, cell_width, cell_height, baseline;
    bool show_hyperlink_targets;
} Options;

typedef struct WindowLogoRenderData {
    window_logo_id_t id;
    WindowLogo *instance;
    ImageAnchorPosition position;
    float alpha;
    bool using_default;
} WindowLogoRenderData;

typedef struct {
    ssize_t vao_idx, gvao_idx;
    float xstart, ystart, dx, dy, xratio, yratio;
    Screen *screen;
} ScreenRenderData;

typedef struct {
    unsigned int left, top, right, bottom;
} WindowGeometry;

typedef struct {
    monotonic_t at;
    int button, modifiers;
    double x, y;
    unsigned long num;
} Click;

#define CLICK_QUEUE_SZ 3
typedef struct {
    Click clicks[CLICK_QUEUE_SZ];
    unsigned int length;
} ClickQueue;

typedef struct MousePosition {
    unsigned int cell_x, cell_y;
    double global_x, global_y;
    bool in_left_half_of_cell;
} MousePosition;

typedef struct WindowBarData {
    unsigned width, height;
    uint8_t *buf;
    PyObject *last_drawn_title_object_id;
    hyperlink_id_type hyperlink_id_for_title_object;
    bool needs_render;
} WindowBarData;

typedef struct {
    id_type id;
    bool visible, cursor_visible_at_last_render;
    unsigned int last_cursor_x, last_cursor_y;
    CursorShape last_cursor_shape;
    PyObject *title;
    ScreenRenderData render_data;
    WindowLogoRenderData window_logo;
    MousePosition mouse_pos;
    struct {
        unsigned int left, top, right, bottom;
    } padding;
    WindowGeometry geometry;
    ClickQueue click_queues[8];
    monotonic_t last_drag_scroll_at;
    uint32_t last_special_key_pressed;
    WindowBarData title_bar_data, url_target_bar_data;
} Window;

typedef struct {
    float left, top, right, bottom;
    uint32_t color;
} BorderRect;

typedef struct {
    BorderRect *rect_buf;
    unsigned int num_border_rects, capacity;
    bool is_dirty;
    ssize_t vao_idx;
} BorderRects;

typedef struct {
    id_type id;
    unsigned int active_window, num_windows, capacity;
    Window *windows;
    BorderRects border_rects;
} Tab;

enum RENDER_STATE { RENDER_FRAME_NOT_REQUESTED, RENDER_FRAME_REQUESTED, RENDER_FRAME_READY };
typedef enum { NO_CLOSE_REQUESTED, CONFIRMABLE_CLOSE_REQUESTED, CLOSE_BEING_CONFIRMED, IMPERATIVE_CLOSE_REQUESTED } CloseRequest;

typedef struct {
    monotonic_t last_resize_event_at;
    bool in_progress;
    bool from_os_notification;
    bool os_says_resize_complete;
    unsigned int width, height, num_of_resize_events;
} LiveResizeInfo;


typedef struct {
    void *handle;
    id_type id;
    uint32_t offscreen_framebuffer;
    struct {
        int x, y, w, h;
        bool is_set, was_maximized;
    } before_fullscreen;
    int viewport_width, viewport_height, window_width, window_height, content_area_width, content_area_height;
    double viewport_x_ratio, viewport_y_ratio;
    Tab *tabs;
    BackgroundImage *bgimage;
    unsigned int active_tab, num_tabs, capacity, last_active_tab, last_num_tabs, last_active_window_id;
    bool focused_at_last_render, needs_render;
    ScreenRenderData tab_bar_render_data;
    struct {
        color_type left, right;
    } tab_bar_edge_color;
    bool tab_bar_data_updated;
    bool is_focused;
    monotonic_t cursor_blink_zero_time, last_mouse_activity_at;
    double mouse_x, mouse_y;
    double logical_dpi_x, logical_dpi_y, font_sz_in_pts;
    bool mouse_button_pressed[32];
    PyObject *window_title;
    bool disallow_title_changes, title_is_overriden;
    bool viewport_size_dirty, viewport_updated_at_least_once;
    monotonic_t viewport_resized_at;
    LiveResizeInfo live_resize;
    bool has_pending_resizes, is_semi_transparent, shown_once, is_damaged;
    uint32_t offscreen_texture_id;
    unsigned int clear_count;
    color_type last_titlebar_color;
    float background_opacity;
    FONTS_DATA_HANDLE fonts_data;
    id_type temp_font_group_id;
    enum RENDER_STATE render_state;
    monotonic_t last_render_frame_received_at;
    uint64_t render_calls;
    id_type last_focused_counter;
    ssize_t gvao_idx;
    CloseRequest close_request;
} OSWindow;


typedef struct {
    Options opts;

    id_type os_window_id_counter, tab_id_counter, window_id_counter, current_opengl_context_id;
    PyObject *boss;
    BackgroundImage *bgimage;
    OSWindow *os_windows;
    size_t num_os_windows, capacity;
    OSWindow *callback_os_window;
    bool is_wayland;
    bool has_render_frames;
    bool debug_rendering, debug_font_fallback;
    bool has_pending_resizes, has_pending_closes;
    bool in_sequence_mode;
    bool check_for_active_animated_images;
    struct { double x, y; } default_dpi;
    id_type active_drag_in_window, tracked_drag_in_window;
    int active_drag_button, tracked_drag_button;
    CloseRequest quit_request;
    bool redirect_mouse_handling;
    WindowLogoTable *all_window_logos;
} GlobalState;

extern GlobalState global_state;

#define call_boss(name, ...) if (global_state.boss) { \
    PyObject *cret_ = PyObject_CallMethod(global_state.boss, #name, __VA_ARGS__); \
    if (cret_ == NULL) { PyErr_Print(); } \
    else Py_DECREF(cret_); \
}

void gl_init(void);
void remove_vao(ssize_t vao_idx);
bool remove_os_window(id_type os_window_id);
void make_os_window_context_current(OSWindow *w);
void set_os_window_size(OSWindow *os_window, int x, int y);
void get_os_window_size(OSWindow *os_window, int *w, int *h, int *fw, int *fh);
void get_os_window_content_scale(OSWindow *os_window, double *xdpi, double *ydpi, float *xscale, float *yscale);
void update_os_window_references(void);
void mark_os_window_for_close(OSWindow* w, CloseRequest cr);
void update_os_window_viewport(OSWindow *window, bool notify_boss);
bool should_os_window_be_rendered(OSWindow* w);
void wakeup_main_loop(void);
void swap_window_buffers(OSWindow *w);
bool make_window_context_current(id_type);
void hide_mouse(OSWindow *w);
bool is_mouse_hidden(OSWindow *w);
void destroy_os_window(OSWindow *w);
void focus_os_window(OSWindow *w, bool also_raise, const char *activation_token);
void run_with_activation_token_in_os_window(OSWindow *w, PyObject *callback);
void set_os_window_title(OSWindow *w, const char *title);
OSWindow* os_window_for_smelly_window(id_type);
OSWindow* add_os_window(void);
OSWindow* current_os_window(void);
void os_window_regions(OSWindow*, Region *main, Region *tab_bar);
bool drag_scroll(Window *, OSWindow*);
void draw_borders(ssize_t vao_idx, unsigned int num_border_rects, BorderRect *rect_buf, bool rect_data_is_dirty, uint32_t viewport_width, uint32_t viewport_height, color_type, unsigned int, bool, OSWindow *w);
ssize_t create_cell_vao(void);
ssize_t create_graphics_vao(void);
ssize_t create_border_vao(void);
bool send_cell_data_to_gpu(ssize_t, ssize_t, float, float, float, float, Screen *, OSWindow *);
void draw_cells(ssize_t, ssize_t, const ScreenRenderData*, float, float, OSWindow *, bool, bool, Window*);
void draw_centered_alpha_mask(OSWindow *w, size_t screen_width, size_t screen_height, size_t width, size_t height, uint8_t *canvas);
void update_surface_size(int, int, uint32_t);
void free_texture(uint32_t*);
void free_framebuffer(uint32_t*);
void send_image_to_gpu(uint32_t*, const void*, int32_t, int32_t, bool, bool, bool, RepeatStrategy);
void send_sprite_to_gpu(FONTS_DATA_HANDLE fg, unsigned int, unsigned int, unsigned int, pixel*);
void blank_canvas(float, color_type);
void blank_os_window(OSWindow *);
void set_titlebar_color(OSWindow *w, color_type color, bool use_system_color, unsigned int system_color);
FONTS_DATA_HANDLE load_fonts_data(double, double, double);
void send_prerendered_sprites_for_window(OSWindow *w);
#ifdef __APPLE__
void get_cocoa_key_equivalent(uint32_t, int, char *key, size_t key_sz, int*);
typedef enum {
    PREFERENCES_WINDOW,
    NEW_OS_WINDOW,
    NEW_OS_WINDOW_WITH_WD,
    NEW_TAB_WITH_WD,
    CLOSE_OS_WINDOW,
    CLOSE_TAB,
    NEW_TAB,
    NEXT_TAB,
    PREVIOUS_TAB,
    DETACH_TAB,
    LAUNCH_URLS,
    NEW_WINDOW,
    CLOSE_WINDOW,
    RESET_TERMINAL,
    CLEAR_TERMINAL_AND_SCROLLBACK,
    RELOAD_CONFIG,
    TOGGLE_MACOS_SECURE_KEYBOARD_ENTRY,
    TOGGLE_FULLSCREEN,
    OPEN_smelly_WEBSITE,
    HIDE,
    HIDE_OTHERS,
    MINIMIZE,
    QUIT,

    NUM_COCOA_PENDING_ACTIONS
} CocoaPendingAction;
void set_cocoa_pending_action(CocoaPendingAction action, const char*);
#endif
void request_frame_render(OSWindow *w);
void request_tick_callback(void);
typedef void (* timer_callback_fun)(id_type, void*);
typedef void (* tick_callback_fun)(void*);
id_type add_main_loop_timer(monotonic_t interval, bool repeats, timer_callback_fun callback, void *callback_data, timer_callback_fun free_callback);
void remove_main_loop_timer(id_type timer_id);
void update_main_loop_timer(id_type timer_id, monotonic_t interval, bool enabled);
void run_main_loop(tick_callback_fun, void*);
void stop_main_loop(void);
void os_window_update_size_increments(OSWindow *window);
void set_os_window_title_from_window(Window *w, OSWindow *os_window);
void update_os_window_title(OSWindow *os_window);
void fake_scroll(Window *w, int amount, bool upwards);
Window* window_for_window_id(id_type smelly_window_id);
bool mouse_open_url(Window *w);
bool mouse_set_last_visited_cmd_output(Window *w);
bool mouse_select_cmd_output(Window *w);
bool move_cursor_to_mouse_if_at_shell_prompt(Window *w);
void mouse_selection(Window *w, int code, int button);
const char* format_mods(unsigned mods);
void send_pending_click_to_window_id(id_type, void*);
void send_pending_click_to_window(Window*, void*);
void get_platform_dependent_config_values(void *glfw_window);
bool draw_window_title(OSWindow *window, const char *text, color_type fg, color_type bg, uint8_t *output_buf, size_t width, size_t height);
uint8_t* draw_single_ascii_char(const char ch, size_t *result_width, size_t *result_height);
bool is_os_window_fullscreen(OSWindow *);
void update_ime_focus(OSWindow* osw, bool focused);
void update_ime_position(Window* w, Screen *screen);
bool update_ime_position_for_window(id_type window_id, bool force, int update_focus);
void set_ignore_os_keyboard_processing(bool enabled);
void update_menu_bar_title(PyObject *title UNUSED);