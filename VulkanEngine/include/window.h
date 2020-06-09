#ifndef __WINDOW_CLASS_H__
#define __WINDOW_CLASS_H__ 1

#include "platform_types.h"
#include <map>
#include <memory>
#include "constants.h"
#include <imgui/imgui.h>


struct GLFWwindow;
namespace VKE {

	class HierarchyContext;
	class RenderContext;
	class Window;

	struct CustomAppConsole {
		char                  InputBuf[256];
		uint32								message_limit_ = 200;
		uint32								current_message_count_ = 0;
		std::vector<std::string> debug_messages_;
		ImVector<char*>       History;
		int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
		ImGuiTextFilter       Filter;
		bool                  AutoScroll;
		bool                  ScrollToBottom;
		Window*								window_ = nullptr;

		CustomAppConsole(Window* window);
		~CustomAppConsole();

		void ClearLog();
		//void AddLog(const char* fmt, ...) IM_FMTARGS(2);
		void AddLog(std::string message);
		void Draw();
		static int TextEditCallbackStub(ImGuiInputTextCallbackData* data);
		int TextEditCallback(ImGuiInputTextCallbackData* data);

		// Portable helpers
		static int   Stricmp(const char* str1, const char* str2) { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
		static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
		static char* Strdup(const char *str) { size_t len = strlen(str) + 1; void* buf = malloc(len); /*IM_ASSERT(buf); */return (char*)memcpy(buf, (const void*)str, len); }
		static void  Strtrim(char* str) { char* str_end = str + strlen(str); while (str_end > str && str_end[-1] == ' ') str_end--; *str_end = 0; }

	private:
		bool open = true;
		char title[15] = "Custom Console";

	};

	struct AppUniformRenderData{
		float ambient_ = 0.1f;
		float sun_angle_ = 0.0f;
		glm::vec3 light_color_ = glm::vec3(1.0f,0.88f,0.67f);
		float light_intensity_ = 1.0f;
		float shininess_exponential_ = 16.0f;
		float exposure_ = 1.0f;
		int32 debug_type_ = 0;
		bool should_recreate_shaders = false;
		void Draw();

	private:
		bool open = true;
		char title[18] = "Rendering Options";
	};

	struct AppHierarchyDetails {
		VKE::HierarchyContext* hier_ctx_ = nullptr;
		void Draw();

		AppHierarchyDetails(VKE::HierarchyContext* hier_ctx);

	private:
		bool open = true;
		char title[18] = "Hierarchy Details";
	};

	class Window {
	public:

		Window();
		~Window();
		Window(const Window& other);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

		void initWindow(uint32 width, uint32 height, HierarchyContext* hier_ctx);

		bool ShouldWindowClose();
		void PollEvents();
		void Update();
		void UI(RenderContext* render_ctx, HierarchyContext* hier_ctx);
		float64 getTimeSinceStartup() { return time_since_startup_; }
		float64 getDeltaTime() { return delta_time_; }

		KeyState GetKeyState(uint64 input_key);
		bool IsKeyUp(uint64 input_key);
		bool IsKeyDown(uint64 input_key);
		bool IsKeyPressed(uint64 input_key);

		KeyState GetMouseButtonState(uint64 input_key);
		bool IsMouseButtonUp(uint64 input_key);
		bool IsMouseButtonDown(uint64 input_key);
		bool IsMouseButtonPressed(uint64 input_key);

		uint32 width() { return win_width; }
		uint32 height() { return win_height; }

		void SetMousePosition(float64 x, float64 y);
		void getMousePosition(float64& x, float64& y);

		void shutdownWindow();

		GLFWwindow* window = nullptr;

		void AddDebugMessage(std::string);

		std::unique_ptr<CustomAppConsole> custom_console;
		std::unique_ptr<AppUniformRenderData> rendering_options;
		std::unique_ptr<AppHierarchyDetails> hierarchy_details;

	private:


		int32 win_width = 800;
		int32 win_height = 600;

		std::map<uint64, KeyState> input_key_states_;
		std::map<uint64, KeyState> previous_input_key_states_;

		std::map<uint64, KeyState> input_mouse_key_states_;
		std::map<uint64, KeyState> previous_input_mouse_key_states_;

		float64 time_since_startup_;
		float64 delta_time_;


		friend class CustomAppConsole;
	};
}
#endif /__PLATFORM_TYPES_H__
