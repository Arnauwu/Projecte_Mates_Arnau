#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#include "Matrix4x4.hpp"
#include "Matrix3x3.hpp"
#include "utils/Mesh.hpp"          
#include "utils/GraphicsUtils.hpp" 

#include "Transform.hpp"
#include "GameObject.hpp"
#include "Camera.hpp"

// -----------------------------------------------------------------------------
// HELPER: Càrrega de fitxers de text (per Shaders)
// -----------------------------------------------------------------------------
std::string LoadShaderFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open shader file: " << filepath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// -----------------------------------------------------------------------------
// HELPER: Compilació de Shaders
// -----------------------------------------------------------------------------
GLuint CompileShader(GLenum type, const std::string& source) {
    const char* srcPtr = source.c_str();
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &srcPtr, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return shader;
}

GLuint CreateShaderProgram(const std::string& vertPath, const std::string& fragPath) {
    std::string vertCode = LoadShaderFile(vertPath);
    std::string fragCode = LoadShaderFile(fragPath);

    if (vertCode.empty() || fragCode.empty()) return 0;

    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertCode);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragCode);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

// -----------------------------------------------------------------------------
// UI: (TODO)
// -----------------------------------------------------------------------------
GameObject* selectedObject = nullptr;

void DrawHierarchyNode(GameObject* node) {
    if (!node) return;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (node == selectedObject) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    // Si l'objecte no té fills (leaf), fer servir aquest codi:
    if (node->isLeaf())
    {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        ImGui::TreeNodeEx((void*)(intptr_t)node, flags, "%s", node->GetName().c_str());
        if (ImGui::IsItemClicked()) selectedObject = node;
    }
    else // Si l'objecte té fills, fer servir aquest codi:
    {
        bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)node, flags, "%s", node->GetName().c_str());
        if (ImGui::IsItemClicked()) selectedObject = node;

        if (nodeOpen) {
            for (int i = 0; i < node->GetChilds().size(); i++)
            {
                DrawHierarchyNode(node->GetChilds()[i]);
            }

            ImGui::TreePop();
        }
    }
}

// -----------------------------------------------------------------------------
// RENDER (TODO)
// -----------------------------------------------------------------------------
void RenderNode(GameObject* node, GLuint shaderProgram, const Matrix4x4& view, const Matrix4x4& proj, Mesh& mesh) {
    if (!node) return;

    // TODO: Implementar el recorregut recursiu de renderitzat
    // 1. Calcular la matriu Model (Global) de l'objecte actual.
    Matrix4x4 model = node->GetGlobalMatrix();
    // 2. Enviar les matrius Model, View i Projection al shader (usant GraphicsUtils).
    GraphicsUtils::UploadMVP(shaderProgram, model, view, proj);
    // 3. Enviar color (usant GraphicsUtils).
    GraphicsUtils::UploadColor(shaderProgram, { 1.0f, 1.0f, 1.0f });
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
    //mesh.Draw();
    GraphicsUtils::UploadColor(shaderProgram, { 0.0f, 0.0f, 0.0f }); 
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-1.0f, -1.0f);
    // 4. Dibuixar la mesh.
    mesh.Draw();
    // 5. Cridar recursivament RenderNode pels fills.
    for (int i = 0; i < node->GetChilds().size(); i++)
    {
        RenderNode(node->GetChilds()[i], shaderProgram, view, proj, mesh);
    }
}

