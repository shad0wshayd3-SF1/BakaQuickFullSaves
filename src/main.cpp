namespace Config
{
	namespace General
	{
		static REX::INI::Bool bAutosaveMode{ "General", "bAutosaveMode", false };
	}

	static void Load()
	{
		const auto ini = REX::INI::SettingStore::GetSingleton();
		ini->Init(
			"Data/SFSE/plugins/BakaQuickFullSaves.ini",
			"Data/SFSE/plugins/BakaQuickFullSavesCustom.ini");
		ini->Load();
	}
}

namespace Hooks
{
	namespace hkQuicksave
	{
		static void Autosave(void* a_this)
		{
			using func_t = decltype(&Autosave);
			REL::Relocation<func_t> func{ REL::ID(98645) };
			return func(a_this);
		}

		static void SaveGame(void* a_this, char* a_saveFileName, std::int32_t a_deviceID, std::uint32_t a_outputStats)
		{
			using func_t = decltype(&SaveGame);
			REL::Relocation<func_t> func{ REL::ID(98622) };
			return func(a_this, a_saveFileName, a_deviceID, a_outputStats);
		}

		static void Quicksave(void* a_this)
		{
			if (Config::General::bAutosaveMode.GetValue())
			{
				Autosave(a_this);
			}
			else
			{
				SaveGame(a_this, nullptr, -1, 0);
			}
		}

		inline static REL::Hook _Hook0{ REL::ID(98691), 0xD5, Quicksave };
	}

	class hkQuickSaveLoadHandler :
		public REX::Singleton<hkQuickSaveLoadHandler>
	{
	private:
		static bool LoadMostRecent()
		{
			using func_t = decltype(&LoadMostRecent);
			static REL::Relocation<func_t> func{ REL::ID(98705) };
			return func();
		}

		static void HandleEvent(void* a_this, RE::ButtonEvent* a_event)
		{
			if (a_event &&
				a_event->strUserEvent == "Quickload"sv &&
				a_event->value == 0.0f &&
				a_event->heldDownSecs > 0.0f)
			{
				if (auto slm = RE::BGSSaveLoadManager::GetSingleton();
					slm && slm->IsLoadingAllowed())
				{
					a_event->status = RE::InputEvent::Status::kStop;
					SFSE::GetTaskInterface()->AddTask(
						[]()
						{
							LoadMostRecent();
						});
					return;
				}
			}

			return _Hook0(a_this, a_event);
		}

		inline static REL::HookVFT _Hook0{ RE::VTABLE::QuickSaveLoadHandler[0], 0x08, HandleEvent };
	};
}

namespace
{
	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept
	{
		switch (a_msg->type)
		{
		case SFSE::MessagingInterface::kPostLoad:
			Config::Load();
			break;
		default:
			break;
		}
	}
}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse)
{
	SFSE::Init(a_sfse, { .trampoline = true });
	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);
	return true;
}
