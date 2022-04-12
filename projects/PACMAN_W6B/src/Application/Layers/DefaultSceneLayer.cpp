#include "DefaultSceneLayer.h"

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <GLM/gtc/random.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)

#include <filesystem>

// Graphics
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/TextureCube.h"
#include "Graphics/Textures/Texture2DArray.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"
#include "Graphics/Framebuffer.h"

// Utilities
#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"
#include "Utils/GlmDefines.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Components/Light.h"

// Components
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
#include "Gameplay/Components/SimpleCameraControl.h"

#include "Gameplay/Components/SimpleObjectControl.h"
#include "Gameplay/Components/SimpleCameraFollow.h"
#include "Gameplay/Components/KillBehaviour.h"
#include "Gameplay/Components/GoalBehaviour.h"
#include "Gameplay/Components/SimpleLightFollow.h"
#include "Gameplay/Components/SimpleToggle.h"
#include "Gameplay/Components/ShaderContainer.h"

// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/Colliders/CylinderCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"

// GUI
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
#include "Gameplay/InputEngine.h"

#include "Application/Application.h"
#include "Gameplay/Components/ParticleSystem.h"
#include "Graphics/Textures/Texture3D.h"
#include "Graphics/Textures/Texture1D.h"
#include "Application/Layers/ImGuiDebugLayer.h"
#include "Application/Windows/DebugWindow.h"
#include "Gameplay/Components/ShadowCamera.h"
#include "Gameplay/Components/ShipMoveBehaviour.h"

DefaultSceneLayer::DefaultSceneLayer() :
	ApplicationLayer()
{
	Name = "Default Scene";
	Overrides = AppLayerFunctions::OnAppLoad;
}

DefaultSceneLayer::~DefaultSceneLayer() = default;

void DefaultSceneLayer::OnAppLoad(const nlohmann::json& config) {
	_CreateScene();
}

void DefaultSceneLayer::_CreateScene()
{
	using namespace Gameplay;
	using namespace Gameplay::Physics;

	Application& app = Application::Get();

	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene && std::filesystem::exists("scene.json")) {
		app.LoadScene("scene.json");
	} else {
		 
		// Basic gbuffer generation with no vertex manipulation
		ShaderProgram::Sptr deferredForward = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
		deferredForward->SetDebugName("Deferred - GBuffer Generation");  

		// Our foliage shader which manipulates the vertices of the mesh
		ShaderProgram::Sptr foliageShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/foliage.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});  
		foliageShader->SetDebugName("Foliage");   

		// This shader handles our multitexturing example
		ShaderProgram::Sptr multiTextureShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/vert_multitextured.glsl" },  
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_multitextured.glsl" }
		});
		multiTextureShader->SetDebugName("Multitexturing"); 

		// This shader handles our displacement mapping example
		ShaderProgram::Sptr displacementShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
		displacementShader->SetDebugName("Displacement Mapping");

		// This shader handles our cel shading example
		ShaderProgram::Sptr celShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/cel_shader.glsl" }
		});
		celShader->SetDebugName("Cel Shader");

		ShaderProgram::Sptr specShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/my_frag/frag_specular.glsl" }
		});
		specShader->SetDebugName("Textured Specular");

		ShaderProgram::Sptr basicShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/my_frag/frag_noLight.glsl" }
		});
		specShader->SetDebugName("Basic Shader");

		ShaderProgram::Sptr blinnShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/my_frag/frag_blinn_phong.glsl" }
		});
		specShader->SetDebugName("Blinn Phong Shader");

		ShaderProgram::Sptr toonShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/my_frag/frag_toon.glsl" }
		});
		specShader->SetDebugName("Toon Shader");

		// Load in the meshes
		MeshResource::Sptr monkeyMesh = ResourceManager::CreateAsset<MeshResource>("Monkey.obj");
		MeshResource::Sptr shipMesh   = ResourceManager::CreateAsset<MeshResource>("fenrir.obj");

		MeshResource::Sptr characterMesh = ResourceManager::CreateAsset<MeshResource>("objs/pacman/pacman.obj");
		MeshResource::Sptr terrainMesh = ResourceManager::CreateAsset<MeshResource>("objs/ground/terrain.obj");

		MeshResource::Sptr goalMesh = ResourceManager::CreateAsset<MeshResource>("objs/Coin.obj");
		MeshResource::Sptr carMesh = ResourceManager::CreateAsset<MeshResource>("objs/Car.obj");
		MeshResource::Sptr plainMesh = ResourceManager::CreateAsset<MeshResource>("objs/terrain/plainTerrain.obj");

		MeshResource::Sptr rock1Mesh = ResourceManager::CreateAsset<MeshResource>("objs/rocks/rock1.obj");
		MeshResource::Sptr branch1Mesh = ResourceManager::CreateAsset<MeshResource>("objs/branches/branches.obj");
		MeshResource::Sptr tree1Mesh = ResourceManager::CreateAsset<MeshResource>("objs/tree/tree.obj");
		MeshResource::Sptr coin1Mesh = ResourceManager::CreateAsset<MeshResource>("objs/coins/coin.obj");

		// Load in some textures
		Texture2D::Sptr    boxTexture   = ResourceManager::CreateAsset<Texture2D>("textures/box-diffuse.png");
		Texture2D::Sptr    boxSpec      = ResourceManager::CreateAsset<Texture2D>("textures/box-specular.png");
		Texture2D::Sptr    monkeyTex    = ResourceManager::CreateAsset<Texture2D>("textures/monkey-uvMap.png");
		Texture2D::Sptr    leafTex      = ResourceManager::CreateAsset<Texture2D>("textures/leaves.png");
		leafTex->SetMinFilter(MinFilter::Nearest);
		leafTex->SetMagFilter(MagFilter::Nearest);

		// Load in some textures
		Texture2D::Sptr		characterTex = ResourceManager::CreateAsset<Texture2D>("textures/RacoonUV.png");
		Texture2D::Sptr		yellowCarTex = ResourceManager::CreateAsset<Texture2D>("textures/YellowStripeUV.png");
		Texture2D::Sptr		terrianTex = ResourceManager::CreateAsset<Texture2D>("textures/GroundTex.png");
		Texture2D::Sptr		goalTex = ResourceManager::CreateAsset<Texture2D>("textures/CoinUV.png");
		Texture2D::Sptr     coinTex = ResourceManager::CreateAsset<Texture2D>("textures/CoinTexture.png");
		Texture2D::Sptr     GrassTex = ResourceManager::CreateAsset<Texture2D>("textures/FullGrass.png");
		Texture2D::Sptr     TreeTex = ResourceManager::CreateAsset<Texture2D>("textures/treeTexture.png");
		Texture2D::Sptr     BrownTex = ResourceManager::CreateAsset<Texture2D>("textures/Brown.png");

		//Texture2D::Sptr    boxTexture = ResourceManager::CreateAsset<Texture2D>("textures/box-diffuse.png");
		//Texture2D::Sptr    boxSpec = ResourceManager::CreateAsset<Texture2D>("textures/box-specular.png");

		// Load some images for drag n' drop
		ResourceManager::CreateAsset<Texture2D>("textures/flashlight.png");
		ResourceManager::CreateAsset<Texture2D>("textures/flashlight-2.png");
		ResourceManager::CreateAsset<Texture2D>("textures/light_projection.png");

		Texture2DArray::Sptr particleTex = ResourceManager::CreateAsset<Texture2DArray>("textures/particles.png", 2, 2);

		//DebugWindow::Sptr debugWindow = app.GetLayer<ImGuiDebugLayer>()->GetWindow<DebugWindow>();

