#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "Camera.h"
#include "GameObject.h"
#include "ObjReader.h"
#include "Collision.h"
#include <DXGItype.h>  
#include <dxgi1_2.h>  
#include <dxgi1_3.h>  
#include <DXProgrammableCapture.h>

class GameApp : public D3DApp
{
public:
	// 摄像机模式
	enum class CameraMode { FirstPerson, ThirdPerson, Free };
	
public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt, float gametime);
	void DrawScene();
	
private:
	bool InitResource();
	void CreateRandomTrees();
	void InitParticle();
	void UpdateParticle(float dt, float gametime);
	void ResetParticle();
	void DrawParticle();
private:
	
	ComPtr<ID2D1SolidColorBrush> m_pColorBrush;				    // 单色笔刷
	ComPtr<IDWriteFont> m_pFont;								// 字体
	ComPtr<IDWriteTextFormat> m_pTextFormat;					// 文本格式
	ComPtr<ID3D11Buffer> m_pVertexBuffers[3];                   //顶点缓冲区数组

	GameObject m_Trees;										    // 树
	GameObject m_Ground;										// 地面
	std::vector<DirectX::XMMATRIX> m_InstancedData;			    // 树的实例数据
	Collision::WireFrameData m_TreeBoxData;					    // 树包围盒线框数据

	ComPtr<ID3D11ShaderResourceView> m_pRainTexture;			    // 雨纹理
	ComPtr<ID3D11ShaderResourceView> m_pRandomTexture;                //随机纹理
	BasicEffect m_BasicEffect;								    // 对象渲染特效管理
	ParticleEffect m_ParticleEffect;                            //粒子渲染特效管理
	bool m_EnableFrustumCulling;								// 视锥体裁剪开启
	bool m_EnableInstancing;									// 硬件实例化开启

	std::shared_ptr<Camera> m_pCamera;						    // 摄像机
	CameraMode m_CameraMode;									// 摄像机模式
	ObjReader m_ObjReader;									    // 模型读取对象

	UINT m_MaxParticles;                                         //粒子数量
	bool m_FirstRun;                                             //是否第一次跑

	float m_GameTime;                                             //游戏时间
	float m_TimeStep;                                             //游戏步长
	float m_Age;                                                  //年龄

	bool m_FogEnabled;										    // 是否开启雾效
	bool m_IsNight;											    // 是否黑夜
	bool m_EnableAlphaToCoverage;							    // 是否开启Alpha-To-Coverage
	float m_FogRange;										    // 雾效范围
};


#endif