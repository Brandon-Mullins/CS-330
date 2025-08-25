///////////////////////////////////////////////////////////////////////////////
// shadermanager.h
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"
#include <string>
#include <glm/glm.hpp>

class SceneManager
{
public:
    SceneManager(ShaderManager* pShaderManager);
    ~SceneManager();

    void PrepareScene();
    void RenderScene();

private:
    ShaderManager* m_pShaderManager;
    ShapeMeshes* m_basicMeshes;

    unsigned int m_woodTexture;
    unsigned int m_brickTexture;
    unsigned int m_metalTexture; // ✅ Added missing declaration

    unsigned int LoadTexture(const char* filePath);
    void SetTransformations(glm::vec3 scale, float rotX, float rotY, float rotZ, glm::vec3 position);
    void SetShaderColor(float r, float g, float b, float a);
    void SetShaderTexture(unsigned int textureID);
};
