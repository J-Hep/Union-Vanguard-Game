#include "DefaultSceneLayer.h"

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <GLM/gtc/random.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)

#include <filesystem>
#include "Application/Timing.h"

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
#include "ToneFire.h"

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
#include "Gameplay/Components/EnemyMovement.h"
#include "Gameplay/Components/CameraVanguard.h"
#include "Gameplay/Components/AudioEngine.h"

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

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>

//Animations
#include "Gameplay/Components/MorphAnimator.h"
#include "Gameplay/Components/MorphMeshRenderer.h"
#include "CMorphMeshRenderer.h"
#include "CMorphAnimator.h"

DefaultSceneLayer::DefaultSceneLayer() :
	ApplicationLayer()
{
	Name = "Default Scene";
	Overrides = AppLayerFunctions::OnAppLoad | AppLayerFunctions::OnUpdate;
}

DefaultSceneLayer::~DefaultSceneLayer() = default;

void DefaultSceneLayer::OnAppLoad(const nlohmann::json& config) {
	_CreateScene();
}

void DefaultSceneLayer::BubbleSort(std::vector<int>& arr)
{
	bool swap = true;
	while (swap) {
		swap = false;
		for (size_t i = 0; i < arr.size() - 1; i++) {
			if (arr[i] > arr[i + 1]) {
				arr[i] += arr[i + 1];
				arr[i + 1] = arr[i] - arr[i + 1];
				arr[i] -= arr[i + 1];
				swap = true;
			}
		}
	}
}

double preFrame = glfwGetTime();
bool uiStart = true;
bool isButtonPressed = false;
int menuSelect = 1, menuType = 1;
std::vector<int> scores;
std::string score;
	//menuType 1 = main menu
	//menuType 2 = settings
	//menuType 3 = in game
	//menuType 4 = pause menu
	//menuType 5 = win screen
	//menuType 6 = lose screen
	//menuType 7 = highscores menu
void DefaultSceneLayer::OnUpdate()
{

	Application& app = Application::Get();
	currScene = app.CurrentScene();


	double currFrame = glfwGetTime();
	float dt = static_cast<float>(currFrame - preFrame);

	Gameplay::GameObject::Sptr mainMenu = currScene->FindObjectByName("Main Menu");
	Gameplay::GameObject::Sptr mainMenuB1 = currScene->FindObjectByName("Button1");
	Gameplay::GameObject::Sptr mainMenuB2 = currScene->FindObjectByName("Button2");
	Gameplay::GameObject::Sptr mainMenuB3 = currScene->FindObjectByName("Button3");

	Gameplay::GameObject::Sptr settingsMenu = currScene->FindObjectByName("Settings Menu");
	Gameplay::GameObject::Sptr settingsMenuB1 = currScene->FindObjectByName("Button4");
	Gameplay::GameObject::Sptr settingsMenuB2 = currScene->FindObjectByName("Button5");
	Gameplay::GameObject::Sptr settingsMenuB3 = currScene->FindObjectByName("Button6");

	Gameplay::GameObject::Sptr inGame = currScene->FindObjectByName("inGameGUI");
	Gameplay::GameObject::Sptr inGameScore = currScene->FindObjectByName("Score");
	Gameplay::GameObject::Sptr inGamePower = currScene->FindObjectByName("Charge Level");
	Gameplay::GameObject::Sptr inGameHealth = currScene->FindObjectByName("Health Level");

	Gameplay::GameObject::Sptr pauseMenu = currScene->FindObjectByName("Pause Menu");
	Gameplay::GameObject::Sptr pauseMenuB1 = currScene->FindObjectByName("Button7");
	Gameplay::GameObject::Sptr pauseMenuB2 = currScene->FindObjectByName("Button8");

	Gameplay::GameObject::Sptr winMenu = currScene->FindObjectByName("Win");
	Gameplay::GameObject::Sptr winMenuScore = currScene->FindObjectByName("FinalScoreW");
	Gameplay::GameObject::Sptr winMenuB1 = currScene->FindObjectByName("Button9");

	Gameplay::GameObject::Sptr loseMenu = currScene->FindObjectByName("Lose");
	Gameplay::GameObject::Sptr loseMenuScore = currScene->FindObjectByName("FinalScoreL");
	Gameplay::GameObject::Sptr loseMenuB1 = currScene->FindObjectByName("Button10");

	Gameplay::GameObject::Sptr highMenu = currScene->FindObjectByName("Highscores Menu");
	Gameplay::GameObject::Sptr highMenuScore = currScene->FindObjectByName("Score Display");
	Gameplay::GameObject::Sptr highMenuB1 = currScene->FindObjectByName("Button11");

	if (uiStart == true)
	{
		settingsMenu->SetEnabled(false);
		inGame->SetEnabled(false);
		pauseMenu->SetEnabled(false);
		winMenu->SetEnabled(false);
		loseMenu->SetEnabled(false);
		highMenu->SetEnabled(false);
		uiStart = false;
	}
	if (InputEngine::GetKeyState(GLFW_KEY_P) == ButtonState::Pressed)
	{
		if (!isButtonPressed)
		{
			if (currScene->IsPlaying == true)
			{
				pauseMenu->SetEnabled(true);
				inGame->SetEnabled(false);
				currScene->IsPlaying = false;
				menuType = 4;
			}
			//possible pause sound effect could go here                                       <------------------ GABE LOOK HERE!!!!
			isButtonPressed = true;
		}
	}
	else
	{
		isButtonPressed = false;
	}
	if (menuType == 1)
	{
		//main menu button selection 
		if (menuSelect == 1)
		{
			mainMenuB1->Get<GuiPanel>()->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		}
		else
		{
			mainMenuB1->Get<GuiPanel>()->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));
			//mainMenuB1->Get<GuiPanel>()->SetColor(glm::vec4(1.0f));
		}
		if (menuSelect == 2)
		{
			mainMenuB2->Get<GuiPanel>()->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		}
		else
		{
			mainMenuB2->Get<GuiPanel>()->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));
		}
		if (menuSelect == 3)
		{
			mainMenuB3->Get<GuiPanel>()->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		}
		else
		{
			mainMenuB3->Get<GuiPanel>()->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));
		}		
	}
	//settings selection color
	else if (menuType == 2)
	{
		//main menu button selection 
		if (menuSelect == 1)
		{
			settingsMenuB1->Get<GuiPanel>()->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		}
		else
		{
			settingsMenuB1->Get<GuiPanel>()->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));
		}
		if (menuSelect == 2)
		{
			settingsMenuB2->Get<GuiPanel>()->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		}
		else
		{
			settingsMenuB2->Get<GuiPanel>()->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));
		}
		if (menuSelect == 3)
		{
			settingsMenuB3->Get<GuiPanel>()->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		}
		else
		{
			settingsMenuB3->Get<GuiPanel>()->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));
		}
	}
	//pause selection color
	else if (menuType == 4)
	{
		if (menuSelect == 1)
		{
			pauseMenuB1->Get<GuiPanel>()->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		}
		else
		{
			pauseMenuB1->Get<GuiPanel>()->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));
		}
		if (menuSelect == 2)
		{
			pauseMenuB2->Get<GuiPanel>()->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		}
		else
		{
			pauseMenuB2->Get<GuiPanel>()->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));
		}
	}
	//win screen selection color
	else if (menuType == 5)
	{
		winMenuB1->Get<GuiPanel>()->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	}
	//lose screen selection color
	else if (menuType == 6)
	{
		loseMenuB1->Get<GuiPanel>()->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	}
	else if (menuType == 7)
	{
		highMenuB1->Get<GuiPanel>()->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	}

	if (InputEngine::GetKeyState(GLFW_KEY_UP) == ButtonState::Pressed) //select up
	{
		if (!isButtonPressed)
		{
			if (menuType == 4)
			{
				if (menuSelect == 1)
				{
					menuSelect = 2;
				}
				else
				{
					menuSelect = 1;
				}
			}
			else
			{
				if (menuSelect == 1)
				{
					menuSelect = 3;
				}
				else
				{
					menuSelect--;
				}
			}
			//button select sound goes here                                       <------------------ GABE LOOK HERE!!!!
		}
		isButtonPressed = true;
	}
	else if (InputEngine::GetKeyState(GLFW_KEY_DOWN) == ButtonState::Pressed) //select down
	{
		if (!isButtonPressed)
		{
			if (menuType == 4)
			{
				if (menuSelect == 2)
				{
					menuSelect = 1;
				}
				else
				{
					menuSelect = 2;
				}
			}
			else
			{
				if (menuSelect == 3)
				{
					menuSelect = 1;
				}
				else
				{
					menuSelect++;
				}
			}
			//button select sound goes here                                       <------------------ GABE LOOK HERE!!!!
		}
		isButtonPressed = true;
	}
	else if (InputEngine::GetKeyState(GLFW_KEY_ENTER) == ButtonState::Pressed)  //button selection
	{
		if (!isButtonPressed)
		{
			//button click sound goes here                                       <------------------ GABE LOOK HERE!!!!
			//main menu
			if (menuType == 1)
			{
				if (menuSelect == 1)
				{
					mainMenu->SetEnabled(false);
					inGame->SetEnabled(true);
					inGame->RenderGUI();
					menuType = 3;
					currScene->IsPlaying = true;
					AudioEngine::playEvents("event:/Daytime Song");
				}
				if (menuSelect == 2)
				{
					mainMenu->SetEnabled(false);
					settingsMenu->SetEnabled(true);
					settingsMenu->RenderGUI();
					menuType = 2;
				}
				if (menuSelect == 3)
				{
					exit(0);
				}
			}
			//settings
			else if (menuType == 2)
			{
				if (menuSelect == 3)
				{
					settingsMenu->SetEnabled(false);
					mainMenu->SetEnabled(true);
					mainMenu->RenderGUI();
					menuType = 1;
				}
				if (menuSelect == 2)
				{
					settingsMenu->SetEnabled(false);
					highMenu->SetEnabled(true);
					highMenu->RenderGUI();
					menuType = 7;
					std::ifstream fileReader("Highscores.txt");
					while (std::getline(fileReader, score))
					{
						scores.push_back(stoi(score));
					}

					BubbleSort(scores);

					if (scores.size() < 5)
					{
						for (int i = scores.size() - 1; i >= 0; i--)
						{
							highMenuScore->Get<GuiText>()->SetText(highMenuScore->Get<GuiText>()->GetText() + std::to_string(scores[i]) + "\n");
						}
					}
					else
					{
						for (int i = scores.size() - 1; i >= scores.size() - 5; i--)
						{
							highMenuScore->Get<GuiText>()->SetText(highMenuScore->Get<GuiText>()->GetText() + std::to_string(scores[i]) + "\n");
						}
					}
				}
			}
			//pause menu
			else if (menuType == 4)
			{
				if (menuSelect == 1)
				{
					exit(0);
				}
				else if (menuSelect == 2)
				{
					pauseMenu->SetEnabled(false);
					inGame->SetEnabled(true);
					inGame->RenderGUI();
					menuType = 3;
					currScene->IsPlaying = true;
				}
			}
			//win screen
			else if (menuType == 5)
			{
				exit(0);
			}
			//lose screen
			else if (menuType == 6)
			{
				exit(0);
			}
			else if (menuType == 7)
			{
				highMenu->SetEnabled(false);
				settingsMenu->SetEnabled(true);
				settingsMenu->RenderGUI();
				menuType = 2;
			}
		}
		isButtonPressed = true;
	}
	else
	{
		isButtonPressed = false;
	}
	
	if (InputEngine::GetKeyState(GLFW_KEY_SPACE) == ButtonState::Pressed && canShoot) {
		//shoot then reset wait timer
		if (shootPower < 70)
		{
			shootPower += dt * 20.0f;
		}
		else
		{
			shootPower = 70.0f;
		}
		charging = true;
		powerLevel = (shootPower / 70);
	}
	else
	{
		if (charging == true)
		{
			//spawn cannonball in the lane we're looking at

			canShoot = false;
			shootTimer = shootTime;
			shootPower = 5.0f;
			charging = false;
		}
	}
}






