class Config
{
public:
	class SectionGeneral
	{
	public:
		bool bAutosaveMode{ false };
	};

	// members
	SectionGeneral General;

public:
	static Config* GetSingleton()
	{
		static Config singleton;
		return std::addressof(singleton);
	}

	static void Load()
	{
		const auto plugin = SFSE::PluginVersionData::GetSingleton();
		auto config = std::filesystem::current_path() /=
			std::format("Data/SFSE/plugins/{}.ini"sv, plugin->GetPluginName());
		try
		{
			auto reader = figcone::ConfigReader{};
			*GetSingleton() = reader.readIniFile<Config>(config.make_preferred());
		}
		catch (const std::exception& e)
		{
			SFSE::log::error("{}", e.what());
		}

		SFSE::log::debug("bAutosaveMode is: {}"sv, GetSingleton()->General.bAutosaveMode);
	}
};

class Hooks
{
public:
	static void Install()
	{
		hkQuicksave::Install();
		hkQuickSaveLoadHandler::Install();
	}

private:
	class hkQuicksave
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::ID(147941), 0xD8 };
			auto& trampoline = SFSE::GetTrampoline();
			trampoline.write_call<5>(target.address(), Quicksave);
		}

	private:
		static void Quicksave(void* a_this)
		{
			if (Config::GetSingleton()->General.bAutosaveMode)
			{
				Autosave(a_this);
			}
			else
			{
				SaveGame(a_this, nullptr, -1, 0);
			}
		}

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
	};

	class hkQuickSaveLoadHandler
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> patch{ REL::ID(167005), 0xAF };
			REL::safe_fill(patch.address(), REL::NOP, 0x09);

			static REL::Relocation<std::uintptr_t> target{ REL::ID(167005), 0xCB };
			auto& trampoline = SFSE::GetTrampoline();
			_QuickSaveLoadHandler = trampoline.write_call<5>(target.address(), QuickSaveLoadHandler);
		}

	private:
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

		inline static REL::Relocation<decltype(&QuickSaveLoadHandler)> _QuickSaveLoadHandler;
	};
};

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

	const auto plugin = SFSE::PluginVersionData::GetSingleton();
	SFSE::log::info("{} {} loaded"sv, plugin->GetPluginName(), plugin->GetPluginVersion());

	Config::Load();

	SFSE::AllocTrampoline(32);
	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
