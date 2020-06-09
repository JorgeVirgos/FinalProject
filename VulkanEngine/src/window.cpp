#include "window.h"
#include <GLFW/glfw3.h>
#include "imgui/imgui_impl_glfw.h"
#include <imgui/imgui_impl_vulkan.h>
#include <iostream>
#include <string>
#include "render_context.h"
#include "hierarchy_context.h"
#include "render_context.h"
#include <chrono>

VKE::CustomAppConsole::CustomAppConsole(VKE::Window* window)
{
	window_ = window;
	debug_messages_.resize(message_limit_);
	ClearLog();
	memset(InputBuf, 0, sizeof(InputBuf));
	HistoryPos = -1;
	AutoScroll = true;
	ScrollToBottom = true;
}
VKE::CustomAppConsole::~CustomAppConsole()
{
	ClearLog();
	for (int i = 0; i < History.Size; i++)
		free(History[i]);
}

VKE::Window::Window() {

}
VKE::Window::~Window() {
	system("pause");
}
VKE::Window::Window(const Window& other) {
	//*this = other;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VKE::Window::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::string complete_message;
	std::string sev = "[UNSPECIFIED] ";
	if (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT >= messageSeverity) sev = "[ERROR] ";
	else if (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT >= messageSeverity) sev = "[WARNING] ";
	else if (VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT >= messageSeverity) sev = "[INFO] ";
	else if (VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT >= messageSeverity) sev = "[VERBOSE] ";

	std::cerr << sev << "validation layer: " << pCallbackData->pMessage << std::endl;

	complete_message = sev + std::string("validation layer:") + pCallbackData->pMessage;

	VKE::Window *window = (VKE::Window*)pUserData;
	window->AddDebugMessage(complete_message);

	return VK_FALSE;
}