void DefaultSceneLayer::_CreateScene()
{
	using namespace Gameplay;
	using namespace Gameplay::Physics;

	
	//Animation states
	std::vector <MeshResource::Sptr> goblinAnimationRunning;
	std::vector <MeshResource::Sptr> goblinAnimationAttacking;
	std::vector <MeshResource::Sptr> goblinAnimationDying;

	std::vector <MeshResource::Sptr> zombieAnimationRunning;
	std::vector <MeshResource::Sptr> zombieAnimationAttacking;
	std::vector <MeshResource::Sptr> zombieAnimationDying;

	std::vector <MeshResource::Sptr> oozeAnimationWalk;

	std::vector <MeshResource::Sptr> birdAnimationFly;

	float universalFrameTime = 0.05; //0.05 is our default

	Application& app = Application::Get();

	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene && std::filesystem::exists("scene.json")) {
		app.LoadScene("scene.json");
	} else {

#pragma region Loading Shader Programs 
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

		ShaderProgram::Sptr animShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/animation_morph.vert" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
		animShader->SetDebugName("Animation Shader");

#pragma endregion

#pragma region Loading Assets
		// Load in the meshes
		MeshResource::Sptr monkeyMesh = ResourceManager::CreateAsset<MeshResource>("models/Monkey.obj");
		MeshResource::Sptr shipMesh   = ResourceManager::CreateAsset<MeshResource>("models/fenrir.obj");

		//Our previous 3d assets
		MeshResource::Sptr towerGardenMesh = ResourceManager::CreateAsset<MeshResource>("models/FinalArea.obj");
		MeshResource::Sptr towerCannonMesh = ResourceManager::CreateAsset<MeshResource>("models/TowerV1.obj");
		MeshResource::Sptr cannonBallMesh = ResourceManager::CreateAsset<MeshResource>("models/Cannonball.obj");
		MeshResource::Sptr goblinMesh = ResourceManager::CreateAsset<MeshResource>("models/goblinfullrig.obj");
		MeshResource::Sptr spearMesh = ResourceManager::CreateAsset<MeshResource>("models/CubeTester.fbx");

		//Our new static 3D Assets
		MeshResource::Sptr winterGardenMesh = ResourceManager::CreateAsset<MeshResource>("models/WinterMap.obj");
		MeshResource::Sptr newGoblinMesh = ResourceManager::CreateAsset<MeshResource>("models/goblinsprint.obj");

		//Frame 1 of anims
		MeshResource::Sptr birdFlyMesh = ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000001.obj");
		MeshResource::Sptr goblinAttackMesh = ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000001.obj");
		MeshResource::Sptr oozeMesh = ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000001.obj");
		MeshResource::Sptr zombieAttackMesh = ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000001.obj");

		//Animation Test
		


		//Cannon
		MeshResource::Sptr cannonBarrelMesh = ResourceManager::CreateAsset<MeshResource>("models/Animated/Cannon/CannonBarrel.obj");
		MeshResource::Sptr cannonBaseMesh = ResourceManager::CreateAsset<MeshResource>("models/Animated/Cannon/CannonBase.obj");




		// Load in some textures
		Texture2D::Sptr    boxTexture   = ResourceManager::CreateAsset<Texture2D>("textures/box-diffuse.png");
		Texture2D::Sptr    boxSpec      = ResourceManager::CreateAsset<Texture2D>("textures/box-specular.png");
		Texture2D::Sptr    monkeyTex    = ResourceManager::CreateAsset<Texture2D>("textures/monkey-uvMap.png");
		Texture2D::Sptr    leafTex      = ResourceManager::CreateAsset<Texture2D>("textures/leaves.png");
		leafTex->SetMinFilter(MinFilter::Nearest);
		leafTex->SetMagFilter(MagFilter::Nearest); //ggggggggggggggggggg

		//our previous texture assets
		Texture2D::Sptr    gardenTowerTexture = ResourceManager::CreateAsset<Texture2D>("textures/YYY5.png");
		Texture2D::Sptr    redTex = ResourceManager::CreateAsset<Texture2D>("textures/red.png");
		Texture2D::Sptr    goblinTex = ResourceManager::CreateAsset<Texture2D>("textures/GoblinUVFill.png");

		//Our new texture assets
		Texture2D::Sptr    winterGardenTexture = ResourceManager::CreateAsset<Texture2D>("textures/WinterGardenTexture.png");

		//frame 1 animated textures
		Texture2D::Sptr    birdTexture = ResourceManager::CreateAsset<Texture2D>("textures/Animated/BirdUV.png");
		Texture2D::Sptr    goblinAttackTexture = ResourceManager::CreateAsset<Texture2D>("textures/Animated/GoblinUvComp.png");
		Texture2D::Sptr    oozeWalkTexture = ResourceManager::CreateAsset<Texture2D>("textures/Animated/oozeuvspot.png");
		Texture2D::Sptr    zombieTexture = ResourceManager::CreateAsset<Texture2D>("textures/Animated/ZombieUVblood.png");


		//cannon
		Texture2D::Sptr	   cannonBaseTexture = ResourceManager::CreateAsset<Texture2D>("textures/Animated/CannonWood.png");
		Texture2D::Sptr    cannonBarrelTexture = ResourceManager::CreateAsset<Texture2D>("textures/Animated/Cannon.png");
#pragma endregion

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

#pragma region Material Creation
		// Loading in a 1D LUT
		Texture1D::Sptr toonLut = ResourceManager::CreateAsset<Texture1D>("luts/toon-1D-Goblin.png"); 
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
		Texture3D::Sptr lut = ResourceManager::CreateAsset<Texture3D>("luts/cool.CUBE");   

		// Configure the color correction LUT
		scene->SetColorLUT(lut);

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
			//toonMaterial->Set("u_Material.AlbedoMap", boxTexture);
			toonMaterial->Set("u_Material.AlbedoMap", goblinTex);
			toonMaterial->Set("u_Material.NormalMap", normalMapDefault);
			toonMaterial->Set("s_ToonTerm", toonLut);
			toonMaterial->Set("u_Material.Shininess", 0.1f); 
			toonMaterial->Set("u_Material.Steps", 4);
			//toonMaterial->Set("u_Material.", )
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

		//Our previous materials
		Material::Sptr gardenTowerMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			gardenTowerMaterial->Set("u_Material.AlbedoMap", gardenTowerTexture);
			gardenTowerMaterial->Set("u_Material.Shininess", 0.1f);
			gardenTowerMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr cannonBallMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			cannonBallMaterial->Set("u_Material.AlbedoMap", boxTexture);
			cannonBallMaterial->Set("u_Material.Shininess", 0.1f);
			cannonBallMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr goblinMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			goblinMaterial->Set("u_Material.AlbedoMap", goblinTex);
			goblinMaterial->Set("u_Material.Shininess", 0.1f);
			goblinMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr newGoblinMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			newGoblinMaterial->Set("u_Material.AlbedoMap", goblinTex);
			newGoblinMaterial->Set("u_Material.Shininess", 0.1f);
			newGoblinMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		//Our new materials
		Material::Sptr winterGardenMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			winterGardenMaterial->Name = "Winter Garden Mat";
			winterGardenMaterial->Set("u_Material.AlbedoMap", winterGardenTexture);
			winterGardenMaterial->Set("u_Material.Shininess", 0.1f);
			winterGardenMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		//frame 1 material stuff
		Material::Sptr birdFlyMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			birdFlyMaterial->Name = "birdFly Mat";
			birdFlyMaterial->Set("u_Material.AlbedoMap", birdTexture);
			birdFlyMaterial->Set("u_Material.Shininess", 0.1f);
			birdFlyMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr goblinAttackMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			goblinAttackMaterial->Name = "goblinAttack Mat";
			goblinAttackMaterial->Set("u_Material.AlbedoMap", goblinAttackTexture);
			goblinAttackMaterial->Set("u_Material.Shininess", 0.1f);
			goblinAttackMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr oozeMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			oozeMaterial->Name = "ooze Mat";
			oozeMaterial->Set("u_Material.AlbedoMap", oozeWalkTexture);
			oozeMaterial->Set("u_Material.Shininess", 0.1f);
			oozeMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr zombieAttackMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			zombieAttackMaterial->Name = "zombieAttack Mat";
			zombieAttackMaterial->Set("u_Material.AlbedoMap", zombieTexture);
			zombieAttackMaterial->Set("u_Material.Shininess", 0.1f);
			zombieAttackMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}



		//ANIMATION MATERIAL TEST 
		Texture2D::Sptr animTestTexture = ResourceManager::CreateAsset<Texture2D>("textures/Animated/oozeuvspot.png");
		// Create our material
		Material::Sptr animTestMaterial = ResourceManager::CreateAsset<Material>(animShader);
		{
			animTestMaterial->Name = "Animation Test Material";
			animTestMaterial-> Set("u_Material.AlbedoMap", animTestTexture);
			animTestMaterial->Set("u_Material.Shininess", 1.0f);
			animTestMaterial->Set("u_Material.NormalMap", normalMapDefault);


		}

		//ANIMATION MATERIAL OOZE
		Texture2D::Sptr animOozeTexture = ResourceManager::CreateAsset<Texture2D>("textures/Animated/oozeuvspot.png");
		// Create our material
		Material::Sptr animOozeMaterial = ResourceManager::CreateAsset<Material>(animShader);
		{
			animOozeMaterial->Name = "Animation Ooze Material";
			animOozeMaterial->Set("u_Material.AlbedoMap", animOozeTexture);
			animOozeMaterial->Set("u_Material.Shininess", 1.0f);
			animOozeMaterial->Set("u_Material.NormalMap", normalMapDefault);


		}

		//ANIMATION MATERIAL bird
		Texture2D::Sptr animbirdTexture = ResourceManager::CreateAsset<Texture2D>("textures/Animated/oozeuvspot.png");
		// Create our material
		Material::Sptr animbirdMaterial = ResourceManager::CreateAsset<Material>(animShader);
		{
			animbirdMaterial->Name = "Animation Bird Material";
			animbirdMaterial->Set("u_Material.AlbedoMap", animbirdTexture);
			animbirdMaterial->Set("u_Material.Shininess", 1.0f);
			animbirdMaterial->Set("u_Material.NormalMap", normalMapDefault);


		}

		//ANIMATION MATERIAL goblin run
		Texture2D::Sptr animGoblinRunTexture = ResourceManager::CreateAsset<Texture2D>("textures/Animated/oozeuvspot.png");
		// Create our material
		Material::Sptr animGoblinRunMaterial = ResourceManager::CreateAsset<Material>(animShader);
		{
			animGoblinRunMaterial->Name = "Animation Goblin Run Material";
			animGoblinRunMaterial->Set("u_Material.AlbedoMap", animGoblinRunTexture);
			animGoblinRunMaterial->Set("u_Material.Shininess", 1.0f);
			animGoblinRunMaterial->Set("u_Material.NormalMap", normalMapDefault);


		}

		//ANIMATION MATERIAL goblin attack
		Texture2D::Sptr animGoblinAttackTexture = ResourceManager::CreateAsset<Texture2D>("textures/Animated/oozeuvspot.png");
		// Create our material
		Material::Sptr animGoblinAttackMaterial = ResourceManager::CreateAsset<Material>(animShader);
		{
			animGoblinAttackMaterial->Name = "Animation Goblin Attack Material";
			animGoblinAttackMaterial->Set("u_Material.AlbedoMap", animGoblinAttackTexture);
			animGoblinAttackMaterial->Set("u_Material.Shininess", 1.0f);
			animGoblinAttackMaterial->Set("u_Material.NormalMap", normalMapDefault);


		}

		//ANIMATION MATERIAL goblin death
		Texture2D::Sptr animGoblinDeathTexture = ResourceManager::CreateAsset<Texture2D>("textures/Animated/oozeuvspot.png");
		// Create our material
		Material::Sptr animGoblinDeathMaterial = ResourceManager::CreateAsset<Material>(animShader);
		{
			animGoblinDeathMaterial->Name = "Animation Goblin Death Material";
			animGoblinDeathMaterial->Set("u_Material.AlbedoMap", animGoblinDeathTexture);
			animGoblinDeathMaterial->Set("u_Material.Shininess", 1.0f);
			animGoblinDeathMaterial->Set("u_Material.NormalMap", normalMapDefault);


		}

		//ANIMATION MATERIAL zombie run
		Texture2D::Sptr animZombieRunTexture = ResourceManager::CreateAsset<Texture2D>("textures/Animated/oozeuvspot.png");
		// Create our material
		Material::Sptr animZombieRunMaterial = ResourceManager::CreateAsset<Material>(animShader);
		{
			animZombieRunMaterial->Name = "Animation Zombie Run Material";
			animZombieRunMaterial->Set("u_Material.AlbedoMap", animZombieRunTexture);
			animZombieRunMaterial->Set("u_Material.Shininess", 1.0f);
			animZombieRunMaterial->Set("u_Material.NormalMap", normalMapDefault);


		}

		//ANIMATION MATERIAL zombie attack
		Texture2D::Sptr animZombieAttackTexture = ResourceManager::CreateAsset<Texture2D>("textures/Animated/oozeuvspot.png");
		// Create our material
		Material::Sptr animZombieAttackMaterial = ResourceManager::CreateAsset<Material>(animShader);
		{
			animZombieAttackMaterial->Name = "Animation Zombie Attack Material";
			animZombieAttackMaterial->Set("u_Material.AlbedoMap", animZombieAttackTexture);
			animZombieAttackMaterial->Set("u_Material.Shininess", 1.0f);
			animZombieAttackMaterial->Set("u_Material.NormalMap", normalMapDefault);


		}

		//ANIMATION MATERIAL zombie death
		Texture2D::Sptr animZombieDeathTexture = ResourceManager::CreateAsset<Texture2D>("textures/Animated/oozeuvspot.png");
		// Create our material
		Material::Sptr animZombieDeathMaterial = ResourceManager::CreateAsset<Material>(animShader);
		{
			animZombieDeathMaterial->Name = "Animation Zombie Death Material";
			animZombieDeathMaterial->Set("u_Material.AlbedoMap", animZombieDeathTexture);
			animZombieDeathMaterial->Set("u_Material.Shininess", 1.0f);
			animZombieDeathMaterial->Set("u_Material.NormalMap", normalMapDefault);


		}


		//Cannon stuff
		Material::Sptr cannonBaseMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			cannonBaseMaterial->Name = "cannonBase mate";
			cannonBaseMaterial->Set("u_Material.AlbedoMap", cannonBaseTexture);
			cannonBaseMaterial->Set("u_Material.Shininess", 0.1f);
			cannonBaseMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr cannonBarrelMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			cannonBarrelMaterial->Name = "cannon Barrel mat";
			cannonBarrelMaterial->Set("u_Material.AlbedoMap", cannonBarrelTexture);
			cannonBarrelMaterial->Set("u_Material.Shininess", 0.1f);
			cannonBarrelMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}


