#include "influx_shader.h"

#if INFLUX_SHADER_BACKEND_SLANG
#include "slang.h"

namespace influx::shader
{
	using sl_result				= SlangResult;
	using sl_compile_result		= slang::ICompileResult;
	using sl_global_session		= slang::IGlobalSession;
	using sl_session			= slang::ISession;
	using sl_compile_request	= SlangCompileRequest;
	using sl_target_desc		= slang::TargetDesc;
	using sl_reflection			= SlangReflection;
	using sl_program			= slang::IComponentType;
	using sl_program_reflection = slang::ProgramLayout;
	using sl_layout_unit		= slang::ParameterCategory;

	using rfl_varlayout = slang::VariableLayoutReflection;
	using rfl_typelayout = slang::TypeLayoutReflection;

	struct variable_reflection final {
		slang::VariableReflection* m_reflection;
		slang::VariableLayoutReflection* m_layout;

		variable_reflection() = default;
		variable_reflection(rfl_varlayout* layout) {
			m_layout = layout;
			m_reflection = layout->getVariable();
		}
	};
	struct type_reflection final
	{
		slang::TypeReflection* m_reflection;
		slang::TypeLayoutReflection* m_layout;

		type_reflection() = default;
		type_reflection(rfl_typelayout* layout) {
			m_layout = layout;
			m_reflection = layout->getType();
		}
	};

	template <typename _t>
	using sl_ptr				= _t*;

	result<sl_ptr<sl_global_session>> create_global_session()
	{
		using result_type = result<sl_ptr<sl_global_session>>;
		SlangGlobalSessionDesc sesh_desc{};
		sl_global_session* sesh = nullptr;
		sl_result sres = slang::createGlobalSession(&sesh_desc, &sesh);
		if (SLANG_FAILED(sres))
			return result_type::make_error("slang::createGlobalSession failed!");
		return sesh;
	}
	result<sl_ptr<sl_session>> create_session(
		sl_global_session& parent_sesh,
		const vector<sl_target_desc>& targets,
		const vector<const char*>& include_folders)
	{
		using result_type = result<sl_ptr<sl_session>>;
		
		slang::SessionDesc sesh_desc{};
		sesh_desc.allowGLSLSyntax;
		sesh_desc.compilerOptionEntries;
		sesh_desc.compilerOptionEntryCount;
		sesh_desc.defaultMatrixLayoutMode;
		sesh_desc.enableEffectAnnotations;
		sesh_desc.fileSystem;
		sesh_desc.flags;
		sesh_desc.preprocessorMacroCount;
		sesh_desc.preprocessorMacros;
		sesh_desc.searchPathCount = include_folders.size();
		sesh_desc.searchPaths = include_folders.data();
		sesh_desc.skipSPIRVValidation;
		sesh_desc.structureSize;
		sesh_desc.targetCount = targets.size();
		sesh_desc.targets = targets.data();

		sl_session* sesh = nullptr;
		sl_result sres = parent_sesh.createSession(sesh_desc, &sesh);
		if (SLANG_FAILED(sres))
			return result_type::make_error("slang::createSession failed!");
		return sesh;
	}
	result<sl_ptr<sl_compile_request>> create_compile_request(sl_session& sesh)
	{
		using result_type = result<sl_ptr<sl_compile_request>>;
		sl_compile_request* request = nullptr;
		sl_result sres = sesh.createCompileRequest(&request);

		if (SLANG_FAILED(sres))
			return result_type::make_error("slang::createCompileRequest failed");

		return request;
	}

