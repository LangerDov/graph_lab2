#include "imgui_window.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <string.h>
#include <stdio.h>

const char* glsl_version = "#version 130";
bool show_demo_window = true;

void ImguiWin::UpdateFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Another Window", &show_demo_window);
	char fps[20];
	sprintf(fps, "%f", ImGui::GetIO().Framerate);
	ImGui::DragFloat("sound speed", val,0.001f);
	ImGui::Text(fps);
	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImguiWin::RenderInit(void* data)
{
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)data, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
}