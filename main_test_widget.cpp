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

class SoLoud3DDemo {
private:
    SoLoud::Soloud soloud;  // Основной объект аудиодвижка
    SoLoud::Wav sample;     // Объект для загрузки WAV файла

public:
    bool initialize() {
        std::cout << "=== Инициализация SoLoud ===" << std::endl;

        // Инициализируем аудиодвижок
        SoLoud::result result = soloud.init();
        if (result != SoLoud::SO_NO_ERROR) {
            std::cerr << "Ошибка инициализации SoLoud: " << result << std::endl;
            return false;
        }

        std::cout << "SoLoud успешно инициализирован" << std::endl;
        std::cout << "Версия: " << soloud.getVersion() << std::endl;
        std::cout << "Backend: " << soloud.getBackendString() << std::endl;
        return true;
    }

    bool loadSound(const char* filename) {
        std::cout << "\n=== Загрузка звукового файла ===" << std::endl;

        // Загружаем WAV файл
        SoLoud::result result = sample.load(filename);
        if (result != SoLoud::SO_NO_ERROR) {
            std::cerr << "Ошибка загрузки файла " << filename << ": " << result << std::endl;
            return false;
        }

        std::cout << "Файл " << filename << " успешно загружен" << std::endl;
        std::cout << "Длительность: " << sample.getLength() << " секунд" << std::endl;

        // Устанавливаем флаг для 3D обработки
        sample.set3dMinMaxDistance(1.0f, 1000.0f);  // Минимальная и максимальная дистанция
        sample.set3dAttenuation(SoLoud::AudioSource::LINEAR_DISTANCE, 1.0f);  // Тип затухания

        return true;
    }

