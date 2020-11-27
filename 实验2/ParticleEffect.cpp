#include "Effects.h"
#include "d3dUtil.h"
#include "EffectHelper.h"	// 必须晚于Effects.h和d3dUtil.h包含
#include "DXTrace.h"
#include "Vertex.h"
using namespace DirectX;
using namespace std::experimental;

//
// ParticleEffect::Impl 需要先于ParticleEffect的定义
//

class ParticleEffect::Impl:public AlignedType<ParticleEffect::Impl>
{
public:
	//
	// 这些结构体对应HLSL的结构体。需要按16字节对齐
	//
	struct CBChangesEveryFrame
	{
		XMFLOAT3 EmitPos;
		float GameTime;
		XMFLOAT3 EmitDir;
		float TimeStep;
		XMMATRIX View;
	};


	struct CBChangesOnResize
	{
		XMMATRIX proj;
	};
public:
	// 必须显式指定
	Impl() = default;
	~Impl() = default;
public:
	CBufferObject<0, CBChangesEveryFrame> m_CBFrame;		// 每帧修改的常量缓冲区
    CBufferObject<1, CBChangesOnResize>	m_CBOnResize;			// 每次窗口大小变更的常量缓冲区
	BOOL m_IsDirty;										    // 是否有值变更
	std::vector<CBufferBase*> m_pCBuffers;				    // 统一管理上面所有的常量缓冲区

	ComPtr<ID3D11VertexShader> m_pParticleDWVS;
	ComPtr<ID3D11VertexShader> m_pParticleVS;
	ComPtr<ID3D11PixelShader> m_pParticlePS;
	ComPtr<ID3D11GeometryShader> m_pParticleSOGS;
	ComPtr<ID3D11GeometryShader> m_pParticleDWGS;

	ComPtr<ID3D11InputLayout> m_pParticleLayout;
	ComPtr<ID3D11InputLayout> m_pVertexoutLayout;

	ComPtr<ID3D11ShaderResourceView> m_pTexture;			// 雨的纹理
	ComPtr<ID3D11ShaderResourceView> m_pRamTexture;            //随机纹理
};


//
// ParticleEffect
//
namespace
{
	// ScreenFadeEffect单例
	static ParticleEffect * g_pInstance = nullptr;
}

ParticleEffect::ParticleEffect()
{
	if (g_pInstance)
		throw std::exception("ParticleEffect is a singleton!");
	g_pInstance = this;
	pImpl = std::make_unique<ParticleEffect::Impl>();
}

ParticleEffect::~ParticleEffect()
{
}

ParticleEffect::ParticleEffect(ParticleEffect && moveFrom)
{
	pImpl.swap(moveFrom.pImpl);
}

ParticleEffect & ParticleEffect::operator=(ParticleEffect && moveFrom)
{
	pImpl.swap(moveFrom.pImpl);
	return *this;
}

ParticleEffect & ParticleEffect::Get()
{
	if (!g_pInstance)
		throw std::exception("ScreenFadeEffect needs an instance!");
	return *g_pInstance;
}

