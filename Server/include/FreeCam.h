#pragma once

#define OFFSET_CLIENTGAMECONTEXT CYPRESS_GW_SELECT(0x141F000E0, 0x142B5BB90, 0x1445DF780)
#define OFFSET_SETACTIVECAMERA CYPRESS_GW_SELECT(0x14059E940, 0x1404DCD80, 0x140F97310)
#define OFFSET_ADDINPUT CYPRESS_GW_SELECT(0x140561E10, 0x140412390, 0x1411F8140)
#define OFFSET_REMOVEINPUT CYPRESS_GW_SELECT(0x140566680, 0x1404124C0, 0x1411FF4A0)

namespace Cypress
{

	struct FreeCamera
	{
		void** vftptr;
		void* unk1;
		char m_cTransform[0x40]; // LinearTransform
		char m_transform[0x40];	 // LinearTransform
		char pad[CYPRESS_GW_SELECT(0x80, 0xC0, 0x128)];
		void* m_input;
	};

	enum CameraIds : int
	{
		NoCameraId,
		FreeCameraId,
		EntryCameraId,
		CameraIdCount
	};

	struct ClientGameView
	{
		void* vftptr;
		CameraIds m_activeCameraId;
		char pad[CYPRESS_GW_SELECT(0xBC, 0xB8, 0xAC)];
		FreeCamera* m_freeCamera;
	};

	struct ClientGameContext
	{
		char pad[CYPRESS_GW_SELECT(0x78, 0x78, 0xB8)];
		ClientGameView* m_clientGameView;

		static ClientGameContext* GetInstance()
		{
			return *reinterpret_cast<ClientGameContext**>(OFFSET_CLIENTGAMECONTEXT);
		}
	};

	inline bool ToggleFreeCam()
	{
		ClientGameContext* ctx = ClientGameContext::GetInstance();
		if (!ctx || !ctx->m_clientGameView || !ctx->m_clientGameView->m_freeCamera)
			return false;

		ClientGameView* view = ctx->m_clientGameView;
		FreeCamera* cam = view->m_freeCamera;
		if (!cam->m_input)
			return false;

		bool isActive = (view->m_activeCameraId == FreeCameraId);

		using tSetActiveCamera = void (*)(ClientGameView*, CameraIds);
		auto setActiveCamera = reinterpret_cast<tSetActiveCamera>(OFFSET_SETACTIVECAMERA);

		if (isActive)
		{
			// Deactivate
			setActiveCamera(view, EntryCameraId);

			using tRemoveInput = void (*)(void*, unsigned int localPlayerId);
			auto removeInput = reinterpret_cast<tRemoveInput>(OFFSET_REMOVEINPUT);
			removeInput(cam->m_input, 0);
		}
		else
		{
			// Activate
			setActiveCamera(view, FreeCameraId);

			using tAddInput = void (*)(void*, int priority, unsigned int localPlayerId);
			auto addInput = reinterpret_cast<tAddInput>(OFFSET_ADDINPUT);
			addInput(cam->m_input, 32, 0);
		}

		return !isActive; // returns new state: true = now in freecam
	}

	inline bool IsFreeCamActive()
	{
		ClientGameContext* ctx = ClientGameContext::GetInstance();
		if (!ctx || !ctx->m_clientGameView)
			return false;
		return ctx->m_clientGameView->m_activeCameraId == FreeCameraId;
	}
} // namespace Cypress
