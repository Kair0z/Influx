#include "renderer_pch.h"

#include "InfluxRenderer/RootRenderer.h"
#include "InfluxRenderer/Renderers/SceneRenderer.h"

#include "InfluxGraphics/RHI.h"

#include "Core/Geometry/Vertex.h"

#include <d3dcompiler.h>
#include <dxcapi.h>

#pragma comment (lib, "dxcompiler.lib")

namespace Influx::Renderer
{
#pragma region SceneData
	void SceneRenderer::SetCamera(const CameraData& cameraData)
	{
		m_cameraData = cameraData;
	}

	void SceneRenderer::AddLight(const LightData& lightData)
	{
		m_lights.push_back(lightData);
	}

	void SceneRenderer::AddMesh(const MeshData& meshData)
	{
		m_meshes.push_back(meshData);
	}

	void SceneRenderer::AddMaterial(const MaterialData& material)
	{
		m_materials.push_back(material);
	}
#pragma endregion

	void SceneRenderer::OnPostInitializeAPI(const Graphics::EGraphicsAPI api, Graphics::RHIDevice* device)
	{
		struct ShaderDesc
		{
			enum class EProfile
			{
				_6_2,
				Max
			};

			enum class EType
			{
				VS,
				PS,
				Max
			};

			ShaderDesc(const WString& filepath, const WString& entrypoint, EType type, EProfile profile)
				: EntryPoint{ entrypoint }, Profile{ profile }, FilePath{ filepath }, Type{ type } {}

			Vector<WString> Defines;
			Vector<WString> Arguments;

			bool bStripPBD = true;
			bool bStripReflection = true;
			bool bDebug = _DEBUG;

			EType Type;
			EProfile Profile;
			WString EntryPoint;
			WString FilePath;
		};

		ShaderDesc vs_desc{ L"D:/Git/Influx/Resources/Shaders/shaders.hlsl", L"VSMain", 
			ShaderDesc::EType::VS, ShaderDesc::EProfile::_6_2 };

		ShaderDesc ps_desc{ L"D:/Git/Influx/Resources/Shaders/shaders.hlsl", L"PSMain", 
			ShaderDesc::EType::PS, ShaderDesc::EProfile::_6_2 };

		auto compile = [](const ShaderDesc& desc) -> Vector<byte>
		{
			Vector<byte> outResult{};

			// https://simoncoenen.com/blog/programming/graphics/DxcCompiling
			HRESULT result;

			IDxcUtils* pUtils;
			result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));

			IDxcBlobEncoding* pShaderSourceFile;
			result = pUtils->LoadFile(desc.FilePath.c_str(), nullptr, &pShaderSourceFile);

			IDxcCompiler3* pCompiler;
			result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

			Vector<LPCWSTR> arguments;
			//-E for the entry point (eg. VSMain)
			arguments.push_back(L"-E");
			arguments.push_back(desc.EntryPoint.c_str());

			//-T for the target profile (eg. ps_6_2)
			arguments.push_back(L"-T");

			WString shaderModelString{};
			switch (desc.Type)
			{
			case ShaderDesc::EType::PS:
				shaderModelString.append(L"ps");
				break;

			case ShaderDesc::EType::VS:
				shaderModelString.append(L"vs");
				break;

			default:
				assert(false);
				break;
			}
			switch (desc.Profile)
			{
			case ShaderDesc::EProfile::_6_2:
				shaderModelString.append(L"_6_2");
				break;

			default:
				assert(false);
				break;
			}
			arguments.push_back(shaderModelString.c_str());

			// Strip reflection data and pdbs (see later)
			// "The compiler will strip both the shader PDBs and reflection data from the Object part"
			// "it will STILL be in the compile result and can be extracted using DXC_OUT_PDB and DXC_OUT_REFLECTION"!!
			if (desc.bStripPBD) arguments.push_back(L"-Qstrip_debug");
			if (desc.bStripReflection) arguments.push_back(L"-Qstrip_reflect");

			// arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX
			if (desc.bDebug) arguments.push_back(DXC_ARG_DEBUG); //-Zi
			arguments.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR); //-Zp

			for (const WString& define : desc.Defines)
			{
				arguments.push_back(L"-D");
				arguments.push_back(define.c_str());
			}

			DxcBuffer sourceBuffer;
			sourceBuffer.Ptr = pShaderSourceFile->GetBufferPointer();
			sourceBuffer.Size = pShaderSourceFile->GetBufferSize();
			sourceBuffer.Encoding = 0;

			// Compiling the shaders...
			IDxcResult* pCompileResult;
			result = pCompiler->Compile(&sourceBuffer, arguments.data(), (uint32)arguments.size(), nullptr, IID_PPV_ARGS(&pCompileResult));

			// Extracting compile errors...
			IDxcBlobUtf8* pErrors = nullptr;
			result = pCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
			if (pErrors && pErrors->GetStringLength() > 0)
			{
				// Compile failed!
				printf((char*)pErrors->GetBufferPointer());
			}

			// Extracting Debug info...
			IDxcBlob* pDebugData = nullptr;
			IDxcBlobUtf16* pDebugDataPath = nullptr;
			result = pCompileResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pDebugData), &pDebugDataPath);

			// Extracting Reflection info...
			IDxcBlob* pReflectionData = nullptr;
			ID3D12ShaderReflection* pShaderReflection = nullptr;
			result = pCompileResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&pReflectionData), nullptr);
			if (pReflectionData)
			{
				DxcBuffer reflectionBuffer;
				reflectionBuffer.Ptr = pReflectionData->GetBufferPointer();
				reflectionBuffer.Size = pReflectionData->GetBufferSize();
				reflectionBuffer.Encoding = 0;

				result = pUtils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&pShaderReflection));
			}

			// Extracting resulting shader byte code...
			IDxcBlob* pResultData = nullptr;
			IDxcBlobUtf16* pResultOutputName = nullptr;
			result = pCompileResult->GetOutput(pCompileResult->PrimaryOutput(), IID_PPV_ARGS(&pResultData), &pResultOutputName);
			if (pResultData)
			{
				
				for (uint32 i = 0; i < pResultData->GetBufferSize(); ++i)
				{
					outResult.push_back(reinterpret_cast<byte*>(pResultData->GetBufferPointer())[i]);
				}
			}

			return outResult;
		};

		m_compiledVertexShader = compile(vs_desc);
		m_compiledPixelShader = compile(ps_desc);
	}

	void SceneRenderer::OnBuildRenderCommandList(const Renderer::RenderContext& context, Graphics::RHICommandList* cmdList)
	{
		// Pipeline & Layout:
		Graphics::RHIGraphicsPipelineDescription pipelineDesc{};
		{
			pipelineDesc.InputElements.push_back(Graphics::RHIGraphicsPipelineDescription::InputElement{ "POSITION", 0u, Graphics::ERHIFormat::RGB_32_Float, 0u, 0u, true, 0u });
			pipelineDesc.InputElements.push_back(Graphics::RHIGraphicsPipelineDescription::InputElement{ "COLOR",	0u, Graphics::ERHIFormat::RGBA_32_Float, 0u, 12u, true, 0u });
			pipelineDesc.InputElements.push_back(Graphics::RHIGraphicsPipelineDescription::InputElement{ "NORMAL", 0u, Graphics::ERHIFormat::RGB_32_Float, 0u, 28u, true, 0u });
			pipelineDesc.InputElements.push_back(Graphics::RHIGraphicsPipelineDescription::InputElement{ "UV",	0u, Graphics::ERHIFormat::RG_32_Float,	0u, 40u, true, 0u });

			pipelineDesc.VS = m_compiledVertexShader;
			pipelineDesc.PS = m_compiledPixelShader;

			pipelineDesc.PrimitiveTopologyType = Graphics::ERHIPrimitiveTopologyType::Triangle;

			pipelineDesc.BlendState = Graphics::RHIBlendState::GetDefault();
			pipelineDesc.RasterizerState = Graphics::RHIRasterizerState::GetDefault();
			pipelineDesc.DepthStencilState = Graphics::RHIDepthStencilState::GetDefault();

			pipelineDesc.RenderTargets[0].Format = Graphics::ERHIFormat::RGBA_8_Unorm;
		}

		Graphics::RHIGraphicsPipelineLayoutDescription layoutDesc{};

		mp_pipelineLayout	= context.GetAndOrCreateGraphicsPipelineLayout(layoutDesc);
		mp_pipeline			= context.GetAndOrCreateGraphicsPipeline(pipelineDesc, layoutDesc);

		// Setup SceneColour Texture:
		const Math::Vectoru2 swapchainDimensions = { context.GetSwapchain()->GetWidth(), context.GetSwapchain()->GetHeight() };

		Graphics::RHITextureDesc sceneColourDesc{};
		sceneColourDesc.Dimensions = swapchainDimensions;
		sceneColourDesc.Format = Graphics::ERHIFormat::RGBA_8_Unorm;
		sceneColourDesc.NumMips = 1;

		Graphics::RHIViewport viewport{};
		viewport.Width =	(float)swapchainDimensions.x;
		viewport.Height =	(float)swapchainDimensions.y;

		Graphics::RHIScissorRect scissorRect{};
		scissorRect.Width = swapchainDimensions.x;
		scissorRect.Height = swapchainDimensions.y;

		mp_sceneColourTexture = context.GetAndOrCreateTexture("SceneColour", sceneColourDesc);
		Graphics::RHIRenderTargetView* sceneColourRTV = mp_sceneColourTexture->GetAndOrCreateRenderTargetView(context.GetDevice());


		// Copy Scene Vertex Data into buffer:
#pragma region Vertex & Index Buffer
		Vector<Math::Vertex> vertices{};
		Vector<uint32> indices{};
		{
			for (const MeshData& mesh : m_meshes)
			{
				for (const Math::Vertex& vertex : mesh.m_meshData.GetVertices())
				{
					vertices.push_back(vertex);
				}

				for (const Scene::Mesh::Index& index : mesh.m_meshData.GetIndices())
				{
					indices.push_back(index);
				}
			}
		}

		const uint64 vertexSize			= sizeof(Influx::Math::Vertex);
		const uint64 numVertices		= vertices.size();
		const uint64 numIndices			= indices.size();
		const uint64 vertexBufferSize	= numVertices * vertexSize;
		const uint64 indexBufferSize	= numIndices * sizeof(uint32);

		if (mp_vertexBufferResource == nullptr || mp_vertexBufferResource->GetNumBytes() < vertexBufferSize)
		{
			mp_vertexBufferResource = context.GetDevice()->CreateVertexBufferResource(Graphics::ERHIResourceState::GenericRead, Graphics::ERHIFormat::Unknown, vertexBufferSize);
			mp_vertexBufferResource->ScopedMap([&vertices, vertexBufferSize](void* cpuHandle) // Copy data to GPU buffer...
			{
				memcpy(cpuHandle, vertices.data(), vertexBufferSize);
			});
		}
		
		if (mp_indexBufferResource == nullptr || mp_indexBufferResource->GetNumBytes() < indexBufferSize)
		{
			mp_indexBufferResource = context.GetDevice()->CreateIndexBufferResource(Graphics::ERHIResourceState::GenericRead, Graphics::ERHIFormat::Unknown, indexBufferSize);
			mp_indexBufferResource->ScopedMap([&indices, indexBufferSize](void* cpuHandle)
			{
				memcpy(cpuHandle, indices.data(), indexBufferSize);
			});
		}
#pragma endregion

		// The actual rendering...
		cmdList->ClearRTV(sceneColourRTV, { 1.0f, 0.0f, 0.0f, 1.0f });
		{
			cmdList->BindPipelineLayout(mp_pipelineLayout);
			cmdList->BindPipelineState(mp_pipeline);

			cmdList->BindRenderTarget(sceneColourRTV);
			cmdList->BindViewports(viewport);
			cmdList->BindScissorRect(scissorRect);

			cmdList->SetPrimitiveTopology(Graphics::ERHIPrimitiveTopology::TriangleList);

			if (mp_vertexBufferResource != nullptr)
			{
				cmdList->BindVertexBuffer(mp_vertexBufferResource, vertexBufferSize, vertexSize);
			}

			if (mp_indexBufferResource != nullptr)
			{
				cmdList->BindIndexBuffer(mp_indexBufferResource, indexBufferSize);
				cmdList->DrawIndexedInstanced(numIndices, 1u, 0u, 0u, 0u);
			}
		}
		context.CopyTextureIntoSwapchain(mp_sceneColourTexture, cmdList);
	}

	void SceneRenderer::OnWindowResize(const Renderer::RenderContext& context, const Math::Vectoru2& oldSize, const Math::Vectoru2& newSize)
	{

	}

	void SceneRenderer::OnPreCleanupAPI(const Graphics::EGraphicsAPI api, Graphics::RHIDevice* device)
	{

	}
}

