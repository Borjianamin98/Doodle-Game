#pragma once

#ifndef DoodleHeader
#define DoodleHeader
#include "Common.h"
#include <cmath>

enum class PlayerState { Left, LeftSit, Right, RightSit, Up, UpSit };

extern std::vector<std::vector<sf::Texture*>> objectsTextures[3];
extern int frameRate, delayTimeBetweenChange;
extern sf::Sound springSound, springShoesSound, trampolineSound, propellerSound, racketSound;

sf::Texture* doodlesTexture[6];
sf::Sprite doodleNoseSprite;

struct EquipmentParameter
{
	EquipmentSybType type;
	int typeState = 0;
	int timeToLive = 0;
	int heart = 0;
	const sf::Texture& getTexture() const {
		return Object::getTexture(ObjectType::Equipment, type, typeState / delayTimeBetweenChange);
	}
	sf::Vector2u getSize() const {
		return Object::getSize(ObjectType::Equipment, type, typeState / delayTimeBetweenChange);
	}
};

class Player {
private:
	sf::Sprite playerSprite;
	PlayerState state = PlayerState::Left;
	float acceleration = defaultAcceleration, velocityInitial = defaultVelocity, yInitial, time = 0.0;
	float angle = 0;
	bool shotEnabled = true;
	bool invertedGravity = false;
	std::deque<EquipmentParameter> equipmentsDeque;
	int starsState = 0; // When doodle lose game
	bool existSpringShoeEquipment = false;
public:
	Player() {
		playerSprite.setTexture(*doodlesTexture[(int)state]);
	}
	static const int defaultAcceleration = 1500, defaultVelocity = -900;
	float getWidth() { return playerSprite.getGlobalBounds().width; }
	float getHeight() { return playerSprite.getGlobalBounds().height; }
	const sf::Vector2f& getPosition() {	
		return playerSprite.getPosition();
	}
	void setPosition(float x, float y) { 	
		playerSprite.setPosition(x, y);
	}
	void setPostionX(float x) {
		playerSprite.setPosition(x, playerSprite.getPosition().y);
	}
	void setPostionY(float y) {
		playerSprite.setPosition(playerSprite.getPosition().x, y);
	}
	void increamentPostionX(float x) { 
		playerSprite.setPosition({ playerSprite.getPosition().x + x, playerSprite.getPosition().y });
	}
	void increamentPostionY(float y) { 
		playerSprite.setPosition({ playerSprite.getPosition().x, playerSprite.getPosition().y + y });
	}
	bool landingOn(const sf::Rect<float>& rect) {
		auto springShoeTextureHeight = objectsTextures[(int)ObjectType::Equipment][(int)EquipmentSybType::SpringShoesEquipment][1]->copyToImage().getSize().y;
		sf::Rect<float> footRect;
		if (existSpringShoeEquipment)
			footRect = { getPosition().x + 25, getPosition().y + 79, 40, getHeight() + springShoeTextureHeight - 79};
		else
			footRect = { getPosition().x + 25, getPosition().y + 79, 40, playerSprite.getGlobalBounds().height - 79 };
		return footRect.intersects(rect);
	}
	void setyInitial(float x) { yInitial = x; }
	float getyInitial() { return yInitial; }
	void setTime(float x) { time = x; }
	float getTime() { return time; }
	void increamentTime(int x) { time += x; }
	void setVelocityInitial(float x) { velocityInitial = x; }
	float getAcceleration() { return acceleration; }
	void setAcceleration(float x) { acceleration = x; }
	void setInvertedGravity(bool x) { invertedGravity = x; }
	bool gettInvertedGravity() { return invertedGravity; }
	bool canShoting() { return shotEnabled; }
	void setState(PlayerState state) { 
		if (state != this->state) {
			inverseRotation();
		}
		this->state = state;
		playerSprite.setTexture(*doodlesTexture[static_cast<int>(state)]);	
	}
	const PlayerState& getState() { return state;}
	int getStarsState() { return starsState; }
	void setStarsState(int x) { starsState = x; }
	bool hasEquipment() { return !equipmentsDeque.empty(); }

	/*const sf::Transform& getRotationTransform() {
		sf::Transform transformation{};
		auto currentTextureSize = sf::Vector2f((*doodlesTexture[(int)state]).copyToImage().getSize());
		transformation.rotate(angle);
		return transformation;
	}*/

	void setRotation(float x) {
		angle = x;		
	}
	void increaseRotation(float x) {
		angle += x;
	}
	void inverseRotation() {
		angle *= -1;
	}
	float getRotation() { return angle; }
	std::deque<EquipmentParameter>& getEquipment() { return equipmentsDeque; }
	const sf::Sprite getSprite() const { return playerSprite; }
	const sf::Sprite getSprite(PlayerState state) {
		setState(state);
		return playerSprite;
	}