	static constexpr SlangSourceLanguage translate(e_shader_language language)
	{
		switch (language)
		{
		case e_shader_language::SLANG: return SlangSourceLanguage::SLANG_SOURCE_LANGUAGE_SLANG;
		case e_shader_language::HLSL: return SlangSourceLanguage::SLANG_SOURCE_LANGUAGE_HLSL;
		}
		influx_assert(false); // append to supported array!
		return SlangSourceLanguage::SLANG_SOURCE_LANGUAGE_UNKNOWN;
	}
	static constexpr SlangCompileTarget translate(e_shader_binary_output format)
	{
		switch (format)
		{
		case e_shader_binary_output::DXIL: return SlangCompileTarget::SLANG_DXIL;
		case e_shader_binary_output::SPIRV: return SlangCompileTarget::SLANG_SPIRV;
		}
		influx_assert(false); // append to supported array!
		return SlangCompileTarget::SLANG_TARGET_UNKNOWN;
	}
	static constexpr SlangStage translate(e_shader_type type)
	{
		switch (type)
		{
		case e_shader_type::vs: return SlangStage::SLANG_STAGE_VERTEX;
		case e_shader_type::ps: return SlangStage::SLANG_STAGE_FRAGMENT;
		case e_shader_type::cs: return SlangStage::SLANG_STAGE_COMPUTE;
		}
		influx_assert(false); // append to supported array!
		return SlangStage::SLANG_STAGE_COUNT;
	}
	static constexpr SlangProfileID translate(e_shader_target target)
	{
		switch (target)
		{
		case e_shader_target::_6_8: return SlangProfileID::SLANG_PROFILE_UNKNOWN;
		}
		return SlangProfileID::SLANG_PROFILE_UNKNOWN;
	}
	static constexpr const char* make_profile_cmd_arg(e_shader_target target)
	{
		// https://docs.shader-slang.org/en/latest/external/slang/docs/user-guide/a3-01-reference-capability-profiles.html
		switch (target)
		{
		case e_shader_target::_6_2: return "sm_6_2";
		case e_shader_target::_6_5: return "sm_6_5";
		case e_shader_target::_6_6: return "sm_6_6";
		case e_shader_target::_6_8: return "sm_6_8";
		}
		return "";
	}
	static result<sl_ptr<sl_global_session>> get_or_create_global_session()
	{
		using result_type = result<sl_ptr<sl_global_session>>;
	
		static sl_global_session* singleton = nullptr;
		if (singleton != nullptr)
			return singleton;

		auto create_res = create_global_session();
		if (create_res.is_success() == false)
			return result_type::make_error("create_global_session failed!");

		singleton = create_res.get();
		return singleton;
	}

	result<reflection> translate_reflection(sl_program_reflection& refl)
	{
		// https://shader-slang.org/slang/user-guide/reflection
		using result_type = result<reflection>;

		static const auto process_var = [](variable_reflection& variable)
		{

		};
		static const auto process_type = [](type_reflection& type) {
			using ekind = slang::TypeReflection::Kind;
			const char* name = type.m_reflection->getName();
			const ekind kind = type.m_reflection->getKind();
			switch (kind)
			{
			case ekind::Scalar:
			{
				// "float x;"
				// name : "float"
				// kind : Scalar
			}break;
			case ekind::Array:
			{

			}break;
			case ekind::Struct:
			{
				const uint32 num_fields = type.m_layout->getFieldCount();
				for (uint32 i = 0u; i < num_fields; ++i) {
					rfl_varlayout* field = type.m_layout->getFieldByIndex(i);
					process_var(variable_reflection(field));
				}
			}break;
			}
		};

		const int num_entrypoints = refl.getEntryPointCount();
		static auto process_scope = [](slang::VariableLayoutReflection& scope)
		{
			slang::TypeLayoutReflection& type_layout = *scope.getTypeLayout();
			const uint32 field_count = type_layout.getFieldCount();

			switch (type_layout.getKind())
			{
			case slang::TypeReflection::Kind::Struct:
			{
				for (int i = 0; i < field_count; i++)
				{
					// print("- ");

					slang::VariableLayoutReflection& field = *type_layout.getFieldByIndex(i);
					printVarLayout(param, &scopeOffsets);
				}
			} break;

			}
		};

		// global scope
		slang::VariableLayoutReflection* global_scope = refl.getGlobalParamsVarLayout();
		process_scope(*global_scope);

		return {};
	}

