
struct ArrowPhysicFix
{
	struct Patch : Xbyak::CodeGenerator
	{
		// Skyrim uses negative projectile velocity to calculate the bounce direction (?)
		// We replace this part in the code with the contact normal calculated before

		Patch()
		{
			if (REL::Module::IsAE())
			{
				movss(xmm2, ptr[rsp + 0x68]); // 6 bytes, AE: 640: 140791F24
				xorps(xmm2, xmm6); // 3 bytes
				movss(xmm1, ptr[rsp + 0x64]); // 6 bytes, AE: 640: 140791F11
				xorps(xmm1, xmm6);// 3 bytes
				movss(xmm0, ptr[rsp + 0x60]); // 6 bytes, AE: 640: 140791EFB
				xorps(xmm0, xmm6); // 3 bytes
				nop(6); // 6 bytes
			}
			else
			{
				movss(xmm2, ptr[rbp - 0x80]); // 5 bytes, SE: 140752345
				xorps(xmm2, xmm6); // 3 bytes, SE: 14075245F
				movss(xmm1, ptr[rsp + 0x7C]); // 6 bytes, SE: 140752329
				xorps(xmm1, xmm6);// 3 bytes
				movss(xmm0, ptr[rsp + 0x78]); // 6 bytes, SE: 14075230C
				xorps(xmm0, xmm6); // 3 bytes
				nop(7); // 7 bytes
			}
		}
	};

	static void Install()
	{
		REL::Relocation<std::uintptr_t> patchTarget{ RELOCATION_ID(43013, 44204), REL::Relocate(0x267, 0x2CF) };

		auto newCode = Patch();
		assert(newCode.getSize() == 33); // REL::Relocate(0x288 - 0x267, 0x2F0 - 0x2CF)
		REL::safe_write(patchTarget.address(), newCode.getCode(), newCode.getSize());

		SKSE::log::info("Installed ArrowPhysicFix!");
	}
};

#define DLLEXPORT __declspec(dllexport)

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []()
	{
		SKSE::PluginVersionData v;
		v.PluginName(Plugin::NAME);
		v.AuthorName("SkyHorizon"sv);
		v.PluginVersion(Plugin::VERSION);
		v.UsesAddressLibrary();
		v.UsesNoStructs();
		return v;
	}
();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
	SKSE::Init(skse, true);

	spdlog::set_pattern("[%H:%M:%S:%e] [%l] %v"s);

#ifndef NDEBUG
	spdlog::set_level(spdlog::level::trace);
	spdlog::flush_on(spdlog::level::trace);
#else
	spdlog::set_level(spdlog::level::info);
	spdlog::flush_on(spdlog::level::info);
#endif

	SKSE::log::info("Game version: {}", skse->RuntimeVersion());
	ArrowPhysicFix::Install();

	return true;
}