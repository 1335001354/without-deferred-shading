#pragma once
#include <string>
#include <vector>
#include "ShaderReflection.h"
#include "adapter.h"
#include "Camera.h"

class ConstantBufferManager
{
private:
    DXcore* core;
    std::vector<ConstantBuffer> vsConstantBuffers;
    std::vector<ConstantBuffer> psConstantBuffers;

	void updateConstant(const std::string& constantBufferName, const std::string& variableName, void* data, std::vector<ConstantBuffer>& buffers) {
        for (auto& buffer : buffers)
        {
            if (buffer.name == constantBufferName)
            {
                buffer.update(variableName, data);
                return;
            }
        }
	}
public:
    ConstantBufferManager(DXcore* _core) : core(_core) {}

    void init(Camera& camera) {
        ConstantBuffer cameraBuffer;
        cameraBuffer.name = "CameraBuffer";
        cameraBuffer.init(core, sizeof(cameraBuffer), 0, ShaderStage::VertexShader);
        addConstantBuffer(cameraBuffer, ShaderStage::VertexShader);
        Vec3 position = camera.Getposition();
        updateConstantVS("CameraBuffer", "cameraPosition", &position);
        uploadVSConstants();
    }

    //add a constant buffer to the system
	void addConstantBuffer(const ConstantBuffer& buffer, ShaderStage stage) {
        if (stage == ShaderStage::VertexShader) vsConstantBuffers.push_back(buffer);
        else if (stage == ShaderStage::PixelShader) psConstantBuffers.push_back(buffer);
	}

    //Update a specific constant buffer variable for the vertex shader
    void updateConstantVS(const std::string& constantBufferName, const std::string& variableName, void* data) {
        updateConstant(constantBufferName, variableName, data, vsConstantBuffers);
    }

    //Update a specific constant buffer variable for the pixel shader
    void updateConstantPS(const std::string& constantBufferName, const std::string& variableName, void* data)
    {
        updateConstant(constantBufferName, variableName, data, psConstantBuffers);
    }

    //Upload all dirty vertex shader constant buffers to the GPU
    void uploadVSConstants()
    {
        for (auto& buffer : vsConstantBuffers)
        {
            if(buffer.dirty) buffer.upload(core);
        }
    }

    // Upload all dirty pixel shader constant buffers to the GPU
    void uploadPSConstants()
    {
        for (auto& buffer : psConstantBuffers)
        {
            if(buffer.dirty) buffer.upload(core);
        }
    }

    void uploadAllConstants()
    {
        uploadVSConstants();
        uploadPSConstants();
    }

};