void Style()
{
	ImGuiStyle & style = ImGui::GetStyle();
	ImVec4 * colors = style.Colors;

	/// 0 = FLAT APPEARENCE
	/// 1 = MORE "3D" LOOK
	int is3D = 0;

	float lim = 1 / 256.0f;

	colors[ImGuiCol_Text] = ImVec4(32.*lim, 17. * lim, 27. * lim, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(213.*lim, 204.*lim, 186.*lim, 0.0f);
	colors[ImGuiCol_WindowBg] = ImVec4(213.*lim, 204.*lim, 186.*lim, 0.9f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
	colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.42f, 0.42f, 0.42f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.17f, 0.17f, 0.17f, 0.90f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.335f, 0.335f, 0.335f, 1.000f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.24f, 0.24f, 0.24f, 0.15f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
	colors[ImGuiCol_Separator] = ImVec4(0.000f, 0.000f, 0.000f, 0.137f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

	style.PopupRounding = 3;

	style.WindowPadding = ImVec2(4, 4);
	style.FramePadding = ImVec2(6, 4);
	style.ItemSpacing = ImVec2(6, 2);

	style.ScrollbarSize = 18;

	style.WindowBorderSize = 1;
	style.ChildBorderSize = 1;
	style.PopupBorderSize = 1;
	style.FrameBorderSize = 1;

	style.WindowRounding = 3;
	style.ChildRounding = 3;
	style.FrameRounding = 3;
	style.ScrollbarRounding = 2;

	style.WindowTitleAlign =ImVec2(0.5f, 0.5f);
}


void VKE::Window::shutdownWindow() {
	ImGui_ImplGlfw_Shutdown();
	glfwDestroyWindow(window);
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {

	if (width == 0 || height == 0) return;

	auto hier_ctx = reinterpret_cast<VKE::HierarchyContext*>(glfwGetWindowUserPointer(window));
	if (hier_ctx != nullptr && hier_ctx->getRenderContext() != nullptr) {
		hier_ctx->getRenderContext()->recreateSwapChain();
		//hier_ctx->UpdateManagers();
		//hier_ctx->getRenderContext()->getCamera().calculateStaticMatrices(70.0f, width, height, 0.1f, 100.0f);
		//hier_ctx->getRenderContext()->draw();
	}
}

void VKE::Window::initWindow(uint32 width, uint32 height, HierarchyContext* hier_ctx) {

	win_height = height;
	win_width = width;

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	auto video_mode = glfwGetVideoMode(monitor);

	window = glfwCreateWindow(win_width, win_height, "VKE Main Window", nullptr, nullptr);
#ifdef _DEBUG
	glfwSetWindowMonitor(window, nullptr, video_mode->width - win_width, (video_mode->height / 2) - win_height/2, win_width, win_height, 60);
#else
	glfwSetWindowMonitor(window, nullptr, (video_mode->width / 2) - win_width / 2, (video_mode->height / 2) - win_height / 2, win_width, win_height, 60);
#endif

	glfwSetWindowUserPointer(window, hier_ctx);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

	glfwGetWindowSize(window, &win_width, &win_height);

	ImGui_ImplGlfw_InitForVulkan(window, true);
	custom_console = std::make_unique<CustomAppConsole>(CustomAppConsole(this));
	rendering_options = std::make_unique<AppUniformRenderData>(AppUniformRenderData());
	hierarchy_details = std::make_unique<AppHierarchyDetails>(AppHierarchyDetails(hier_ctx));
	Style();
}

bool VKE::Window::ShouldWindowClose() {
	return glfwWindowShouldClose(window);
}
void VKE::Window::PollEvents() {


	glfwPollEvents();

	previous_input_key_states_ = input_key_states_;
	previous_input_mouse_key_states_ = input_mouse_key_states_;

	for (uint64 key_id = 32; key_id < GLFW_KEY_LAST; ++key_id) {
		input_key_states_[key_id] = (KeyState)glfwGetKey(window, key_id);
	}

	for (uint64 mouse_button_id = GLFW_MOUSE_BUTTON_1; mouse_button_id < GLFW_MOUSE_BUTTON_8 + 1; ++mouse_button_id) {
		input_mouse_key_states_[mouse_button_id] = (KeyState)glfwGetMouseButton(window, mouse_button_id);
	}

}

void VKE::Window::Update() {

	glfwGetWindowSize(window, &win_width, &win_height);


	static auto startTime = std::chrono::high_resolution_clock::now();
	static auto current_time = startTime;

	auto prev_time = current_time;
	current_time = std::chrono::high_resolution_clock::now();

	time_since_startup_ = std::chrono::duration<float32, std::chrono::seconds::period>(current_time - startTime).count();
	delta_time_ = std::chrono::duration<float32, std::chrono::seconds::period>(current_time - prev_time).count();


}


VKE::KeyState VKE::Window::GetKeyState(uint64 input_key) {
	return input_key_states_[input_key];
}
bool VKE::Window::IsKeyUp(uint64 input_key) {
	return previous_input_key_states_[input_key] == VKE::KeyState_Pressed &&
		input_key_states_[input_key] == VKE::KeyState_Released;
}
bool VKE::Window::IsKeyDown(uint64 input_key) {
	return previous_input_key_states_[input_key] == VKE::KeyState_Released &&
		input_key_states_[input_key] == VKE::KeyState_Pressed;
}
bool VKE::Window::IsKeyPressed(uint64 input_key) {
	return input_key_states_[input_key] == VKE::KeyState_Pressed;
}

VKE::KeyState VKE::Window::GetMouseButtonState(uint64 input_key) {
	return input_mouse_key_states_[input_key];
}
bool VKE::Window::IsMouseButtonUp(uint64 input_key) {
	return previous_input_mouse_key_states_[input_key] == VKE::KeyState_Pressed &&
		input_mouse_key_states_[input_key] == VKE::KeyState_Released;
}
bool VKE::Window::IsMouseButtonDown(uint64 input_key) {
	return previous_input_mouse_key_states_[input_key] == VKE::KeyState_Released &&
		input_mouse_key_states_[input_key] == VKE::KeyState_Pressed;
}
bool VKE::Window::IsMouseButtonPressed(uint64 input_key) {
	return input_mouse_key_states_[input_key] == VKE::KeyState_Pressed;
}

void VKE::Window::SetMousePosition(float64 x, float64 y) {
	glfwSetCursorPos(window, x, y);
}
void VKE::Window::getMousePosition(float64& x, float64& y) {
	glfwGetCursorPos(window, &x, &y);
}

void VKE::Window::AddDebugMessage(std::string debug_str) {
	custom_console->AddLog(debug_str);
}


void VKE::Window::UI(RenderContext* render_ctx, HierarchyContext* hier_ctx) {

	static bool open = true;
	ImFont* cool_font = nullptr;
	auto& io = ImGui::GetIO();


	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow(false);

	custom_console->Draw();
	rendering_options->Draw();
	hierarchy_details->Draw();

	ImGui::Render();
}

void VKE::CustomAppConsole::ClearLog()
{
	for (auto string : debug_messages_) string = "";
}

void VKE::CustomAppConsole::AddLog(std::string message) {


	for (auto inside_message : debug_messages_) {
		if (message == inside_message) return;
	}

	debug_messages_[current_message_count_ % message_limit_] = message;
	current_message_count_++;
}
void VKE::CustomAppConsole::Draw()
{
	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(title, &open))
	{
		ImGui::End();
		return;
	}

	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Close Console"))
			open = false;
		ImGui::EndPopup();
	}

	if (ImGui::Button("Clear")) { ClearLog(); }
	ImGui::SameLine();

	bool copy_to_clipboard = ImGui::Button("Copy");
	ImGui::SameLine();

	// Options menu
	if (ImGui::BeginPopup("Options")) {
		//ImGui::Checkbox("Auto-scroll", &AutoScroll);
		ImGui::EndPopup();
	}
	if (ImGui::Button("Options"))
		ImGui::OpenPopup("Options");
	ImGui::SameLine();

	Filter.Draw("Filter", 120);
	ImGui::Separator();

	const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
	ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::Selectable("Clear")) ClearLog();
		ImGui::EndPopup();
	}

	// Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
	// NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
	// You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
	// To use the clipper we could replace the 'for (int i = 0; i < Items.Size; i++)' loop with:
	//     ImGuiListClipper clipper(Items.Size);
	//     while (clipper.Step())
	//         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
	// However, note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
	// A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
	// and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
	// If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
	if (copy_to_clipboard)
		ImGui::LogToClipboard();

	float32 text_width = ImGui::GetWindowWidth() - 10.0f - ImGui::GetWindowWidth() * 0.025f;

	for (int i = 0; i < debug_messages_.size(); i++)
	{
		if (debug_messages_[i] == "") continue;
		const char* item = debug_messages_[i].data();
		if (!Filter.PassFilter(item))
			continue;

		// Normally you would store more information in your item (e.g. make Items[] an array of structure, store color/type etc.)
		bool pop_color = false;
		//if (strstr(item, "[ERROR]")) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(170.0f/256.0f, 16.0f/256.0f, 14.0f / 256.0f, 1.0f)); pop_color = true; }
		//if (strstr(item, "[WARN]")) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(140.0f / 256.0f, 50.0f / 256.0f, 14.0f / 256.0f, 1.0f)); pop_color = true; }
		ImGui::PushTextWrapPos(text_width);
		ImGui::Text(item, text_width);
		//ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 0, 255));
		ImGui::PopTextWrapPos();
		if (pop_color)
			ImGui::PopStyleColor();
		ImGui::NewLine();
	}
	if (copy_to_clipboard)
		ImGui::LogFinish();

	if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
		ImGui::SetScrollHereY(1.0f);

	ScrollToBottom = false;

	ImGui::PopStyleVar();
	ImGui::EndChild();
	ImGui::Separator();

	ImGui::End();
}

