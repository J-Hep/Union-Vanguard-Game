#pragma once
#include "Application/ApplicationLayer.h"
#include "json.hpp"
#include "ToneFire.h"
#include "Gameplay/Scene.h"


/**
 * This example layer handles creating a default test scene, which we will use 
 * as an entry point for creating a sample scene
 */
class DefaultSceneLayer final : public ApplicationLayer {
public:
	MAKE_PTRS(DefaultSceneLayer)

	DefaultSceneLayer();
	virtual ~DefaultSceneLayer();

	// Inherited from ApplicationLayer

	virtual void OnAppLoad(const nlohmann::json& config) override;
	void OnUpdate() override;


protected:
	void _CreateScene();

	//shooting variables
	bool canShoot = false;
	bool charging = false;
	float shootPower = 0.f;
	float powerLevel = 0.f;
	float shootTimer = 0.f;
	float shootTime = 1.75f;

	//enemy spawn variables
	float enemySpawnTimer = 0.f;
	float newSpawnDelay = 20.f;
	int	enemyInstances = 1;
	int enemySpawnCount = 0;

	Gameplay::GameObject::Sptr enemiesGroup[20];

	bool sPressed = false;
	bool isPaused = false;
	bool start = false;

	bool playGoblinSound = false;
	bool playGrowlSound = false;

	//Current scene of application
	Gameplay::Scene::Sptr currScene;

};