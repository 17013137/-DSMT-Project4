#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CCollider;
class CShader;
class CRenderer;
class CModel;
END

BEGIN(Client)

class CRock_Pile_03 final : public CGameObject
{
private:
	explicit CRock_Pile_03(ID3D11Device* pDeviceOut, ID3D11DeviceContext* pDeviceContextOut);
	explicit CRock_Pile_03(const CRock_Pile_03& rhs);
	virtual ~CRock_Pile_03() = default;

public:
	virtual HRESULT NativeConstruct_Prototype() override;
	virtual HRESULT NativeConstruct(void* pArg);
	virtual void Tick(_double TimeDelta) override;
	virtual void LateTick(_double TimeDelta) override;
	virtual HRESULT Render() override;

private:	
	CCollider*			m_pSphereCom = nullptr;
	CRenderer*			m_pRendererCom = nullptr;
	CShader*			m_pShaderCom = nullptr;	
	CModel*				m_pModelCom = nullptr;

private:
	HRESULT SetUp_Components();
	HRESULT SetUp_ConstantTable();	

public:
	static CRock_Pile_03* Create(ID3D11Device* pDeviceOut, ID3D11DeviceContext* pDeviceContextOut);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END