int VKE::CustomAppConsole::TextEditCallbackStub(ImGuiInputTextCallbackData* data) // In C++11 you are better off using lambdas for this sort of forwarding callbacks
{
	CustomAppConsole* console = (CustomAppConsole*)data->UserData;
	return console->TextEditCallback(data);
}

int VKE::CustomAppConsole::TextEditCallback(ImGuiInputTextCallbackData* data)
{
	//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
	switch (data->EventFlag)
	{
	case ImGuiInputTextFlags_CallbackCompletion:
	{
		// Example of TEXT COMPLETION

		// Locate beginning of current word
		const char* word_end = data->Buf + data->CursorPos;
		const char* word_start = word_end;
		while (word_start > data->Buf)
		{
			const char c = word_start[-1];
			if (c == ' ' || c == '\t' || c == ',' || c == ';')
				break;
			word_start--;
		}

		// Build a list of candidates
		ImVector<const char*> candidates;
		for (int i = 0; i < debug_messages_.size(); i++)
			if (Strnicmp(debug_messages_[i].data(), word_start, (int)(word_end - word_start)) == 0)
				candidates.push_back(debug_messages_[i].data());

		if (candidates.Size == 0)
		{
			// No match
			//AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
		}
		else if (candidates.Size == 1)
		{
			// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
			data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
			data->InsertChars(data->CursorPos, candidates[0]);
			data->InsertChars(data->CursorPos, " ");
		}
		else
		{
			// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
			int match_len = (int)(word_end - word_start);
			for (;;)
			{
				int c = 0;
				bool all_candidates_matches = true;
				for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
					if (i == 0)
						c = toupper(candidates[i][match_len]);
					else if (c == 0 || c != toupper(candidates[i][match_len]))
						all_candidates_matches = false;
				if (!all_candidates_matches)
					break;
				match_len++;
			}

			if (match_len > 0)
			{
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
			}

			// List matches
			//AddLog("Possible matches:\n");
			//for (int i = 0; i < candidates.Size; i++)
			//	AddLog("- %s\n", candidates[i]);
		}

		break;
	}
	case ImGuiInputTextFlags_CallbackHistory:
	{
		// Example of HISTORY
		const int prev_history_pos = HistoryPos;
		if (data->EventKey == ImGuiKey_UpArrow)
		{
			if (HistoryPos == -1)
				HistoryPos = History.Size - 1;
			else if (HistoryPos > 0)
				HistoryPos--;
		}
		else if (data->EventKey == ImGuiKey_DownArrow)
		{
			if (HistoryPos != -1)
				if (++HistoryPos >= History.Size)
					HistoryPos = -1;
		}

		// A better implementation would preserve the data on the current input line along with cursor position.
		if (prev_history_pos != HistoryPos)
		{
			const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
			data->DeleteChars(0, data->BufTextLen);
			data->InsertChars(0, history_str);
		}
	}
	}
	return 0;
}


