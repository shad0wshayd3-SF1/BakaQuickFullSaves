class Hooks
{
public:
	static void Install()
	{
		hkQuickSaveLoadHandler::Install();
	}

private:
	class hkQuickSaveLoadHandler
	{
	public:
		static void Install()
		{
			{
				static REL::Relocation<std::uintptr_t> target{ REL::Offset(0x028A826C), 0xAE };
				REL::safe_fill(target.address(), REL::NOP, 0x09);
			}

			{
				static REL::Relocation<std::uintptr_t> target{ REL::Offset(0x028A826C), 0xC3 };
				auto& trampoline = SFSE::GetTrampoline();
				_QuickSaveLoadHandler = trampoline.write_call<5>(target.address(), QuickSaveLoadHandler);
			}
		}

	private:
		static bool LoadMostRecent()
		{
			using func_t = decltype(&LoadMostRecent);
			static REL::Relocation<func_t> func{ REL::Offset(0x023A3F10) };
			return func();
		}

		static void QuickSaveLoadHandler(void* a_this, std::uint32_t a_flag)
		{
			switch (a_flag) {
			case 0x08:
				_QuickSaveLoadHandler(a_this, 0x02);
				return;
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

DLLEXPORT constinit auto SFSEPlugin_Version = []() noexcept {
	SFSE::PluginVersionData data{};

	data.PluginVersion(Plugin::Version);
	data.PluginName(Plugin::NAME);
	data.AuthorName(Plugin::AUTHOR);
	data.UsesSigScanning(false);
	//data.UsesAddressLibrary(true);
	data.HasNoStructUse(true);
	//data.IsLayoutDependent(true);
	data.CompatibleVersions({ SFSE::RUNTIME_LATEST });

	return data;
}();

namespace
{
	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept
	{
		switch (a_msg->type) {
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
	while (!IsDebuggerPresent()) {
		Sleep(100);
	}
#endif

	SFSE::Init(a_sfse);

	DKUtil::Logger::Init(Plugin::NAME, std::to_string(Plugin::Version));

	INFO("{} v{} loaded", Plugin::NAME, Plugin::Version);

	SFSE::AllocTrampoline(1 << 6);
	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
