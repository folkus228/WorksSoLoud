#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Обработчик ошибок GLFW
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
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
            if (ImGui::Button("Нажми меня"))
            {
                printf("Кнопка нажата!\n");
            }
            ImGui::Text("Это пример ImGui");
            ImGui::End();
        }

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

    // Очистка
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}