// -----------------------------------------------------------------------------
// MAIN (TODO)
// -----------------------------------------------------------------------------
int main(int argc, char** argv) {
    // 1. Setup SDL & OpenGL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Configuració de context OpenGL 3.3 Core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window* window = SDL_CreateWindow("Project: Mini-Scene 3D", 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) return 1;

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1); // VSync

    if (glewInit() != GLEW_OK) return 1;

    glEnable(GL_DEPTH_TEST);

    // 2. Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 3. Inicialització de recursos
    Mesh cubeMesh;
    cubeMesh.InitCube();

    // TODO: Assegureu-vos de tenir els fitxers vs.glsl i fs.glsl al mateix nivell de l'executable
    GLuint shaderProgram = CreateShaderProgram("vs.glsl", "fs.glsl");
    if (shaderProgram == 0) std::cerr << "Warning: Shaders not loaded properly." << std::endl;

    // 4. TODO: Preparar escena Inicial
    Transform rootTransform;
    GameObject* rootObject = new GameObject(rootTransform, nullptr); // root no tiene padre
    rootObject->SetName("Root Node");
    GameObject* firstChild = new GameObject({ Vec3(1,1,1), Vec3(1,1,1), Vec3(1,1,1) }, rootObject);
    firstChild->SetName("Initial Child");
    rootObject->AddChild(firstChild);

    std::vector<GameObject*> sceneRoots = { rootObject };

    Vec3 initialCamPos = { 0.0f, 0.0f, 5.0f };
    Camera mainCamera(initialCamPos, 60.0f, 0.1f, 100.0f);
    mainCamera.SetAspectRatio(1280.0f / 720.0f);

    // 5. Loop Principal
    bool running = true;
    while (running) {
        // --- INPUT ---
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) running = false;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window)) running = false;
        }

        // --- UPDATE UI ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // UI: Jerarquia
        ImGui::Begin("Hierarchy");
        if (ImGui::Button("Add Object to Root"))
        {
            //TODO: Afegir un nou GameObject a l'arrel de l'escena
            GameObject* newRoot = new GameObject({}, nullptr);
            newRoot->SetName("New Root Node");
            sceneRoots.push_back(newRoot);
        }
        ImGui::Separator();
        for (auto* obj : sceneRoots) DrawHierarchyNode(obj);
        ImGui::End();

        // UI: Inspector
        ImGui::Begin("Inspector");
        if (selectedObject) {
            ImGui::Text("Selected: %s", selectedObject->GetName().c_str());
            ImGui::Separator();

             // TODO: Agafar la posició del selectedObject
            Vec3 posSelected = selectedObject->GetPosition();
            float pos[3] = { posSelected.x,posSelected.y,posSelected.z };
            if (ImGui::DragFloat3("Position", pos, 0.1f))
            {
                //TODO: Actualitzar la posició del selectedObject
                selectedObject->SetPosition({ pos[0],pos[1],pos[2] });
            }

            // TODO: Agafar la rotació del selectedObject
            Vec3 rotSelected = selectedObject->GetRotation();
            float rot[3] = { rotSelected.x,rotSelected.y,rotSelected.z };
            if (ImGui::DragFloat3("Rotation (Euler)", rot, 0.5f))
            {
                // TODO: Actualitzar la rotació del selectedObject
                selectedObject->SetRotation({ rot[0],rot[1],rot[2] });
            }

            // TODO: Agafar l'escala del selectedObject
            Vec3 sclSelected = selectedObject->GetScale();
            float scl[3] = { sclSelected.x,sclSelected.y,sclSelected.z };
            if (ImGui::DragFloat3("Scale", scl, 0.1f))
            {
                // TODO: Actualitzar l'escala del selectedObject
                selectedObject->SetScale({ scl[0],scl[1],scl[2] });
            }

            ImGui::Separator();
            if (ImGui::Button("Add Child"))
            {
                // TODO: Afegir un nou GameObject com a fill del selectedObject
                Transform transformChild = {};
                GameObject* newChild = new GameObject(transformChild, selectedObject);
                newChild->SetName("New Child");
                selectedObject->AddChild(newChild);
            }

            // Actualitzar Color
            Vec3 vecSelected = selectedObject->GetColor();
            float color[3] = { vecSelected.x,vecSelected.y,vecSelected.z };
            if (ImGui::DragFloat3("Color", color, 0.01f, 0.0f, 1.0f))
            {
                selectedObject->SetColor(color[0], color[1], color[2]);
            }
        }
        else {
            ImGui::Text("Select an object from Hierarchy.");
        }
        ImGui::End();

        // UI: Camera
        ImGui::Begin("Camera Settings");
        float fov = 0; // TODO: Agafar el FOV de la càmera
        if (ImGui::SliderFloat("FOV (Y)", &fov, 10.0f, 170.0f))
        {
            // TODO: Actualitzar el FOV de la càmera
            mainCamera.SetFOV(fov);
        }

        float nearP = 0, farP = 0; // TODO: Agafar near i far de la càmera
        ImGui::DragFloat("Near Plane", &nearP, 0.1f);
        ImGui::DragFloat("Far Plane", &farP, 1.0f);

        // TODO: Actualitzar near i far de la càmera si canvien
        if (nearP != mainCamera.GetNearPlane()){mainCamera.SetNearPlane(nearP);}
        if (farP != mainCamera.GetFarPlane()){mainCamera.SetFarPlane(farP);}

        ImGui::Separator();
        ImGui::Text("Camera Transform");
        // TODO: Agafar la posició de la càmera
        Vec3 vectorCamera = mainCamera.GetTransform().position;
        float cPos[3] = { vectorCamera.x,vectorCamera.y,vectorCamera.z };
        if (ImGui::DragFloat3("Pos", cPos, 0.1f))
        {
            // TODO: Actualitzar la posició de la càmera
            mainCamera.SetPosition(cPos[0], cPos[1], cPos[2]);
        }
        ImGui::End();

        // --- RENDER ---
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        glViewport(0, 0, w, h);
        if (h > 0)
        {
            // TODO: Actualitzar aspect ratio de la càmera
            mainCamera.SetAspectRatio((float)w / (float)h);
        }

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Matrix4x4 view = mainCamera.GetViewMatrix();
        Matrix4x4 proj = mainCamera.GetProjectionMatrix();

        if (shaderProgram != 0) {
            glUseProgram(shaderProgram);

            // TODO: Càlculs de Càmera (View i Projection)
            for (auto* root : sceneRoots) {
                RenderNode(root, shaderProgram, view, proj, cubeMesh);
            }

            // TODO: Recorregut de l'escena i renderitzat (RenderNode)
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    glDeleteProgram(shaderProgram);
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}