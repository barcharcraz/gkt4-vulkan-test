#include "glib.h"
#include <errhandlingapi.h>
#include <libloaderapi.h>
#include <minwindef.h>
#include <windef.h>
#include <winnt.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "gdk/gdk.h"
#include "gio/gio.h"
#include "glib-object.h"

#include <gdk/gdk.h>
#include <gdk/win32/gdkwin32.h>
#include <gtk/gtk.h>
#include <windows.h>

_Pragma("GCC diagnostic ignored \"-Wincompatible-pointer-types\"")

    LRESULT myWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch(uMsg) {
            case WM_PAINT:
        PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // All painting occurs here, between BeginPaint and EndPaint.

            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

            EndPaint(hWnd, &ps);
            break;
        }
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

#define VULKAN_UI_TEST_TYPE_MAIN_WINDOW (vulkan_ui_test_main_window_get_type())

struct _VulkanUiTestMainWindow {
  GtkApplicationWindow parent;
  HWND rendering_window;
};

static void error_and_print_lasterror(const char *desc) {
  LPCSTR buf = nullptr;
  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, GetLastError(), 0, &buf, 0, 0);
  g_error("%s: %lu: %s\n", desc, GetLastError(), buf);
}

G_DECLARE_FINAL_TYPE(VulkanUiTestMainWindow, vulkan_ui_test_main_window,
                     VULKAN_UI_TEST, MAIN_WINDOW, GtkApplicationWindow);
G_DEFINE_FINAL_TYPE(VulkanUiTestMainWindow, vulkan_ui_test_main_window,
                    GTK_TYPE_APPLICATION_WINDOW);

static void on_layout(GdkSurface* self, guint width, guint height, VulkanUiTestMainWindow* win) {
    HWND gtkwnd = gdk_win32_surface_get_handle(self);
    RECT r;
    double x,y;
    gtk_native_get_surface_transform(win, &x, &y);
    GetClientRect(gtkwnd, &r);
    graphene_rect_t bounds;
    gtk_widget_compute_bounds(win, win, &bounds);
    MapWindowPoints(gtkwnd, HWND_DESKTOP, &r, 2);
    SetWindowPos(win->rendering_window, 0, r.left+x, r.top+y, bounds.size.width, bounds.size.height, SWP_NOACTIVATE | SWP_NOCOPYBITS);
}

static void on_realize(VulkanUiTestMainWindow *self, gpointer) {
  HINSTANCE exe_instance = GetModuleHandleW(nullptr);
  ATOM class = RegisterClassExW(&(WNDCLASSEXW){.cbSize = sizeof(WNDCLASSEXW),
                                               .lpszClassName = L"myWindow",
                                               .hInstance = exe_instance,
                                               .lpfnWndProc = &myWndProc});
  if (class == 0) {
    error_and_print_lasterror("RegisterClassExW");
  }
  GdkSurface* surf = gtk_native_get_surface(self);
  HWND gtkhwnd = gdk_win32_surface_get_handle(surf);
  RECT gtkrect = {};
  GetClientRect(gtkhwnd, &gtkrect);
  self->rendering_window =
      CreateWindowExW(WS_EX_NOACTIVATE, L"myWindow",
                      L"rendering window", WS_POPUPWINDOW, gtkrect.left, gtkrect.top, gtkrect.right - gtkrect.left,
                      gtkrect.bottom - gtkrect.top, 0, 0, exe_instance, nullptr);
  if (self->rendering_window == nullptr) {
    error_and_print_lasterror("CreateWindowExW");
  }
  SetWindowLongPtr(gtkhwnd, GWLP_HWNDPARENT, (LONG_PTR)self->rendering_window);
  g_signal_connect(surf, "layout", on_layout, self);
  //SetWindowLong(gtkhwnd, GWL_STYLE, GetWindowLong(gtkhwnd, GWL_STYLE) & ~WS_CLIPCHILDREN);
}

static void on_show(VulkanUiTestMainWindow *self, gpointer) {
  ShowWindow(self->rendering_window, SW_NORMAL);
}

static void
vulkan_ui_test_main_window_class_init(VulkanUiTestMainWindowClass *klass) {
}
static void vulkan_ui_test_main_window_init(VulkanUiTestMainWindow *self) {}

#define VULKAN_UI_TEST_TYPE_APP (vulkan_ui_test_app_get_type())

struct _VulkanUiTestApp {
  GtkApplication parent;
  GtkCssProvider *css_provider;

  // non-owning
  GtkApplicationWindow *win;
};

G_DECLARE_FINAL_TYPE(VulkanUiTestApp, vulkan_ui_test_app, VULKAN_UI_TEST, APP,
                     GtkApplication);
G_DEFINE_FINAL_TYPE(VulkanUiTestApp, vulkan_ui_test_app, GTK_TYPE_APPLICATION);

void vulkan_ui_test_app_activate(VulkanUiTestApp *self, gpointer user) {
  //VulkanUiTestMainWindow* win = g_object_new(VULKAN_UI_TEST_TYPE_MAIN_WINDOW, "application", self, nullptr);
  GtkApplicationWindow* win = gtk_application_window_new(self);
  g_signal_connect(win, "show", on_show, nullptr);
  g_signal_connect(win, "realize", on_realize, nullptr);
  gtk_window_set_default_size(win, 800, 600);
  gtk_widget_set_name(win, "root_window");
  self->win = win;
    self->css_provider = gtk_css_provider_new();
  const char *css_data =
      "#root_window { background: rgba(0.0, 1.0, 1.0, 0.5); }";
  gtk_css_provider_load_from_string(self->css_provider, css_data);
  gtk_style_context_add_provider_for_display(
      gdk_display_get_default(), self->css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
//   auto sub = gtk_button_new();
//   gtk_button_set_label(sub, "foo");
//   gtk_window_set_child(self->win, sub);
  
  gtk_window_present(win);
}
static void vulkan_ui_test_app_init(VulkanUiTestApp *self) {}

static void vulkan_ui_test_app_shutdown(VulkanUiTestApp *self) {
  g_object_unref(self->css_provider);
  G_APPLICATION_CLASS(vulkan_ui_test_app_parent_class)->shutdown(self);
}

static void vulkan_ui_test_app_class_init(VulkanUiTestAppClass *klass) {
  G_APPLICATION_CLASS(klass)->activate = vulkan_ui_test_app_activate;
  G_APPLICATION_CLASS(klass)->shutdown = vulkan_ui_test_app_shutdown;
}

int main(int argc, char **argv) {

  g_autoptr(VulkanUiTestApp) app = g_object_new(
      vulkan_ui_test_app_get_type(), "application-id", "site.barto.test",
      "flags", G_APPLICATION_NON_UNIQUE, nullptr);

  return g_application_run(app, argc, argv);
}