#pragma region Basic Texture Creation
		Texture2DDescription singlePixelDescriptor;
		singlePixelDescriptor.Width = singlePixelDescriptor.Height = 1;
		singlePixelDescriptor.Format = InternalFormat::RGB8;

		float normalMapDefaultData[3] = { 0.5f, 0.5f, 1.0f };
		Texture2D::Sptr normalMapDefault = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		normalMapDefault->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, normalMapDefaultData);

		float solidGrey[3] = { 0.5f, 0.5f, 0.5f };
		float solidBlack[3] = { 0.0f, 0.0f, 0.0f };
		float solidWhite[3] = { 1.0f, 1.0f, 1.0f };

		Texture2D::Sptr solidBlackTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidBlackTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidBlack);

		Texture2D::Sptr solidGreyTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidGreyTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidGrey);

		Texture2D::Sptr solidWhiteTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidWhiteTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidWhite);

#pragma endregion 

		// Loading in a 1D LUT
		Texture1D::Sptr toonLut = ResourceManager::CreateAsset<Texture1D>("luts/toon-1D.png"); 
		toonLut->SetWrap(WrapMode::ClampToEdge);

		// Here we'll load in the cubemap, as well as a special shader to handle drawing the skybox
		TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/ocean/ocean.jpg");
		ShaderProgram::Sptr      skyboxShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/skybox_vert.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/skybox_frag.glsl" } 
		});
		  
		// Create an empty scene
		Scene::Sptr scene = std::make_shared<Scene>();  

		// Setting up our enviroment map
		scene->SetSkyboxTexture(testCubemap); 
		scene->SetSkyboxShader(skyboxShader);
		// Since the skybox I used was for Y-up, we need to rotate it 90 deg around the X-axis to convert it to z-up 
		scene->SetSkyboxRotation(glm::rotate(MAT4_IDENTITY, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));

		// Loading in a color lookup table
		/*
		Texture3D::Sptr lut_ori = ResourceManager::CreateAsset<Texture3D>("luts/cool.CUBE");
		Texture3D::Sptr lut_coo = ResourceManager::CreateAsset<Texture3D>("luts/COOL_LUT.CUBE");
		Texture3D::Sptr lut_war = ResourceManager::CreateAsset<Texture3D>("luts/WARM_LUT.CUBE");
		Texture3D::Sptr lut_cus = ResourceManager::CreateAsset<Texture3D>("luts/CUS_LUT.CUBE"); */
		Texture3D::Sptr lut_nor = ResourceManager::CreateAsset<Texture3D>("luts/NORM_LUT.CUBE");

		// Configure the color correction LUT
		scene->SetColorLUT(lut_nor);

		Material::Sptr terrianMaterial = ResourceManager::CreateAsset<Material>(blinnShader);
		{
			terrianMaterial->Name = "terrianMaterial";
			terrianMaterial->Set("u_Material.AlbedoMap", terrianTex);
			terrianMaterial->Set("u_Material.Shininess", 0.8f);
			terrianMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}


		Material::Sptr characterTextureMat = ResourceManager::CreateAsset<Material>(blinnShader);
		{
			characterTextureMat->Name = "characterTextureMat";
			characterTextureMat->Set("u_Material.Diffuse", characterTex);
			characterTextureMat->Set("u_Material.Shininess", 1.0f);
		}

		Material::Sptr carTextureMat = ResourceManager::CreateAsset<Material>(blinnShader);
		{
			carTextureMat->Name = "CAR Material";
			carTextureMat->Set("u_Material.Diffuse", yellowCarTex);
			carTextureMat->Set("u_Material.Shininess", 0.8f);
		}

		Material::Sptr carTextureToonMat = ResourceManager::CreateAsset<Material>(toonShader);
		{
			carTextureToonMat->Name = "CAR Material";
			carTextureToonMat->Set("u_Material.Diffuse", yellowCarTex);
			carTextureToonMat->Set("u_Material.NormalMap", normalMapDefault);
			carTextureToonMat->Set("toonTex", toonLut);
			carTextureToonMat->Set("u_Material.Shininess", 1.0f);
		}

		Material::Sptr goalTextureMat = ResourceManager::CreateAsset<Material>(blinnShader);
		{
			goalTextureMat->Name = "wallMaterial";
			goalTextureMat->Set("u_Material.Diffuse", coinTex);
			goalTextureMat->Set("u_Material.Shininess", 0.8f);

		}

		Material::Sptr boxSpecMat = ResourceManager::CreateAsset<Material>(specShader);
		{
			boxSpecMat->Name = "Box-Specular";
			boxSpecMat->Set("u_Material.Diffuse", boxTexture);
			boxSpecMat->Set("u_Material.Specular", boxSpec);
			boxSpecMat->Set("u_Material.Shininess", 0.5f);
		}

		Material::Sptr terrainMat = ResourceManager::CreateAsset<Material>(blinnShader);
		{
			terrainMat->Name = "Terrain-Mat";
			terrainMat->Set("u_Material.Diffuse", GrassTex);
			terrainMat->Set("u_Material.Shininess", 1.0f);
		}

		Material::Sptr rockMat = ResourceManager::CreateAsset<Material>(blinnShader);
		{
			rockMat->Name = "Rock-Mat";
			rockMat->Set("u_Material.Diffuse", solidWhiteTex);
			rockMat->Set("u_Material.Shininess", 1.0f);
		}

		Material::Sptr treeMat = ResourceManager::CreateAsset<Material>(blinnShader);
		{
			treeMat->Name = "Tree-Mat";
			treeMat->Set("u_Material.Diffuse", TreeTex);
			treeMat->Set("u_Material.Shininess", 1.0f);
		}

		Material::Sptr brownMat = ResourceManager::CreateAsset<Material>(blinnShader);
		{
			brownMat->Name = "Branch-Mat";
			brownMat->Set("u_Material.Diffuse", BrownTex);
			brownMat->Set("u_Material.Shininess", 1.0f);
		}

		/// <summary>
		/// Shader Texture manager
		/// </summary>
		GameObject::Sptr shaderContainer = scene->CreateGameObject("ShaderContainer");
		{
			ShaderContainer::Sptr shaderCont = shaderContainer->Add<ShaderContainer>();
			shaderCont->addShader(blinnShader, "BLIN");
			shaderCont->addShader(specShader, "SPEC");
			shaderCont->addShader(basicShader, "BASI");


			//shaderCont->addD3Tex(lut_nor, "LUT_NORM");
			//shaderCont->addD3Tex(lut_coo, "LUT_COOL");
			//shaderCont->addD3Tex(lut_war, "LUT_WARM");
			//shaderCont->addD3Tex(lut_cus, "LUT_CUS");

			SimpleToggle::Sptr toggle = shaderContainer->Add<SimpleToggle>();
			toggle->_shaderContainer = shaderContainer;
			toggle->toggleMode = 0;
		}

		GameObject::Sptr dynamicLight = scene->CreateGameObject("DynamicLight");
		{
			Light::Sptr light = dynamicLight->Add<Light>();
			light->SetRadius(20.0f);
			light->SetIntensity(-8.0f);
			light->SetType(LightType::Cone);
			dynamicLight->Add<SimpleLightFollow>();
		}

		GameObject::Sptr ambientLight = scene->CreateGameObject("ambientLight");
		{
			ambientLight->SetPostion(glm::vec3(0.0f, 0.0f, 200.0f));
			Light::Sptr light = ambientLight->Add<Light>();
			light->SetType(LightType::Directional);
			light->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
			light->SetRadius(20000.0f);
			light->SetIntensity(1.0f);
		}
