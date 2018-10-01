#pragma once

#ifndef ObjectHeader
#define ObjectHeader 

#include "Common.h"

extern std::vector<std::vector<sf::Texture*>> objectsTextures[3];
extern int delayTimeBetweenChange;

enum class ObjectType { Obstacle, Equipment, Enemy };
enum class ObstacleSubType { GreenObstacle, BlueObstacle, GrayObstacle, BrownObstacle, WhiteObstacle, OrangeObstacle };
enum class EnemySubType { FlyEnemy1, FlyEnemy2, FlyEnemy3, FlyEnemy4, BlackHoleEnemy, FlyingSaucerEnemy, NormalEnemy1, NormalEnemy2, NormalEnemy3 };
enum class EquipmentSybType { SpringEquipment, TrampolineEquipment, PropellerEquipment, RacketEquipment, ShieldEquipment, SpringShoesEquipment };

class Object {
private:
	bool visible = true, dependency = false, alive = true;	
public:
	ObjectType type;
	int subType;
	Position pos, initialpos;
	Object(ObjectType t, int sub) {
		type = t;
		subType = sub;
	}
	Object(ObjectType t, int sub, Position pos) : Object(t, sub) {
		this->pos = pos;
	}
	int direction = 0, typeState = 0, timetolive = 0;
	int heart = 1; // use for Enemy to determine after how much shot they will die
	bool hasGravity = false;
	bool hasDependency() const { return dependency; }
	void setDependency(bool x) { dependency = x; }
	bool isVisible() const { return visible; }
	bool isAlive() const { return alive; }
	void setVisible(bool x) { visible = x; }
	void setAlive(bool x) { alive = x; }
	int getStateCounts()const {
		return objectsTextures[(int)type][subType].size();
	}
	sf::Rect<float> getGlobalBounds() const {
		auto textureSize = objectsTextures[(int)type][subType][typeState / delayTimeBetweenChange]->copyToImage().getSize();
		return { pos.x, pos.y, static_cast<float>(textureSize.x), static_cast<float>(textureSize.y) };
	}
	static sf::Vector2u getSize(ObjectType type, int subType, int typeState = 0) {
		return objectsTextures[int(type)][subType][0]->copyToImage().getSize();
	}
	static sf::Vector2u getSize(ObjectType type, EquipmentSybType subType, int typeState = 0) {
		return objectsTextures[int(type)][int(subType)][0]->copyToImage().getSize();
	}
	static sf::Vector2u getSize(ObjectType type, ObstacleSubType subType, int typeState = 0) {
		return objectsTextures[int(type)][int(subType)][0]->copyToImage().getSize();
	}
	static sf::Vector2u getSize(ObjectType type, EnemySubType subType, int typeState = 0) {
		return objectsTextures[int(type)][int(subType)][0]->copyToImage().getSize();
	}
	static const sf::Texture& getTexture(ObjectType type, int subType, int typeState = 0) {
		return *objectsTextures[int(type)][subType][typeState];
	}
	static const sf::Texture& getTexture(ObjectType type, EquipmentSybType subType, int typeState = 0) {
		return *objectsTextures[int(type)][int(subType)][typeState];
	}
	static const sf::Texture& getTexture(ObjectType type, ObstacleSubType subType, int typeState = 0) {
		return *objectsTextures[int(type)][int(subType)][typeState];
	}
	static const sf::Texture& getTexture(ObjectType type, EnemySubType subType, int typeState = 0) {
		return *objectsTextures[int(type)][int(subType)][typeState];
	}
	sf::Texture* getTexture() const {
		return objectsTextures[int(type)][subType][typeState / delayTimeBetweenChange];
	}
	bool operator==(const Object& obj) {
		return obj.type == type && obj.subType == subType && obj.pos == pos && obj.initialpos == initialpos &&
			obj.direction == direction && obj.typeState == typeState && obj.timetolive == timetolive && obj.heart == heart &&
			obj.alive == alive && obj.dependency == dependency && obj.visible == visible && obj.hasGravity == hasGravity;
	}
};

struct Shot {
	Position pos;
	int directionX, directionY;
};

#endif // !ObjectHeader
