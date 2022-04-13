#include "Gameplay/Components/EnemyMovement.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

void EnemyMovement::Awake()
{
	_body = GetComponent<Gameplay::Physics::RigidBody>();
	if (_body == nullptr) {
		std::cout << "ENemy is not enabled\n";
		IsEnabled = false;
	}
	_isSpawned = false;
	//initPos = _gameObject->GetPosition();
}

void EnemyMovement::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Speed", &_moveSpeed, 1.0f);
	LABEL_LEFT(ImGui::DragFloat, "Damage", &_damage, 1.0f);
}

nlohmann::json EnemyMovement::ToJson() const {
	return {
		{ "move speed", _moveSpeed },
		{ "damage", _damage }
	};
}

EnemyMovement::EnemyMovement() :
	IComponent(),
	_moveSpeed(5.f),
	_damage(10.f)
{ }

EnemyMovement::~EnemyMovement() = default;

EnemyMovement::Sptr EnemyMovement::FromJson(const nlohmann::json& blob) {
	EnemyMovement::Sptr result = std::make_shared<EnemyMovement>();
	result->_moveSpeed = blob["move speed"];
	result->_damage = blob["damage"];
	return result;
}

void EnemyMovement::Update(float deltaTime) {
	if (_isSpawned == true) {
		//get position, move towards origin
		glm::vec3 direction = -GetGameObject()->GetPosition();
		//Enemy spawn locations
		glm::vec3 pathPositions[] = {glm::vec3(40.f,0.f,0.f),glm::vec3(-40.f,0.f,0.f),glm::vec3(0.f,40.f,0.f),glm::vec3(0.f,-40.f,0.f)};



		direction = glm::normalize(direction);
		_body->SetLinearVelocity(direction);
	}
}

void EnemyMovement::OnEnteredTrigger(const std::shared_ptr<Gameplay::Physics::TriggerVolume>& trigger) {
	
}

void EnemyMovement::OnTriggerVolumeEntered(const std::shared_ptr<Gameplay::Physics::RigidBody>& body) {
	//increase score here
	std::cout << "GOBLIN DIEDED!!!\n";
	_isSpawned = false;
	//_gameObject->SetPostion(initPos);
}


void EnemyMovement::setGameObject(Gameplay::GameObject::Sptr object) {
	_gameObject = object;
}

void EnemyMovement::setIsSpawned(bool value) {
	_isSpawned = value;
}

bool EnemyMovement::getIsSpawned() {
	return _isSpawned;
}