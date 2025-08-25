#include "SceneManager.h"
#include "stb_image.h"
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <vector>
#include <string>

#if defined(_WIN32)
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

namespace {
    const char* g_ModelName = "model";
    const char* g_ColorValueName = "objectColor";
    const char* g_TextureValueName = "objectTexture";
    const char* g_UseTextureName = "bUseTexture";

    // Helper: print current working directory once
    void LogCwdOnce()
    {
        static bool logged = false;
        if (logged) return;
        char buf[1024] = { 0 };
        if (getcwd(buf, sizeof(buf) - 1))
            std::cout << "Runtime CWD: " << buf << std::endl;
        logged = true;
    }

    // Helper: try a list of candidate paths until one loads
    unsigned int LoadTextureFromCandidates(const std::vector<std::string>& candidates)
    {
        LogCwdOnce();

        int width = 0, height = 0, channels = 0;
        unsigned char* data = nullptr;
        std::string usedPath;

        // Try each candidate
        for (const auto& p : candidates)
        {
            data = stbi_load(p.c_str(), &width, &height, &channels, 0);
            if (data) { usedPath = p; break; }
        }

        if (!data)
        {
            std::cerr << "stb_image failed. Last reason: "
                << (stbi_failure_reason() ? stbi_failure_reason() : "unknown")
                << "\nTried paths:\n";
            for (const auto& p : candidates) std::cerr << "  " << p << "\n";
            return 0;
        }

        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

        unsigned int textureID = 0;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
            GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        std::cout << "Loaded texture: " << usedPath
            << " (" << width << "x" << height << ", channels=" << channels << ")\n";

        return textureID;
    }
}

SceneManager::SceneManager(ShaderManager* pShaderManager)
{
    m_pShaderManager = pShaderManager;
    m_basicMeshes = new ShapeMeshes();

    // If you previously had link errors with this, comment it out:
    stbi_set_flip_vertically_on_load(1);

    // Common root paths relative to the .exe working folder (Debug/)
    const std::string root = "";
    const std::string up = "../";

    // Build candidate lists (try exact case you actually have on disk)
    std::vector<std::string> woodPaths = {
        root + "textures/wood.jpg",
        up + "textures/wood.jpg"
    };
    std::vector<std::string> brickPaths = {
        root + "textures/brick.jpg",
        up + "textures/brick.jpg"
    };
    std::vector<std::string> metalPaths = {
        root + "textures/metal.jpg",
        up + "textures/metal.jpg"
    };

    m_woodTexture = LoadTextureFromCandidates(woodPaths);
    m_brickTexture = LoadTextureFromCandidates(brickPaths);
    m_metalTexture = LoadTextureFromCandidates(metalPaths);

    if (!m_woodTexture)  std::cerr << "WARNING: wood texture not loaded.\n";
    if (!m_brickTexture) std::cerr << "WARNING: brick texture not loaded.\n";
    if (!m_metalTexture) std::cerr << "WARNING: metal texture not loaded.\n";
}

SceneManager::~SceneManager()
{
    delete m_basicMeshes;
    m_basicMeshes = nullptr;
    m_pShaderManager = nullptr;
}

unsigned int SceneManager::LoadTexture(const char* filePath)
{
    // Kept for compatibility if you call it elsewhere; now just uses the candidate loader.
    return LoadTextureFromCandidates({ filePath, std::string("../") + filePath });
}

void SceneManager::PrepareScene()
{
    m_basicMeshes->LoadPlaneMesh();
    m_basicMeshes->LoadBoxMesh();
    m_basicMeshes->LoadCylinderMesh();
    m_basicMeshes->LoadSphereMesh();
}