#pragma endregion

#pragma region Lights Camera Action
		// Create some lights for our scene
		GameObject::Sptr lightParent = scene->CreateGameObject("Lights");

		GameObject::Sptr mainLight = scene->CreateGameObject("mainLight");
		mainLight->SetPostion(glm::vec3(0.0f,0.0f,10.0f));
		lightParent->AddChild(mainLight);

		Light::Sptr lightComponent = mainLight->Add<Light>();
		lightComponent->SetColor(glm::vec3(1.0f,1.0f,1.0f));
		lightComponent->SetRadius(50.0f); //25
		lightComponent->SetIntensity(200.0f); //25

		//Additional lights randomized. Default 50.
		for (int ix = 0; ix < 0; ix++) {
			GameObject::Sptr light = scene->CreateGameObject("Light");
			light->SetPostion(glm::vec3(glm::diskRand(25.0f), 1.0f));
			lightParent->AddChild(light);

			toonMaterial->Set("u_Material.LightPos", light->GetPosition());

			Light::Sptr lightComponent = light->Add<Light>();
			lightComponent->SetColor(glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f)));
			lightComponent->SetRadius(glm::linearRand(0.1f, 10.0f));
			lightComponent->SetIntensity(glm::linearRand(1.0f, 2.0f));
		}

		// Set up the scene's camera
		GameObject::Sptr camera = scene->MainCamera->GetGameObject()->SelfRef();
		{
			camera->SetPostion({ 2.75, 0, 5 }); //-9,-6,15 ; 2.75, 0, 5 
			camera->SetRotation(glm::vec3(50.0f,0.f,-90.0f)); //90, 0,0 
			//camera->LookAt(glm::vec3(0.0f));

			//Need to create a camera controller for gameplay
			//camera->Add<CameraVanguard>();
			 
			//camera->Add<SimpleCameraControl>();


			// This is now handled by scene itself!
			//Camera::Sptr cam = camera->Add<Camera>();
			// Make sure that the camera is set as the scene's main camera!
			//scene->MainCamera = cam;
		}