#pragma region EXAMPLES1
		/*
		// Create our materials
		// This will be our box material, with no environment reflections
		Material::Sptr boxMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			boxMaterial->Name = "Box";
			boxMaterial->Set("u_Material.AlbedoMap", boxTexture);
			boxMaterial->Set("u_Material.Shininess", 0.1f);
			boxMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		// This will be the reflective material, we'll make the whole thing 90% reflective
		Material::Sptr monkeyMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			monkeyMaterial->Name = "Monkey";
			monkeyMaterial->Set("u_Material.AlbedoMap", monkeyTex);
			monkeyMaterial->Set("u_Material.NormalMap", normalMapDefault);
			monkeyMaterial->Set("u_Material.Shininess", 0.5f);
		}

		// This will be the reflective material, we'll make the whole thing 50% reflective
		Material::Sptr testMaterial = ResourceManager::CreateAsset<Material>(deferredForward); 
		{
			testMaterial->Name = "Box-Specular";
			testMaterial->Set("u_Material.AlbedoMap", boxTexture); 
			testMaterial->Set("u_Material.Specular", boxSpec);
			testMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		// Our foliage vertex shader material 
		Material::Sptr foliageMaterial = ResourceManager::CreateAsset<Material>(foliageShader);
		{
			foliageMaterial->Name = "Foliage Shader";
			foliageMaterial->Set("u_Material.AlbedoMap", leafTex);
			foliageMaterial->Set("u_Material.Shininess", 0.1f);
			foliageMaterial->Set("u_Material.DiscardThreshold", 0.1f);
			foliageMaterial->Set("u_Material.NormalMap", normalMapDefault);

			foliageMaterial->Set("u_WindDirection", glm::vec3(1.0f, 1.0f, 0.0f));
			foliageMaterial->Set("u_WindStrength", 0.5f);
			foliageMaterial->Set("u_VerticalScale", 1.0f);
			foliageMaterial->Set("u_WindSpeed", 1.0f);
		}

		// Our toon shader material
		Material::Sptr toonMaterial = ResourceManager::CreateAsset<Material>(celShader);
		{
			toonMaterial->Name = "Toon"; 
			toonMaterial->Set("u_Material.AlbedoMap", boxTexture);
			toonMaterial->Set("u_Material.NormalMap", normalMapDefault);
			toonMaterial->Set("s_ToonTerm", toonLut);
			toonMaterial->Set("u_Material.Shininess", 0.1f); 
			toonMaterial->Set("u_Material.Steps", 8);
		}


		Material::Sptr displacementTest = ResourceManager::CreateAsset<Material>(displacementShader);
		{
			Texture2D::Sptr displacementMap = ResourceManager::CreateAsset<Texture2D>("textures/displacement_map.png");
			Texture2D::Sptr normalMap       = ResourceManager::CreateAsset<Texture2D>("textures/normal_map.png");
			Texture2D::Sptr diffuseMap      = ResourceManager::CreateAsset<Texture2D>("textures/bricks_diffuse.png");

			displacementTest->Name = "Displacement Map";
			displacementTest->Set("u_Material.AlbedoMap", diffuseMap);
			displacementTest->Set("u_Material.NormalMap", normalMap);
			displacementTest->Set("s_Heightmap", displacementMap);
			displacementTest->Set("u_Material.Shininess", 0.5f);
			displacementTest->Set("u_Scale", 0.1f);
		}

		Material::Sptr grey = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			grey->Name = "Grey";
			grey->Set("u_Material.AlbedoMap", solidGreyTex);
			grey->Set("u_Material.Specular", solidBlackTex);
			grey->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr polka = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			polka->Name = "Polka";
			polka->Set("u_Material.AlbedoMap", ResourceManager::CreateAsset<Texture2D>("textures/polka.png"));
			polka->Set("u_Material.Specular", solidBlackTex);
			polka->Set("u_Material.NormalMap", normalMapDefault);
			polka->Set("u_Material.EmissiveMap", ResourceManager::CreateAsset<Texture2D>("textures/polka.png"));
		}

		Material::Sptr whiteBrick = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			whiteBrick->Name = "White Bricks";
			whiteBrick->Set("u_Material.AlbedoMap", ResourceManager::CreateAsset<Texture2D>("textures/displacement_map.png"));
			whiteBrick->Set("u_Material.Specular", solidGrey);
			whiteBrick->Set("u_Material.NormalMap", ResourceManager::CreateAsset<Texture2D>("textures/normal_map.png"));
		}

		Material::Sptr normalmapMat = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap       = ResourceManager::CreateAsset<Texture2D>("textures/normal_map.png");
			Texture2D::Sptr diffuseMap      = ResourceManager::CreateAsset<Texture2D>("textures/bricks_diffuse.png");

			normalmapMat->Name = "Tangent Space Normal Map";
			normalmapMat->Set("u_Material.AlbedoMap", diffuseMap);
			normalmapMat->Set("u_Material.NormalMap", normalMap);
			normalmapMat->Set("u_Material.Shininess", 0.5f);
			normalmapMat->Set("u_Scale", 0.1f);
		}

		Material::Sptr multiTextureMat = ResourceManager::CreateAsset<Material>(multiTextureShader);
		{
			Texture2D::Sptr sand  = ResourceManager::CreateAsset<Texture2D>("textures/terrain/sand.png");
			Texture2D::Sptr grass = ResourceManager::CreateAsset<Texture2D>("textures/terrain/grass.png");

			multiTextureMat->Name = "Multitexturing";
			multiTextureMat->Set("u_Material.DiffuseA", sand);
			multiTextureMat->Set("u_Material.DiffuseB", grass);
			multiTextureMat->Set("u_Material.NormalMapA", normalMapDefault);
			multiTextureMat->Set("u_Material.NormalMapB", normalMapDefault);
			multiTextureMat->Set("u_Material.Shininess", 0.5f);
			multiTextureMat->Set("u_Scale", 0.1f); 
		}
		
		// Create some lights for our scene
		GameObject::Sptr lightParent = scene->CreateGameObject("Lights");

		for (int ix = 0; ix < 50; ix++) {
			GameObject::Sptr light = scene->CreateGameObject("Light");
			light->SetPostion(glm::vec3(glm::diskRand(25.0f), 1.0f));
			lightParent->AddChild(light);

			Light::Sptr lightComponent = light->Add<Light>();
			lightComponent->SetColor(glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f)));
			lightComponent->SetRadius(glm::linearRand(0.1f, 10.0f));
			lightComponent->SetIntensity(glm::linearRand(1.0f, 2.0f));
		}
		*/
		// We'll create a mesh that is a simple plane that we can resize later
		MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>();
		planeMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(1.0f)));
		planeMesh->GenerateMesh();

		MeshResource::Sptr sphere = ResourceManager::CreateAsset<MeshResource>();
		sphere->AddParam(MeshBuilderParam::CreateIcoSphere(ZERO, ONE, 5));
		sphere->GenerateMesh();

		// Set up the scene's camera
		GameObject::Sptr camera = scene->MainCamera->GetGameObject()->SelfRef();
		{
			camera->SetPostion({ -3, -1, 5 });
			camera->LookAt(glm::vec3(0.0f));

			//camera->Add<SimpleCameraControl>();

			SimpleCameraFollow::Sptr cmrFollow = camera->Add<SimpleCameraFollow>();
			// This is now handled by scene itself!
			//Camera::Sptr cam = camera->Add<Camera>();
			// Make sure that the camera is set as the scene's main camera!
			//scene->MainCamera = cam;
		}

		GameObject::Sptr plainTerrain = scene->CreateGameObject("Plane");
		{
			RenderComponent::Sptr renderer = plainTerrain->Add<RenderComponent>();
			renderer->SetMesh(plainMesh);
			renderer->SetMaterial(terrainMat);

			RigidBody::Sptr physics = plainTerrain->Add<RigidBody>();
			physics->AddCollider(ConvexMeshCollider::Create());
		}

		GameObject::Sptr rock1 = scene->CreateGameObject("Rock1");
		{
			rock1->SetPostion(glm::vec3(0.0f, 0.0f, 0.0f));
			rock1->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));

			RenderComponent::Sptr renderer = rock1->Add<RenderComponent>();
			renderer->SetMesh(rock1Mesh);
			renderer->SetMaterial(rockMat);

			RigidBody::Sptr physics = rock1->Add<RigidBody>();
			physics->AddCollider(ConvexMeshCollider::Create());
		}

		GameObject::Sptr tree1 = scene->CreateGameObject("Tree1");
		{
			tree1->SetPostion(glm::vec3(10.0f, 7.5f, 0.0f));
			tree1->SetScale(glm::vec3(10.0f, 10.0f, 10.0f));

			RenderComponent::Sptr renderer = tree1->Add<RenderComponent>();
			renderer->SetMesh(tree1Mesh);
			renderer->SetMaterial(treeMat);

			RigidBody::Sptr physics = tree1->Add<RigidBody>();
			physics->AddCollider(ConvexMeshCollider::Create());
		}

		GameObject::Sptr branch1 = scene->CreateGameObject("Branch1");
		{
			branch1->SetPostion(glm::vec3(5.0f, 5.0f, 0.0f));
			branch1->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));

			RenderComponent::Sptr renderer = branch1->Add<RenderComponent>();
			renderer->SetMesh(branch1Mesh);
			renderer->SetMaterial(brownMat);

			RigidBody::Sptr physics = branch1->Add<RigidBody>();
			physics->AddCollider(ConvexMeshCollider::Create());
		}

		GameObject::Sptr branch2 = scene->CreateGameObject("Branch2");
		{
			branch2->SetPostion(glm::vec3(9.0f, 0.0f, 0.0f));
			branch2->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));

			RenderComponent::Sptr renderer = branch2->Add<RenderComponent>();
			renderer->SetMesh(branch1Mesh);
			renderer->SetMaterial(brownMat);

			RigidBody::Sptr physics = branch2->Add<RigidBody>();
			physics->AddCollider(ConvexMeshCollider::Create());
		}

		GameObject::Sptr branch3 = scene->CreateGameObject("Branch3");
		{
			branch3->SetPostion(glm::vec3(-9.0f, 0.0f, 0.0f));
			branch3->SetScale(glm::vec3(1.1f, 1.1f, 1.1f));

			RenderComponent::Sptr renderer = branch3->Add<RenderComponent>();
			renderer->SetMesh(branch1Mesh);
			renderer->SetMaterial(brownMat);

			RigidBody::Sptr physics = branch3->Add<RigidBody>();
			physics->AddCollider(ConvexMeshCollider::Create());
		}

		GameObject::Sptr branch4 = scene->CreateGameObject("Branch4");
		{
			branch4->SetPostion(glm::vec3(0.0f, 9.0f, 0.0f));
			branch4->SetScale(glm::vec3(1.3f, 1.3f, 1.3f));

			RenderComponent::Sptr renderer = branch4->Add<RenderComponent>();
			renderer->SetMesh(branch1Mesh);
			renderer->SetMaterial(brownMat);

			RigidBody::Sptr physics = branch4->Add<RigidBody>();
			physics->AddCollider(ConvexMeshCollider::Create());
		}

		GameObject::Sptr branch5 = scene->CreateGameObject("Branch5");
		{
			branch5->SetPostion(glm::vec3(0.0f, -9.0f, 0.0f));
			branch5->SetScale(glm::vec3(1.5f, 1.5f, 1.5f));

			RenderComponent::Sptr renderer = branch5->Add<RenderComponent>();
			renderer->SetMesh(branch1Mesh);
			renderer->SetMaterial(brownMat);

			RigidBody::Sptr physics = branch5->Add<RigidBody>();
			physics->AddCollider(ConvexMeshCollider::Create());
		}

		GameObject::Sptr mainChara = scene->CreateGameObject("PACMAN");
		{
			mainChara->SetPostion(glm::vec3(0.0f, 0.0f, 1.5f));
			mainChara->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

			mainChara->Add<JumpBehaviour>();

			RenderComponent::Sptr renderer = mainChara->Add<RenderComponent>();
			renderer->SetMesh(characterMesh);
			renderer->SetMaterial(characterTextureMat);


			SimpleObjectControl::Sptr objControl = mainChara->Add<SimpleObjectControl>();
			objControl->setCamera(camera);

			scene->MainCamera->GetGameObject()->Get<SimpleCameraFollow>()->setFollowObj(mainChara);



			RigidBody::Sptr physics = mainChara->Add<RigidBody>(RigidBodyType::Dynamic);
			ICollider::Sptr collider = physics->AddCollider(SphereCollider::Create());
			//collider->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
			//collider->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));
			physics->AddComponent<TriggerVolume>();



			scene->FindObjectByName("DynamicLight")->Get<SimpleLightFollow>()->setFollowObj(mainChara);

			SimpleToggle::Sptr toggle = mainChara->Add<SimpleToggle>();
			toggle->_shaderContainer = shaderContainer;
			toggle->toggleMode = 2;


		}

		GameObject::Sptr car = scene->CreateGameObject("Car For LOSE");
		{
			car->SetPostion(glm::vec3(15.0f, 15.0f, 0.0f));
			car->SetScale(glm::vec3(3.0f, 3.0f, 3.0f));
			car->SetRotation(glm::vec3(-90.0f, 0.0f, 0.0f));

			RenderComponent::Sptr renderer = car->Add<RenderComponent>();
			renderer->SetMesh(carMesh);
			renderer->SetMaterial(carTextureMat);




			TriggerVolume::Sptr volume = car->Add<TriggerVolume>();
			volume->AddCollider(ConvexMeshCollider::Create());
			volume->AddComponent<KillBehaviour>();

			volume->AddComponent<RigidBody>(RigidBodyType::Static);

			SimpleToggle::Sptr toggle = car->Add<SimpleToggle>();
			toggle->_shaderContainer = shaderContainer;
			toggle->toggleMode = 2;
		}

		GameObject::Sptr carToon = scene->CreateGameObject("Car For Toon");
		{
			carToon->SetPostion(glm::vec3(15.0f, 0.0f, 0.0f));
			carToon->SetScale(glm::vec3(3.0f, 3.0f, 3.0f));
			carToon->SetRotation(glm::vec3(-90.0f, 0.0f, 0.0f));

			RenderComponent::Sptr renderer = carToon->Add<RenderComponent>();
			renderer->SetMesh(carMesh);
			renderer->SetMaterial(carTextureToonMat);

			SimpleToggle::Sptr toggle = carToon->Add<SimpleToggle>();
			toggle->_shaderContainer = shaderContainer;
			toggle->toggleMode = 2;
		}

		GameObject::Sptr goal = scene->CreateGameObject("Point For GOAL");
		{
			goal->SetPostion(glm::vec3(30.0f, 0.0f, 3.0f));
			goal->SetScale(glm::vec3(3.0f, 3.0f, 3.0f));
			goal->SetRotation(glm::vec3(0.0f, -90.0f, 0.0f));

			RenderComponent::Sptr renderer = goal->Add<RenderComponent>();
			renderer->SetMesh(coin1Mesh);
			renderer->SetMaterial(goalTextureMat);

			TriggerVolume::Sptr volume = goal->Add<TriggerVolume>();
			volume->AddCollider(ConvexMeshCollider::Create());
			volume->AddComponent<GoalBehaviour>();

			SimpleToggle::Sptr toggle = goal->Add<SimpleToggle>();
			toggle->_shaderContainer = shaderContainer;
			toggle->toggleMode = 2;
		}