	void makeItNew() {
		state = PlayerState::Left;
		setPosition(0, 0);
		acceleration = defaultAcceleration;
		velocityInitial = defaultVelocity;
		time = 0;
		setRotation(0);
		shotEnabled = true;
		invertedGravity = false;
		equipmentsDeque.clear();
		starsState = 0; // When doodle lose game
		existSpringShoeEquipment = false;
	}

    // Update doodle state and positiOn depend On other features
	void ConvertStateToSit()
	{
		if (state == PlayerState::Left)
			state = PlayerState::LeftSit;
		else if (state == PlayerState::Right)
			state = PlayerState::RightSit;
		else if (state == PlayerState::Up)
			state = PlayerState::UpSit;
	}
	void Update()
	{
		// Update state of doodle
		if (state == PlayerState::LeftSit)
			state = PlayerState::Left;
		else if (state == PlayerState::RightSit)
			state = PlayerState::Right;
		else if (state == PlayerState::UpSit)
			state = PlayerState::Up;

		UpdateEquipmentsDeque();
		UpdateShotEnabled();
		setPostionY(((0.5 * acceleration * pow(time / 1000.0, 2.0)) + (velocityInitial * (time / 1000.0)) + yInitial));
	}
	// return time for go Up (max height) in miliseconds
	double GetMaxHeightTime()
	{
		return abs(velocityInitial) / acceleration * 1000;
	}
	void ResetMovement()
	{
		velocityInitial = defaultVelocity;
		acceleration = defaultAcceleration;
	}
	void UpdateStateAfterLost()
	{
		ResetMovement();
		equipmentsDeque.clear();
		time = 0;
		velocityInitial = -1.5 * defaultVelocity;
		acceleration = 0;
		yInitial = getPosition().y;
	}


	void UpdateEquipmentsDeque()
	{
		if (!equipmentsDeque.empty())
		{
			for (int i = equipmentsDeque.size() - 1; i >= 0; i--)
			{
				auto currentEquipmentTextures = objectsTextures[(int)ObjectType::Equipment][(int)equipmentsDeque[i].type];
				equipmentsDeque[i].timeToLive -= frameRate;

				// Update Equipment
				switch (equipmentsDeque[i].type)
				{
				case EquipmentSybType::SpringEquipment:
				case EquipmentSybType::SpringShoesEquipment:
					// do nothing
					break;
				case EquipmentSybType::TrampolineEquipment:
					increaseRotation(10);
					break;
				case EquipmentSybType::PropellerEquipment:
					equipmentsDeque[i].typeState += delayTimeBetweenChange;
					if (equipmentsDeque[i].typeState == currentEquipmentTextures.size() * delayTimeBetweenChange)
						equipmentsDeque[i].typeState = delayTimeBetweenChange; // first state of propeller
					break;
				case EquipmentSybType::RacketEquipment:
					equipmentsDeque[i].typeState += delayTimeBetweenChange;
					if (equipmentsDeque[i].typeState == currentEquipmentTextures.size() * delayTimeBetweenChange)
						equipmentsDeque[i].typeState = delayTimeBetweenChange; // first state of racket
					break;
				case EquipmentSybType::ShieldEquipment:
					if (equipmentsDeque[i].timeToLive > 1500)
						equipmentsDeque[i].typeState = delayTimeBetweenChange;
					else
					{
						equipmentsDeque[i].typeState += delayTimeBetweenChange;
						if (equipmentsDeque[i].typeState == currentEquipmentTextures.size() * delayTimeBetweenChange)
							equipmentsDeque[i].typeState = delayTimeBetweenChange; // first state of racket
					}
					break;
				}

				// Remove finished Equipments
				if (equipmentsDeque[i].timeToLive <= 0) // It must removed
				{
					switch (equipmentsDeque[i].type)
					{
					case EquipmentSybType::SpringEquipment:
					case EquipmentSybType::TrampolineEquipment:
						if (!ExistEquipment(EquipmentSybType::PropellerEquipment) &&
							!ExistEquipment(EquipmentSybType::RacketEquipment)) // Propeler or Racket must continue to work
						{
							yInitial = getPosition().y;
							velocityInitial = 0;
							time = 0;
							acceleration = defaultAcceleration;
							if (equipmentsDeque[i].type == EquipmentSybType::TrampolineEquipment)
								setRotation(0);
						}
						break;
					case EquipmentSybType::PropellerEquipment:
					case EquipmentSybType::RacketEquipment:
						yInitial = getPosition().y;
						time = 0;
						acceleration = defaultAcceleration;
						break;
					case EquipmentSybType::ShieldEquipment:
						// do nothing
						break;
					case EquipmentSybType::SpringShoesEquipment:
						existSpringShoeEquipment = false;
						break;
					}
					if (equipmentsDeque[i].heart == 0 || equipmentsDeque[i].heart == 1)
						equipmentsDeque.erase(equipmentsDeque.begin() + i);
					continue;
				}

			}
		}
	}
	
