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



// Обработчик ошибок GLFW
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
    std::cout << "\n=== ТЕСТ 2: 3D позиционирование ===" << std::endl;

    // Настраиваем слушателя в центре координат
    soloud.set3dListenerParameters(
        0, 0, 0,      // позиция слушателя (x, y, z)
        0, 0, -1,     // направление взгляда (forward vector)
        0, 1, 0       // вектор "вверх" (up vector)
    );

    // Тестируем разные позиции
    std::cout << " (x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z << ")" << std::endl;

        // Воспроизводим звук в 3D позиции
        int handle = soloud.play3d(sample, pos.x, pos.y, pos.z);
    
}

int main()
{
    // Инициализация GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return -1;

    const char* glsl_version = "#version 330"; // Версия GLSL
    // Создаем окно
    GLFWwindow* window = glfwCreateWindow(800, 600, "ImGui minimal example", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Включить V-SYNC

    // Инициализация glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize glad\n");
        return -1;
    }

    // Инициализация ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Настройка стилей
    ImGui::StyleColorsDark();

    // Инициализация ImGui для GLFW и OpenGL3
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    SoLoud::Soloud soloud;  // Основной объект аудиодвижка
    soloud.init();

    SoLoud::Wav sample;
    sample.load("sound.wav");
    sample.set3dMinMaxDistance(1.0f, 1000.0f);  // Минимальная и максимальная дистанция
    sample.set3dAttenuation(SoLoud::AudioSource::LINEAR_DISTANCE, 1.0f);  // Тип затухания

    XYZ pos{10, 0, 0};

    soloud.set3dListenerParameters(
        0, 0, 0,      // позиция слушателя (x, y, z)
        0, 0, -1,     // направление взгляда (forward vector)
        0, 1, 0       // вектор "вверх" (up vector)
    );

    sample.setLooping(true);

    int handle = soloud.play3d(sample, pos.x, pos.y, pos.z);
    std::cout << "play sound\n";

    //std::this_thread::sleep_for(std::chrono::seconds(7));

    //std::cout << "set play position on -10 0 0\n";
    //pos.x = -10;
    //soloud.set3dSourceParameters(handle, pos.x, pos.y, pos.z);
    //soloud.update3dAudio();


    // Основной цикл
    while (!glfwWindowShouldClose(window))
    {
        // Обработка событий
        glfwPollEvents();

        // Начинаем новый кадр ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Создаем окно ImGui
        {
            ImGui::Begin("Простейший виджет");
            ImGui::SliderFloat3("XYZ:", &pos.x, -1000, 1000);
            ImGui::Text("Это пример ImGui");
            ImGui::End();
        }

        std::cout << "set pos on X: " << pos.x << "  Y: " << pos.y << "  Z: " << pos.z << std::endl;
        soloud.set3dSourceParameters(handle, pos.x, pos.y, pos.z);
        soloud.update3dAudio();

        // Раздел кадра
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Рисуем ImGui
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    soloud.deinit();

    // Очистка
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}