void SceneManager::RenderScene()
{
    glm::vec3 scale, position;

    // Floor
    scale = glm::vec3(100, 1, 100);
    position = glm::vec3(0, -0.5f, 0);
    SetTransformations(scale, 0, 0, 0, position);
    SetShaderColor(0.9f, 0.9f, 0.9f, 1.0f);
    m_basicMeshes->DrawPlaneMesh();

    // Wall (brick)
    scale = glm::vec3(5, 5, 0.1f);
    position = glm::vec3(0, 4, -4.9f);
    SetTransformations(scale, 0, 0, 0, position);
    SetShaderTexture(m_brickTexture);
    m_basicMeshes->DrawBoxMesh();

    // Mirror (blue tint)
    scale = glm::vec3(3.6f, 3.6f, 0.05f);
    position = glm::vec3(0, 4, -4.85f);
    SetTransformations(scale, 0, 0, 0, position);
    SetShaderColor(0.7f, 0.8f, 0.9f, 1.0f);
    m_basicMeshes->DrawBoxMesh();

    // Countertop (wood)
    scale = glm::vec3(10, 1, 3);
    position = glm::vec3(0, 0.5f, -3);
    SetTransformations(scale, 0, 0, 0, position);
    SetShaderTexture(m_woodTexture);
    m_basicMeshes->DrawBoxMesh();

    // Sink bowl (white)
    scale = glm::vec3(2, 0.5f, 2);
    position = glm::vec3(0, 1, -3);
    SetTransformations(scale, 0, 0, 0, position);
    SetShaderColor(1, 1, 1, 1);
    m_basicMeshes->DrawCylinderMesh();

    // Faucet (metal)
    scale = glm::vec3(0.1f, 1, 0.1f);
    position = glm::vec3(0, 1.75f, -3);
    SetTransformations(scale, -20.0f, 0, 0, position);
    SetShaderTexture(m_metalTexture);
    m_basicMeshes->DrawCylinderMesh();

    // Knobs (gray)
    for (float x : {-0.5f, 0.5f}) {
        scale = glm::vec3(0.2f);
        position = glm::vec3(x, 1.2f, -3);
        SetTransformations(scale, 0, 0, 0, position);
        SetShaderColor(0.2f, 0.2f, 0.2f, 1);
        m_basicMeshes->DrawSphereMesh();
    }

    // Vase (brown)
    scale = glm::vec3(0.4f, 1, 0.4f);
    position = glm::vec3(-3, 1, -3);
    SetTransformations(scale, 0, 0, 0, position);
    SetShaderColor(0.6f, 0.4f, 0.2f, 1);
    m_basicMeshes->DrawCylinderMesh();

    // Leaf (green)
    scale = glm::vec3(0.1f, 1.3f, 0.02f);
    position = glm::vec3(-2.8f, 2.2f, -3);
    SetTransformations(scale, 30, 0, 20, position);
    SetShaderColor(0.0f, 0.8f, 0.0f, 1);
    m_basicMeshes->DrawSphereMesh();

    // Leaf 2
    scale = glm::vec3(0.1f, 1.0f, 0.02f);
    position = glm::vec3(-3.2f, 2.0f, -3.2f);
    SetTransformations(scale, 40.0f, 10.0f, 0, position);
    SetShaderColor(0.0f, 0.8f, 0.0f, 1.0f);
    m_basicMeshes->DrawSphereMesh();

    // Light (yellow) — just a visual sphere, not an actual GLSL light
    scale = glm::vec3(1);
    position = glm::vec3(0, 8, -3);
    SetTransformations(scale, 0, 0, 0, position);
    SetShaderColor(1, 1, 0.8f, 1);
    m_basicMeshes->DrawSphereMesh();
}

void SceneManager::SetTransformations(glm::vec3 scale, float rx, float ry, float rz, glm::vec3 position)
{
    glm::mat4 model = glm::translate(position)
        * glm::rotate(glm::radians(rx), glm::vec3(1, 0, 0))
        * glm::rotate(glm::radians(ry), glm::vec3(0, 1, 0))
        * glm::rotate(glm::radians(rz), glm::vec3(0, 0, 1))
        * glm::scale(scale);

    if (m_pShaderManager)
        m_pShaderManager->setMat4Value(g_ModelName, model);
}

void SceneManager::SetShaderColor(float r, float g, float b, float a)
{
    if (m_pShaderManager)
    {
        m_pShaderManager->setIntValue(g_UseTextureName, false);
        m_pShaderManager->setVec4Value(g_ColorValueName, glm::vec4(r, g, b, a));
    }
}

void SceneManager::SetShaderTexture(unsigned int textureID)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    if (m_pShaderManager)
    {
        m_pShaderManager->setIntValue(g_UseTextureName, true);
        m_pShaderManager->setIntValue(g_TextureValueName, 0);
    }
}
