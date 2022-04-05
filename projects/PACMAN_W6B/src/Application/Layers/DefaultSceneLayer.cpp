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
		MeshResource::Sptr characterMesh = ResourceManager::CreateAsset<MeshResource>("objs/pacman/pacman.obj");
		MeshResource::Sptr terrainMesh = ResourceManager::CreateAsset<MeshResource>("objs/ground/terrain.obj");

		MeshResource::Sptr goalMesh = ResourceManager::CreateAsset<MeshResource>("objs/Coin.obj");
		MeshResource::Sptr carMesh = ResourceManager::CreateAsset<MeshResource>("objs/Car.obj");
		MeshResource::Sptr plainMesh = ResourceManager::CreateAsset<MeshResource>("objs/terrain/plainTerrain.obj");

		MeshResource::Sptr rock1Mesh = ResourceManager::CreateAsset<MeshResource>("objs/rocks/rock1.obj");
		MeshResource::Sptr branch1Mesh = ResourceManager::CreateAsset<MeshResource>("objs/branches/branches.obj");

		// Load in some textures
		Texture2D::Sptr		characterTex = ResourceManager::CreateAsset<Texture2D>("textures/RacoonUV.png");
		Texture2D::Sptr		yellowCarTex = ResourceManager::CreateAsset<Texture2D>("textures/YellowStripeUV.png");
		Texture2D::Sptr		terrianTex = ResourceManager::CreateAsset<Texture2D>("textures/GroundTex.png");
		Texture2D::Sptr		goalTex = ResourceManager::CreateAsset<Texture2D>("textures/CoinUV.png");

		Texture2D::Sptr    boxTexture = ResourceManager::CreateAsset<Texture2D>("textures/box-diffuse.png");
		Texture2D::Sptr    boxSpec = ResourceManager::CreateAsset<Texture2D>("textures/box-specular.png");

#pragma region Basic Texture Creation
		Texture2DDescription singlePixelDescriptor;
		singlePixelDescriptor.Width = singlePixelDescriptor.Height = 1;
		singlePixelDescriptor.Format = InternalFormat::RGB8;

		float normalMapDefaultData[3] = { 0.5f, 0.5f, 1.0f };
		Texture2D::Sptr normalMapDefault = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		normalMapDefault->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, normalMapDefaultData);

		float solidBlack[3] = { 0.5f, 0.5f, 0.5f };
		Texture2D::Sptr solidBlackTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidBlackTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidBlack);

		float solidGrey[3] = { 0.0f, 0.0f, 0.0f };
		Texture2D::Sptr solidGreyTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidGreyTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidGrey);

		float solidWhite[3] = { 1.0f, 1.0f, 1.0f };
		Texture2D::Sptr solidWhiteTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidWhiteTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidWhite);

#pragma endregion 

		// Here we'll load in the cubemap, as well as a special shader to handle drawing the skybox
		TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/space/space.png");
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
		Texture3D::Sptr lut_cus = ResourceManager::CreateAsset<Texture3D>("luts/CUS_LUT.CUBE");*/
		Texture3D::Sptr lut_nor = ResourceManager::CreateAsset<Texture3D>("luts/NORM_LUT.CUBE");
		 
		Texture1D::Sptr toonLut = ResourceManager::CreateAsset<Texture1D>("luts/toon-1D.png");
		toonLut->SetWrap(WrapMode::ClampToEdge);

		// Configure the color correction LUT
		scene->SetColorLUT(lut_nor);

		// Create our materials
		// This will be our box material, with no environment reflections
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
			goalTextureMat->Set("u_Material.Diffuse", goalTex);
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
			terrainMat->Set("u_Material.Diffuse", solidWhiteTex);
			terrainMat->Set("u_Material.Shininess", 0.5f);
		}

		Material::Sptr rockMat = ResourceManager::CreateAsset<Material>(blinnShader);
		{
			rockMat->Name = "Rock-Mat";
			rockMat->Set("u_Material.Diffuse", solidWhiteTex);
			rockMat->Set("u_Material.Shininess", 1.0f);
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

		

		
		// We'll create a mesh that is a simple plane that we can resize later
		MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>();
		planeMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(1.0f)));
		planeMesh->GenerateMesh();

		

		// Set up the scene's camera
		GameObject::Sptr camera = scene->MainCamera->GetGameObject()->SelfRef();
		{
			camera->SetPostion({ -9, -6, 15 });
			camera->LookAt(glm::vec3(0.0f)); 

			SimpleCameraFollow::Sptr cmrFollow = camera->Add<SimpleCameraFollow>();
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
			RenderComponent::Sptr renderer = rock1->Add<RenderComponent>();
			renderer->SetMesh(rock1Mesh);
			renderer->SetMaterial(rockMat);

			RigidBody::Sptr physics = rock1->Add<RigidBody>();
			physics->AddCollider(ConvexMeshCollider::Create());
		}

		GameObject::Sptr branch1 = scene->CreateGameObject("Branch1");
		{
			RenderComponent::Sptr renderer = branch1->Add<RenderComponent>();
			renderer->SetMesh(branch1Mesh);
			renderer->SetMaterial(rockMat);

			RigidBody::Sptr physics = branch1->Add<RigidBody>();
			physics->AddCollider(ConvexMeshCollider::Create());
		}

		/*
		GameObject::Sptr plane = scene->CreateGameObject("Plane");
		{
			
			MeshResource::Sptr tiledMesh = ResourceManager::CreateAsset<MeshResource>();
			tiledMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(50.0f), glm::vec2(50.0f)));
			tiledMesh->GenerateMesh();

			plane->SetScale(glm::vec3(2.0f, 2.0f, 1.0f));

			
			RenderComponent::Sptr renderer = plane->Add<RenderComponent>();
			renderer->SetMesh(tiledMesh);
			renderer->SetMaterial(boxSpecMat);	 

			SimpleToggle::Sptr toggle = plane->Add<SimpleToggle>();
			toggle->_shaderContainer = shaderContainer;
			toggle->toggleMode = 3;

			RigidBody::Sptr physics = plane->Add<RigidBody>();
			physics->AddCollider(BoxCollider::Create(glm::vec3(50.0f, 50.0f, 1.0f)))->SetPosition({ 0,0,-1 });
			
		}
		*/
		/*
		GameObject::Sptr terrain = scene->CreateGameObject("Terrian");
		{
			terrain->SetPostion(glm::vec3(0.0f, 0.0f, -3.0f));

			RenderComponent::Sptr renderer = terrain->Add<RenderComponent>();
			renderer->SetMesh(terrainMesh);
			renderer->SetMaterial(terrainMat);  

			RigidBody::Sptr physics = terrain->Add<RigidBody>(RigidBodyType::Static);
			physics->AddCollider(ConvexMeshCollider::Create());
		}
		 */

		GameObject::Sptr mainChara = scene->CreateGameObject("PACMAN");
		{
			mainChara->SetPostion(glm::vec3(0.0f, 0.0f, 0.0f));
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
			goal->SetPostion(glm::vec3(-15.0f, -15.0f, 3.0f));
			goal->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));

			RenderComponent::Sptr renderer = goal->Add<RenderComponent>();
			renderer->SetMesh(goalMesh);
			renderer->SetMaterial(goalTextureMat);

			TriggerVolume::Sptr volume = goal->Add<TriggerVolume>();
			volume->AddCollider(ConvexMeshCollider::Create());
			volume->AddComponent<GoalBehaviour>(); 

			SimpleToggle::Sptr toggle = goal->Add<SimpleToggle>();
			toggle->_shaderContainer = shaderContainer;
			toggle->toggleMode = 2;
		}

