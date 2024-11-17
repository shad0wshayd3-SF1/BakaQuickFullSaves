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
			REL::Relocation<func_t> func{ REL::ID(147827) };
			return func(a_this);
		}

		static void SaveGame(void* a_this, char* a_saveFileName, std::int32_t a_deviceID, std::uint32_t a_outputStats)
		{
			using func_t = decltype(&SaveGame);
			REL::Relocation<func_t> func{ REL::ID(147915) };
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

		static void Install()
		{
			static REL::Relocation target{ REL::ID(147941), 0xD8 };
			target.write_call<5>(Quicksave);
		}
	}

	namespace hkQuickSaveLoadHandler
	{
		static REL::Relocation<void(void*, std::uint32_t)> _QuickSaveLoadHandler;

		static bool LoadMostRecent()
		{
			using func_t = decltype(&LoadMostRecent);
			static REL::Relocation<func_t> func{ REL::ID(147756) };
			return func();
		}

		static void QuickSaveLoadHandler(void* a_this, std::uint32_t a_flag)
		{
			switch (a_flag)
			{
			case 0x10:
				LoadMostRecent();
				return;
			default:
				_QuickSaveLoadHandler(a_this, a_flag);
				return;
			}
		}

		static void Install()
		{
			static REL::Relocation patch{ REL::ID(167005), 0xAF };
			REL::safe_fill(patch.address(), REL::NOP, 0x09);

			static REL::Relocation target{ REL::ID(167005), 0xCB };
			_QuickSaveLoadHandler = target.write_call<5>(QuickSaveLoadHandler);
		}
	}

	static void Install()
	{
		hkQuicksave::Install();
		hkQuickSaveLoadHandler::Install();
	}
}

namespace
{
	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept
	{
		switch (a_msg->type)
		{
		case SFSE::MessagingInterface::kPostLoad:
		{
			Hooks::Install();
			break;
		}
		default:
			break;
		}
	}
}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse)
{
	SFSE::Init(a_sfse);

	Config::Load();

	SFSE::AllocTrampoline(32);
	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
