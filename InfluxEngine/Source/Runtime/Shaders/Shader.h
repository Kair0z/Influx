#pragma once

namespace Influx::Shaders
{
	enum class EShaderType
	{
		PixelShader,
		VertexShader
	};

	enum class EShaderModel
	{
		SM_5_0
	};

	enum class EShaderFlags
	{
		None = 0,
		DisableOptimization = 1 << 0,
	};

	struct ShaderCompileOutput
	{
		const uint8_t* pShaderData;
		size_t ShaderSize;
		String ErrorMessage = "";

		bool IsValid() const;
	};

	class Shader final
	{
		Shader(); // Private constructor, only create Shaders through Shader::Compile(...)

	public:
		/* Runtime-compile a .hlsl file*/
		static Shader* Compile(const String& filepath, const String& entryPoint, const EShaderType type, const EShaderModel model,
			const Vector<String>& includeDirectories = {}, const Vector<String>& defines = {});

		/* Parses Shader config string from enum values */
		static bool ParseShaderString(const String& string, EShaderType& outType, EShaderModel& outModel);
		
		/* Create a Shader config string based on enum values*/
		static String MakeShaderString(const EShaderType type, const EShaderModel model);

		Shader(const Shader&) = delete;
		Shader(Shader&&) = delete;
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&&) = delete;
		virtual ~Shader() = default;

	private:
		EShaderType mShaderType;
		EShaderModel mShaderModel;
		EShaderFlags mFlags;

		ShaderCompileOutput mCompileOutput;
		String mCompileFilepath;
	};
}