#pragma region EXAMPLES
/*
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

			RigidBody::Sptr physics = monkey1->Add<RigidBody>(RigidBodyType::Kinematic);
			physics->AddCollider(ConvexMeshCollider::Create());
		}

		GameObject::Sptr monkey2 = scene->CreateGameObject("Monkey 2 Controlled");
		{
			// Set position in the scene
			monkey2->SetPostion(glm::vec3(3.5f, 1.0f, 2.0f));

			// Add some behaviour that relies on the physics body
			monkey2->Add<JumpBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = monkey2->Add<RenderComponent>();
			renderer->SetMesh(monkeyMesh);
			renderer->SetMaterial(monkeyMaterial);

			// Example of a trigger that interacts with static and kinematic bodies as well as dynamic bodies

			TriggerVolume::Sptr trigger = monkey2->Add<TriggerVolume>();
			trigger->SetFlags(TriggerTypeFlags::Statics | TriggerTypeFlags::Kinematics);
			trigger->AddCollider(BoxCollider::Create(glm::vec3(1.0f)));

			//monkey2->Add<TriggerVolumeEnterBehaviour>();
			SimpleObjectControl::Sptr objControl = monkey2->Add<SimpleObjectControl>();
			objControl->setCamera(camera);

			//scene->MainCamera->GetGameObject()->Get<SimpleCameraFollow>()->setFollowObj(monkey2);

			RigidBody::Sptr physics = monkey2->Add<RigidBody>(RigidBodyType::Dynamic);
			physics->AddCollider(ConvexMeshCollider::Create());
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

		/////////////////////////// UI //////////////////////////////

		GameObject::Sptr canvas = scene->CreateGameObject("UI Canvas");
		{
			RectTransform::Sptr transform = canvas->Add<RectTransform>();
			transform->SetMin({ 16, 16 });
			transform->SetMax({ 256, 256 });

			GuiPanel::Sptr canPanel = canvas->Add<GuiPanel>();


			GameObject::Sptr subPanel = scene->CreateGameObject("Sub Item");
			{
				RectTransform::Sptr transform = subPanel->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 128, 128 });

				GuiPanel::Sptr panel = subPanel->Add<GuiPanel>();
				panel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

				panel->SetTexture(ResourceManager::CreateAsset<Texture2D>("textures/upArrow.png"));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 16.0f);
				font->Bake();

				GuiText::Sptr text = subPanel->Add<GuiText>();
				text->SetText("Hello world!");
				text->SetFont(font);

				pacman->Get<JumpBehaviour>()->Panel = text;
			}

			canvas->AddChild(subPanel);
		}


		GameObject::Sptr particles = scene->CreateGameObject("Particles");
		{
			ParticleSystem::Sptr particleManager = particles->Add<ParticleSystem>();
			particleManager->AddEmitter(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 10.0f), 10.0f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
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
