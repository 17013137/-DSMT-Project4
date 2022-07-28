#include "..\Public\Navigation.h"
#include "Cell.h"
#include "Shader.h"
#include "PipeLine.h"
#include "Transform.h"
#include "VIBuffer_Triangle.h"

CNavigation::CNavigation(ID3D11Device * pDevice, ID3D11DeviceContext * pDeviceContext)
	: CComponent(pDevice, pDeviceContext)
{

}

CNavigation::CNavigation(const CNavigation & rhs)
	: CComponent(rhs)
	, m_Cells(rhs.m_Cells)	
	, m_pVIBuffer(rhs.m_pVIBuffer)
	, m_pShader(rhs.m_pShader)
{
	for (auto& pCell : m_Cells)
		Safe_AddRef(pCell);

	Safe_AddRef(m_pShader);
	Safe_AddRef(m_pVIBuffer);
}

HRESULT CNavigation::NativeConstruct_Prototype(const _tchar * pNaviDataFilePath)
{
	_ulong			dwByte = 0;
	HANDLE			hFile = CreateFile(pNaviDataFilePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (0 == hFile)
		return E_FAIL;

	_float3			vPoints[3];

	while (true)
	{
		ReadFile(hFile, vPoints, sizeof(_float3) * 3, &dwByte, nullptr);

		if (0 == dwByte)
			break;

		CCell*		pCell = CCell::Create(m_pDevice, m_pDeviceContext, vPoints, m_Cells.size());
		if (nullptr == pCell)
			return E_FAIL;

		m_Cells.push_back(pCell);		
	}	

	CloseHandle(hFile);

	if (FAILED(SetUp_Neighbor()))
		return E_FAIL;

#ifdef _DEBUG
	m_pVIBuffer = CVIBuffer_Triangle::Create(m_pDevice, m_pDeviceContext);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

	m_pShader = CShader::Create(m_pDevice, m_pDeviceContext, TEXT("../Bin/ShaderFiles/Shader_Navigation.hlsl"), VTXCOL_DECLARATION::Elements, VTXCOL_DECLARATION::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;
#endif // _DEBUG

	return S_OK;
}

HRESULT CNavigation::NativeConstruct(void * pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	memcpy(&m_NaviDesc, pArg, sizeof(NAVIDESC));	

	Safe_AddRef(m_NaviDesc.pParent);

	return S_OK;
}

_bool CNavigation::Move_OnNavigation(_fvector vPosition)
{
	_int		iNeighborIndex = -1;

	/* _fvector vPosition : �����̰� �� ���� ��ġ��. */
	if (false == m_Cells[m_NaviDesc.iCurrentIndex]->isIn(vPosition, &iNeighborIndex))
	{
		/* �����ʿ� �̿��� �ִٶ��. */
		if (0 <= iNeighborIndex)
		{
			while (true)
			{
				_int	iCurrentNeighborIndex = -1;

				if (true == m_Cells[iNeighborIndex]->isIn(vPosition, &iCurrentNeighborIndex))
				{
					m_NaviDesc.iCurrentIndex = iNeighborIndex;
					break;
				}
				else 
				{
					if (-1 == iCurrentNeighborIndex)
						return false;									
				}
				iNeighborIndex = iCurrentNeighborIndex;

			}
			return true;
		}

		/* �̿��� ���� ������ �����ٸ�. */
		else
		{
			/* �����̵�. */
			return false;
		}
	}
	else
		return true;	
}

HRESULT CNavigation::Render()
{
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

	CPipeLine*		pPipeLine = GET_INSTANCE(CPipeLine);

	_float4x4		WorldMatrix;

	if (nullptr == m_NaviDesc.pParent)
		XMStoreFloat4x4(&WorldMatrix, XMMatrixIdentity());
	else
		XMStoreFloat4x4(&WorldMatrix, XMMatrixTranspose(m_NaviDesc.pParent->Get_WorldMatrix()));
	
	m_pShader->Set_RawValue("g_WorldMatrix", &WorldMatrix, sizeof(_float4x4));
	m_pShader->Set_RawValue("g_ViewMatrix", &pPipeLine->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4));
	m_pShader->Set_RawValue("g_ProjMatrix", &pPipeLine->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4));

	_uint		iIndex = 0;

	if (-1 == m_NaviDesc.iCurrentIndex)
	{
		for (auto& pCell : m_Cells)
		{
			if (nullptr != pCell)
			{
				pCell->Render(m_pVIBuffer, m_pShader, iIndex++ == m_NaviDesc.iCurrentIndex ? _float4(1.f, 0.f, 0.f, 1.f) : _float4(1.f, 1.f, 1.f, 1.f));
			}
		}
	}
	else
	{
		m_Cells[m_NaviDesc.iCurrentIndex]->Render(m_pVIBuffer, m_pShader, _float4(1.f, 0.f, 0.f, 1.f));

	}
	

	RELEASE_INSTANCE(CPipeLine);

	return S_OK;
}

HRESULT CNavigation::SetUp_Neighbor()
{
	for (auto& pSourCell : m_Cells)
	{
		for (auto& pDestCell : m_Cells)
		{
			if (pSourCell == pDestCell)
				continue;

			if (true == pDestCell->Compare_Points(pSourCell->Get_Point(CCell::POINT_A), pSourCell->Get_Point(CCell::POINT_B)))
			{
				pSourCell->Set_NeighborIndex(CCell::LINE_AB, pDestCell->Get_Index());
			}

			if (true == pDestCell->Compare_Points(pSourCell->Get_Point(CCell::POINT_B), pSourCell->Get_Point(CCell::POINT_C)))
			{
				pSourCell->Set_NeighborIndex(CCell::LINE_BC, pDestCell->Get_Index());
			}

			if (true == pDestCell->Compare_Points(pSourCell->Get_Point(CCell::POINT_C), pSourCell->Get_Point(CCell::POINT_A)))
			{
				pSourCell->Set_NeighborIndex(CCell::LINE_CA, pDestCell->Get_Index());

			}
		}
	}

	return S_OK;
}

CNavigation * CNavigation::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pDeviceContext, const _tchar * pNaviDataFilePath)
{
	CNavigation*	pInstance = new CNavigation(pDevice, pDeviceContext);

	if (FAILED(pInstance->NativeConstruct_Prototype(pNaviDataFilePath)))
	{
		MSGBOX("Failed to Creating CNavigation");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CNavigation::Clone(void * pArg)
{
	CNavigation*	pInstance = new CNavigation(*this);

	if (FAILED(pInstance->NativeConstruct(pArg)))
	{
		MSGBOX("Failed to Creating CNavigation");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNavigation::Free()
{
	__super::Free();

	Safe_Release(m_NaviDesc.pParent);

	for (auto& pCell : m_Cells)
		Safe_Release(pCell);

	m_Cells.clear();

	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pShader);

}
