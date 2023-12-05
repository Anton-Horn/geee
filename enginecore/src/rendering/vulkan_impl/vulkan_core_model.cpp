
#include <cgltf.h>
#include "vulkan_core.h"
#include "utils/file_utils.h"
#include "rendering/stb_image.h"

namespace ec {

	void VulkanModel::create(VulkanContext& context, VulkanModelCreateInfo& createInfo) {

		if (createInfo.sourceFormat == VulkanModelSourceFormat::GLTF) {

			cgltf_options options = {};
			cgltf_data* data = nullptr;

			cgltf_result result = cgltf_parse_file(&options, createInfo.filepath.string().c_str(), &data);

			if (result == cgltf_result_success) {

				result = cgltf_load_buffers(&options, data, createInfo.filepath.string().c_str());

				if (result == cgltf_result_success) {

					assert(data->meshes_count == 1);
					assert(data->meshes[0].primitives_count == 1);
					assert(data->meshes[0].primitives[0].attributes_count > 1);
					assert(data->meshes[0].primitives[0].indices->component_type == cgltf_component_type_r_16u);
					assert(data->meshes[0].primitives[0].indices->stride == sizeof(uint8_t) * 2);

					// Indices

					uint8_t* bufferBase = (uint8_t*)data->meshes[0].primitives[0].indices->buffer_view->buffer->data;
					uint32_t indexDataSize = (uint32_t) data->meshes[0].primitives[0].indices->buffer_view->size;

					void* indexData = bufferBase + data->meshes[0].primitives[0].indices->buffer_view->offset;

					m_mesh.indexBuffer.create(context, indexDataSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
					m_mesh.indexBuffer.uploadData(context, indexData, indexDataSize);

					uint32_t indexCount = (uint32_t)data->meshes[0].primitives[0].indices->count;
					m_mesh.indexCount = indexCount;
	

					//Verticies
					uint32_t vertexCount = (uint32_t) data->meshes[0].primitives[0].attributes[0].data->count;
					uint32_t vertexSize = sizeof(VulkanModelVertex);
					uint32_t vertexDataSize = vertexSize * vertexCount;
					uint8_t* vertexData = new uint8_t[vertexDataSize];

					m_mesh.vertexCount = vertexCount;

					for (uint32_t i = 0; i < data->meshes[0].primitives[0].attributes_count; i++) {

						cgltf_attribute* attribute = data->meshes[0].primitives[0].attributes + i;
						bufferBase = (uint8_t*)attribute->data->buffer_view->buffer->data;
						uint32_t dataSize = (uint32_t) attribute->data->buffer_view->size;
						void const* data = bufferBase + attribute->data->buffer_view->offset;
						uint32_t stride = (uint32_t) attribute->data->stride;

						if (attribute->type == cgltf_attribute_type_position) {
							assert(stride == sizeof(float) * 3);
							fillBuffer((uint8_t const*)data, vertexData, stride, vertexSize, vertexCount, stride);
						}
						else if (attribute->type == cgltf_attribute_type_normal) {
							assert(stride == sizeof(float) * 3);
							fillBuffer((uint8_t const*)data, vertexData + sizeof(float) * 3, stride, vertexSize, vertexCount, stride);
						}
						else if (attribute->type == cgltf_attribute_type_texcoord) {
							assert(stride == sizeof(float) * 2);

							fillBuffer((uint8_t const*)data, vertexData + sizeof(float) * 6, stride, vertexSize, vertexCount, stride);
						}
					};

					m_mesh.vertexBuffer.create(context, vertexDataSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
					m_mesh.vertexBuffer.uploadData(context, vertexData, vertexDataSize);

					delete[] vertexData;

					//Material 

					assert(data->materials_count == 1);
					cgltf_material* material = &data->materials[0];
					assert(material->has_pbr_metallic_roughness);

					cgltf_texture_view albedoTextureView = material->pbr_metallic_roughness.base_color_texture;
					assert(!albedoTextureView.has_transform);
					assert(albedoTextureView.texcoord == 0);
					assert(albedoTextureView.texture);
					cgltf_texture* albedoTexture = albedoTextureView.texture;

					// Load texture

					cgltf_buffer_view* bufferView = albedoTexture->image->buffer_view;
					assert(bufferView->size < INT32_MAX);

					int bpp, width, height;
					uint8_t* textureData = stbi_load_from_memory((stbi_uc*)bufferView->buffer->data, (int)bufferView->size, &width, &height, &bpp, 4);
					assert(textureData);
					bpp = 4;

					m_mat.albedo.create(context, width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 1);

					m_mat.albedo.uploadData(context, textureData, width, height, bpp, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

					stbi_image_free(textureData);

				}
				else {
					EC_ERROR("Could not load model data");
				}

				cgltf_free(data);
			}
			else {
				EC_ERROR("Could not load model file");
			}

		}

	}

	void VulkanModel::destroy(VulkanContext& context) {

		m_mat.albedo.destroy(context);
		m_mesh.indexBuffer.destroy(context);
		m_mesh.vertexBuffer.destroy(context);

	}

	const VulkanModelMesh& VulkanModel::getMesh() const
	{
		return m_mesh;
	}

	const VulkanModelMaterial& VulkanModel::getMat() const
	{
		return m_mat;
	}

	void VulkanModel::fillBuffer(uint8_t const * inputBuffer, uint8_t* outputBuffer, uint32_t inputStride, uint32_t outputStride, uint32_t count, uint32_t elementSize)
	{

		for (uint32_t i = 0; i < count; i++) {

			for (uint32_t j = 0; j < elementSize; j++) {
				outputBuffer[j] = inputBuffer[j];
			}

			inputBuffer += inputStride;
			outputBuffer += outputStride;

		}

	}

}