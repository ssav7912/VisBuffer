#include "GLTFImporter.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../tinygltf/tiny_gltf.h";
#include "PSORegistry.h"
tinygltf::Model GLTFImporter::LoadGLB(const std::string& AssetPath)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err;
    std::string warning;
    bool success = loader.LoadBinaryFromFile(&model, &err, &warning, AssetPath);

    return model;
}

std::vector<std::unique_ptr<Model>> GLTFImporter::LoadMesh(const tinygltf::Model& model, GeometryHeap* Heap, PSORegistry* Registry, ID3D12GraphicsCommandList2* CommandList)
{
    std::vector<std::unique_ptr<Model>> models;
    for (const auto& mesh : model.meshes)
    {
        std::unique_ptr<Model> m = std::make_unique<Model>();

        m->LocalToWorld = DirectX::SimpleMath::Matrix::Identity;

        for (const auto& prim : mesh.primitives)
        {
            Model::Section meshSection{};

            constexpr size_t NUMATTRIBUTES = 2ul;
            std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescriptions{};
            inputElementDescriptions.reserve(NUMATTRIBUTES); 

            if (prim.attributes.find("POSITION") != prim.attributes.end())
            {
                tinygltf::Accessor positionAccessor = model.accessors[prim.attributes.at(std::string("POSITION"))];
                tinygltf::BufferView positionsBV = model.bufferViews[positionAccessor.bufferView];
                meshSection.Vertices.BufferIndex = positionsBV.buffer;
                meshSection.Vertices.OffsetInBytes = positionsBV.byteOffset + positionAccessor.byteOffset; //accessor has additional offset
                meshSection.Vertices.StrideInBytes = positionsBV.byteStride;
                meshSection.Vertices.Nmembers = positionAccessor.count;
                meshSection.Vertices.ElemType = BufferElem::VEC3;
                meshSection.Vertices.ComponentType = BufferType::FLOAT;
                meshSection.Vertices.LengthInBytes = positionAccessor.count * sizeof(DirectX::XMFLOAT3);

                inputElementDescriptions.emplace_back(D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

                //TODO: scene bounds
            }

            if (prim.attributes.find("COLOR_0") != prim.attributes.end())
            {
                tinygltf::Accessor colourAccessor = model.accessors[prim.attributes.at(std::string("COLOR_0"))];
                tinygltf::BufferView colorBV = model.bufferViews[colourAccessor.bufferView];
                meshSection.Colours.BufferIndex = colorBV.buffer;
                meshSection.Colours.OffsetInBytes = colorBV.byteOffset + colourAccessor.byteOffset;
                meshSection.Colours.StrideInBytes = colorBV.byteStride;
                meshSection.Colours.Nmembers = colourAccessor.count;

                switch (colourAccessor.type)
                {
                case TINYGLTF_TYPE_VEC3: { meshSection.Colours.ElemType = BufferElem::VEC3; break; };
                case TINYGLTF_TYPE_VEC4: { meshSection.Colours.ElemType = BufferElem::VEC4; break; };
                }

                switch (colourAccessor.componentType)
                {
                case TINYGLTF_COMPONENT_TYPE_FLOAT: { meshSection.Colours.ComponentType = BufferType::FLOAT; break; };
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: { meshSection.Colours.ComponentType = BufferType::UCHAR; break; };
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: { meshSection.Colours.ComponentType = BufferType::USHORT; break; };
                }
                inputElementDescriptions.emplace_back(D3D12_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 });

            }

            if (prim.indices != -1)
            {
                tinygltf::Accessor indicesAccessor = model.accessors[prim.indices];
                tinygltf::BufferView indicesBV = model.bufferViews[indicesAccessor.bufferView];
                meshSection.Indices.BufferIndex = indicesBV.buffer;
                meshSection.Indices.OffsetInBytes = indicesBV.byteOffset + indicesAccessor.byteOffset;
                meshSection.Indices.LengthInBytes = indicesBV.byteLength;
                meshSection.Indices.StrideInBytes = indicesBV.byteStride;
                meshSection.Indices.Nmembers = indicesAccessor.count;
                meshSection.Indices.ElemType = BufferElem::SCALAR;

                switch (indicesAccessor.componentType)
                {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                {
                    meshSection.Indices.ComponentType = BufferType::UCHAR;
                    break;
                }
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                {
                    meshSection.Indices.ComponentType = BufferType::USHORT;
                    break;
                }
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                    meshSection.Indices.ComponentType = BufferType::UINT;
                }

            }


            auto PSO = PSORegistry::DefaultPSO();
            PSO.PrimitiveTopologyType = ConvertTopologyType(prim.mode);
            PSO.InputLayout = { .pInputElementDescs = inputElementDescriptions.data(), .NumElements = static_cast<uint32_t>(inputElementDescriptions.size())};

            if (Registry->Registry.contains(PSO))
            {
                
            }

            //TODO: Normals, etc. 
            m->Sections.push_back(meshSection);
        }
        models.push_back(std::move(m));
    }
    Heap->DefaultGLTFHeap.UploadBuffers(model.buffers, CommandList);
    return models;
}

constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE GLTFImporter::ConvertTopologyType(uint32_t TinyGltfModeFlag)
{
    switch (TinyGltfModeFlag)
    {
        case TINYGLTF_MODE_LINE:
        case TINYGLTF_MODE_LINE_LOOP:
        case TINYGLTF_MODE_LINE_STRIP: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        case TINYGLTF_MODE_TRIANGLES:
        case TINYGLTF_MODE_TRIANGLE_FAN:
        case TINYGLTF_MODE_TRIANGLE_STRIP: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        case TINYGLTF_MODE_POINTS: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
        default: ASSERT(false, "switch overflow, This should not be reachable!");
    }
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
}
