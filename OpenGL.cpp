#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include "OpenGL.h"
#include "Repacking.h"

using namespace std;

void init(GLFWwindow* window){}

static float red = 1.0f;
static float green = 1.0f;
static float blue = 1.0f;
static float alpha = 1.0f;

void draw()
{
    glClearColor(red, green, blue, alpha);
    glClear(GL_COLOR_BUFFER_BIT);
        
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    ImGui::Begin("Color", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowSize(ImVec2(display_w,display_h));
    
    //ImGui::ColorPicker4("##Color##5", reinterpret_cast<float*>(&color), ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);
    //red = color.x;
    //green = color.y;
    //blue = color.z;
    //alpha = color.w;
    ImGui::Text("Chameleon's texture and sound repacker");
    ImGui::Dummy(ImVec2(0, 20));
    
    if (ImGui::Button("choose game folder"))
    {
        cout << "choosing game folder..\n";
    }
    ImGui::SameLine();
    ImGui::InputText("Path", hot_file_path, IM_ARRAYSIZE(hot_file_path));
    ImGui::Text(hot_file_path);

    ImGui::Dummy(ImVec2(0,30));
    
    if (ImGui::RadioButton("textures", &e, 0))
    {
        repacking_mode = "textures";
    }
    if (ImGui::RadioButton("sounds", &e, 1))
    {
        repacking_mode = "sounds";
    }
    
    ImGui::Dummy(ImVec2(0,30));
    
    if (ImGui::Button("Repack"))
    {
        repack();
    }

    ImGui::Dummy(ImVec2(0,30));
    
    ImGui::ProgressBar(level_progress, ImVec2(0, 0));
    ImGui::SameLine();
    ImGui::Text("Repack progress");

    ImGui::ProgressBar(files_progress, ImVec2(0, 0));
    ImGui::SameLine();
    ImGui::Text("Files progress");
    
    ImGui::End();
    ImGui::Render();
    
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
    glfwSwapBuffers(window);
    glfwPollEvents();
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    display_w = (float)width;
    display_h = (float)height;
    draw();
}

int main(void)
{
    if(!glfwInit()){ exit(EXIT_FAILURE); }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    display_w = 800;
    display_h = 600;
    window = glfwCreateWindow((int)display_w, int(display_h), "Chapter2 - program1", nullptr,nullptr);
    glfwMakeContextCurrent(window);
    glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);  // disables resizing
    if(glewInit() != GLEW_OK){ exit(EXIT_FAILURE); }
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    init(window);
    
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    repacking_mode = "texture";
    init();
    
    while(!glfwWindowShouldClose(window))
    {
        draw();
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown(); 

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