    void test1_BasicPlayback() {
        std::cout << "\n=== ТЕСТ 1: Базовое воспроизведение ===" << std::endl;
        std::cout << "Воспроизводим звук без 3D эффектов..." << std::endl;

        // Простое воспроизведение без 3D
        int handle = soloud.play(sample);

        // Ждем завершения воспроизведения
        while (soloud.isValidVoiceHandle(handle)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::cout << "Базовое воспроизведение завершено" << std::endl;
    }

    void test2_3DPositioning() {
        std::cout << "\n=== ТЕСТ 2: 3D позиционирование ===" << std::endl;

        // Настраиваем слушателя в центре координат
        soloud.set3dListenerParameters(
            0, 0, 0,      // позиция слушателя (x, y, z)
            0, 0, -1,     // направление взгляда (forward vector)
            0, 1, 0       // вектор "вверх" (up vector)
        );

        // Тестируем разные позиции
        struct TestPosition {
            float x, y, z;
            const char* description;
        };

        TestPosition positions[] = {
            {-5.0f, 0.0f, 0.0f, "Слева"},
            {5.0f, 0.0f, 0.0f, "Справа"},
            {0.0f, 0.0f, -5.0f, "Спереди"},
            {0.0f, 0.0f, 5.0f, "Сзади"},
            {0.0f, 5.0f, 0.0f, "Сверху"},
            {-3.0f, 2.0f, -2.0f, "Слева-сверху-спереди"}
        };

        for (const auto& pos : positions) {
            std::cout << "Воспроизведение: " << pos.description
                << " (x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z << ")" << std::endl;

            // Воспроизводим звук в 3D позиции
            int handle = soloud.play3d(sample, pos.x, pos.y, pos.z);

            // Ждем 2 секунды
            std::this_thread::sleep_for(std::chrono::seconds(2));

            // Останавливаем звук
            soloud.stop(handle);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    void test3_MovingSound() {
        std::cout << "\n=== ТЕСТ 3: Перемещение источника звука ===" << std::endl;
        std::cout << "Звук будет перемещаться по кругу вокруг слушателя..." << std::endl;

        // Запускаем зацикленное воспроизведение
        sample.setLooping(true);
        int handle = soloud.play3d(sample, 5.0f, 0.0f, 0.0f);

        // Двигаем источник звука по кругу
        const float radius = 5.0f;
        const float duration = 20.0f; // секунд на полный оборот
        const float steps = 200;
        const float angleStep = (2.0f * 3.14159f) / steps;
        const float timeStep = duration / steps;

        for (int i = 0; i < steps; ++i) {
            float angle = i * angleStep;
            float x = radius * cos(angle);
            float z = radius * sin(angle);

            // Обновляем позицию звука
            soloud.set3dSourceParameters(handle, x, 0.0f, z);

            std::this_thread::sleep_for(std::chrono::milliseconds((int)(timeStep * 1000)));
        }

        // Останавливаем зацикленный звук
        sample.setLooping(false);
        soloud.stop(handle);

        std::cout << "Круговое движение завершено" << std::endl;
    }

    void test4_ComplexMovement() {
        std::cout << "\n=== ТЕСТ 4: Сложное перемещение ===" << std::endl;
        std::cout << "Звук перемещается по спирали с изменением высоты..." << std::endl;

        sample.setLooping(true);
        int handle = soloud.play3d(sample, 0.0f, -5.0f, 0.0f);

        const float maxRadius = 8.0f;
        const float duration = 6.0f;
        const float steps = 120;
        const float timeStep = duration / steps;

        for (int i = 0; i < steps; ++i) {
            float progress = (float)i / steps;
            float angle = progress * 4 * 3.14159f; // 2 полных оборота
            float radius = maxRadius * progress;
            float height = -5.0f + 10.0f * progress; // от -5 до +5

            float x = radius * cos(angle);
            float z = radius * sin(angle);

            soloud.set3dSourceParameters(handle, x, height, z);

            // Также можем изменять скорость воспроизведения для эффекта Доплера
            float speed = 0.8f + 0.4f * sin(angle); // от 0.8 до 1.2
            soloud.setRelativePlaySpeed(handle, speed);

            std::this_thread::sleep_for(std::chrono::milliseconds((int)(timeStep * 1000)));
        }

        sample.setLooping(false);
        soloud.stop(handle);

        std::cout << "Спиральное движение завершено" << std::endl;
    }

    void test5_MultipleSourcesAndListener() {
        std::cout << "\n=== ТЕСТ 5: Несколько источников + движение слушателя ===" << std::endl;
        std::cout << "Запускаем 3 источника звука и двигаем слушателя..." << std::endl;

        // Запускаем несколько источников в разных позициях
        sample.setLooping(true);
        int handle1 = soloud.play3d(sample, -8.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f); // слева, тише
        int handle2 = soloud.play3d(sample, 8.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f);  // справа, тише
        int handle3 = soloud.play3d(sample, 0.0f, 0.0f, -8.0f, 0.0f, 0.0f, 0.0f, 0.3f); // спереди, тише

        // Перемещаем слушателя между источниками
        const float duration = 8.0f;
        const float steps = 80;
        const float timeStep = duration / steps;

        for (int i = 0; i < steps; ++i) {
            float progress = (float)i / steps;
            float angle = progress * 2 * 3.14159f;

            float listenerX = 4.0f * cos(angle);
            float listenerZ = 4.0f * sin(angle);

            // Поворачиваем слушателя к центру
            float forwardX = -listenerX;
            float forwardZ = -listenerZ;
            float length = sqrt(forwardX * forwardX + forwardZ * forwardZ);
            forwardX /= length;
            forwardZ /= length;

            soloud.set3dListenerParameters(
                listenerX, 0.0f, listenerZ,     // позиция
                forwardX, 0.0f, forwardZ,       // направление
                0.0f, 1.0f, 0.0f                // вверх
            );

            std::this_thread::sleep_for(std::chrono::milliseconds((int)(timeStep * 1000)));
        }

        // Останавливаем все источники
        sample.setLooping(false);
        soloud.stop(handle1);
        soloud.stop(handle2);
        soloud.stop(handle3);

        // Возвращаем слушателя в центр
        soloud.set3dListenerParameters(0, 0, 0, 0, 0, -1, 0, 1, 0);

        std::cout << "Тест с несколькими источниками завершен" << std::endl;
    }

    void test6_AdvancedEffects() {
        std::cout << "\n=== ТЕСТ 6: Дополнительные эффекты ===" << std::endl;

        // Демонстрируем различные параметры 3D звука
        struct EffectTest {
            const char* name;
            float minDist, maxDist, rolloff;
            SoLoud::AudioSource::ATTENUATION_MODELS model;
        };

        EffectTest effects[] = {
            {"Линейное затухание", 1.0f, 10.0f, 1.0f, SoLoud::AudioSource::LINEAR_DISTANCE},
            {"Экспоненциальное затухание", 1.0f, 10.0f, 1.0f, SoLoud::AudioSource::EXPONENTIAL_DISTANCE},
            {"Обратное затухание", 1.0f, 10.0f, 1.0f, SoLoud::AudioSource::INVERSE_DISTANCE}
        };

        for (const auto& effect : effects) {
            std::cout << "Тестируем: " << effect.name << std::endl;

            // Настраиваем параметры затухания
            sample.set3dMinMaxDistance(effect.minDist, effect.maxDist);
            sample.set3dAttenuation(effect.model, effect.rolloff);

            // Воспроизводим звук, двигая его от близкого к далекому
            int handle = soloud.play3d(sample, 2.0f, 0.0f, 0.0f);

            for (int i = 0; i < 30; ++i) {
                float distance = 2.0f + i * 0.5f; // от 2 до 16.5
                soloud.set3dSourceParameters(handle, distance, 0.0f, 0.0f);
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }

            soloud.stop(handle);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    void runAllTests() {
        std::cout << "Начинаем демонстрацию возможностей SoLoud 3D Audio" << std::endl;
        std::cout << "Убедитесь, что используете наушники или стерео колонки!" << std::endl;
        std::cout << "\nНажмите Enter для продолжения...";
        std::cin.get();

        test1_BasicPlayback();

        std::cout << "\nНажмите Enter для следующего теста...";
        std::cin.get();
        test2_3DPositioning();

        std::cout << "\nНажмите Enter для следующего теста...";
        std::cin.get();
        test3_MovingSound();

        std::cout << "\nНажмите Enter для следующего теста...";
        std::cin.get();
        test4_ComplexMovement();

        std::cout << "\nНажмите Enter для следующего теста...";
        std::cin.get();
        test5_MultipleSourcesAndListener();

        std::cout << "\nНажмите Enter для следующего теста...";
        std::cin.get();
        test6_AdvancedEffects();

        std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    }

    ~SoLoud3DDemo() {
        // Очищаем ресурсы
        soloud.deinit();
        std::cout << "SoLoud деинициализирован" << std::endl;
    }
};

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