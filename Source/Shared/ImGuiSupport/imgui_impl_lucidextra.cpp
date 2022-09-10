#include "imgui_impl_lucidextra.h"

#include "imgui.h"
#include <d3d11.h>
#include <cstdint>

extern ID3D11Device*		GetDx11Device();
extern ID3D11DeviceContext*	GetDx11DeviceContext();


// assume it's 8bpp
ImTextureID ImGui_ImplDX11_CreateTextureRGBA(unsigned char* pixels, int width, int height)
{
	ImTextureID newTex;
	ID3D11ShaderResourceView*	pTextureView = NULL;
	ID3D11Device* pDevice = GetDx11Device();
	
	if (pDevice == nullptr)	// no d3d device so return false
		return nullptr;

	// Upload texture to graphics system
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		ID3D11Texture2D *pTexture = NULL;
		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = pixels;
		subResource.SysMemPitch = desc.Width * 4;
		subResource.SysMemSlicePitch = 0;
		pDevice->CreateTexture2D(&desc, pixels == nullptr ? nullptr: &subResource, &pTexture);

		// Create texture view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		pDevice->CreateShaderResourceView(pTexture, &srvDesc, &pTextureView);
		pTexture->Release();
	}

	// Store our identifier
	newTex = (ImTextureID)pTextureView;

	return newTex;
}

void ImGui_ImplDX11_FreeTexture(ImTextureID texture)
{
	// Free texture
	ID3D11ShaderResourceView* pTextureView = (ID3D11ShaderResourceView*)texture;
	pTextureView->Release();
}

void ImGui_ImplDX11_UpdateTextureRGBA(ImTextureID texture,unsigned char* pixels)
{
	ID3D11ShaderResourceView*	pTextureView = (ID3D11ShaderResourceView*)texture;
	ID3D11DeviceContext* pDeviceCtx = GetDx11DeviceContext();

	
	ID3D11Texture2D *pTexture = nullptr;
	pTextureView->GetResource(reinterpret_cast<ID3D11Resource**>(&pTexture));

	D3D11_TEXTURE2D_DESC desc;
	pTexture->GetDesc(&desc);
	
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = pDeviceCtx->Map(pTexture,0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (hr == S_OK)
	{
		memcpy(mappedResource.pData, pixels, desc.Width * desc.Height * 4);	// assume 32bpp
		pDeviceCtx->Unmap(pTexture, 0);
	}
}

void ImGui_ImplDX11_UpdateTextureRGBA(ImTextureID texture, unsigned char* pixels, int srcWidth, int srcHeight)
{
	ID3D11ShaderResourceView* pTextureView = (ID3D11ShaderResourceView*)texture;
	ID3D11DeviceContext* pDeviceCtx = GetDx11DeviceContext();


	ID3D11Texture2D* pTexture = nullptr;
	pTextureView->GetResource(reinterpret_cast<ID3D11Resource**>(&pTexture));

	D3D11_TEXTURE2D_DESC desc;
	pTexture->GetDesc(&desc);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = pDeviceCtx->Map(pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (hr == S_OK)
	{
		const uint8_t* srcPtr = pixels;
		uint8_t* destPtr = (uint8_t*)mappedResource.pData;
		for (int line = 0; line < srcHeight; line++)
		{
			const size_t srcStride = srcWidth * 4;// assume 32bpp
			memcpy(destPtr, srcPtr, srcStride);	// copy one line

			srcPtr += srcStride;
			destPtr += mappedResource.RowPitch;
		}
		pDeviceCtx->Unmap(pTexture, 0);
	}
}
