#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "soloud.h"
#include "soloud_wav.h"



// ���������� ������ GLFW
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

struct XYZ
{
    float x, y, z;
};

void test2_3DPositioning(SoLoud::Soloud& soloud, SoLoud::Wav& sample, XYZ& pos)
{
    std::cout << "\n=== ���� 2: 3D ���������������� ===" << std::endl;

    // ����������� ��������� � ������ ���������
    soloud.set3dListenerParameters(
        0, 0, 0,      // ������� ��������� (x, y, z)
        0, 0, -1,     // ����������� ������� (forward vector)
        0, 1, 0       // ������ "�����" (up vector)
    );

    // ��������� ������ �������
    std::cout << " (x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z << ")" << std::endl;

        // ������������� ���� � 3D �������
        int handle = soloud.play3d(sample, pos.x, pos.y, pos.z);
    
}

int main()
{
    // ������������� GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return -1;

    const char* glsl_version = "#version 330"; // ������ GLSL
    // ������� ����
    GLFWwindow* window = glfwCreateWindow(800, 600, "ImGui minimal example", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // �������� V-SYNC

    // ������������� glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize glad\n");
        return -1;
    }

    // ������������� ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // ��������� ������
    ImGui::StyleColorsDark();

    // ������������� ImGui ��� GLFW � OpenGL3
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    SoLoud::Soloud soloud;  // �������� ������ �����������
    soloud.init();

    SoLoud::Wav sample;
    sample.load("sound.wav");
    sample.set3dMinMaxDistance(1.0f, 1000.0f);  // ����������� � ������������ ���������
    sample.set3dAttenuation(SoLoud::AudioSource::LINEAR_DISTANCE, 1.0f);  // ��� ���������

    XYZ pos{10, 0, 0};

    soloud.set3dListenerParameters(
        0, 0, 0,      // ������� ��������� (x, y, z)
        0, 0, -1,     // ����������� ������� (forward vector)
        0, 1, 0       // ������ "�����" (up vector)
    );

    sample.setLooping(true);

    int handle = soloud.play3d(sample, pos.x, pos.y, pos.z);
    std::cout << "play sound\n";

    //std::this_thread::sleep_for(std::chrono::seconds(7));

    //std::cout << "set play position on -10 0 0\n";
    //pos.x = -10;
    //soloud.set3dSourceParameters(handle, pos.x, pos.y, pos.z);
    //soloud.update3dAudio();


    // �������� ����
    while (!glfwWindowShouldClose(window))
    {
        // ��������� �������
        glfwPollEvents();

        // �������� ����� ���� ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ������� ���� ImGui
        {
            ImGui::Begin("���������� ������");
            ImGui::SliderFloat3("XYZ:", &pos.x, -1000, 1000);
            ImGui::Text("��� ������ ImGui");
            ImGui::End();
        }

        std::cout << "set pos on X: " << pos.x << "  Y: " << pos.y << "  Z: " << pos.z << std::endl;
        soloud.set3dSourceParameters(handle, pos.x, pos.y, pos.z);
        soloud.update3dAudio();

        // ������ �����
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        // ������ ImGui
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    soloud.deinit();

    // �������
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}