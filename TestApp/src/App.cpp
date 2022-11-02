#include "SDG/SDG.h"
#include "SDG/EntryPoint.h"

class ExampleLayer : public SDG::Layer {
private:
	uint32_t* m_data = nullptr;
	SDG::Ref<SDG::Texture> m_texture;
	SDG::Ref<SDG::ComputeShader> m_shader;
	SDG::Ref<SDG::UniformBuffer> m_shaderDataBuffer;

	struct Sphere {
		glm::vec3 albedo = glm::vec3(1.0f, 1.0f, 1.0f);
		float radius = 1.0f;
		glm::vec3 pos = glm::vec3(0.0f);
		float placeholder1 = .0f;
	};
	struct Light {
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
		float lightMultiplier = 1.0f;
		glm::vec3 pos = glm::vec3(4.0f, 4.0f, 4.0f);
		float placeholder1 = .0f;
	};
	struct SceneData {
		glm::vec4 cameraPos = glm::vec4(0.0f,0.0f,8.0f,1.0f);
		Sphere spheres[50];
		Light lights[50];
		uint32_t sphereCount = 0;
		uint32_t lightCount = 0;
	}m_sceneData;
public:

	virtual void onAttach() override {
		const uint32_t width = 720, height = 720;
		m_texture = SDG::Texture::create(width, height);
		m_shaderDataBuffer = SDG::UniformBuffer::create(sizeof(SceneData), 0);

		Sphere sphere;
		sphere.pos = glm::vec3(0, 2, 0);
		sphere.albedo = glm::vec3(1, 1, 0);
		m_sceneData.spheres[0] = sphere;
		m_sceneData.sphereCount++;

		Light light;
		light.pos = glm::vec3(3.0f);
		m_sceneData.lights[0] = light;
		m_sceneData.lightCount++;


		{
			m_shader = SDG::ComputeShader::create("Shader", R"(
#version 430 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform writeonly image2D result;

struct Sphere {
	vec3 albedo;
	float radius;
	vec3 pos;
	float shininess;
};
struct Light {
	vec3 color;
	float multiplier;
	vec3 pos;
};

layout(std140, binding = 0) uniform ShaderData {
	vec4 cameraPos;
	Sphere spheres[50];
	Light lights[50];
	uint sphereCount;
	uint lightCount;
};

vec3 calcSphereColor(int sphereID, float hitDistance, vec3 rayDirection) {
	Sphere sphere = spheres[sphereID];
	
	vec3 origin = cameraPos.xyz - sphere.pos;
	vec3 worldPos = origin + rayDirection * hitDistance;
	vec3 normal = normalize(worldPos);
	vec3 view = normalize(rayDirection - worldPos);
	
	vec3 color = vec3(0);
	
	for(uint i = 0; i < lightCount; i++) {
		Light light = lights[i];
		vec3 lightDir = normalize(light.pos);
		vec3 diffuse = (max(dot(normal, lightDir), 0.0) * light.multiplier) * light.color;
		color += (diffuse) * sphere.albedo;
	}

	return color;
}

void main() {
	ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
	vec2 texCoord = { 1-(float(pixelCoord.x) / float(gl_NumWorkGroups.x)), 1-(float(pixelCoord.y) / float(gl_NumWorkGroups.y)) };
	vec2 coord = texCoord * 2 - 1; // -1 -> 1
	
	vec3 rayDirection = vec3(coord.x, coord.y, -1);
	
	int closestSphere = -1;
	float hitDistance = 5000;
	for(uint i = 0; i < sphereCount; i++) {
		Sphere sphere = spheres[i];
		
		vec3 origin = cameraPos.xyz - sphere.pos;
		float a = dot(rayDirection, rayDirection);
		float b = 2 * dot(origin, rayDirection);
		float c = dot(origin, origin) - sphere.radius * sphere.radius;
		float discriminant = b * b - 4 * a * c;
		if (discriminant < 0.0f)
			continue;
			
		float closestT = (-b - sqrt(discriminant)) / (2.0f * a);
		if (closestT > 0.0f && closestT < hitDistance)
		{
			hitDistance = closestT;
			closestSphere = int(i);
		}
	}
	vec3 color = vec3(0);
	if(closestSphere < 0) {
		vec3 skyColor1 = vec3(0.6, 0.6, 0.6);
		vec3 skyColor2 = vec3(0.53, 0.81, 0.92);
		color += mix(skyColor1, skyColor2, texCoord.y);
	}else {
		color += calcSphereColor(closestSphere, hitDistance, rayDirection);
	}
    
	imageStore(result, pixelCoord, vec4(color, 1.0));
}
)");
		}
	}
	virtual void onRender() override {
		uint32_t width = m_texture->getWidth(), height = m_texture->getHeight();

		m_shaderDataBuffer->setData(&m_sceneData, sizeof(SceneData));
		m_shader->setTexture("result", 0, m_texture, 0, SDG::AccesMode::WriteOnly);
		m_shader->Dispatch(m_texture->getWidth(), m_texture->getHeight(), 1, SDG::ComputeShader::Usage::Default);

		if (ImGui::Begin("Viewport")) {
			static float scale = 1.0f;
			ImGui::Text("Frametime: %.2fms", SDG::Application::get().getFrameTime() * 1000);
			ImGui::Text("FPS: %.2f", 1.0f / SDG::Application::get().getFrameTime());
			ImGui::DragFloat("Scale", &scale, .01f);
			scale = max(scale, .01f);
			ImGui::Image(m_texture, glm::vec2((float) m_texture->getWidth(), (float)m_texture->getHeight()) * scale);
		}
		ImGui::End();
		if (ImGui::Begin("Scene")) {
			static enum class InspectorMode { None,
				Sphere, Light
			} inspectorMode = InspectorMode::None;
			static int selected = -1;
			if (ImGui::TreeNodeEx("Spheres")) {
				if (ImGui::Button("Create")) {
					selected = m_sceneData.sphereCount;
					m_sceneData.sphereCount++;
				}
				for (int i = 0; i < (int)m_sceneData.sphereCount; i++) {
					if (ImGui::RadioButton((std::string("Sphere_") + std::to_string(i)).c_str(), (selected == i && inspectorMode == InspectorMode::Sphere))) {
						inspectorMode = InspectorMode::Sphere;
						selected = i;
					}
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNodeEx("Lights")) {
				if(ImGui::Button("Create")) {
					selected = m_sceneData.lightCount;
					m_sceneData.lightCount++;
				}
				for (int i = 0; i < (int)m_sceneData.lightCount; i++) {
					if (ImGui::RadioButton((std::string("Light_") + std::to_string(i)).c_str(), (selected == i && inspectorMode == InspectorMode::Light))) {
						inspectorMode = InspectorMode::Light;
						selected = i;
					}
				}
				ImGui::TreePop();
			}
			if (selected >= 0 && selected < 50 && ImGui::CollapsingHeader("Inspector")) {
				if (inspectorMode == InspectorMode::Sphere) {
					Sphere& sphere = m_sceneData.spheres[selected];
					ImGui::ColorEdit3("Albedo", glm::value_ptr(sphere.albedo));
					ImGui::DragFloat3("Position", glm::value_ptr(sphere.pos), .1f);
					ImGui::DragFloat("Radius", &sphere.radius, .1f, .0f, 100.0f);
				}
				else if (inspectorMode == InspectorMode::Light) {
					Light& light = m_sceneData.lights[selected];
					ImGui::ColorEdit3("Color", glm::value_ptr(light.color));
					ImGui::DragFloat3("Position", glm::value_ptr(light.pos), .1f);
					ImGui::DragFloat("Strength", &light.lightMultiplier, .1f, .0f, 10.0f);
				}
			}
		}
		ImGui::End();
	}
};

SDG::Application* SDG::CreateApplication()
{
	SDG::ApplicationInfo info;
	info.title = "Example";
	info.VSync = false;

	SDG::Application* app = new SDG::Application(info);
	app->pushLayer<ExampleLayer>();
	
	return app;
}