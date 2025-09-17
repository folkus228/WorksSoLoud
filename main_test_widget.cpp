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
    SoLoud::Soloud soloud;  // �������� ������ �����������
    SoLoud::Wav sample;     // ������ ��� �������� WAV �����

public:
    bool initialize() {
        std::cout << "=== ������������� SoLoud ===" << std::endl;

        // �������������� �����������
        SoLoud::result result = soloud.init();
        if (result != SoLoud::SO_NO_ERROR) {
            std::cerr << "������ ������������� SoLoud: " << result << std::endl;
            return false;
        }

        std::cout << "SoLoud ������� ���������������" << std::endl;
        std::cout << "������: " << soloud.getVersion() << std::endl;
        std::cout << "Backend: " << soloud.getBackendString() << std::endl;
        return true;
    }

    bool loadSound(const char* filename) {
        std::cout << "\n=== �������� ��������� ����� ===" << std::endl;

        // ��������� WAV ����
        SoLoud::result result = sample.load(filename);
        if (result != SoLoud::SO_NO_ERROR) {
            std::cerr << "������ �������� ����� " << filename << ": " << result << std::endl;
            return false;
        }

        std::cout << "���� " << filename << " ������� ��������" << std::endl;
        std::cout << "������������: " << sample.getLength() << " ������" << std::endl;

        // ������������� ���� ��� 3D ���������
        sample.set3dMinMaxDistance(1.0f, 1000.0f);  // ����������� � ������������ ���������
        sample.set3dAttenuation(SoLoud::AudioSource::LINEAR_DISTANCE, 1.0f);  // ��� ���������

        return true;
    }

    void test1_BasicPlayback() {
        std::cout << "\n=== ���� 1: ������� ��������������� ===" << std::endl;
        std::cout << "������������� ���� ��� 3D ��������..." << std::endl;

        // ������� ��������������� ��� 3D
        int handle = soloud.play(sample);

        // ���� ���������� ���������������
        while (soloud.isValidVoiceHandle(handle)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::cout << "������� ��������������� ���������" << std::endl;
    }

    void test2_3DPositioning() {
        std::cout << "\n=== ���� 2: 3D ���������������� ===" << std::endl;

        // ����������� ��������� � ������ ���������
        soloud.set3dListenerParameters(
            0, 0, 0,      // ������� ��������� (x, y, z)
            0, 0, -1,     // ����������� ������� (forward vector)
            0, 1, 0       // ������ "�����" (up vector)
        );

        // ��������� ������ �������
        struct TestPosition {
            float x, y, z;
            const char* description;
        };

        TestPosition positions[] = {
            {-5.0f, 0.0f, 0.0f, "�����"},
            {5.0f, 0.0f, 0.0f, "������"},
            {0.0f, 0.0f, -5.0f, "�������"},
            {0.0f, 0.0f, 5.0f, "�����"},
            {0.0f, 5.0f, 0.0f, "������"},
            {-3.0f, 2.0f, -2.0f, "�����-������-�������"}
        };

        for (const auto& pos : positions) {
            std::cout << "���������������: " << pos.description
                << " (x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z << ")" << std::endl;

            // ������������� ���� � 3D �������
            int handle = soloud.play3d(sample, pos.x, pos.y, pos.z);

            // ���� 2 �������
            std::this_thread::sleep_for(std::chrono::seconds(2));

            // ������������� ����
            soloud.stop(handle);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    void test3_MovingSound() {
        std::cout << "\n=== ���� 3: ����������� ��������� ����� ===" << std::endl;
        std::cout << "���� ����� ������������ �� ����� ������ ���������..." << std::endl;

        // ��������� ����������� ���������������
        sample.setLooping(true);
        int handle = soloud.play3d(sample, 5.0f, 0.0f, 0.0f);

        // ������� �������� ����� �� �����
        const float radius = 5.0f;
        const float duration = 20.0f; // ������ �� ������ ������
        const float steps = 200;
        const float angleStep = (2.0f * 3.14159f) / steps;
        const float timeStep = duration / steps;

        for (int i = 0; i < steps; ++i) {
            float angle = i * angleStep;
            float x = radius * cos(angle);
            float z = radius * sin(angle);

            // ��������� ������� �����
            soloud.set3dSourceParameters(handle, x, 0.0f, z);

            std::this_thread::sleep_for(std::chrono::milliseconds((int)(timeStep * 1000)));
        }

        // ������������� ����������� ����
        sample.setLooping(false);
        soloud.stop(handle);

        std::cout << "�������� �������� ���������" << std::endl;
    }

    void test4_ComplexMovement() {
        std::cout << "\n=== ���� 4: ������� ����������� ===" << std::endl;
        std::cout << "���� ������������ �� ������� � ���������� ������..." << std::endl;

        sample.setLooping(true);
        int handle = soloud.play3d(sample, 0.0f, -5.0f, 0.0f);

        const float maxRadius = 8.0f;
        const float duration = 6.0f;
        const float steps = 120;
        const float timeStep = duration / steps;

        for (int i = 0; i < steps; ++i) {
            float progress = (float)i / steps;
            float angle = progress * 4 * 3.14159f; // 2 ������ �������
            float radius = maxRadius * progress;
            float height = -5.0f + 10.0f * progress; // �� -5 �� +5

            float x = radius * cos(angle);
            float z = radius * sin(angle);

            soloud.set3dSourceParameters(handle, x, height, z);

            // ����� ����� �������� �������� ��������������� ��� ������� �������
            float speed = 0.8f + 0.4f * sin(angle); // �� 0.8 �� 1.2
            soloud.setRelativePlaySpeed(handle, speed);

            std::this_thread::sleep_for(std::chrono::milliseconds((int)(timeStep * 1000)));
        }

        sample.setLooping(false);
        soloud.stop(handle);

        std::cout << "���������� �������� ���������" << std::endl;
    }

    void test5_MultipleSourcesAndListener() {
        std::cout << "\n=== ���� 5: ��������� ���������� + �������� ��������� ===" << std::endl;
        std::cout << "��������� 3 ��������� ����� � ������� ���������..." << std::endl;

        // ��������� ��������� ���������� � ������ ��������
        sample.setLooping(true);
        int handle1 = soloud.play3d(sample, -8.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f); // �����, ����
        int handle2 = soloud.play3d(sample, 8.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f);  // ������, ����
        int handle3 = soloud.play3d(sample, 0.0f, 0.0f, -8.0f, 0.0f, 0.0f, 0.0f, 0.3f); // �������, ����

        // ���������� ��������� ����� �����������
        const float duration = 8.0f;
        const float steps = 80;
        const float timeStep = duration / steps;

        for (int i = 0; i < steps; ++i) {
            float progress = (float)i / steps;
            float angle = progress * 2 * 3.14159f;

            float listenerX = 4.0f * cos(angle);
            float listenerZ = 4.0f * sin(angle);

            // ������������ ��������� � ������
            float forwardX = -listenerX;
            float forwardZ = -listenerZ;
            float length = sqrt(forwardX * forwardX + forwardZ * forwardZ);
            forwardX /= length;
            forwardZ /= length;

            soloud.set3dListenerParameters(
                listenerX, 0.0f, listenerZ,     // �������
                forwardX, 0.0f, forwardZ,       // �����������
                0.0f, 1.0f, 0.0f                // �����
            );

            std::this_thread::sleep_for(std::chrono::milliseconds((int)(timeStep * 1000)));
        }

        // ������������� ��� ���������
        sample.setLooping(false);
        soloud.stop(handle1);
        soloud.stop(handle2);
        soloud.stop(handle3);

        // ���������� ��������� � �����
        soloud.set3dListenerParameters(0, 0, 0, 0, 0, -1, 0, 1, 0);

        std::cout << "���� � ����������� ����������� ��������" << std::endl;
    }

    void test6_AdvancedEffects() {
        std::cout << "\n=== ���� 6: �������������� ������� ===" << std::endl;

        // ������������� ��������� ��������� 3D �����
        struct EffectTest {
            const char* name;
            float minDist, maxDist, rolloff;
            SoLoud::AudioSource::ATTENUATION_MODELS model;
        };

        EffectTest effects[] = {
            {"�������� ���������", 1.0f, 10.0f, 1.0f, SoLoud::AudioSource::LINEAR_DISTANCE},
            {"���������������� ���������", 1.0f, 10.0f, 1.0f, SoLoud::AudioSource::EXPONENTIAL_DISTANCE},
            {"�������� ���������", 1.0f, 10.0f, 1.0f, SoLoud::AudioSource::INVERSE_DISTANCE}
        };

        for (const auto& effect : effects) {
            std::cout << "���������: " << effect.name << std::endl;

            // ����������� ��������� ���������
            sample.set3dMinMaxDistance(effect.minDist, effect.maxDist);
            sample.set3dAttenuation(effect.model, effect.rolloff);

            // ������������� ����, ������ ��� �� �������� � ��������
            int handle = soloud.play3d(sample, 2.0f, 0.0f, 0.0f);

            for (int i = 0; i < 30; ++i) {
                float distance = 2.0f + i * 0.5f; // �� 2 �� 16.5
                soloud.set3dSourceParameters(handle, distance, 0.0f, 0.0f);
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }

            soloud.stop(handle);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    void runAllTests() {
        std::cout << "�������� ������������ ������������ SoLoud 3D Audio" << std::endl;
        std::cout << "���������, ��� ����������� �������� ��� ������ �������!" << std::endl;
        std::cout << "\n������� Enter ��� �����������...";
        std::cin.get();

        test1_BasicPlayback();

        std::cout << "\n������� Enter ��� ���������� �����...";
        std::cin.get();
        test2_3DPositioning();

        std::cout << "\n������� Enter ��� ���������� �����...";
        std::cin.get();
        test3_MovingSound();

        std::cout << "\n������� Enter ��� ���������� �����...";
        std::cin.get();
        test4_ComplexMovement();

        std::cout << "\n������� Enter ��� ���������� �����...";
        std::cin.get();
        test5_MultipleSourcesAndListener();

        std::cout << "\n������� Enter ��� ���������� �����...";
        std::cin.get();
        test6_AdvancedEffects();

        std::cout << "\n=== ������������ ��������� ===" << std::endl;
    }

    ~SoLoud3DDemo() {
        // ������� �������
        soloud.deinit();
        std::cout << "SoLoud �����������������" << std::endl;
    }
};

// ���������� ������ GLFW
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
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
            if (ImGui::Button("����� ����"))
            {
                printf("������ ������!\n");
            }
            ImGui::Text("��� ������ ImGui");
            ImGui::End();
        }

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

    // �������
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}