#pragma endregion

#pragma region Setting the Scene
		// We'll create a mesh that is a simple plane that we can resize later
		MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>();
		planeMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(1.0f)));
		planeMesh->GenerateMesh();

		MeshResource::Sptr sphere = ResourceManager::CreateAsset<MeshResource>();
		sphere->AddParam(MeshBuilderParam::CreateIcoSphere(ZERO, ONE, 5));
		sphere->GenerateMesh();

		//Parents for organization
		GameObject::Sptr defaultsParent = scene->CreateGameObject("Defaults"); {
			defaultsParent->SetPostion(glm::vec3(0.0f,0.0f,0.0f));
		}
		GameObject::Sptr mapParent = scene->CreateGameObject("Map"); {

		}

		GameObject::Sptr cameraOffset = scene->CreateGameObject("Camera Offset"); {
			cameraOffset->Add<CameraVanguard>();
		}

		GameObject::Sptr gameObjectsParent = scene->CreateGameObject("Game Objects");
		GameObject::Sptr enemiesParent = scene->CreateGameObject("Enemies");
		GameObject::Sptr uiParent = scene->CreateGameObject("UI");
		GameObject::Sptr cannonParent = scene->CreateGameObject("CannonParts");

		cameraOffset->AddChild(camera);
		gameObjectsParent->AddChild(enemiesParent);
		gameObjectsParent->AddChild(cannonParent);
		

		// Set up all our sample objects
		GameObject::Sptr plane = scene->CreateGameObject("Plane");
		{
			plane->SetPostion(glm::vec3(0.0f,0.0f,-4.0f));

			// Make a big tiled mesh
			MeshResource::Sptr tiledMesh = ResourceManager::CreateAsset<MeshResource>();
			tiledMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(100.0f), glm::vec2(20.0f)));
			tiledMesh->GenerateMesh();

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = plane->Add<RenderComponent>();
			renderer->SetMesh(tiledMesh);
			renderer->SetMaterial(boxMaterial);

			// Attach a plane collider that extends infinitely along the X/Y axis
			RigidBody::Sptr physics = plane->Add<RigidBody>(/*static by default*/);
			physics->AddCollider(BoxCollider::Create(glm::vec3(50.0f, 50.0f, 1.0f)))->SetPosition({ 0,0,-1 });

			defaultsParent->AddChild(plane);
		}


		GameObject::Sptr WinterGarden = scene->CreateGameObject("Winter Garden");
		{
			// Set position in the scene
			WinterGarden->SetPostion(glm::vec3(0.0f, 0.0f, 0.0f));
			WinterGarden->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			WinterGarden->SetScale(glm::vec3(0.100f,0.100f,0.100f));

			RenderComponent::Sptr renderer = WinterGarden->Add<RenderComponent>();
			renderer->SetMesh(winterGardenMesh);
			renderer->SetMaterial(winterGardenMaterial);

			mapParent->AddChild(WinterGarden);
		}


		GameObject::Sptr towerGarden = scene->CreateGameObject("towerGarden");
		{
			// Set position in the scene
			towerGarden->SetPostion(glm::vec3(-130.69f, -143.80f, -400.0f)); //-130.69, -143.80, -4
			towerGarden->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

			RenderComponent::Sptr renderer = towerGarden->Add<RenderComponent>();
			renderer->SetMesh(towerGardenMesh);
			renderer->SetMaterial(gardenTowerMaterial);
			
			mapParent->AddChild(towerGarden);
		}

		GameObject::Sptr cannonBall = scene->CreateGameObject("cannonBall");
		{
			cannonBall->SetPostion(glm::vec3(12.6f, -10.4f, 1.0f));
			cannonBall->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			cannonBall->SetScale(glm::vec3(1.f));

			//Add a rigidbody to hit with force
			RigidBody::Sptr ballPhy = cannonBall->Add<RigidBody>(RigidBodyType::Dynamic);
			ballPhy->SetMass(5.0f);
			ballPhy->AddCollider(SphereCollider::Create(1.f))->SetPosition({ 0, 0, 0 });

			/*TriggerVolume::Sptr volume = cannonBall->Add<TriggerVolume>();
			  SphereCollider::Sptr collider = SphereCollider::Create(1.f);
			  collider->SetPosition(glm::vec3(0.f));
			  volume->AddCollider(collider);

			  cannonBall->Add<TriggerVolumeEnterBehaviour>();*/

			  // Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = cannonBall->Add<RenderComponent>();
			renderer->SetMesh(cannonBallMesh);
			renderer->SetMaterial(cannonBallMaterial);

			gameObjectsParent->AddChild(cannonBall);
		}

		GameObject::Sptr cannonBarrel = scene->CreateGameObject("Cannon Barrel"); {
			cannonBarrel->SetPostion(glm::vec3(12.6f, -10.4f, 1.0f));
			cannonBarrel->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			cannonBarrel->SetScale(glm::vec3(1.f));

			RenderComponent::Sptr renderer = cannonBarrel->Add<RenderComponent>();
			renderer->SetMesh(cannonBarrelMesh);
			renderer->SetMaterial(cannonBarrelMaterial); //needs

			cannonParent->AddChild(cannonBarrel);
		};

		GameObject::Sptr cannonBase = scene->CreateGameObject("Cannon Base"); {
			cannonBase->SetPostion(glm::vec3(12.6f, -10.4f, 1.0f));
			cannonBase->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			cannonBase->SetScale(glm::vec3(1.f));

			RenderComponent::Sptr renderer = cannonBase->Add<RenderComponent>();
			renderer->SetMesh(cannonBaseMesh);
			renderer->SetMaterial(cannonBaseMaterial); //needs

			cannonParent->AddChild(cannonBase);
		};

		GameObject::Sptr towerCannon = scene->CreateGameObject("towerCannon");
		{
			towerCannon->SetPostion(glm::vec3(0.0f, 0.0f, 0.0f));
			towerCannon->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

			// Add some behaviour that relies on the physics body
			//towerGarden->Add<JumpBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = towerCannon->Add<RenderComponent>();
			renderer->SetMesh(towerCannonMesh);
			renderer->SetMaterial(gardenTowerMaterial);
			mapParent->AddChild(towerCannon);
		}

		GameObject::Sptr towerSpears = scene->CreateGameObject("towerSpears");
		{
			towerSpears->SetPostion(glm::vec3(12.6f, -10.4f, 1.0f));
			towerSpears->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

			// Add some behaviour that relies on the physics body
			//towerGarden->Add<JumpBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = towerSpears->Add<RenderComponent>();
			renderer->SetMesh(spearMesh);
			renderer->SetMaterial(goblinMaterial);

			mapParent->AddChild(towerSpears);

		}


		GameObject::Sptr goblin1 = scene->CreateGameObject("goblin1");
		{
			// Set position in the scene
			goblin1->SetPostion(glm::vec3(12.760f, 0.0f, 1.0f));
			goblin1->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
			goblin1->SetScale(glm::vec3(2.0f));

			// Add some behaviour that relies on the physics body
			//towerGarden->Add<JumpBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = goblin1->Add<RenderComponent>();
			renderer->SetMesh(newGoblinMesh);
			//renderer->SetMaterial(goblinMaterial);
			toonMaterial->Set("u_Material.WorldPos", goblin1->GetPosition());
			renderer->SetMaterial(toonMaterial);

			//RigidBody::Sptr goblinRB = goblin1->Add<RigidBody>(RigidBodyType::Dynamic);
			//goblinRB->AddCollider(BoxCollider::Create())->SetPosition(glm::vec3(0.f));

			TriggerVolume::Sptr volume = goblin1->Add<TriggerVolume>();
			CylinderCollider::Sptr col = CylinderCollider::Create(glm::vec3(1.f, 1.f, 1.f));
			volume->AddCollider(col);

			goblin1->Add<TriggerVolumeEnterBehaviour>();
			goblin1->Add<EnemyMovement>();

			goblin1->Get<EnemyMovement>()->setGameObject(goblin1);
			

			// Add a dynamic rigid body to this monkey
			RigidBody::Sptr physics = goblin1->Add<RigidBody>(RigidBodyType::Dynamic);
			physics->AddCollider(ConvexMeshCollider::Create());

			enemiesParent->AddChild(goblin1);

		}

		//Frame 1 stuff and more stuff
		GameObject::Sptr birdFly = scene->CreateGameObject("birdFly"); {
			birdFly->SetPostion(glm::vec3(10, 5.f, 5.0f));
			birdFly->SetRotation(glm::vec3(90.0f, 145.0f, 96.0f));
			birdFly->SetScale(glm::vec3(1.f));

			RenderComponent::Sptr renderer = birdFly->Add<RenderComponent>();
			renderer->SetMesh(birdFlyMesh);
			renderer->SetMaterial(birdFlyMaterial); //needs

			MorphMeshRenderer::Sptr initialMorph = birdFly->Add<MorphMeshRenderer>();
			initialMorph->SetMorphMeshRenderer(birdFlyMesh, birdFlyMaterial);
			MorphAnimator::Sptr afterMorph = birdFly->Add<MorphAnimator>();

			MeshResource::Sptr birdAnimationFrames[] = {
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000001.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000002.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000003.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000004.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000005.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000006.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000007.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000008.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000009.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000010.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000011.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000012.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000013.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000014.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000015.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000016.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000017.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000018.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000019.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Bird/Birdfly_000020.obj")
				//20 FRAMES OF ANIMATIONS
			};

			for (int i = 0; i < 20; i++) {
				birdAnimationFly.push_back(birdAnimationFrames[i]);
			}

			afterMorph->SetInitial();
			afterMorph->SetFrameTime(universalFrameTime);
			afterMorph->SetFrames(birdAnimationFly);

			enemiesParent->AddChild(birdFly);
		};

		GameObject::Sptr goblinAttack = scene->CreateGameObject("goblinAttack"); {
			goblinAttack->SetPostion(glm::vec3(7.62f,-2.97f, 1.0f));
			goblinAttack->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
			goblinAttack->SetScale(glm::vec3(1.f));

			RenderComponent::Sptr renderer = goblinAttack->Add<RenderComponent>();
			renderer->SetMesh(goblinAttackMesh);
			//renderer->SetMaterial(goblinAttackMaterial); //needs
			renderer->SetMaterial(toonMaterial);

			MorphMeshRenderer::Sptr initialMorph = goblinAttack->Add<MorphMeshRenderer>();
			initialMorph->SetMorphMeshRenderer(goblinAttackMesh, animGoblinAttackMaterial);
			MorphAnimator::Sptr afterMorph = goblinAttack->Add<MorphAnimator>();

			MeshResource::Sptr goblinRunningAnimationFrames[] = {
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000001.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000002.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000003.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000004.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000005.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000006.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000007.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000008.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000009.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000010.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000011.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000012.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000013.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000014.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000015.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000016.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000017.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000018.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000019.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/Run/GoblinRun_000020.obj")
				//20 FRAMES OF ANIMATIONS
			};

			MeshResource::Sptr goblinAttackAnimationFrames[] = {
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000001.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000002.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000003.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000004.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000005.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000006.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000007.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000008.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000009.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000010.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000011.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000012.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000013.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000014.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000015.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000016.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000017.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000018.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000019.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/attack/GoblinAttack_000020.obj")
				//20 FRAMES OF ANIMATIONS
			};

			MeshResource::Sptr goblinDyingAnimationFrames[] = {
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000001.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000002.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000003.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000004.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000005.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000006.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000007.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000008.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000009.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000010.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000011.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000012.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000013.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000014.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000015.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000016.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000017.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000018.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000019.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Goblin/die/Goblindie_000020.obj")
				//20 FRAMES OF ANIMATIONS
			};

			for (int i = 0; i < 20; i++) {
				goblinAnimationRunning.push_back(goblinRunningAnimationFrames[i]);
				goblinAnimationAttacking.push_back(goblinAttackAnimationFrames[i]);
				goblinAnimationDying.push_back(goblinDyingAnimationFrames[i]);
			}

			afterMorph->SetInitial();
			afterMorph->SetFrameTime(universalFrameTime);
			afterMorph->SetFrames(goblinAnimationRunning);


			enemiesParent->AddChild(goblinAttack);
		};

		GameObject::Sptr oozeWalk = scene->CreateGameObject("oozeWalk"); {
			oozeWalk->SetPostion(glm::vec3(5, 0.f, 2.0f));
			oozeWalk->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
			oozeWalk->SetScale(glm::vec3(1.f));

			RenderComponent::Sptr renderer = oozeWalk->Add<RenderComponent>();
			renderer->SetMesh(oozeMesh);
			renderer->SetMaterial(oozeMaterial); //needs

			//TESTING OOZE ANIMATION
			MorphMeshRenderer::Sptr initialMorph = oozeWalk->Add<MorphMeshRenderer>();
			initialMorph->SetMorphMeshRenderer(oozeMesh,animOozeMaterial);
			MorphAnimator::Sptr afterMorph = oozeWalk->Add<MorphAnimator>();

			MeshResource::Sptr oozeAnimationFrames[] = {
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000001.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000002.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000003.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000004.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000005.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000006.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000007.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000008.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000009.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000010.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000011.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000012.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000013.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000014.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000015.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000016.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000017.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000018.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000019.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Ooze/walk/oozewalk_000020.obj")
				//20 FRAMES OF ANIMATIONS
			};

			for (int i = 0; i < 20; i++) {
				oozeAnimationWalk.push_back(oozeAnimationFrames[i]);
			}

			afterMorph->SetInitial();
			afterMorph->SetFrameTime(universalFrameTime);
			afterMorph->SetFrames(oozeAnimationWalk);

			enemiesParent->AddChild(oozeWalk);

		};

		GameObject::Sptr zombieAttack = scene->CreateGameObject("zombieAttack"); {
			zombieAttack->SetPostion(glm::vec3(6.70, 2.970f, 2.0f));
			zombieAttack->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
			zombieAttack->SetScale(glm::vec3(1.f));

			RenderComponent::Sptr renderer = zombieAttack->Add<RenderComponent>();
			renderer->SetMesh(zombieAttackMesh);
			renderer->SetMaterial(zombieAttackMaterial); //needs


			MorphMeshRenderer::Sptr initialMorph = zombieAttack->Add<MorphMeshRenderer>();
			initialMorph->SetMorphMeshRenderer(zombieAttackMesh, animZombieAttackMaterial);
			MorphAnimator::Sptr afterMorph = zombieAttack->Add<MorphAnimator>();

			MeshResource::Sptr zombieRunningAnimationFrames[] = {
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000001.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000002.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000003.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000004.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000005.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000006.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000007.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000008.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000009.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000010.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000011.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000012.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000013.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000014.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000015.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000016.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000017.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000018.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000019.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/run/ZombieRun_000020.obj")
				//20 FRAMES OF ANIMATIONS
			};

			MeshResource::Sptr zombieAttackAnimationFrames[] = {
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000001.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000002.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000003.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000004.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000005.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000006.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000007.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000008.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000009.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000010.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000011.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000012.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000013.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000014.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000015.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000016.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000017.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000018.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000019.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/attack/ZombieAttack_000020.obj")
				//20 FRAMES OF ANIMATIONS
			};

			MeshResource::Sptr zombieDyingAnimationFrames[] = {
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000001.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000002.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000003.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000004.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000005.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000006.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000007.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000008.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000009.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000010.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000011.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000012.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000013.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000014.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000015.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000016.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000017.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000018.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000019.obj"),
				ResourceManager::CreateAsset<MeshResource>("models/Animated/Zombie/die/zombieDie_000020.obj")
				//20 FRAMES OF ANIMATIONS
				//Does not exist so goblin used as stand in
			};

			for (int i = 0; i < 20; i++) {
				zombieAnimationRunning.push_back(zombieRunningAnimationFrames[i]);
				zombieAnimationAttacking.push_back(zombieAttackAnimationFrames[i]);
				zombieAnimationDying.push_back(zombieDyingAnimationFrames[i]);
			}

			afterMorph->SetInitial();
			afterMorph->SetFrameTime(universalFrameTime);
			afterMorph->SetFrames(zombieAnimationDying);


			enemiesParent->AddChild(zombieAttack);
		};
		

