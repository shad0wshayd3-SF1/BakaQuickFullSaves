class Config
{
public:
	class General
	{
	public:
		inline static DKUtil::Alias::Boolean bAutosaveMode{ "bAutosaveMode", "General" };
	};

	static void Load()
	{
		static auto MainConfig = COMPILE_PROXY("BakaQuickFullSaves.ini");
		MainConfig.Bind(General::bAutosaveMode, false);
		MainConfig.Load();
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
			static REL::Relocation<std::uintptr_t> target{ REL::ID(147941), 0xCF };
			auto& trampoline = SFSE::GetTrampoline();
			trampoline.write_call<5>(target.address(), Quicksave);
		}

	private:
		static void Quicksave(void* a_this)
		{
			if (*Config::General::bAutosaveMode)
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
			static REL::Relocation<std::uintptr_t> patch{ REL::ID(167005), 0xAE };
			REL::safe_fill(patch.address(), REL::NOP, 0x09);

			static REL::Relocation<std::uintptr_t> target{ REL::ID(167005), 0xC8 };
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

DLLEXPORT constinit auto SFSEPlugin_Version = []() noexcept
{
	SFSE::PluginVersionData data{};

	data.PluginVersion(Plugin::Version);
	data.PluginName(Plugin::NAME);
	data.AuthorName(Plugin::AUTHOR);
	data.UsesSigScanning(false);
	data.UsesAddressLibrary(true);
	data.HasNoStructUse(true);
	data.IsLayoutDependent(true);
	data.CompatibleVersions({ SFSE::RUNTIME_LATEST });

	return data;
}();

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

DLLEXPORT bool SFSEAPI SFSEPlugin_Load(const SFSE::LoadInterface* a_sfse)
{
#ifndef NDEBUG
	while (!IsDebuggerPresent())
	{
		Sleep(100);
	}
#endif

	SFSE::Init(a_sfse);

	DKUtil::Logger::Init(Plugin::NAME, std::to_string(Plugin::Version));

	INFO("{} v{} loaded", Plugin::NAME, Plugin::Version);

	SFSE::AllocTrampoline(1 << 5);
	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	Config::Load();

	return true;
}