bool ParticleEffect::InitAll(ComPtr<ID3D11Device> device)
{
	if (!device)
		return false;
	if (!pImpl->m_pCBuffers.empty())
		return true;
	if (!RenderStates::IsInit())
		throw std::exception("RenderStates need to be initialized first!");
	ComPtr<ID3DBlob> blob;

	const D3D11_SO_DECLARATION_ENTRY particleLayout[5] = {
		{ 0, "POSITION", 0, 0, 3, 0 },
		{ 0, "VELOCITY", 0, 0, 3, 0 },
		{ 0, "SIZE", 0, 0, 2, 0 },
		{ 0, "AGE", 0, 0, 1, 0 },
		{ 0, "TYPE", 0, 0, 1, 0 }
	};
	UINT strideParticle = sizeof(Particle);
	// ******************
	// 创建顶点着色器
	//
	HR(CreateShaderFromFile(L"HLSL\\Particle_VS.cso", L"HLSL\\Particle_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pParticleVS.GetAddressOf()));

	// 创建顶点布局
	HR(device->CreateInputLayout(Particle::inputLayout, ARRAYSIZE(Particle::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pParticleLayout.GetAddressOf()));

	// ******************
	// 创建顶点着色器
	//
	HR(CreateShaderFromFile(L"HLSL\\ParticleDW_VS.cso", L"HLSL\\ParticleDW_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pParticleDWVS.GetAddressOf()));

	// 创建顶点布局
	HR(device->CreateInputLayout(Particle::inputLayout, ARRAYSIZE(Particle::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pParticleLayout.GetAddressOf()));

	// ******************
	// 创建几何着色器
	HR(CreateShaderFromFile(L"HLSL\\ParticleDW_GS.cso", L"HLSL\\ParticleDW_GS.hlsl", "GS", "gs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pParticleDWGS.GetAddressOf()));

	//流输出几何着色器
	HR(CreateShaderFromFile(L"HLSL\\ParticleSO_GS.cso", L"HLSL\\ParticleSO_GS.hlsl", "GS", "gs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateGeometryShaderWithStreamOutput(blob->GetBufferPointer(), blob->GetBufferSize(), particleLayout, ARRAYSIZE(particleLayout),
		&strideParticle, 1, D3D11_SO_NO_RASTERIZED_STREAM, nullptr, pImpl->m_pParticleSOGS.GetAddressOf()));


	// ******************
	// 创建像素着色器
	//
	HR(CreateShaderFromFile(L"HLSL\\Particle_PS.cso", L"HLSL\\Particle_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pParticlePS.GetAddressOf()));


	pImpl->m_pCBuffers.assign({
		&pImpl->m_CBFrame,
		&pImpl->m_CBOnResize
		});

	// 创建常量缓冲区
	for (auto& pBuffer : pImpl->m_pCBuffers)
	{
		HR(pBuffer->CreateBuffer(device));
	}

	return true;
}

void ParticleEffect::SetRenderDefault(ComPtr<ID3D11DeviceContext> deviceContext)
{
	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->IASetInputLayout(pImpl->m_pParticleLayout.Get());
	deviceContext->VSSetShader(pImpl->m_pParticleDWVS.Get(), nullptr, 0);
	// 关闭流输出
	deviceContext->GSSetShader(pImpl->m_pParticleDWGS.Get(), nullptr, 0);
	ID3D11Buffer* bufferArray[1] = { nullptr };
	UINT offset = 0;
	deviceContext->SOSetTargets(1, bufferArray, &offset);

	deviceContext->RSSetState(RenderStates::RSWireframe.Get());

	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->PSSetShader(pImpl->m_pParticlePS.Get(), nullptr, 0);
	deviceContext->OMSetDepthStencilState(nullptr, 0);
	deviceContext->OMSetBlendState(RenderStates::BSAdditiveP.Get(), nullptr, 0xFFFFFFFF);
}

void ParticleEffect::SetRenderStreamOutParticle(ComPtr<ID3D11DeviceContext> deviceContext, ComPtr<ID3D11Buffer> vertexBufferIn, ComPtr<ID3D11Buffer> vertexBufferOut)
{
	UINT stride = sizeof(Particle);
	UINT offset = 0;
	ID3D11Buffer* nullBuffer = nullptr;
	deviceContext->SOSetTargets(1, &nullBuffer, &offset);
	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->IASetInputLayout(pImpl->m_pParticleLayout.Get());
	deviceContext->IASetVertexBuffers(0, 1, vertexBufferIn.GetAddressOf(), &stride, &offset);

	deviceContext->SOSetTargets(1, vertexBufferOut.GetAddressOf(), &offset);
	deviceContext->VSSetShader(pImpl->m_pParticleVS.Get(), nullptr, 0);
	deviceContext->GSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->GSSetShader(pImpl->m_pParticleSOGS.Get(), nullptr, 0);

	deviceContext->RSSetState(RenderStates::RSWireframe.Get());
	deviceContext->PSSetShader(nullptr, nullptr, 0);
	deviceContext->OMSetDepthStencilState(nullptr, 0);
	deviceContext->OMSetBlendState(RenderStates::BSAdditiveP.Get(), nullptr, 0xFFFFFFFF);
}

void XM_CALLCONV ParticleEffect::SetViewMatrix(DirectX::FXMMATRIX V)
{
	auto& cBuffer = pImpl->m_CBFrame;
	cBuffer.data.View = XMMatrixTranspose(V);
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV ParticleEffect::SetProjMatrix(DirectX::FXMMATRIX P)
{
	auto& cBuffer = pImpl->m_CBOnResize;
	cBuffer.data.proj = XMMatrixTranspose(P);
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void ParticleEffect::SetEmitPosition(DirectX::XMFLOAT3 pos)
{
	auto& cBuffer = pImpl->m_CBFrame;
	cBuffer.data.EmitPos = pos;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void ParticleEffect::SetEmitDirection(DirectX::XMFLOAT3 dir)
{
	auto& cBuffer = pImpl->m_CBFrame;
	cBuffer.data.EmitDir = dir;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void ParticleEffect::SetGameTime(float f)
{
	auto& cBuffer = pImpl->m_CBFrame;
	cBuffer.data.GameTime = f;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void ParticleEffect::SetTimeStep(float f)
{
	auto& cBuffer = pImpl->m_CBFrame;
	cBuffer.data.TimeStep = f;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void ParticleEffect::SetTexture(ComPtr<ID3D11ShaderResourceView> texture)
{
	pImpl->m_pTexture = texture;
}

void ParticleEffect::SetRamdonTexture(ComPtr<ID3D11ShaderResourceView> texture)
{
	pImpl->m_pRamTexture = texture;
}

void ParticleEffect::Apply(ComPtr<ID3D11DeviceContext> deviceContext)
{
	auto& pCBuffers = pImpl->m_pCBuffers;
	// 将缓冲区绑定到渲染管线上
	pCBuffers[0]->BindGS(deviceContext);
	pCBuffers[1]->BindGS(deviceContext);
     // 设置SRV
	deviceContext->PSSetShaderResources(0, 1, pImpl->m_pTexture.GetAddressOf());
	deviceContext->GSSetShaderResources(1, 1, pImpl->m_pRamTexture.GetAddressOf());
	if (pImpl->m_IsDirty)
	{
		pImpl->m_IsDirty = false;
		for (auto& pCBuffer : pCBuffers)
		{
			pCBuffer->UpdateBuffer(deviceContext);
		}
	}
}

