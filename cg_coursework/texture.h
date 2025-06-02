#pragma once

#include "stb_image.h"
#include <iostream>
#include "shader.h"
#include "Gemloader.h"


class Texture
{
public:
	int width = 0;
	int height = 0;
	int channels = 0;

    ID3D11Texture2D* texture;
    ID3D11ShaderResourceView* srv;
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    void init(DXcore* dx, int width, int height, int channels, unsigned char *data) {
        D3D11_TEXTURE2D_DESC texDesc;
        memset(&texDesc, 0, sizeof(D3D11_TEXTURE2D_DESC));
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = format;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        D3D11_SUBRESOURCE_DATA initData;
        memset(&initData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
        initData.pSysMem = data;
        initData.SysMemPitch = width * channels;
        dx->device->CreateTexture2D(&texDesc, &initData, &texture);
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format = format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        dx->device->CreateShaderResourceView(texture, &srvDesc, &srv);
        //shader.updateShaderResources(*dx, name, srv);
    }
    void load(DXcore* dx, std::string filename) {
        unsigned char* texels = stbi_load(filename.c_str(), &width, &height, &channels, 0);
        if (channels == 3) {
            channels = 4;
            unsigned char* texelsWithAlpha = new unsigned char[width * height * channels];
            for (int i = 0; i < (width * height); i++) {
                texelsWithAlpha[i * 4] = texels[i * 3];
                texelsWithAlpha[(i * 4) + 1] = texels[(i * 3) + 1];
                texelsWithAlpha[(i * 4) + 2] = texels[(i * 3) + 2];
                texelsWithAlpha[(i * 4) + 3] = 255;
            }
            // Initialize texture using width, height, channels, and texelsWithAlpha
            init(dx, width, height, channels, texelsWithAlpha);
            delete[] texelsWithAlpha;
        }
        else {
            // Initialize texture using width, height, channels, and texels
            init(dx, width, height, channels, texels);

        }
        stbi_image_free(texels);
    }
    void free() {
        srv->Release();
        texture->Release();
    }
    
};

class Sampler
{
public:
    ID3D11SamplerState* state;
    void init(DXcore* dx) {
        D3D11_SAMPLER_DESC samplerDesc;
        ZeroMemory(&samplerDesc, sizeof(samplerDesc));
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        dx->device->CreateSamplerState(&samplerDesc, &state);
        dx->devicecontext->PSSetSamplers(0, 1, &state);
    }
};

class TextureManager
{
public:
    std::map<std::string, Texture*> textures;
    void load(DXcore* core, std::string filename) //load texture by filename
    {
        std::map<std::string, Texture*>::iterator it = textures.find(filename);
        if (it != textures.end())
        {
            return;
        }
        Texture* texture = new Texture();
        texture->load(core, filename);
        textures.insert({ filename, texture });
    }

    void load(DXcore* core, vector<std::string> textureFilenames) { //load texture by model
        for (int i = 0; i < textureFilenames.size(); i++) {
            load(core, textureFilenames[i]);
        }
    }

    ID3D11ShaderResourceView* find(std::string name)
    {
        return textures[name]->srv;
    }
    void unload(std::string name)
    {
        textures[name]->free();
        textures.erase(name);
    }
    ~TextureManager()
    {
        for (auto it = textures.cbegin(); it != textures.cend(); )
        {
            it->second->free();
            textures.erase(it++);
        }
    }
};