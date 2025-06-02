#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <D3Dcompiler.h>
#include <d3d11shader.h>
#include "cgmath.h"
#include "adapter.h"
#include "ShaderReflection.h"

#pragma comment(lib, "dxguid.lib")
struct World {
	Matrix Ws;
};

struct viewProjection {
	Matrix VP;
};

struct animatedMesh {
	Matrix bones[256];
};


class Shaders
{
public:
	std::vector<ConstantBuffer> psConstantBuffers;
	std::vector<ConstantBuffer> vsConstantBuffers;
	DXcore* dx;

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

	void uploadConstant(const std::string& constBufferName, const std::string& variableName, std::vector<ConstantBuffer>& buffers) 
	{
		for (auto& buffer : buffers)
		{
			if (buffer.name == constBufferName)
			{
				buffer.upload(dx);
				return;
			}
		}
	}

	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	//ID3D11GeometryShader* geometryShaderWithSO;
	ID3D11InputLayout* layout;
	ID3D11Buffer* constantBuffer;
	std::map<std::string, int> textureBindPointsVS;
	std::map<std::string, int> textureBindPointsPS;

	Shaders(DXcore* _dx) : dx(_dx) {}

	//add a constant buffer to the system
	void addConstantBuffer(const ConstantBuffer& buffer, ShaderStage stage) {
		if (stage == ShaderStage::VertexShader) vsConstantBuffers.push_back(buffer);
		else if (stage == ShaderStage::PixelShader) psConstantBuffers.push_back(buffer);
	}

	//Update a specific constant buffer variable for the vertex shader
	void updateConstantVS(const std::string& constantBufferName, const std::string& variableName, void* data) {
		updateConstant(constantBufferName, variableName, data, vsConstantBuffers);
	}

	void uploadConstantVS(const std::string& constantBufferName, const std::string& variableName) {
		uploadConstant(constantBufferName, variableName, vsConstantBuffers);
	}

	//Update a specific constant buffer variable for the pixel shader
	void updateConstantPS(const std::string& constantBufferName, const std::string& variableName, void* data)
	{
		updateConstant(constantBufferName, variableName, data, psConstantBuffers);
	}

	void uploadConstantPS(const std::string& constantBufferName, const std::string& variableName) {
		uploadConstant(constantBufferName, variableName, psConstantBuffers);
	}

	void updateShaderResources(DXcore& dx, string name, ID3D11ShaderResourceView* tex) {
		dx.devicecontext->PSSetShaderResources(textureBindPointsPS[name], 1, &tex);
	}

	std::string LoadShaders(std::string filePath) {
		std::ifstream file(filePath);
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open HLSL file");
		}

		//Read HLSL file
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string shaderCode = buffer.str();
		return shaderCode;

	}

	void loadPS(std::string psfile) {
		ID3DBlob* compiledPixelShader;
		ID3DBlob* status_pixel;
		HRESULT hr = D3DCompile(psfile.c_str(), strlen(psfile.c_str()), NULL, NULL, NULL, "PS", "ps_5_0", 0, 0, &compiledPixelShader, &status_pixel); //PS is the entry point of the shader;
		if (FAILED(hr)) {
			(char*)status_pixel->GetBufferPointer();
			exit(0);
		}
		dx->device->CreatePixelShader(compiledPixelShader->GetBufferPointer(), compiledPixelShader->GetBufferSize(), NULL, &pixelShader);
		ConstantBufferReflection reflection;
		reflection.build(dx, compiledPixelShader, psConstantBuffers, textureBindPointsPS, ShaderStage::PixelShader);
		compiledPixelShader->Release();
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create Pixel Shader");
		}
		
	}

	void loadVS(std::string vsfile) { //particle, basic...
		ID3DBlob* compiledVertexShader;
		ID3DBlob* status_vertex;
		HRESULT hr = D3DCompile(vsfile.c_str(), strlen(vsfile.c_str()), NULL, NULL, NULL, "VS", "vs_5_0", 0, 0, &compiledVertexShader, &status_vertex); //VS is the entry point of the shader
		if (FAILED(hr)) {
			MessageBoxA(NULL, (char*)status_vertex->GetBufferPointer(), "Error", 0);
			exit(0);
		}
		dx->device->CreateVertexShader(compiledVertexShader->GetBufferPointer(), compiledVertexShader->GetBufferSize(), NULL, &vertexShader);
		D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
		{
			{ "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONEIDS", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TYPE", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		dx->device->CreateInputLayout(layoutDesc, 7, compiledVertexShader->GetBufferPointer(), compiledVertexShader->GetBufferSize(), &layout);
		//Stream Output Declaration
		//D3D11_SO_DECLARATION_ENTRY soDecl[] = {
		//	{ 0, "SV_POSITION", 0, 0, 4, 0 },  // Output position (float4)
		//	{ 0, "NORMAL", 0, 0, 3, 0 },       // Normal vector (float3)
		//	{ 0, "TANGENT", 0, 0, 3, 0 },      // Tangent vector (float3)
		//	{ 0, "TEXCOORD", 0, 0, 2, 0 }      // Texture coordinates (float2)
		//};

		////Strides for the stream output (total size of one output vertex)
		//UINT soStrides[] = { sizeof(Vec4) + sizeof(Vec3) + sizeof(Vec3) + sizeof(Vec2) };
		////Create the vertex shader with stream output
		//hr = dx->device->CreateGeometryShaderWithStreamOutput(
		//	compiledVertexShader->GetBufferPointer(), //Compiled shader bytecode
		//	compiledVertexShader->GetBufferSize(),    //Shader bytecode size
		//	soDecl,									  //Stream output declaration
		//	ARRAYSIZE(soDecl),						  //Number of entries in soDecl
		//	soStrides,								  //Strides for stream output
		//	1,										  //Number of output streams
		//	0,										  //Rasterized stream index
		//	nullptr,								  //Class linkage
		//	&geometryShaderWithSO
		//	);
		//if (FAILED(hr)) {
		//	MessageBoxA(NULL, "Failed to create geometry shader with stream output", "Error", 0);
		//	exit(0);
		//}
		ConstantBufferReflection reflection;
		reflection.build(dx, compiledVertexShader, vsConstantBuffers, textureBindPointsVS, ShaderStage::VertexShader);
		compiledVertexShader->Release();
	}

	void apply() {
		dx->devicecontext->IASetInputLayout(layout);
		dx->devicecontext->VSSetShader(vertexShader, NULL, 0);
		//dx->devicecontext->GSSetShader(geometryShaderWithSO, NULL, 0);
		dx->devicecontext->PSSetShader(pixelShader, NULL, 0);
		dx->devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void init(std::string PSfilename, std::string VSfilename) {
		std::string code_ps = LoadShaders(PSfilename);
		std::string code_vs = LoadShaders(VSfilename);
		loadVS(code_vs);
		loadPS(code_ps);
	}
};