#pragma endregion

#pragma region UI creation

		//pain
/*
		GameObject::Sptr canvas = scene->CreateGameObject("UI Canvas");
		{
			RectTransform::Sptr transform = canvas->Add<RectTransform>();
			transform->SetMin({ 16, 16 });
			transform->SetMax({ 128, 128 });
			transform->SetPosition(glm::vec2(100.0f, 177.0f));
			GuiPanel::Sptr canPanel = canvas->Add<GuiPanel>();


			GameObject::Sptr subPanel = scene->CreateGameObject("Sub Item");
			{
				RectTransform::Sptr transform = subPanel->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 64, 64 });

				GuiPanel::Sptr panel = subPanel->Add<GuiPanel>();
				panel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

				//panel->SetTexture(ResourceManager::CreateAsset<Texture2D>("textures/upArrow.png"));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 16.0f);
				font->Bake();

				GuiText::Sptr text = subPanel->Add<GuiText>();
				text->SetText("Hello world!");
				text->SetFont(font);

			}

			canvas->AddChild(subPanel);
		}*/
		///////////////////////////// UI //////////////////////////////
		GameObject::Sptr canvas = scene->CreateGameObject("Main Menu");
		{
			RectTransform::Sptr transform = canvas->Add<RectTransform>();
			transform->SetMin({ 100, 100 });
			transform->SetMax({ 700, 800 });
			transform->SetPosition(glm::vec2(960.0f, 400.0f));

			GuiPanel::Sptr canPanel = canvas->Add<GuiPanel>();
			canPanel->SetColor(glm::vec4(0.6f, 0.3f, 0.0f, 1.0f));

			GameObject::Sptr subPanel = scene->CreateGameObject("Button1");
			{
				RectTransform::Sptr transform = subPanel->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 300.0f));

				GuiPanel::Sptr panel = subPanel->Add<GuiPanel>();
				//panel->SetTexture(ResourceManager::CreateAsset<Texture2D>("textures/PlayIdle.png"));				
				panel->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel->Add<GuiText>();
				text->SetText("Play");
				text->SetFont(font);

			}
			canvas->AddChild(subPanel);

			GameObject::Sptr subPanel2 = scene->CreateGameObject("Button2");
			{
				RectTransform::Sptr transform = subPanel2->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 450.0f));

				GuiPanel::Sptr panel = subPanel2->Add<GuiPanel>();
				panel->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel2->Add<GuiText>();
				text->SetText("Settings");
				text->SetFont(font);

			}
			canvas->AddChild(subPanel2);

			GameObject::Sptr subPanel3 = scene->CreateGameObject("Button3");
			{
				RectTransform::Sptr transform = subPanel3->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 600.0f));

				GuiPanel::Sptr panel = subPanel3->Add<GuiPanel>();
				panel->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel3->Add<GuiText>();
				text->SetText("Exit");
				text->SetFont(font);

			}
			canvas->AddChild(subPanel3);

			GameObject::Sptr subPanel4 = scene->CreateGameObject("Title");
			{
				RectTransform::Sptr transform = subPanel4->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 100.0f));

				GuiPanel::Sptr panel = subPanel4->Add<GuiPanel>();
				panel->SetColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel4->Add<GuiText>();
				text->SetText("Vanguard");
				text->SetFont(font);

			}
			canvas->AddChild(subPanel4);

			//uiParent->AddChild(canvas);
		}

		GameObject::Sptr canvas2 = scene->CreateGameObject("Settings Menu");
		{
			RectTransform::Sptr transform = canvas2->Add<RectTransform>();
			transform->SetMin({ 100, 100 });
			transform->SetMax({ 700, 800 });
			transform->SetPosition(glm::vec2(960.0f, 400.0f));

			GuiPanel::Sptr canPanel = canvas2->Add<GuiPanel>();
			canPanel->SetColor(glm::vec4(0.6f, 0.3f, 0.0f, 1.0f));

			GameObject::Sptr subPanel = scene->CreateGameObject("Button4");
			{
				RectTransform::Sptr transform = subPanel->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 300.0f));

				GuiPanel::Sptr panel = subPanel->Add<GuiPanel>();
				panel->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel->Add<GuiText>();
				text->SetText("Settings stuff");
				text->SetFont(font);

			}
			canvas2->AddChild(subPanel);

			GameObject::Sptr subPanel2 = scene->CreateGameObject("Button5");
			{
				RectTransform::Sptr transform = subPanel2->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 450.0f));

				GuiPanel::Sptr panel = subPanel2->Add<GuiPanel>();
				panel->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel2->Add<GuiText>();
				text->SetText("Highscores");
				text->SetFont(font);

			}
			canvas2->AddChild(subPanel2);

			GameObject::Sptr subPanel3 = scene->CreateGameObject("Button6");
			{
				RectTransform::Sptr transform = subPanel3->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 600.0f));

				GuiPanel::Sptr panel = subPanel3->Add<GuiPanel>();
				panel->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel3->Add<GuiText>();
				text->SetText("Back");
				text->SetFont(font);

			}
			canvas2->AddChild(subPanel3);

			GameObject::Sptr subPanel4 = scene->CreateGameObject("Settings Title");
			{
				RectTransform::Sptr transform = subPanel4->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 100.0f));

				GuiPanel::Sptr panel = subPanel4->Add<GuiPanel>();
				panel->SetColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel4->Add<GuiText>();
				text->SetText("Settings");
				text->SetFont(font);

			}
			canvas2->AddChild(subPanel4);
			//uiParent->AddChild(canvas2);
		}

		GameObject::Sptr canvas3 = scene->CreateGameObject("inGameGUI");
		{
			RectTransform::Sptr transform = canvas3->Add<RectTransform>();
			GameObject::Sptr subPanel1 = scene->CreateGameObject("Score");
			{
				RectTransform::Sptr transform = subPanel1->Add<RectTransform>();
				transform->SetMin({ 6, 10 });
				transform->SetMax({ 110, 50 });
				transform->SetPosition(glm::vec2(100.0f, 960.0f));

				GuiPanel::Sptr canPanel = subPanel1->Add<GuiPanel>();
				canPanel->SetColor(glm::vec4(0.6f, 0.3f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 16.0f);
				font->Bake();

				GuiText::Sptr text = subPanel1->Add<GuiText>();
				text->SetText("0");
				text->SetFont(font);
			}
			canvas3->AddChild(subPanel1);

			GameObject::Sptr subPanel2 = scene->CreateGameObject("Power Bar");
			{
				RectTransform::Sptr transform = subPanel2->Add<RectTransform>();
				transform->SetMin({ 6, 10 });
				transform->SetMax({ 180, 50 });
				transform->SetPosition(glm::vec2(1800.0f, 960.0f));

				GuiPanel::Sptr panel = subPanel2->Add<GuiPanel>();
				panel->SetColor(glm::vec4(0.6f, 0.3f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 16.0f);
				font->Bake();

				GuiText::Sptr text = subPanel2->Add<GuiText>();
				text->SetText("Power");
				text->SetFont(font);

			}
			canvas3->AddChild(subPanel2);

			GameObject::Sptr subPanel3 = scene->CreateGameObject("Charge Level");
			{
				RectTransform::Sptr transform = subPanel3->Add<RectTransform>();
				transform->SetMin({ 0, 10 });
				transform->SetMax({ 10, 20 });
				transform->SetPosition(glm::vec2(1730.0f, 964.0f));

				GuiPanel::Sptr panel = subPanel3->Add<GuiPanel>();
				panel->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
				//panel->SetTexture(ResourceManager::CreateAsset<Texture2D>("textures/red.png"));
			}
			canvas3->AddChild(subPanel3);

			GameObject::Sptr subPanel4 = scene->CreateGameObject("Health Bar");
			{
				RectTransform::Sptr transform = subPanel4->Add<RectTransform>();
				transform->SetMin({ 6, 10 });
				transform->SetMax({ 180, 50 });
				transform->SetPosition(glm::vec2(100.0f, 30.0f));

				GuiPanel::Sptr panel = subPanel4->Add<GuiPanel>();
				panel->SetColor(glm::vec4(0.6f, 0.3f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 16.0f);
				font->Bake();

				GuiText::Sptr text = subPanel4->Add<GuiText>();
				text->SetText("Tower Health");
				text->SetFont(font);

			}
			canvas3->AddChild(subPanel4);

			GameObject::Sptr subPanel5 = scene->CreateGameObject("Health Level");
			{
				RectTransform::Sptr transform = subPanel5->Add<RectTransform>();
				transform->SetMin({ 0, 10 });
				transform->SetMax({ 150, 20 });
				transform->SetPosition(glm::vec2(100.0f, 35.0f));

				GuiPanel::Sptr panel = subPanel5->Add<GuiPanel>();
				panel->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
				//panel->SetTexture(ResourceManager::CreateAsset<Texture2D>("textures/red.png"));

			}
			canvas3->AddChild(subPanel5);
			//uiParent->AddChild(canvas3);
		}

		GameObject::Sptr canvas4 = scene->CreateGameObject("Pause Menu");
		{
			RectTransform::Sptr transform = canvas4->Add<RectTransform>();
			transform->SetMin({ 100, 100 });
			transform->SetMax({ 700, 800 });
			transform->SetPosition(glm::vec2(960.0f, 400.0f));

			GuiPanel::Sptr canPanel = canvas4->Add<GuiPanel>();
			canPanel->SetColor(glm::vec4(0.6f, 0.3f, 0.0f, 1.0f));

			GameObject::Sptr subPanel2 = scene->CreateGameObject("Button7");
			{
				RectTransform::Sptr transform = subPanel2->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 450.0f));

				GuiPanel::Sptr panel = subPanel2->Add<GuiPanel>();
				panel->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel2->Add<GuiText>();
				text->SetText("Exit Game");
				text->SetFont(font);

			}
			canvas4->AddChild(subPanel2);

			GameObject::Sptr subPanel3 = scene->CreateGameObject("Button8");
			{
				RectTransform::Sptr transform = subPanel3->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 600.0f));

				GuiPanel::Sptr panel = subPanel3->Add<GuiPanel>();
				panel->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel3->Add<GuiText>();
				text->SetText("Resume");
				text->SetFont(font);

			}
			canvas4->AddChild(subPanel3);

			GameObject::Sptr subPanel4 = scene->CreateGameObject("Paused Title");
			{
				RectTransform::Sptr transform = subPanel4->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 100.0f));

				GuiPanel::Sptr panel = subPanel4->Add<GuiPanel>();
				panel->SetColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel4->Add<GuiText>();
				text->SetText("Paused");
				text->SetFont(font);

			}
			canvas4->AddChild(subPanel4);
			//uiParent->AddChild(canvas4);
		}

		GameObject::Sptr canvas5 = scene->CreateGameObject("Win");
		{
			RectTransform::Sptr transform = canvas5->Add<RectTransform>();
			transform->SetMin({ 100, 100 });
			transform->SetMax({ 700, 800 });
			transform->SetPosition(glm::vec2(960.0f, 400.0f));

			GuiPanel::Sptr canPanel = canvas5->Add<GuiPanel>();
			canPanel->SetColor(glm::vec4(0.6f, 0.3f, 0.0f, 1.0f));

			GameObject::Sptr subPanel2 = scene->CreateGameObject("FinalScoreW");
			{
				RectTransform::Sptr transform = subPanel2->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 450.0f));

				GuiPanel::Sptr panel = subPanel2->Add<GuiPanel>();
				panel->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel2->Add<GuiText>();
				text->SetText("0");
				text->SetFont(font);

			}
			canvas5->AddChild(subPanel2);

			GameObject::Sptr subPanel3 = scene->CreateGameObject("Button9");
			{
				RectTransform::Sptr transform = subPanel3->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 600.0f));

				GuiPanel::Sptr panel = subPanel3->Add<GuiPanel>();
				panel->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel3->Add<GuiText>();
				text->SetText("Exit Game");
				text->SetFont(font);

			}
			canvas5->AddChild(subPanel3);

			GameObject::Sptr subPanel4 = scene->CreateGameObject("Win Title");
			{
				RectTransform::Sptr transform = subPanel4->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 100.0f));

				GuiPanel::Sptr panel = subPanel4->Add<GuiPanel>();
				panel->SetColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel4->Add<GuiText>();
				text->SetText("YOU WIN!");
				text->SetFont(font);

			}
			canvas5->AddChild(subPanel4);
			//uiParent->AddChild(canvas5);
		}

		GameObject::Sptr canvas6 = scene->CreateGameObject("Lose");
		{
			RectTransform::Sptr transform = canvas6->Add<RectTransform>();
			transform->SetMin({ 100, 100 });
			transform->SetMax({ 700, 800 });
			transform->SetPosition(glm::vec2(960.0f, 400.0f));

			GuiPanel::Sptr canPanel = canvas6->Add<GuiPanel>();
			canPanel->SetColor(glm::vec4(0.6f, 0.3f, 0.0f, 1.0f));

			GameObject::Sptr subPanel2 = scene->CreateGameObject("FinalScoreL");
			{
				RectTransform::Sptr transform = subPanel2->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 450.0f));

				GuiPanel::Sptr panel = subPanel2->Add<GuiPanel>();
				panel->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel2->Add<GuiText>();
				text->SetText("0");
				text->SetFont(font);

			}
			canvas6->AddChild(subPanel2);

			GameObject::Sptr subPanel3 = scene->CreateGameObject("Button10");
			{
				RectTransform::Sptr transform = subPanel3->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 600.0f));

				GuiPanel::Sptr panel = subPanel3->Add<GuiPanel>();
				panel->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel3->Add<GuiText>();
				text->SetText("Exit Game");
				text->SetFont(font);

			}
			canvas6->AddChild(subPanel3);

			GameObject::Sptr subPanel4 = scene->CreateGameObject("Win Title");
			{
				RectTransform::Sptr transform = subPanel4->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 100.0f));

				GuiPanel::Sptr panel = subPanel4->Add<GuiPanel>();
				panel->SetColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel4->Add<GuiText>();
				text->SetText("GAME OVER!");
				text->SetFont(font);

			}
			canvas6->AddChild(subPanel4);
			//uiParent->AddChild(canvas6);
		}

		GameObject::Sptr canvas7 = scene->CreateGameObject("Highscores Menu");
		{
			RectTransform::Sptr transform = canvas7->Add<RectTransform>();
			transform->SetMin({ 100, 100 });
			transform->SetMax({ 700, 800 });
			transform->SetPosition(glm::vec2(960.0f, 400.0f));

			GuiPanel::Sptr canPanel = canvas7->Add<GuiPanel>();
			canPanel->SetColor(glm::vec4(0.6f, 0.3f, 0.0f, 1.0f));

			GameObject::Sptr subPanel1 = scene->CreateGameObject("Button11");
			{
				RectTransform::Sptr transform = subPanel1->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 600.0f));

				GuiPanel::Sptr panel = subPanel1->Add<GuiPanel>();
				panel->SetColor(glm::vec4(0.3f, 0.15f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel1->Add<GuiText>();
				text->SetText("Back");
				text->SetFont(font);

			}
			canvas7->AddChild(subPanel1);

			GameObject::Sptr subPanel2 = scene->CreateGameObject("Highscores Title");
			{
				RectTransform::Sptr transform = subPanel2->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 128 });
				transform->SetPosition(glm::vec2(300.0f, 100.0f));

				GuiPanel::Sptr panel = subPanel2->Add<GuiPanel>();
				panel->SetColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel2->Add<GuiText>();
				text->SetText("Highscores");
				text->SetFont(font);

			}
			canvas7->AddChild(subPanel2);

			GameObject::Sptr subPanel3 = scene->CreateGameObject("Score Display");
			{
				RectTransform::Sptr transform = subPanel3->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 590, 354 });
				transform->SetPosition(glm::vec2(300.0f, 350.0f));

				GuiPanel::Sptr panel = subPanel3->Add<GuiPanel>();
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 36.0f);
				font->Bake();

				GuiText::Sptr text = subPanel3->Add<GuiText>();
				text->SetText("");
				text->SetFont(font);

			}
			canvas7->AddChild(subPanel3);
			//uiParent->AddChild(canvas2);
		}

		
#pragma endregion

#pragma region Commented Defaults
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
		}


		GameObject::Sptr ship = scene->CreateGameObject("Fenrir");
		{
			// Set position in the scene
			ship->SetPostion(glm::vec3(1.5f, 0.0f, 4.0f));
			ship->SetScale(glm::vec3(0.1f));

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = ship->Add<RenderComponent>();
			renderer->SetMesh(shipMesh);
			renderer->SetMaterial(monkeyMaterial);
		}

		GameObject::Sptr demoBase = scene->CreateGameObject("Demo Parent");

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
		/*
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

				monkey1->Get<JumpBehaviour>()->Panel = text;
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
