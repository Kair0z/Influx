#include "influx_shader.h"

#if INFLUX_SHADER_BACKEND_SLANG
#include "slang.h"

#pragma comment(lib, "slang.lib")

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
	template <typename _t>
	using sl_ptr = _t*;

	struct target_info final
	{
		e_shader_binary_output	m_output_format;
		e_shader_target			m_shader_level;
	};

	using rfl_varlayout = slang::VariableLayoutReflection;
	using rfl_typelayout = slang::TypeLayoutReflection;

	// reflection handles 2 units:
	// variables: float x;
	// types: float, myStruct, etc.
	struct variable_reflection final {
		sl_ptr<slang::VariableReflection> m_reflection;
		sl_ptr<slang::VariableLayoutReflection> m_layout;

		variable_reflection() = default;
		variable_reflection(rfl_varlayout* layout) {
			m_layout = layout;
			m_reflection = layout->getVariable();
		}

		void print(std::stringstream& stream) {

		}
	};

	struct type_reflection final {
		sl_ptr<slang::TypeReflection> m_reflection;
		sl_ptr<rfl_typelayout> m_layout;

		type_reflection() = default;
		type_reflection(rfl_typelayout* layout) {
			m_layout = layout;
			m_reflection = layout->getType();
		}

		void print(std::stringstream& stream) {

		}
	};

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
	static const slang::TargetDesc translate(const target_info& target, sl_global_session& sesh) {
		sl_target_desc result{};
		const e_shader_target sh_target = target.m_shader_level;
		const char* target_cmd_arg = make_profile_cmd_arg(sh_target);
		result.format = translate(target.m_output_format);
		result.profile = sesh.findProfile(target_cmd_arg);
		result.compilerOptionEntries;
		result.compilerOptionEntryCount;
		result.flags;
		result.floatingPointMode;
		result.forceGLSLScalarBufferLayout;
		result.lineDirectiveMode;
		return result;
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

	template <typename _func>
	result<> scoped_compile_request(
		const vector<target_info>& target_infos,
		const vector<string>& include_folders,
		_func&& func)
	{
		using result_type = result<>;

		// get the global session
		result<sl_ptr<sl_global_session>> global_sesh_res = get_or_create_global_session();
		if (global_sesh_res.is_success() == false)
			return result_type::make_error(global_sesh_res);
		sl_global_session& global_session = *global_sesh_res.get();

		// translate the target descs using the global session
		vector<sl_target_desc> target_descs{};
		for (const auto& info : target_infos) {
			target_descs.push_back(translate(info, global_session));
		}
		vector<const char*> include_folders_cstr{};
		for (const auto& folder : include_folders) {
			include_folders_cstr.push_back(folder.c_str());
		}

		auto child_sesh_res = create_session(global_session, target_descs, include_folders_cstr);
		if (!child_sesh_res.is_success())
			return result_type::make_error(child_sesh_res);
		sl_session& child_session = *child_sesh_res.get();

		// create the compile request
		auto create_request_res = create_compile_request(child_session);
		if (!create_request_res.is_success())
			return result_type::make_error(create_request_res);
		
		// run the compile request function
		auto request_res = func(*create_request_res.get());

		// cleanup session & request
		create_request_res.get()->release();
		child_session.release();

		if (!request_res.is_success())
			return result_type::make_error(request_res);

		return {};
	}

	template <typename _func>
	void traverse_variable_layout(rfl_varlayout& var_layout, const uint32 level, _func&& func) {

		func(var_layout, level);

		// what type is this var?
		auto type = var_layout.getType();
		auto type_layout = var_layout.getTypeLayout();
		
		// if the type layout specifies sub-field variables, traverse those variables
		const int field_count = type_layout->getFieldCount();
		for (int i = 0; i < field_count; ++i) {
			auto field_var_layout = type_layout->getFieldByIndex(i);
			traverse_variable_layout(*field_var_layout, level + 1, func);
		}

		// traverse the element var
		const auto kind = type_layout->getKind();
		switch (kind)
		{
		case slang::TypeReflection::Kind::ConstantBuffer:
		case slang::TypeReflection::Kind::ParameterBlock:
		case slang::TypeReflection::Kind::TextureBuffer:
		case slang::TypeReflection::Kind::ShaderStorageBuffer:
		{
			rfl_varlayout* element_var_layout = type_layout->getElementVarLayout();
			traverse_variable_layout(*element_var_layout, level, func);
		}
		break;
		}
	}

	result<reflection> translate_reflection(sl_program_reflection& refl)
	{
		// https://shader-slang.org/slang/user-guide/reflection
		using result_type = result<reflection>;
		reflection output{};

		std::stringstream stream{};
		stream << "[Reflection]";

		uint32 current_parmblock = reflection::k_parmblock_invalid;
		uint32 current_resource = reflection::k_parmblock_invalid;
		static const char* k_no_name = "none";
		static const auto print_var = [&stream](slang::TypeReflection::Kind kind, const char* var_name) {
			switch (kind)
			{
			case slang::TypeReflection::Kind::ParameterBlock:
			{
				stream << "Paramblock ";
				stream << var_name;
			}break;
			case slang::TypeReflection::Kind::Struct:
			{
				stream << "Struct ";
				stream << var_name;
			}break;
			case slang::TypeReflection::Kind::ConstantBuffer:
			{
				stream << "ConstantBuffer ";
				stream << var_name;
			}break;
			case slang::TypeReflection::Kind::Resource:
			{
				stream << "ShaderResource ";
				stream << var_name;
			}break;
			case slang::TypeReflection::Kind::SamplerState:
			{
				stream << "SamplerState ";
				stream << var_name;
			}break;
			case slang::TypeReflection::Kind::Matrix:
			{
				stream << "Matrix ";
				stream << var_name;
			}break;
			case slang::TypeReflection::Kind::None:
			{
				stream << "none ";
			}break;
			case slang::TypeReflection::Kind::Vector:
			{
				stream << "vector ";
				stream << var_name;
			}break;
			case slang::TypeReflection::Kind::Scalar:
			{
				stream << "scalar ";
				stream << var_name;
			}break;
			default:
				influx_assert(false); // increment this clause as we go.
			}
		};

		static auto traverse_func = [&stream, &refl, &output, &current_parmblock, &current_resource]
		(rfl_varlayout& var_layout, const uint32 level)
		{
			// add nested print
			stream << "\n";
			for (uint32 i = 0u; i < level; ++i) {
				stream << "  ";
			}

			auto type = var_layout.getType();
			auto var = var_layout.getVariable();
			const char* var_name = var ? var->getName() : k_no_name;
			auto type_layout = var_layout.getTypeLayout();
			const auto kind = type_layout->getKind();

			const bool is_paramblock_kind =
				kind == slang::TypeReflection::Kind::ParameterBlock;
			const bool is_resource_kind =
				kind == slang::TypeReflection::Kind::ConstantBuffer ||
				kind == slang::TypeReflection::Kind::Resource ||
				kind == slang::TypeReflection::Kind::SamplerState ||
				kind == slang::TypeReflection::Kind::ShaderStorageBuffer ||
				kind == slang::TypeReflection::Kind::TextureBuffer;
			const bool is_field_kind =
				kind == slang::TypeReflection::Kind::Matrix ||
				kind == slang::TypeReflection::Kind::Scalar ||
				kind == slang::TypeReflection::Kind::Vector;

			reflection::resource* resource = nullptr;
			reflection::parmblock* parmblock = nullptr;
			if (is_resource_kind)
			{
				resource = &output.add_resource(current_parmblock);
				current_resource = output.m_resources.size() - 1u;
				resource->m_arraysize;
				resource->m_bytesize;
				reflection::set_name(resource->m_name, var_name);
				resource->m_nested_block_index;
				resource->m_parent_block_index = current_parmblock;
				resource->m_register_index;
				resource->m_register_space;
			}
			if (is_paramblock_kind)
			{
				parmblock = &output.add_parmblock();
				reflection::set_name(parmblock->m_name, var_name);
				current_parmblock = output.m_parmblocks.size() - 1u;
			}
				
			switch (kind)
			{
			case slang::TypeReflection::Kind::ConstantBuffer:
			{
				if (resource) resource->m_type = reflection::e_resource_type::constbuffer;
			}break;
			case slang::TypeReflection::Kind::Resource:
			{
				if (resource) resource->m_type = reflection::e_resource_type::structbuff;
			}break;
			case slang::TypeReflection::Kind::SamplerState:
			{
				if (resource) resource->m_type = reflection::e_resource_type::sampler;
			}break;
			}

			print_var(kind, var_name);
		};

		stream << "\nGlobals: ";
		slang::VariableLayoutReflection* global_var_layout = refl.getGlobalParamsVarLayout();
		traverse_variable_layout(*global_var_layout, 0u, traverse_func);

		static auto entrypoint_input_traverse_func = [&output, &stream](rfl_varlayout& var_layout, const uint32 level) {
			// add nested print
			stream << "\n";
			for (uint32 i = 0u; i < level; ++i) {
				stream << "  ";
			}

			auto type = var_layout.getType();
			auto var = var_layout.getVariable();
			const char* var_name = var ? var->getName() : k_no_name;
			auto type_layout = var_layout.getTypeLayout();
			const auto kind = type_layout->getKind();
			const char* varlayout_name = var_layout.getName();
			const char* type_name = type ? type->getName() : k_no_name;
			const char* typelayout_name = type_layout ? type_layout->getName() : k_no_name;
			const bool is_field_kind =
				kind == slang::TypeReflection::Kind::Matrix ||
				kind == slang::TypeReflection::Kind::Scalar ||
				kind == slang::TypeReflection::Kind::Vector;
			
			if (is_field_kind)
			{
				auto& newparam = output.add_ioparam(0u);
				newparam.m_is_input = true;
				reflection::set_name(newparam.m_name, var_name);
				reflection::set_name(newparam.m_typename, type_name);
			}

			print_var(kind, var_name);
		};
		static auto entrypoint_output_traverse_func = [&output, &stream](rfl_varlayout& var_layout, const uint32 level) {
			// add nested print
			stream << "\n";
			for (uint32 i = 0u; i < level; ++i) {
				stream << "  ";
			}

			auto type = var_layout.getType();
			auto var = var_layout.getVariable();
			const char* var_name = var ? var->getName() : k_no_name;
			auto type_layout = var_layout.getTypeLayout();
			const auto kind = type_layout->getKind();
			const char* varlayout_name = var_layout.getName();
			const char* type_name = type ? type->getName() : k_no_name;
			const char* typelayout_name = type_layout ? type_layout->getName() : k_no_name;
			
			const bool is_field_kind =
				kind == slang::TypeReflection::Kind::Matrix ||
				kind == slang::TypeReflection::Kind::Scalar ||
				kind == slang::TypeReflection::Kind::Vector;

			if (is_field_kind)
			{
				auto& newparam = output.add_ioparam(0u);
				newparam.m_is_input = false;
				reflection::set_name(newparam.m_name, var_name);
				reflection::set_name(newparam.m_typename, typelayout_name);
			}

			print_var(kind, var_name);
		};

		stream << "\nEntrypoints: ";
		SlangUInt entryPointCount = refl.getEntryPointCount();
		for (int i = 0; i < entryPointCount; ++i)
		{
			slang::EntryPointReflection& entrypoint = *refl.getEntryPointByIndex(i);
			stream << "\nstage: ";
			switch (entrypoint.getStage())
			{
			case SlangStage::SLANG_STAGE_VERTEX: stream << "vertex"; break;
			case SlangStage::SLANG_STAGE_PIXEL: stream << "pixel"; break;
			case SlangStage::SLANG_STAGE_COMPUTE: stream << "compute"; break;
			}

			stream << "\ninputs: ";
			traverse_variable_layout(*entrypoint.getVarLayout(), 0u, entrypoint_input_traverse_func);
			stream << "\noutputs: ";
			traverse_variable_layout(*entrypoint.getResultVarLayout(), 0u, entrypoint_output_traverse_func);
		}

		std::cout << stream.str() << "\n";
		return output;
	}

	result<compile_output> compile_shader_in_file(
		const string& filepath,
		const shader_signature& signature,
		const compile_args& args)
	{
		vector<target_info> targets{};
		{
			target_info info = {};
			info.m_output_format = args.m_output_format;
			info.m_shader_level = signature.m_target;
			targets.push_back(info);
		}
		vector<string> include_folders{};
		{
			include_folders.push_back(args.m_include_folder);
		}

		compile_output output{};
		auto request_res = scoped_compile_request(targets, include_folders,
			[&args, &filepath, &signature, &output](sl_compile_request& request) -> result<> {

				using result_type = result<>;

				// 1. add our 1 translation unit (shader)
				const e_shader_language source_lang = args.m_source_language;
				const int trans_unit_index = request.addTranslationUnit(translate(source_lang), "shader");
				request.addTranslationUnitSourceFile(trans_unit_index, filepath.c_str());
				const e_shader_type sh_type = signature.m_type;
				const char* sh_entrypoint = signature.m_entrypoint.c_str();
				request.addEntryPoint(trans_unit_index, sh_entrypoint, translate(sh_type));
				
				// add commandline
				{
					vector<const char*> args;
					request.processCommandLineArguments(args.data(), (uint32)args.size());
				}

				// 2. compile the shader
				sl_result compile_sres = request.compile();
				if (SLANG_FAILED(compile_sres))
				{
					const char* diag_output = request.getDiagnosticOutput();
					return result_type::make_error(diag_output);
				}

				const int entrypointIndex = 0;
				uint64 out_bytecode_size = 0u;
				const void* bytecode = request.getEntryPointCode(entrypointIndex, &out_bytecode_size);

				// output: get the compiled bytecode
				output.m_bytecode.resize(out_bytecode_size);
				memcpy(output.m_bytecode.data(), bytecode, out_bytecode_size);

				// output: get the reflection
				if (args.m_reflection_enabled || true)
				{
					sl_ptr<sl_program> program = nullptr;
					sl_result get_prgm_res = request.getProgramWithEntryPoints(&program);
					if (SLANG_FAILED(get_prgm_res))
						return result_type::make_error("getProgram() failed!");

					auto translated_reflection = translate_reflection(*program->getLayout());
					if (!translated_reflection.is_success())
						return result_type::make_error(translated_reflection);

					output.m_reflection = translated_reflection.get();
				}

				return {};
			});

		using result_type = result<compile_output>;
		if (!request_res.is_success())
			return result_type::make_error(request_res);

		return output;
	}

	result<compile_output> compile_shader_in_source(
		const string& shader_source,
		const shader_signature& signature,
		const compile_args& args)
	{
		using result_type = result<compile_output>;

		vector<target_info> targets{}; {
			target_info info = {};
			info.m_output_format = args.m_output_format;
			info.m_shader_level = signature.m_target;
			targets.push_back(info);
		}
		vector<string> include_folders{}; {
			include_folders.push_back(args.m_include_folder);
		}

		compile_output output{};
		auto request_res = scoped_compile_request(targets, include_folders,
			[&args, &signature, &output, &shader_source](sl_compile_request& request) -> result<> {
				using result_type = result<>;

				// 1. add our 1 translation unit (shader)
				const e_shader_language source_lang = args.m_source_language;
				const int trans_unit_index = request.addTranslationUnit(translate(source_lang), "");
				request.addTranslationUnitSourceString(trans_unit_index, "", shader_source.c_str());
				const e_shader_type sh_type = signature.m_type;
				const char* sh_entrypoint = signature.m_entrypoint.c_str();
				request.addEntryPoint(trans_unit_index, sh_entrypoint, translate(sh_type));

				sl_result compile_sres = request.compile();
				if (SLANG_FAILED(compile_sres))
				{
					const char* diag_output = request.getDiagnosticOutput();
					return result_type::make_error(diag_output);
				}

				const int entrypointIndex = 0;
				uint64 out_bytecode_size = 0u;
				const void* bytecode = request.getEntryPointCode(entrypointIndex, &out_bytecode_size);

				// output: get the compiled bytecode
				output.m_bytecode.resize(out_bytecode_size);
				memcpy(output.m_bytecode.data(), bytecode, out_bytecode_size);

				// output: get the reflection
				if (args.m_reflection_enabled)
				{
					sl_ptr<sl_program> program = nullptr;
					sl_result get_prgm_res = request.getProgram(&program);
					if (SLANG_FAILED(get_prgm_res))
						return result_type::make_error("getProgram() failed!");

					auto translated_reflection = translate_reflection(*program->getLayout());
					if (!translated_reflection.is_success())
						return result_type::make_error(translated_reflection);

					output.m_reflection = translated_reflection.get();
				}

				return {};
			});

		using result_type = result<compile_output>;
		if (!request_res.is_success())
			return result_type::make_error(request_res);

		return output;
	}

	result<reflect_output> reflect_bytecode(const bytecode& bytecode)
	{
		using result_type = result<reflect_output>;
		return {};
	}
}

#endif // INFLUX_SHADER_BACKEND_SLANG