	result<compile_output> compile_shader_in_file(
		const string& filepath,
		const shader_signature& signature,
		const compile_args& args)
	{
		using result_type = result<compile_output>;

		// get the global session
		result<sl_ptr<sl_global_session>> global_sesh_res = get_or_create_global_session();
		if (global_sesh_res.is_success() == false)
			return result_type::make_error(global_sesh_res);
		sl_global_session& global_session = *global_sesh_res.get();

		vector<sl_target_desc> targets{};
		{
			sl_target_desc target{};
			target.format = translate(args.m_output_format);
			const e_shader_target sh_target = signature.m_target;
			const char* target_cmd_arg = make_profile_cmd_arg(sh_target);
			target.profile = global_session.findProfile(target_cmd_arg);
			targets.push_back(target);
		}
		const char* main_include_folder = args.m_include_folder.c_str();
		vector<const char*> include_folders{};
		{
			include_folders.push_back(main_include_folder);
		}

		// create child sesh
		auto child_sesh_res = create_session(global_session, targets, include_folders);
		if (!child_sesh_res.is_success())
			return result_type::make_error(child_sesh_res);
		sl_session& child_session = *child_sesh_res.get();

		// create compile request
		auto create_request_res = create_compile_request(child_session);
		if (!create_request_res.is_success())
			return result_type::make_error(create_request_res);

		// 1. add our 1 translation unit (shader)
		sl_ptr<sl_compile_request> request = create_request_res.get();
		const e_shader_language source_lang = args.m_source_language;
		const int trans_unit_index = request->addTranslationUnit(translate(source_lang), "no-name");

#if 0
		slang::TargetDesc target_desc{};
		{
			target_desc.compilerOptionEntries;
			target_desc.compilerOptionEntryCount;
			target_desc.flags;
			target_desc.floatingPointMode;
			target_desc.forceGLSLScalarBufferLayout;
			target_desc.format;
			target_desc.lineDirectiveMode;
			target_desc.profile;
			target_desc.structureSize;
		}
#endif	

		// 2. setup inputs (source file, include folder, entrypoint)
		request->addTranslationUnitSourceFile(trans_unit_index, filepath.c_str());
		request->addSearchPath(main_include_folder);
		const e_shader_type sh_type = signature.m_type;
		const char* sh_entrypoint = signature.m_entrypoint.c_str();
		request->addEntryPoint(trans_unit_index, sh_entrypoint, translate(sh_type));
		
		// finally, compile
		sl_result compile_sres = request->compile();
		if (SLANG_FAILED(compile_sres))
		{
			const char* diag_output = request->getDiagnosticOutput();
			return result_type::make_error(diag_output);
		}

		const int entrypointIndex = 0;
		uint64 out_bytecode_size = 0u;
		const void* bytecode = request->getEntryPointCode(entrypointIndex, &out_bytecode_size);

		// output: get the compiled bytecode
		compile_output compile_result{};
		compile_result.m_bytecode.resize(out_bytecode_size);
		memcpy(compile_result.m_bytecode.data(), bytecode, out_bytecode_size);
		
		// output: get the reflection
		if (args.m_reflection_enabled || true)
		{
			sl_ptr<sl_program> program = nullptr;
			sl_result get_prgm_res = request->getProgram(&program);
			if (SLANG_FAILED(get_prgm_res))
				return result_type::make_error("getProgram() failed!");

			auto translated_reflection = translate_reflection(*program->getLayout());
			if (!translated_reflection.is_success())
				return result_type::make_error(translated_reflection);

			compile_result.m_reflection = translated_reflection.get();
		}

		request->release();
		child_session.release();
		return compile_result;
	}

	result<compile_output> compile_shader_in_source(
		const string& shader_source,
		const shader_signature& signature,
		const compile_args& args)
	{
		using result_type = result<compile_output>;
		return {};
	}

	result<reflect_output> reflect_bytecode(const bytecode& bytecode)
	{
		using result_type = result<reflect_output>;
		return {};
	}

	result<parse_output> parse_shaders_in_file(const string& filepath)
	{
		using result_type = result<parse_output>;
		return {};
	}
}

#endif // INFLUX_SHADER_BACKEND_SLANG