void VKE::AppUniformRenderData::Draw()
{
	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(title, &open))
	{
		ImGui::End();
		return;
	}

	ImGui::SameLine(ImGui::GetWindowWidth() * 0.05f);
	ImGui::Text("General Rendering variables for the engine");
	ImGui::Separator();
	ImGui::NewLine();

	
	ImGui::SliderFloat("Sun Trajectory", &sun_angle_, -glm::half_pi<float32>(), glm::half_pi<float32>());

	static float32 color[3] = { 1.0f,0.88f,0.67f };
	ImGui::ColorEdit3("Light Color", color);
	light_color_ = glm::vec3(color[0], color[1], color[2]);

	ImGui::DragFloat("Sun Light Intensity", &light_intensity_, 0.02f, 0.0f, 10000.0f);
	ImGui::DragFloat("Ambient Lighting", &ambient_, 0.001f, 0.0f, 1.0f);
	ImGui::DragFloat("HDR Exposure", &exposure_, 0.01f);

	ImGui::SliderFloat("Shininess Exponential", &shininess_exponential_, 2.0f, 256.0f, "%.0f", 2.0f);


	char* debug_types[] = { "None","Normals", "Light Intensity", "Specular Light" };
	ImGui::Combo("Debug Shader Visualizations", &debug_type_, debug_types, 4);
	ImGui::NewLine();

	ImGui::Separator();
	ImVec2 button_size = ImVec2(ImGui::GetWindowWidth() - 10.0f, 0.0f);
	should_recreate_shaders = ImGui::Button("Update Shaders", button_size);

	ImGui::End();
}

VKE::AppHierarchyDetails::AppHierarchyDetails(VKE::HierarchyContext* hier_ctx) {
	hier_ctx_ = hier_ctx;
}

void VKE::AppHierarchyDetails::Draw() {
	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(title, &open))
	{
		ImGui::End();
		return;
	}
	static int32 clicked_entity = -1;
	static std::vector<bool> entities_open;
	std::vector<VKE::ui_data>& entities = hier_ctx_->getUIData();

	if (entities_open.size() != entities.size()) {
		entities_open.resize(entities.size(), false);
	}

	ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | 
		ImGuiTreeNodeFlags_OpenOnDoubleClick | 
		ImGuiTreeNodeFlags_SpanAvailWidth |
		ImGuiTreeNodeFlags_FramePadding;

	if (ImGui::IsMouseClicked(0) && ImGui::GetIO().KeyCtrl) clicked_entity = -1;

	uint32 it = 0;
	if (ImGui::TreeNodeEx((void*)(intptr_t)it, base_flags | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth, "Entities")) {

		ImGui::Separator();

		ImGui::Columns(2, "Names", true);
		for (auto& entity_data : entities) {

			if(it == clicked_entity)
				entities_open[it] = ImGui::TreeNodeEx((void*)(intptr_t)it, base_flags | ImGuiTreeNodeFlags_Selected, entity_data.entity_name);
			else
				entities_open[it] = ImGui::TreeNodeEx((void*)(intptr_t)it, base_flags, entity_data.entity_name);

			if (ImGui::IsItemClicked()) clicked_entity = it;

			ImGui::NextColumn();
			std::string comps_trans = std::to_string(entity_data.num_transform) + " Transform  |";
			std::string comps_rend = std::to_string(entity_data.num_render) + " Renderer";
			
			ImGui::AlignTextToFramePadding();  ImGui::Text(comps_trans.data());
			ImGui::SameLine(0.0f, 10.0f);
			ImGui::AlignTextToFramePadding();  ImGui::Text(comps_rend.data());


			ImGui::NextColumn();
			if (entities_open[it]) ImGui::TreePop();
			ImGui::Separator();
			it++;
		}

		ImGui::TreePop();
	}


	ImGui::End();
}