#pragma region EXAMPLES2
/*	
		// Set up all our sample objects
		GameObject::Sptr plane = scene->CreateGameObject("Plane");
		{
			// Make a big tiled mesh
			MeshResource::Sptr tiledMesh = ResourceManager::CreateAsset<MeshResource>();
			tiledMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(100.0f), glm::vec2(20.0f)));
			tiledMesh->GenerateMesh();

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = plane->Add<RenderComponent>();
			renderer->SetMesh(tiledMesh);
			renderer->SetMaterial(boxMaterial);

			// Attach a plane collider that extends infinitely along the X/Y axis
			RigidBody::Sptr physics = plane->Add<RigidBody>(//static by default);
			physics->AddCollider(BoxCollider::Create(glm::vec3(50.0f, 50.0f, 1.0f)))->SetPosition({ 0,0,-1 });
		}

		// Add some walls :3
		{
			MeshResource::Sptr wall = ResourceManager::CreateAsset<MeshResource>();
			wall->AddParam(MeshBuilderParam::CreateCube(ZERO, ONE));
			wall->GenerateMesh();

			GameObject::Sptr wall1 = scene->CreateGameObject("Wall1");
			wall1->Add<RenderComponent>()->SetMesh(wall)->SetMaterial(whiteBrick);
			wall1->SetScale(glm::vec3(20.0f, 1.0f, 3.0f));
			wall1->SetPostion(glm::vec3(0.0f, 10.0f, 1.5f));
			plane->AddChild(wall1);

			GameObject::Sptr wall2 = scene->CreateGameObject("Wall2");
			wall2->Add<RenderComponent>()->SetMesh(wall)->SetMaterial(whiteBrick);
			wall2->SetScale(glm::vec3(20.0f, 1.0f, 3.0f));
			wall2->SetPostion(glm::vec3(0.0f, -10.0f, 1.5f));
			plane->AddChild(wall2);

			GameObject::Sptr wall3 = scene->CreateGameObject("Wall3");
			wall3->Add<RenderComponent>()->SetMesh(wall)->SetMaterial(whiteBrick);
			wall3->SetScale(glm::vec3(1.0f, 20.0f, 3.0f));
			wall3->SetPostion(glm::vec3(10.0f, 0.0f, 1.5f));
			plane->AddChild(wall3);

			GameObject::Sptr wall4 = scene->CreateGameObject("Wall4");
			wall4->Add<RenderComponent>()->SetMesh(wall)->SetMaterial(whiteBrick);
			wall4->SetScale(glm::vec3(1.0f, 20.0f, 3.0f));
			wall4->SetPostion(glm::vec3(-10.0f, 0.0f, 1.5f));
			plane->AddChild(wall4);
		}

		GameObject::Sptr monkey1 = scene->CreateGameObject("Monkey 1");
		{
			// Set position in the scene
			monkey1->SetPostion(glm::vec3(1.5f, 0.0f, 1.0f));

			// Add some behaviour that relies on the physics body
			monkey1->Add<JumpBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = monkey1->Add<RenderComponent>();
			renderer->SetMesh(monkeyMesh);
			renderer->SetMaterial(monkeyMaterial);

			// Example of a trigger that interacts with static and kinematic bodies as well as dynamic bodies
			TriggerVolume::Sptr trigger = monkey1->Add<TriggerVolume>();
			trigger->SetFlags(TriggerTypeFlags::Statics | TriggerTypeFlags::Kinematics);
			trigger->AddCollider(BoxCollider::Create(glm::vec3(1.0f)));

			monkey1->Add<TriggerVolumeEnterBehaviour>();
		}

		GameObject::Sptr ship = scene->CreateGameObject("Fenrir");
		{
			// Set position in the scene
			ship->SetPostion(glm::vec3(1.5f, 0.0f, 4.0f));
			ship->SetScale(glm::vec3(0.1f));

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = ship->Add<RenderComponent>();
			renderer->SetMesh(shipMesh);
			renderer->SetMaterial(grey);

			GameObject::Sptr particles = scene->CreateGameObject("Particles");
			ship->AddChild(particles);
			particles->SetPostion({ 0.0f, -7.0f, 0.0f});

			ParticleSystem::Sptr particleManager = particles->Add<ParticleSystem>();
			particleManager->Atlas = particleTex;

			particleManager->_gravity = glm::vec3(0.0f);

			ParticleSystem::ParticleData emitter;
			emitter.Type = ParticleType::SphereEmitter;
			emitter.TexID = 2;
			emitter.Position = glm::vec3(0.0f);
			emitter.Color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
			emitter.Lifetime = 1.0f / 50.0f;
			emitter.SphereEmitterData.Timer = 1.0f / 50.0f;
			emitter.SphereEmitterData.Velocity = 0.5f;
			emitter.SphereEmitterData.LifeRange = { 1.0f, 3.0f };
			emitter.SphereEmitterData.Radius = 0.5f;
			emitter.SphereEmitterData.SizeRange = { 0.5f, 1.0f };

			ParticleSystem::ParticleData emitter2;
			emitter2.Type = ParticleType::SphereEmitter;
			emitter2.TexID = 2;
			emitter2.Position = glm::vec3(0.0f);
			emitter2.Color = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
			emitter2.Lifetime = 1.0f / 40.0f;
			emitter2.SphereEmitterData.Timer = 1.0f / 40.0f;
			emitter2.SphereEmitterData.Velocity = 0.1f;
			emitter2.SphereEmitterData.LifeRange = { 0.5f, 1.0f };
			emitter2.SphereEmitterData.Radius = 0.25f;
			emitter2.SphereEmitterData.SizeRange = { 0.25f, 0.5f };

			particleManager->AddEmitter(emitter);
			particleManager->AddEmitter(emitter2);

			ShipMoveBehaviour::Sptr move = ship->Add<ShipMoveBehaviour>();
			move->Center = glm::vec3(0.0f, 0.0f, 4.0f);
			move->Speed = 180.0f;
			move->Radius = 6.0f;
		}

		GameObject::Sptr demoBase = scene->CreateGameObject("Demo Parent");

		GameObject::Sptr polkaBox = scene->CreateGameObject("Polka Box");
		{
			MeshResource::Sptr boxMesh = ResourceManager::CreateAsset<MeshResource>();
			boxMesh->AddParam(MeshBuilderParam::CreateCube(ZERO, ONE));
			boxMesh->GenerateMesh();

			// Set and rotation position in the scene
			polkaBox->SetPostion(glm::vec3(0, 0.0f, 3.0f));

			// Add a render component
			RenderComponent::Sptr renderer = polkaBox->Add<RenderComponent>();
			renderer->SetMesh(boxMesh);
			renderer->SetMaterial(polka);

			demoBase->AddChild(polkaBox);
		}

		// Box to showcase the specular material
		GameObject::Sptr specBox = scene->CreateGameObject("Specular Object");
		{
			MeshResource::Sptr boxMesh = ResourceManager::CreateAsset<MeshResource>();
			boxMesh->AddParam(MeshBuilderParam::CreateCube(ZERO, ONE));
			boxMesh->GenerateMesh();

			// Set and rotation position in the scene
			specBox->SetPostion(glm::vec3(0, -4.0f, 1.0f));

			// Add a render component
			RenderComponent::Sptr renderer = specBox->Add<RenderComponent>();
			renderer->SetMesh(boxMesh);
			renderer->SetMaterial(testMaterial); 

			demoBase->AddChild(specBox);
		}

		

		// sphere to showcase the foliage material
		GameObject::Sptr foliageBall = scene->CreateGameObject("Foliage Sphere");
		{
			// Set and rotation position in the scene
			foliageBall->SetPostion(glm::vec3(-4.0f, -4.0f, 1.0f));

			// Add a render component
			RenderComponent::Sptr renderer = foliageBall->Add<RenderComponent>();
			renderer->SetMesh(sphere);
			renderer->SetMaterial(foliageMaterial);

			demoBase->AddChild(foliageBall);
		}

		// Box to showcase the foliage material
		GameObject::Sptr foliageBox = scene->CreateGameObject("Foliage Box");
		{
			MeshResource::Sptr box = ResourceManager::CreateAsset<MeshResource>();
			box->AddParam(MeshBuilderParam::CreateCube(glm::vec3(0, 0, 0.5f), ONE));
			box->GenerateMesh();

			// Set and rotation position in the scene
			foliageBox->SetPostion(glm::vec3(-6.0f, -4.0f, 1.0f));

			// Add a render component
			RenderComponent::Sptr renderer = foliageBox->Add<RenderComponent>();
			renderer->SetMesh(box);
			renderer->SetMaterial(foliageMaterial);

			demoBase->AddChild(foliageBox);
		}

		// Box to showcase the specular material
		GameObject::Sptr toonBall = scene->CreateGameObject("Toon Object");
		{
			// Set and rotation position in the scene
			toonBall->SetPostion(glm::vec3(-2.0f, -4.0f, 1.0f));

			// Add a render component
			RenderComponent::Sptr renderer = toonBall->Add<RenderComponent>();
			renderer->SetMesh(sphere);
			renderer->SetMaterial(toonMaterial);

			demoBase->AddChild(toonBall);
		}

		GameObject::Sptr displacementBall = scene->CreateGameObject("Displacement Object");
		{
			// Set and rotation position in the scene
			displacementBall->SetPostion(glm::vec3(2.0f, -4.0f, 1.0f));

			// Add a render component
			RenderComponent::Sptr renderer = displacementBall->Add<RenderComponent>();
			renderer->SetMesh(sphere);
			renderer->SetMaterial(displacementTest);

			demoBase->AddChild(displacementBall);
		}

		GameObject::Sptr multiTextureBall = scene->CreateGameObject("Multitextured Object");
		{
			// Set and rotation position in the scene 
			multiTextureBall->SetPostion(glm::vec3(4.0f, -4.0f, 1.0f));

			// Add a render component 
			RenderComponent::Sptr renderer = multiTextureBall->Add<RenderComponent>();
			renderer->SetMesh(sphere);
			renderer->SetMaterial(multiTextureMat);

			demoBase->AddChild(multiTextureBall);
		}

		GameObject::Sptr normalMapBall = scene->CreateGameObject("Normal Mapped Object");
		{
			// Set and rotation position in the scene 
			normalMapBall->SetPostion(glm::vec3(6.0f, -4.0f, 1.0f));

			// Add a render component 
			RenderComponent::Sptr renderer = normalMapBall->Add<RenderComponent>();
			renderer->SetMesh(sphere);
			renderer->SetMaterial(normalmapMat);

			demoBase->AddChild(normalMapBall);
		}

		// Create a trigger volume for testing how we can detect collisions with objects!
		GameObject::Sptr trigger = scene->CreateGameObject("Trigger");
		{
			TriggerVolume::Sptr volume = trigger->Add<TriggerVolume>();
			CylinderCollider::Sptr collider = CylinderCollider::Create(glm::vec3(3.0f, 3.0f, 1.0f));
			collider->SetPosition(glm::vec3(0.0f, 0.0f, 0.5f));
			volume->AddCollider(collider);

			trigger->Add<TriggerVolumeEnterBehaviour>();
		}

		GameObject::Sptr shadowCaster = scene->CreateGameObject("Shadow Light");
		{
			// Set position in the scene
			shadowCaster->SetPostion(glm::vec3(3.0f, 3.0f, 5.0f));
			shadowCaster->LookAt(glm::vec3(0.0f));

			// Create and attach a renderer for the monkey
			ShadowCamera::Sptr shadowCam = shadowCaster->Add<ShadowCamera>();
			shadowCam->SetProjection(glm::perspective(glm::radians(120.0f), 1.0f, 0.1f, 100.0f));
		}

		/////////////////////////// UI //////////////////////////////
		
		GameObject::Sptr canvas = scene->CreateGameObject("UI Canvas"); 
		{
			RectTransform::Sptr transform = canvas->Add<RectTransform>();
			transform->SetMin({ 16, 16 });
			transform->SetMax({ 128, 128 });

			GuiPanel::Sptr canPanel = canvas->Add<GuiPanel>();


			GameObject::Sptr subPanel = scene->CreateGameObject("Sub Item");
			{
				RectTransform::Sptr transform = subPanel->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 64, 64 });

				GuiPanel::Sptr panel = subPanel->Add<GuiPanel>();
				panel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

				panel->SetTexture(ResourceManager::CreateAsset<Texture2D>("textures/upArrow.png"));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 16.0f);
				font->Bake();

				GuiText::Sptr text = subPanel->Add<GuiText>();
				text->SetText("Hello world!");
				text->SetFont(font);

				monkey1->Get<JumpBehaviour>()->Panel = text;
			}

			canvas->AddChild(subPanel);
		}
		

		GameObject::Sptr particles = scene->CreateGameObject("Particles"); 
		{
			particles->SetPostion({ -2.0f, 0.0f, 2.0f });

			ParticleSystem::Sptr particleManager = particles->Add<ParticleSystem>();  
			particleManager->Atlas = particleTex;

			ParticleSystem::ParticleData emitter;
			emitter.Type = ParticleType::SphereEmitter;
			emitter.TexID = 2;
			emitter.Position = glm::vec3(0.0f);
			emitter.Color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
			emitter.Lifetime = 0.0f;
			emitter.SphereEmitterData.Timer = 1.0f / 50.0f;
			emitter.SphereEmitterData.Velocity = 0.5f;
			emitter.SphereEmitterData.LifeRange = { 1.0f, 4.0f };
			emitter.SphereEmitterData.Radius = 1.0f;
			emitter.SphereEmitterData.SizeRange = { 0.5f, 1.5f };

			particleManager->AddEmitter(emitter);
		}
		*/
#pragma endregion
		GuiBatcher::SetDefaultTexture(ResourceManager::CreateAsset<Texture2D>("textures/ui-sprite.png"));
		GuiBatcher::SetDefaultBorderRadius(8);

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("scene-manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");

		// Send the scene to the application
		app.LoadScene(scene);
	}
}
