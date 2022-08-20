#include "stdafx.h"
#include "..\Public\Interact.h"
#include "GameInstance.h"
#include "Imgui_Manager.h"

CInteract::CInteract(ID3D11Device* pDeviceOut, ID3D11DeviceContext* pDeviceContextOut)
	: CGameObject(pDeviceOut, pDeviceContextOut)
{

}

CInteract::CInteract(const CInteract & rhs)
	: CGameObject(rhs)
{
}

HRESULT CInteract::NativeConstruct_Prototype()
{

	if (FAILED(__super::NativeConstruct_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CInteract::NativeConstruct(void * pArg)
{
	CTransform::TRANSFORMDESC		TransformDesc;
	ZeroMemory(&TransformDesc, sizeof(CTransform::TRANSFORMDESC));

	TransformDesc.SpeedPerSec = 5.0f;
	TransformDesc.RotationPerSec = XMConvertToRadians(90.0f);
	
	if (FAILED(__super::NativeConstruct(pArg, &TransformDesc)))
		return E_FAIL;

	if (FAILED(SetUp_Components()))
		return E_FAIL;

	if (pArg != nullptr) {
		memcpy(&m_ModelIndex, pArg, sizeof(int));
	}
	
	for (auto& iter : m_Models) {
		m_NameTag.push_back(iter->Get_FileName());
	}

	if (CImgui_Manager::GetInstance()->m_InteractTag.size() == 0) {
		for (auto& iter : m_NameTag) {
			CImgui_Manager::Item item;
			strcpy_s(item.Name, iter);
			item.is_Selected = false;
			CImgui_Manager::GetInstance()->m_InteractTag.push_back(item);
		}
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(-50.f, 0.f, -50.f, 0.f));
	}

	return S_OK;
}

_int CInteract::Tick(_double TimeDelta)
{
	__super::Tick(TimeDelta);
	if (m_Dead == true)
		return 1;

	m_pSphereCom->Update(m_pTransformCom->Get_WorldMatrix());

	return 0;
}

void CInteract::LateTick(_double TimeDelta)
{
	__super::LateTick(TimeDelta);

	if (nullptr != m_pRendererCom  && m_Culling == true)
	{
		m_pRendererCom->Add_RenderGroup(CRenderer::GROUP_NONBLEND, this);

	}
}

HRESULT CInteract::Render()
{
	if (nullptr == m_pShaderCom || 
		nullptr == m_Models[m_ModelIndex])
		return E_FAIL;

	if (FAILED(__super::Render()))
		return E_FAIL;

	if (FAILED(SetUp_ConstantTable()))
		return E_FAIL;

	_uint		iNumMeshContainers = m_Models[m_ModelIndex]->Get_NumMeshContainer();

	for (_uint i = 0; i < iNumMeshContainers; ++i)
	{
		if (FAILED(m_Models[m_ModelIndex]->Bind_Material_OnShader(m_pShaderCom, aiTextureType_DIFFUSE, "g_DiffuseTexture", i)))
			return E_FAIL;

		if (FAILED(m_Models[m_ModelIndex]->Render(m_pShaderCom, "g_BoneMatrices", i, 0)))
			return E_FAIL;
	}	

#ifdef _DEBUG
	m_pSphereCom->Render();
#endif // _DEBUG

	return S_OK;
}

HRESULT CInteract::Using_Stage1()
{
	CModel* ComModel = nullptr;
	
	if (FAILED(__super::SetUp_Components(TEXT("Model0"), LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Treasure"), (CComponent**)&ComModel)))
		return E_FAIL;
	m_Models.push_back(ComModel);
	
	if (FAILED(__super::SetUp_Components(TEXT("Model1"), LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Melee_Tier1"), (CComponent**)&ComModel)))
		return E_FAIL;
	m_Models.push_back(ComModel);
	if (FAILED(__super::SetUp_Components(TEXT("Model2"), LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Melee_Tier2"), (CComponent**)&ComModel)))
		return E_FAIL;
	m_Models.push_back(ComModel);
	if (FAILED(__super::SetUp_Components(TEXT("Model3"), LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Melee_Tier3"), (CComponent**)&ComModel)))
		return E_FAIL;
	m_Models.push_back(ComModel);
	if (FAILED(__super::SetUp_Components(TEXT("Model4"), LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Squishy"), (CComponent**)&ComModel)))
		return E_FAIL;
	m_Models.push_back(ComModel);
	if (FAILED(__super::SetUp_Components(TEXT("Model5"), LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Turret_AOE"), (CComponent**)&ComModel)))
		return E_FAIL;
	m_Models.push_back(ComModel);
	if (FAILED(__super::SetUp_Components(TEXT("Model6"), LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Turret_Beam"), (CComponent**)&ComModel)))
		return E_FAIL;
	m_Models.push_back(ComModel);
	if (FAILED(__super::SetUp_Components(TEXT("Model7"), LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Turret_Blast"), (CComponent**)&ComModel)))
		return E_FAIL;
	m_Models.push_back(ComModel);
	if (FAILED(__super::SetUp_Components(TEXT("Model8"), LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Turret_Debuff"), (CComponent**)&ComModel)))
		return E_FAIL;
	m_Models.push_back(ComModel);
	if (FAILED(__super::SetUp_Components(TEXT("Model9"), LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Turret_Melee_Arms_Attack"), (CComponent**)&ComModel)))
		return E_FAIL;
	m_Models.push_back(ComModel);



	return S_OK;
}

HRESULT CInteract::SetUp_Components()
{
	/* For.Com_Renderer */
	if (FAILED(__super::SetUp_Components(TEXT("Com_Renderer"), LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::SetUp_Components(TEXT("Com_Shader"), LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxNonAnim"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;
	CCollider::COLLIDERDESC			ColliderDesc;
	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));

	/* For.Com_AABB */
	ColliderDesc.vPosition = _float3(0.f, 0.0f, 0.f);
	ColliderDesc.fRadius = 3.f;
	if (FAILED(__super::SetUp_Components(TEXT("Com_Sphere"), LEVEL_STATIC, TEXT("Prototype_Component_Collider_SPHERE"), (CComponent**)&m_pSphereCom, &ColliderDesc)))
		return E_FAIL;
	
	// LEVEL_GAMEPLAY -> Current LevelIndex��
	if (FAILED(Using_Stage1()))
		return E_FAIL;

	return S_OK;
}

HRESULT CInteract::SetUp_ConstantTable()
{
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	if (FAILED(m_pTransformCom->Bind_WorldMatrixOnShader(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;	

	RELEASE_INSTANCE(CGameInstance);

	return S_OK;
}

CInteract * CInteract::Create(ID3D11Device* pDeviceOut, ID3D11DeviceContext* pDeviceContextOut)
{
	CInteract*	pInstance = new CInteract(pDeviceOut, pDeviceContextOut);

	if (FAILED(pInstance->NativeConstruct_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created CInteract"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CInteract::Clone(void * pArg)
{
	CInteract*	pInstance = new CInteract(*this);

	if (FAILED(pInstance->NativeConstruct(pArg)))
	{
		MSG_BOX(TEXT("Failed to Created CInteract"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CInteract::Free()
{
	__super::Free();	
	
	for (auto* Models : m_Models) {
		Safe_Release(Models);
	}

	Safe_Release(m_pSphereCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);

}