	void AddEquipment(EquipmentSybType newEquipment)
	{
		if (!equipmentsDeque.empty())
		{
			for (int i = equipmentsDeque.size() - 1; i >= 0; i--)
			{
				switch (equipmentsDeque[i].type)
				{
				case EquipmentSybType::PropellerEquipment: // if there is a propeller
					if (newEquipment == EquipmentSybType::SpringEquipment || newEquipment == EquipmentSybType::TrampolineEquipment ||
						newEquipment == EquipmentSybType::PropellerEquipment || newEquipment == EquipmentSybType::RacketEquipment)
						return;
					break;
				case EquipmentSybType::RacketEquipment:
					if (newEquipment == EquipmentSybType::SpringEquipment || newEquipment == EquipmentSybType::TrampolineEquipment ||
						newEquipment == EquipmentSybType::PropellerEquipment || newEquipment == EquipmentSybType::RacketEquipment)
						return;
					break;
				case EquipmentSybType::ShieldEquipment:
					if (newEquipment == EquipmentSybType::ShieldEquipment) // remove this and replace new One
						equipmentsDeque.erase(equipmentsDeque.begin() + i);
					break;
				case EquipmentSybType::SpringEquipment:
					if (newEquipment == EquipmentSybType::SpringEquipment) // remove this and replace new One
						equipmentsDeque.erase(equipmentsDeque.begin() + i);
					break;
				case EquipmentSybType::TrampolineEquipment:
					if (newEquipment != EquipmentSybType::ShieldEquipment) // remove this because we must avoid angle
					{
						setRotation(0);
						equipmentsDeque.erase(equipmentsDeque.begin() + i);
					}
					break;
				case EquipmentSybType::SpringShoesEquipment:
					if (newEquipment == EquipmentSybType::SpringEquipment || newEquipment == EquipmentSybType::TrampolineEquipment ||
						newEquipment == EquipmentSybType::RacketEquipment || newEquipment == EquipmentSybType::PropellerEquipment)
						return;
					if (newEquipment == EquipmentSybType::SpringShoesEquipment) // remove this and replace new One
						equipmentsDeque.erase(equipmentsDeque.begin() + i);
					break;
				}
			}
		}

		EquipmentParameter newEquipmentFeature;
		newEquipmentFeature.type = newEquipment;
		switch (newEquipment)
		{
		case EquipmentSybType::SpringEquipment:
			ResetMovement();
			playSound(springSound);
			velocityInitial = defaultVelocity * 1.5;
			newEquipmentFeature.timeToLive = GetMaxHeightTime();
			break;
		case EquipmentSybType::SpringShoesEquipment:
			ResetMovement();
			playSound(springShoesSound);
			newEquipmentFeature.heart = 5;
			velocityInitial = defaultVelocity * 1.5;
			newEquipmentFeature.timeToLive = GetMaxHeightTime();
			existSpringShoeEquipment = true;
			break;
		case EquipmentSybType::TrampolineEquipment:
			ResetMovement();
			playSound(trampolineSound);
			velocityInitial = defaultVelocity * 2;
			newEquipmentFeature.timeToLive = GetMaxHeightTime();
			newEquipmentFeature.typeState = delayTimeBetweenChange;
			break;
		case EquipmentSybType::PropellerEquipment:
			ResetMovement();
			playSound(propellerSound);
			velocityInitial = defaultVelocity;
			acceleration = 0;
			newEquipmentFeature.timeToLive = 3000;
			break;
		case EquipmentSybType::RacketEquipment:
			ResetMovement();
			playSound(racketSound);
			acceleration = 0;
			velocityInitial = defaultVelocity * 1.5;
			newEquipmentFeature.timeToLive = 2000;
			break;
		case EquipmentSybType::ShieldEquipment:
			newEquipmentFeature.timeToLive = 3000;
			break;
		}
		equipmentsDeque.push_back(newEquipmentFeature);
		UpdateShotEnabled();
	}
	
	
	void UpdateShotEnabled()
	{
		if (!equipmentsDeque.empty())
		{
			for (int i = equipmentsDeque.size() - 1; i >= 0; i--)
			{
				if (equipmentsDeque[i].type == EquipmentSybType::TrampolineEquipment || equipmentsDeque[i].type == EquipmentSybType::RacketEquipment
					|| equipmentsDeque[i].type == EquipmentSybType::PropellerEquipment)
				{
					shotEnabled = false;
					return;
				}
			}
			shotEnabled = true;
		}
		else
			shotEnabled = true;
	}
	bool ExistEquipment(EquipmentSybType subtype)
	{
		if (!equipmentsDeque.empty())
		{
			for (int i = equipmentsDeque.size() - 1; i >= 0; i--)
			{
				if (equipmentsDeque[i].type == subtype)
					return true;
			}
			return false;
		}
		else
			return false;
	}
	EquipmentParameter* GetEquipment(EquipmentSybType subtype)
	{
		for (auto& equipment : equipmentsDeque)
		{
			if (equipment.type == subtype)
				return &equipment;
		}
		return nullptr;
	}
};

#endif // !DoodleHeader

