#pragma once

namespace D3D11Framework
{
	class RenderState
	{
	public:
		RenderState(ID3D11Device *pd3dDevice, ID3D11DeviceContext *pImmediateContext);

		bool Init();
		void Close();

		void TurnZBufferOn();
		void TurnZBufferOff();

		void TurnOnAlphaBlending();
		void TurnOffAlphaBlending();

	private:
		bool m_createdepthstencilstate();
		bool m_createblendingstate();
		bool m_createsamplerstate();

		ID3D11Device *m_pd3dDevice;
		ID3D11DeviceContext *m_pImmediateContext;

		ID3D11DepthStencilState *m_pDepthEnableStencilState;
		ID3D11DepthStencilState *m_pDepthDisabledStencilState;
		ID3D11BlendState* m_pAlphaEnableBlendingState;
		ID3D11BlendState* m_pAlphaDisableBlendingState;

		ID3D11SamplerState *m_pSampleState;

		bool m_depthenable;
		bool m_alphaenable;
	};
}


