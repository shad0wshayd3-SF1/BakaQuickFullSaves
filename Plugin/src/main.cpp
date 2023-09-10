class Hooks
{
public:
	static void Install()
	{
		hkGenerateSaveFileName::Install();
	}

private:
	class hkGenerateSaveFileName
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::Offset(0x023AC9AC), 0xAC };
			REL::safe_fill(target.address(), REL::NOP, 0x03);

			auto code = GenerateSaveFileName();
			assert(code.getSize() <= 0x03);
			REL::safe_write(target.address(), code.getCode(), code.getSize());
		}

	private:
		struct GenerateSaveFileName : Xbyak::CodeGenerator
		{
			GenerateSaveFileName()
			{
				cmp(edi, 10);
			}
		};
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

	// SFSE::AllocTrampoline(1 << 10);

	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
