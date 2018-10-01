#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <deque>
#include <vector>
#include <algorithm>
#include <string>
#include "Common.h"
#include "Object.h"
#include "Doodle.h"
#include "FileHandler.h"

using namespace std;

enum class Page { MenuPage, PlayPage, TryAgainPage, ScorePage };
enum class PlayState { Play, Pause, Lost };
enum ButtonState { Off, On };


// Play Page function
void UpdateObjectsFeatures();
void UpdateObjectsDeque();
void CheckShots();
void LostStateMain();
void PlayStateMain();
void PlayMain();
void DrawObjectsInScreen();
void ScoresMain();
void TryAgainMain();
void LoadAssets();
void UpdateSettings();
void LoadSettings();
Object InitializeNewObject(ObjectType objType, int objSubType, float objX, float objY);

// Page Sprite
sf::Sprite backgroundSprite, menuPageSprite, scorePageSprite, pausePageSprite, gameOverSprite;
sf::Texture* resumeButtonTexture[2], *menuButtonTexture[2], *playAgainButtonTexture[2]; // 0 for Off state and 1 for On state
sf::Texture* soundButtonTexture[2], *playButtonTexture[2], *scoresButtonTexture[2]; // 0 for Off state and 1 for On state
sf::Sprite resumeButtonSprite, menuButtonSprite, playAgainButtonSprite; // 0 for Off state and 1 for On state
sf::Sprite soundButtonSprite, playButtonSprite, scoresButtonSprite; // 0 for Off state and 1 for On state

// Object Sprite
vector<vector<sf::Texture*>> objectsTextures[3];
sf::Texture shotTexture;
sf::Sprite topScoreSprite;
sf::Sprite pauseButtonSprite;
sf::Sprite starsSprite[3];
Player* doodlePlayer;
deque<pair<ObjectType, int>> randomObjectDeque; // a list of pairs of random Objects with specific Type and subtype

// Sounds
sf::Sound jumpSound, jumpOnMonsterSound, dieSound, propellerSound, racketSound, shotSound, springSound;
sf::Sound springShoesSound, trampolineSound, brokenSound, explodingPlatformSound;
sf::Music ufoMusic, monsterMusic;

// Fonts
sf::Sprite fontSprite;
sf::Font defaultfont;

Page currentFrame = Page::MenuPage;
PlayState playState;

int screenWidth = 640, screenHeight = 1024, distanceFromTop = 300;
bool firstPlay = true, firstSeenMenu = true; // for check and if true, initialize our objects deque for Playing
bool soundEnabled = true;
long long int records[10] = { 0 }, score = 0;
int frameRate = 30, delayTimeBetweenChange = 200;
int initialDistanceBetweenObstacle = 20, distanceBetweenObstacle = 20, maxVerticalMovement = 80, maxHorizontalMovement = 5;
int timeBetweenShots = 150, currentReminderBetweenShot = 150, shotSpeed = 50;
int screenBorder = 400;
sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Doodle");

deque<Object> objectsDeque;
deque<Shot> shotsDeque;

int main()
{
	LoadAssets();
	srand(time(nullptr));
	doodlePlayer = new Player{};
	Position playButtonPos = { 100, 300 };
	soundButtonSprite.setPosition(screenWidth - 120, screenHeight - 110);
	scoresButtonSprite.setPosition(screenWidth - 250, screenHeight - 110);
	playButtonSprite.setPosition(playButtonPos.x, playButtonPos.y);

	LoadSettings();
	
	sf::Clock clock;
	while (window.isOpen())
	{
		window.clear();
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		if (currentFrame == Page::MenuPage)
		{
			auto greenObstacleTexture = Object::getTexture(ObjectType::Obstacle, ObstacleSubType::GreenObstacle);
			sf::Sprite greenObstacleSprite;
			greenObstacleSprite.setTexture(greenObstacleTexture);
			Position greenObstaclePos = { 100, 850 };
			sf::Rect<float> greenObstacleRect{ greenObstaclePos.x, greenObstaclePos.y, 120, 30 };		
			if (firstSeenMenu)
			{
				doodlePlayer->makeItNew(); // make empty
				doodlePlayer->setPostionX(greenObstaclePos.x + greenObstacleSprite.getGlobalBounds().width / 2 - doodlePlayer->getWidth() / 2);
				doodlePlayer->setyInitial(greenObstaclePos.y - doodlePlayer->getHeight()); // default position of Green Obstacle in menu Page
				firstSeenMenu = false;
			}

			window.draw(menuPageSprite);

			// sound button
			if (soundEnabled)
				soundButtonSprite.setTexture(*soundButtonTexture[Off]);
			else
				soundButtonSprite.setTexture(*soundButtonTexture[On]);
			window.draw(soundButtonSprite);
			if (MouseOverSomething(window, soundButtonSprite) && sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				soundEnabled = !soundEnabled;
				UpdateSettings();
			}

			// scores button
			if (MouseOverSomething(window, scoresButtonSprite))
			{
				scoresButtonSprite.setTexture(*scoresButtonTexture[On]);
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
					currentFrame = Page::ScorePage;
			}
			else
				scoresButtonSprite.setTexture(*scoresButtonTexture[Off]);
			window.draw(scoresButtonSprite);

			// Play button
			if (MouseOverSomething(window, playButtonSprite))
			{
				playButtonSprite.setTexture(*playButtonTexture[On]);
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					firstPlay = true;
					currentFrame = Page::PlayPage;
				}
			}
			else
				playButtonSprite.setTexture(*playButtonTexture[Off]);
			window.draw(playButtonSprite);

			doodlePlayer->Update();
			if (doodlePlayer->landingOn(greenObstacleRect))
			{
				doodlePlayer->ResetMovement();
				doodlePlayer->setTime(0);
				doodlePlayer->setyInitial(greenObstaclePos.y - doodlePlayer->getHeight());
				doodlePlayer->setPostionY(doodlePlayer->getyInitial());
				playSound(jumpSound);
			}
			greenObstacleSprite.setPosition(greenObstaclePos.x, greenObstaclePos.y);
			window.draw(greenObstacleSprite);

			window.draw(doodlePlayer->getSprite(PlayerState::Right));
			doodlePlayer->increamentTime(frameRate);
		}
		else if (currentFrame == Page::PlayPage)
			PlayMain();
		else if (currentFrame == Page::TryAgainPage)
			TryAgainMain();
		else if (currentFrame == Page::ScorePage)
			ScoresMain();

		while (clock.getElapsedTime().asMilliseconds() <= frameRate);
		clock.restart();
		window.display();
	}

	return 0;
}

void LoadAssets()
{
	if (!defaultfont.loadFromFile("assets/doodlejumpbold.ttf"))
		throw runtime_error{ "file couldn't load" };

	vector<sf::Texture*> subVector;

	// Sounds
	jumpSound.setBuffer(*loadSoundFromFile("assets/sounds/jump.ogg"));
	jumpOnMonsterSound.setBuffer(*loadSoundFromFile("assets/sounds/jumpOnMonster.ogg"));
	dieSound.setBuffer(*loadSoundFromFile("assets/sounds/die.ogg"));
	propellerSound.setBuffer(*loadSoundFromFile("assets/sounds/propeller.ogg"));
	racketSound.setBuffer(*loadSoundFromFile("assets/sounds/racket.ogg"));
	shotSound.setBuffer(*loadSoundFromFile("assets/sounds/shot.ogg"));
	springSound.setBuffer(*loadSoundFromFile("assets/sounds/spring.ogg"));
	springShoesSound.setBuffer(*loadSoundFromFile("assets/sounds/springShoes.ogg"));
	trampolineSound.setBuffer(*loadSoundFromFile("assets/sounds/trampoline.ogg"));
	brokenSound.setBuffer(*loadSoundFromFile("assets/sounds/broken.ogg"));
	explodingPlatformSound.setBuffer(*loadSoundFromFile("assets/sounds/explodingPlatform.ogg"));

	// Musics
	monsterMusic.openFromFile("assets/sounds/monster.ogg");
	ufoMusic.openFromFile("assets/sounds/ufo.ogg");

	// Obstacles
	objectsTextures[(int)ObjectType::Obstacle].push_back(LoadOneTextureToVector("assets/Obstacle/GreenObstacle.png"));
	objectsTextures[(int)ObjectType::Obstacle].push_back(LoadOneTextureToVector("assets/Obstacle/BlueObstacle.png"));
	objectsTextures[(int)ObjectType::Obstacle].push_back(LoadOneTextureToVector("assets/Obstacle/GrayObstacle.png"));
	objectsTextures[(int)ObjectType::Obstacle].push_back(LoadTexturesToVector("assets/Obstacle/BrownObstacle", 1, 4));
	objectsTextures[(int)ObjectType::Obstacle].push_back(LoadTexturesToVector("assets/Obstacle/WhiteObstacle", 1, 21));
	objectsTextures[(int)ObjectType::Obstacle].push_back(LoadTexturesToVector("assets/Obstacle/OrangeObstacle", 1, 8));

	// Equipments
	subVector.push_back(loadTextureFromFile("assets/Equipments/spring.png"));
	subVector.push_back(loadTextureFromFile("assets/Equipments/springUp.png"));
	objectsTextures[(int)ObjectType::Equipment].push_back(subVector);
	subVector.clear();

	subVector.push_back(loadTextureFromFile("assets/Equipments/trampoline.png"));
	subVector.push_back(loadTextureFromFile("assets/Equipments/trampolineUp.png"));
	objectsTextures[(int)ObjectType::Equipment].push_back(subVector);
	subVector.clear();

	objectsTextures[(int)ObjectType::Equipment].push_back(LoadTexturesToVector("assets/Equipments/propeller", 1, 4));
	objectsTextures[(int)ObjectType::Equipment].push_back(LoadTexturesToVector("assets/Equipments/racket", 1, 10, true));
	objectsTextures[(int)ObjectType::Equipment].push_back(LoadTexturesToVector("assets/Equipments/shield", 1, 3, true));
	objectsTextures[(int)ObjectType::Equipment].push_back(LoadTexturesToVector("assets/Equipments/springShoe", 1, 1, true));

	// Enemies
	objectsTextures[(int)ObjectType::Enemy].push_back(LoadTexturesToVector("assets/Enemy/flyEnemy1_", 1, 5));
	objectsTextures[(int)ObjectType::Enemy].push_back(LoadTexturesToVector("assets/Enemy/flyEnemy2_", 1, 2));
	objectsTextures[(int)ObjectType::Enemy].push_back(LoadOneTextureToVector("assets/Enemy/flyEnemy3.png"));
	objectsTextures[(int)ObjectType::Enemy].push_back(LoadOneTextureToVector("assets/Enemy/flyEnemy4.png"));
	objectsTextures[(int)ObjectType::Enemy].push_back(LoadOneTextureToVector("assets/Enemy/blackhole.png"));
	objectsTextures[(int)ObjectType::Enemy].push_back(LoadTexturesToVector("assets/Enemy/flyingsaucer", 1, 2));
	objectsTextures[(int)ObjectType::Enemy].push_back(LoadOneTextureToVector("assets/Enemy/normalEnemy1.png"));
	objectsTextures[(int)ObjectType::Enemy].push_back(LoadTexturesToVector("assets/Enemy/normalEnemy2_", 1, 2));
	objectsTextures[(int)ObjectType::Enemy].push_back(LoadTexturesToVector("assets/Enemy/normalEnemy3_", 1, 2));

	subVector = LoadTexturesToVector("assets/Doodle/stars", 1, 3);
	for (size_t i = 0; i < subVector.size(); i++)
		starsSprite[i].setTexture(*subVector[i]);


	subVector.clear();

	// Add buttons
	resumeButtonTexture[0] = loadTextureFromFile("assets/Buttons/resume.png");
	resumeButtonTexture[1] = loadTextureFromFile("assets/Buttons/resumeOn.png");
	menuButtonTexture[0] = loadTextureFromFile("assets/Buttons/menu.png");
	menuButtonTexture[1] = loadTextureFromFile("assets/Buttons/menuOn.png");
	playAgainButtonTexture[0] = loadTextureFromFile("assets/Buttons/Playagain.png");
	playAgainButtonTexture[1] = loadTextureFromFile("assets/Buttons/PlayagainOn.png");
	soundButtonTexture[0] = loadTextureFromFile("assets/Buttons/soundOn.png");
	soundButtonTexture[1] = loadTextureFromFile("assets/Buttons/soundOff.png");
	playButtonTexture[0] = loadTextureFromFile("assets/Buttons/Play.png");
	playButtonTexture[1] = loadTextureFromFile("assets/Buttons/PlayOn.png");
	scoresButtonTexture[0] = loadTextureFromFile("assets/Buttons/scores.png");
	scoresButtonTexture[1] = loadTextureFromFile("assets/Buttons/scoresOn.png");

	doodlesTexture[0] = loadTextureFromFile("assets/Doodle/doodleLeft.png");
	doodlesTexture[1] = loadTextureFromFile("assets/Doodle/doodleLeftS.png");
	doodlesTexture[2] = loadTextureFromFile("assets/Doodle/doodleRight.png");
	doodlesTexture[3] = loadTextureFromFile("assets/Doodle/doodleRightS.png");
	doodlesTexture[4] = loadTextureFromFile("assets/Doodle/doodleUp.png");
	doodlesTexture[5] = loadTextureFromFile("assets/Doodle/doodleUpS.png");
	doodleNoseSprite.setTexture(*loadTextureFromFile("assets/Doodle/doodleNose.png"));

	shotTexture = *loadTextureFromFile("assets/Shot.png");
	topScoreSprite.setTexture(*loadTextureFromFile("assets/TopScore.png"));
	topScoreSprite.setColor(sf::Color(255, 255, 255, 200));
	pauseButtonSprite.setTexture(*loadTextureFromFile("assets/Pause.png"));

	backgroundSprite.setTexture(*loadTextureFromFile("assets/Background.png"));
	menuPageSprite.setTexture(*loadTextureFromFile("assets/MenuBackground.png"));
	pausePageSprite.setTexture(*loadTextureFromFile("assets/PauseFrame.png"));
	pausePageSprite.setColor(sf::Color(255, 255, 255, 250));
	gameOverSprite.setTexture(*loadTextureFromFile("assets/GameOver.png"));
	pausePageSprite.setColor(sf::Color(255, 255, 255, 250));
	scorePageSprite.setTexture(*loadTextureFromFile("assets/ScoreFrame.png"));
}

////////////////////////////////////////////////////////////////////////////// Playing

void PlayMain()
{
	if (firstPlay)
	{
		objectsDeque.clear();
		auto greenObstacleTexture = Object::getTexture(ObjectType::Obstacle, ObstacleSubType::GreenObstacle);
		auto greenObstacleSize = Object::getSize(ObjectType::Obstacle, ObstacleSubType::GreenObstacle);
		for (size_t i = 0; i * (greenObstacleSize.y + initialDistanceBetweenObstacle) < screenHeight; i++)
		{
			Object temp{ ObjectType::Obstacle, (int)ObstacleSubType::GreenObstacle,{ static_cast<float>(randomPosition(greenObstacleTexture)) , i * (static_cast<float>(greenObstacleSize.y) + initialDistanceBetweenObstacle) } };
			objectsDeque.push_back(temp);
		}
		playState = PlayState::Play;
		doodlePlayer->makeItNew();
		doodlePlayer->setyInitial(objectsDeque.back().pos.y - doodlePlayer->getHeight()); //accroding to windows coordinate system
		doodlePlayer->setPosition(objectsDeque.back().pos.x, doodlePlayer->getyInitial());
		score = 0;
		firstPlay = false;
	}

	switch (playState)
	{
	case PlayState::Play:
		PlayStateMain();
		doodlePlayer->increamentTime(frameRate);
		if (MouseOverSomething(window, pauseButtonSprite) && sf::Mouse::isButtonPressed(sf::Mouse::Left))
			playState = PlayState::Pause;
		break;
	case PlayState::Pause:
		window.draw(backgroundSprite);
		DrawObjectsInScreen();
		window.draw(pausePageSprite);
		resumeButtonSprite.setPosition(300, 700);
		if (MouseOverSomething(window, resumeButtonSprite))
		{
			resumeButtonSprite.setTexture(*resumeButtonTexture[On]);
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				playState = PlayState::Play;
		}
		else
			resumeButtonSprite.setTexture(*resumeButtonTexture[Off]);
		window.draw(resumeButtonSprite);

		menuButtonSprite.setPosition(300, 800);
		if (MouseOverSomething(window, menuButtonSprite))
		{
			menuButtonSprite.setTexture(*menuButtonTexture[On]);
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				firstSeenMenu = true;
				firstPlay = true;
				currentFrame = Page::MenuPage;
			}
		}
		else
			menuButtonSprite.setTexture(*menuButtonTexture[Off]);
		window.draw(menuButtonSprite);

		break;
	case PlayState::Lost:
		LostStateMain();
		doodlePlayer->increamentTime(frameRate);
		break;
	}
	topScoreSprite.setPosition(0, 0);
	window.draw(topScoreSprite);
	sf::Text text{ to_string(score), defaultfont };
	text.setFillColor(sf::Color::Black);
	text.setCharacterSize(40);
	text.setPosition(15, 0);
	window.draw(text);
	pauseButtonSprite.setPosition(screenWidth - pauseButtonSprite.getGlobalBounds().width - 20, 0);
	window.draw(pauseButtonSprite);	
}

void PlayStateMain()
{
	window.draw(backgroundSprite);
	distanceBetweenObstacle = initialDistanceBetweenObstacle + score / 400;
	doodlePlayer->Update();
	if (doodlePlayer->getPosition().y + doodlePlayer->getHeight() > screenHeight)
	{
		doodlePlayer->UpdateStateAfterLost();
		playState = PlayState::Lost;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		doodlePlayer->setState(PlayerState::Right);
		doodlePlayer->increamentPostionX(10);
		if (doodlePlayer->getPosition().x + doodlePlayer->getWidth() / 2 > screenWidth)
			doodlePlayer->setPostionX(-1 * (doodlePlayer->getWidth() / 2));
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		doodlePlayer->setState(PlayerState::Left);
		doodlePlayer->increamentPostionX(-10);
		if (doodlePlayer->getPosition().x + doodlePlayer->getWidth() / 2 <= 0)
			doodlePlayer->setPostionX(screenWidth - (doodlePlayer->getWidth() / 2));
	}

	EquipmentParameter *existSpringShoeEquipment = doodlePlayer->GetEquipment(EquipmentSybType::SpringShoesEquipment);
	for (auto& object : objectsDeque)
	{
		sf::Rect<float> platformRect = object.getGlobalBounds();
		bool condition = doodlePlayer->landingOn(platformRect);
		if (object.isVisible() && object.isAlive() && condition) // if there is any intersection between rect of doodle and sth
		{
			switch (object.type)
			{
			case ObjectType::Obstacle:
				if (doodlePlayer->getAcceleration() != 0 && doodlePlayer->getTime() > doodlePlayer->GetMaxHeightTime()) // If there is any intersection between frame of doodle and something
				{
					doodlePlayer->ConvertStateToSit();
					if (object.subType == (int)ObstacleSubType::BrownObstacle || object.subType == (int)ObstacleSubType::WhiteObstacle)
					{
						if (object.subType == (int)ObstacleSubType::BrownObstacle)
							playSound(brokenSound);
						object.setAlive(false);
					}
					if (object.subType != (int)ObstacleSubType::BrownObstacle)
					{
						if (existSpringShoeEquipment != nullptr)
						{
							playSound(springShoesSound);
							existSpringShoeEquipment->heart--;
							if (existSpringShoeEquipment->heart == 0) // If it is finished, jump normally!!
								doodlePlayer->ResetMovement();
						}
						else
						{
							doodlePlayer->ResetMovement();
							playSound(jumpSound);
						}
						doodlePlayer->setTime(0);
						if (existSpringShoeEquipment != nullptr && existSpringShoeEquipment->heart != 0 && existSpringShoeEquipment->heart != 1)
							doodlePlayer->setyInitial(object.pos.y - doodlePlayer->getHeight() - Object::getSize(ObjectType::Equipment, EquipmentSybType::SpringShoesEquipment).y);
						else
							doodlePlayer->setyInitial(object.pos.y - doodlePlayer->getHeight());

						doodlePlayer->setPostionY(doodlePlayer->getyInitial());
					}
				}
				break;

			case ObjectType::Equipment:
				if (doodlePlayer->landingOn(platformRect) && doodlePlayer->getAcceleration() != 0 && doodlePlayer->getTime() > doodlePlayer->GetMaxHeightTime())
				{
					if (object.subType == (int)EquipmentSybType::TrampolineEquipment || object.subType == (int)EquipmentSybType::SpringEquipment || object.subType == (int)EquipmentSybType::SpringShoesEquipment)
					{
						object.setAlive(false);
						doodlePlayer->setTime(0);
						doodlePlayer->setVelocityInitial(0);
						doodlePlayer->setyInitial(object.pos.y - doodlePlayer->getHeight());
						doodlePlayer->setPostionY(doodlePlayer->getyInitial());
						doodlePlayer->AddEquipment((EquipmentSybType)object.subType);
					}
				}
				else
				{
					switch ((EquipmentSybType)object.subType)
					{
					case EquipmentSybType::PropellerEquipment:
					case EquipmentSybType::RacketEquipment:
					case EquipmentSybType::ShieldEquipment:
						object.setAlive(false);
						doodlePlayer->setTime(0);
						doodlePlayer->setyInitial(doodlePlayer->getPosition().y); // Set this height as initial position
						doodlePlayer->AddEquipment((EquipmentSybType)object.subType);
						break;
					}
				}
				break;

			case ObjectType::Enemy:
				// We choose Only small above part of Enemy				
				if (doodlePlayer->ExistEquipment(EquipmentSybType::ShieldEquipment))
					break; // do nothing
				if (doodlePlayer->ExistEquipment(EquipmentSybType::RacketEquipment) || doodlePlayer->ExistEquipment(EquipmentSybType::PropellerEquipment))
				{
					if (object.subType != (int)EnemySubType::BlackHoleEnemy || object.subType != (int)EnemySubType::FlyingSaucerEnemy)
						objectsDeque.erase(find(objectsDeque.begin(), objectsDeque.end(), object)); // We kill Enemy
					break;
				}

				if (object.subType == (int)EnemySubType::BlackHoleEnemy)
				{
					doodlePlayer->UpdateStateAfterLost();
					playState = PlayState::Lost;
					break;
				}

				if (object.subType == (int)EnemySubType::FlyingSaucerEnemy)
				{
					if (!object.hasGravity)
					{
						object.hasGravity = true;
						doodlePlayer->setTime(0);
						doodlePlayer->setAcceleration(-100);
						doodlePlayer->setVelocityInitial(Player::defaultVelocity / 4);
						doodlePlayer->setyInitial(doodlePlayer->getPosition().y);
						doodlePlayer->setInvertedGravity(true);
					}
					if (doodlePlayer->getPosition().y <= object.pos.y + 47) // 47 is height of flyingsaucer
					{
						doodlePlayer->UpdateStateAfterLost();
						playState = PlayState::Lost;
					}
					break;
				}

				platformRect.height /= 3;
				if (doodlePlayer->landingOn(platformRect) && doodlePlayer->getAcceleration() != 0 && doodlePlayer->getTime() > doodlePlayer->GetMaxHeightTime())
				{
					playSound(jumpOnMonsterSound);
					object.setAlive(false);
					object.setVisible(false);
					doodlePlayer->ResetMovement();
					doodlePlayer->setTime(0);
					doodlePlayer->setyInitial(object.pos.y - doodlePlayer->getHeight());
					doodlePlayer->setPostionY(doodlePlayer->getyInitial());
				}
				else // otherwise we lose game
				{
					doodlePlayer->UpdateStateAfterLost();
					playState = PlayState::Lost;
				}

				break;
			}
		}
		else if (object.isVisible() && object.hasGravity)
		{
			doodlePlayer->setVelocityInitial(doodlePlayer->getAcceleration() * (doodlePlayer->getTime() / 1000)); // continue with last speed that it has
			doodlePlayer->setAcceleration(Player::defaultAcceleration);
			doodlePlayer->setyInitial(doodlePlayer->getPosition().y);
			doodlePlayer->setTime(0);
			object.hasGravity = false;
		}
	}



	if (doodlePlayer->getPosition().y < screenBorder)
	{
		for (auto& object : objectsDeque)
		{
			object.pos.y += (screenBorder - doodlePlayer->getPosition().y);
			object.initialpos.y += (screenBorder - doodlePlayer->getPosition().y);
		}

		for (auto& shot : shotsDeque)
			shot.pos.y += (screenBorder - doodlePlayer->getPosition().y);

		score += (screenBorder - doodlePlayer->getPosition().y);
		doodlePlayer->setyInitial(doodlePlayer->getyInitial() + (screenBorder - doodlePlayer->getPosition().y));
	}

	UpdateObjectsFeatures();
	UpdateObjectsDeque();
	CheckShots();

	if (doodlePlayer->canShoting() && sf::Mouse::isButtonPressed(sf::Mouse::Left) && currentReminderBetweenShot <= 0) // Firing
	{
		// Create doodle nose
		float distanceMouseFromDoodle = sf::Mouse::getPosition(window).x - (doodlePlayer->getPosition().x + doodlePlayer->getWidth() / 2);
		double angleDegree = (distanceMouseFromDoodle / screenWidth) * 360;
		double angleRadian;
		if (angleDegree > 30)
			angleDegree = 30;
		else if (angleDegree < -30)
			angleDegree = -30;
		doodlePlayer->setState(PlayerState::Up);
		doodleNoseSprite.setPosition(doodlePlayer->getPosition().x + doodlePlayer->getWidth() / 2 - doodleNoseSprite.getGlobalBounds().width / 2,
			doodlePlayer->getPosition().y + 5);
		window.draw(doodleNoseSprite);


		// Create shots
		angleRadian = angleDegree > 0 ? (90 - angleDegree) : (90 + angleDegree);
		angleRadian = angleRadian * 3.14 / 180;
		Shot temp;
		temp.pos = { doodlePlayer->getPosition().x + doodleNoseSprite.getGlobalBounds().width / 2 - shotTexture.copyToImage().getSize().x / 2, doodlePlayer->getPosition().y };
		temp.pos = rotationMatrix(temp.pos, { temp.pos.x, doodlePlayer->getPosition().y + doodleNoseSprite.getGlobalBounds().height / 2 }, angleDegree);
		temp.pos.y = doodlePlayer->getPosition().y;
		temp.directionX = angleDegree < 0 ? (-1 * shotSpeed * cos(angleRadian)) : (shotSpeed * cos(angleRadian));
		temp.directionY = -1 * shotSpeed * abs(sin(angleRadian));
		shotsDeque.push_back(temp);
		currentReminderBetweenShot = timeBetweenShots;

		playSound(shotSound);
	}
	else
	{
		if (doodlePlayer->getState() == PlayerState::Up)
			doodlePlayer->setState(PlayerState::Left);
		else if (doodlePlayer->getState() == PlayerState::UpSit)
			doodlePlayer->setState(PlayerState::LeftSit);
	}

	DrawObjectsInScreen();
	currentReminderBetweenShot -= frameRate;
}

void LostStateMain()
{
	window.draw(backgroundSprite);

	if (!objectsDeque.empty()) {
		playSound(dieSound);
		doodlePlayer->Update();

		if (doodlePlayer->getPosition().y + doodlePlayer->getHeight() > screenHeight)
		{
			for (auto& object : objectsDeque)
			{
				object.pos.y -= doodlePlayer->getPosition().y + doodlePlayer->getHeight() - screenHeight;
			}
			doodlePlayer->setyInitial(screenHeight - doodlePlayer->getHeight());
			doodlePlayer->setPostionY(screenHeight - doodlePlayer->getHeight());
			doodlePlayer->setTime(0);
		}

		while (!objectsDeque.empty() && objectsDeque.front().pos.y + objectsDeque.front().getGlobalBounds().height < 0)
			objectsDeque.pop_front();

		// Create objects in screen
		for (const auto& object : objectsDeque)
		{
			if (object.isVisible())
			{
				sf::Sprite temp;
				temp.setTexture(*object.getTexture());
				temp.setPosition(object.pos.x, object.pos.y);
				window.draw(temp);
			}
		}
		doodlePlayer->setRotation(0);
		window.draw(doodlePlayer->getSprite());


		doodlePlayer->setStarsState(doodlePlayer->getStarsState() + delayTimeBetweenChange);
		if (doodlePlayer->getStarsState() == 3 * delayTimeBetweenChange)
			doodlePlayer->setStarsState(0);

		// draw Stars
		for (auto& x : starsSprite) {
			x.setPosition(doodlePlayer->getPosition().x + doodlePlayer->getWidth() / 2 - starsSprite[doodlePlayer->getStarsState() / delayTimeBetweenChange].getGlobalBounds().width / 2,
				doodlePlayer->getPosition().y + 50 /* distance from above doodle */ - starsSprite[doodlePlayer->getStarsState() / delayTimeBetweenChange].getGlobalBounds().height);
		}
		window.draw(starsSprite[doodlePlayer->getStarsState() / delayTimeBetweenChange]);
	}
	else
	{
		//SDL::stopAllSounds();
		monsterMusic.stop();
		ufoMusic.stop();
		currentFrame = Page::TryAgainPage;
	}
}

// Check if a shot kill a Enemy or not
void CheckShots()
{
	for (int i = shotsDeque.size() - 1; i >= 0; i--)
	{
		sf::Rect<float> shotRect{ shotsDeque[i].pos.x, shotsDeque[i].pos.y, static_cast<float>(shotTexture.copyToImage().getSize().x), static_cast<float>(shotTexture.copyToImage().getSize().y) };
		for (auto& object : objectsDeque)
		{
			if (object.isAlive() && object.isVisible() && object.type == ObjectType::Enemy)
			{				
				if (object.getGlobalBounds().intersects(shotRect))
				{
					if (object.heart == 1) {
						object.setAlive(false);
						object.setVisible(false);
					}
					else
					{
						object.heart--;
						object.typeState = delayTimeBetweenChange;
					}
					shotsDeque.erase(shotsDeque.begin() + i);
					break;
				}
			}
		}
	}
}

// It used for Update position, state, timetolive, etc of Obstacle such as blue Obstacle, etc,
void UpdateObjectsFeatures()
{
	int positionIncrement = 2, temp;
	for (int i = objectsDeque.size() - 1; i >= 0; i--)
	{
		if (objectsDeque[i].type == ObjectType::Obstacle)
		{
			switch ((ObstacleSubType)objectsDeque[i].subType)
			{
			case ObstacleSubType::GreenObstacle:
				// do Nothing
				break;
			case ObstacleSubType::BlueObstacle:
				temp = objectsDeque[i].pos.x + (objectsDeque[i].direction * positionIncrement);
				if (temp + objectsDeque[i].getGlobalBounds().width >= screenWidth || temp <= 0)
					objectsDeque[i].direction *= -1;

				objectsDeque[i].pos.x += (objectsDeque[i].direction * positionIncrement);
				if (objectsDeque[i].hasDependency())
					objectsDeque[i - 1].pos.x += (objectsDeque[i].direction * positionIncrement);
				break;
			case ObstacleSubType::GrayObstacle:
				temp = objectsDeque[i].pos.y + (objectsDeque[i].direction * positionIncrement);
				if (temp >= objectsDeque[i].initialpos.y + maxVerticalMovement || temp <= objectsDeque[i].initialpos.y - maxVerticalMovement)
					objectsDeque[i].direction *= -1;

				objectsDeque[i].pos.y += objectsDeque[i].direction * positionIncrement;
				if (objectsDeque[i].hasDependency())
					objectsDeque[i - 1].pos.y += (objectsDeque[i].direction * positionIncrement);
				break;
			case ObstacleSubType::BrownObstacle:
				if (!objectsDeque[i].isAlive() && objectsDeque[i].typeState + delayTimeBetweenChange < objectsDeque[i].getStateCounts() * delayTimeBetweenChange)
					objectsDeque[i].typeState += delayTimeBetweenChange;
				if (objectsDeque[i].typeState + delayTimeBetweenChange == objectsDeque[i].getStateCounts() * delayTimeBetweenChange)
					objectsDeque[i].pos.y += 15;
				break;
			case ObstacleSubType::WhiteObstacle:
				if (!objectsDeque[i].isAlive())
					objectsDeque[i].typeState += delayTimeBetweenChange;
				if (objectsDeque[i].typeState == objectsDeque[i].getStateCounts() * delayTimeBetweenChange)
					objectsDeque.erase(objectsDeque.begin() + i);
				break;
			case ObstacleSubType::OrangeObstacle:
				objectsDeque[i].timetolive -= frameRate;
				if (objectsDeque[i].timetolive <= 1000 && objectsDeque[i].typeState < objectsDeque[i].getStateCounts() / 2 * delayTimeBetweenChange)
					objectsDeque[i].typeState += delayTimeBetweenChange; // make it red after some secOnds;
				else if (objectsDeque[i].timetolive <= 0) // There is no more orange Obstacle
					objectsDeque[i].typeState += delayTimeBetweenChange;
				if (objectsDeque[i].typeState == objectsDeque[i].getStateCounts() * delayTimeBetweenChange)
				{
					playSound(explodingPlatformSound);
					objectsDeque.erase(objectsDeque.begin() + i);
				}
				break;
			}
		}
		else if (objectsDeque[i].type == ObjectType::Equipment)
		{
			switch ((EquipmentSybType)objectsDeque[i].subType)
			{
			case EquipmentSybType::SpringEquipment:
			case EquipmentSybType::TrampolineEquipment:
				if (!objectsDeque[i].isAlive() && objectsDeque[i].typeState + delayTimeBetweenChange < objectsDeque[i].getStateCounts() * delayTimeBetweenChange)
				{
					objectsDeque[i].typeState += delayTimeBetweenChange;
					objectsDeque[i].pos.y = objectsDeque[i + 1].pos.y - objectsDeque[i].getGlobalBounds().height;
				}
				break;
			case EquipmentSybType::PropellerEquipment:
			case EquipmentSybType::RacketEquipment:
			case EquipmentSybType::ShieldEquipment:
				if (!objectsDeque[i].isAlive())
					objectsDeque[i].setVisible(false);
				break;
			case EquipmentSybType::SpringShoesEquipment:
				if (!objectsDeque[i].isAlive())
					objectsDeque[i].setVisible(false);
				break;
			}
		}
		else if (objectsDeque[i].type == ObjectType::Enemy)
		{
			switch ((EnemySubType)objectsDeque[i].subType)
			{
			case EnemySubType::FlyEnemy1:
				temp = objectsDeque[i].pos.x + (objectsDeque[i].direction * positionIncrement);
				if (temp >= objectsDeque[i].initialpos.x + maxHorizontalMovement || temp <= objectsDeque[i].initialpos.x - maxHorizontalMovement)
					objectsDeque[i].direction *= -1;

				objectsDeque[i].pos.x += (objectsDeque[i].direction * positionIncrement);

				objectsDeque[i].typeState += delayTimeBetweenChange;
				if (objectsDeque[i].typeState == objectsDeque[i].getStateCounts() * delayTimeBetweenChange) // Update state of Enemy
					objectsDeque[i].typeState = 0;
				break;
			case EnemySubType::FlyEnemy2:
				temp = objectsDeque[i].pos.x + (objectsDeque[i].direction * positionIncrement);
				if (temp + objectsDeque[i].getGlobalBounds().width >= screenWidth || temp <= 0)
					objectsDeque[i].direction *= -1;

				objectsDeque[i].pos.x += (objectsDeque[i].direction * positionIncrement);
				if (objectsDeque[i].direction == 1) // go to Right
					objectsDeque[i].typeState = 0; // Right state of Enemy
				else if (objectsDeque[i].direction == -1) // go to Left
					objectsDeque[i].typeState = delayTimeBetweenChange; // Left state of Enemy
				break;
			case EnemySubType::FlyEnemy3:
			case EnemySubType::FlyEnemy4:
				temp = objectsDeque[i].pos.x + (objectsDeque[i].direction * positionIncrement);
				if (temp >= objectsDeque[i].initialpos.x + maxHorizontalMovement || temp <= objectsDeque[i].initialpos.x - maxHorizontalMovement)
					objectsDeque[i].direction *= -1;

				objectsDeque[i].pos.x += (objectsDeque[i].direction * positionIncrement);
				break;
			case EnemySubType::FlyingSaucerEnemy:
				objectsDeque[i].typeState += delayTimeBetweenChange;

				if (objectsDeque[i].typeState == objectsDeque[i].getStateCounts() * delayTimeBetweenChange) // Update state of Enemy
					objectsDeque[i].typeState = 0;
				break;
			case EnemySubType::BlackHoleEnemy:
			case EnemySubType::NormalEnemy1:
				// do nothing
				break;
			case EnemySubType::NormalEnemy2:
			case EnemySubType::NormalEnemy3:
				objectsDeque[i].typeState = 0;
				break;
			}
		}
	}

	for (int i = shotsDeque.size() - 1; i >= 0; i--)
	{
		(*(shotsDeque.begin() + i)).pos.x += (*(shotsDeque.begin() + i)).directionX;
		(*(shotsDeque.begin() + i)).pos.y += (*(shotsDeque.begin() + i)).directionY;
		if ((*(shotsDeque.begin() + i)).pos.x + shotTexture.copyToImage().getSize().x >= screenWidth || (*(shotsDeque.begin() + i)).pos.x <= 0) { // Check.x position	
			shotsDeque.erase(shotsDeque.begin() + i);
		}
		else if ((*(shotsDeque.begin() + i)).pos.y + shotTexture.copyToImage().getSize().y <= 0) { // Check.y position
			shotsDeque.erase(shotsDeque.begin() + i);
		}
	}
}

// Remove objects which go out of screen and add objects if necessary
void UpdateObjectsDeque()
{
	// Remove object which go out of screen depend On their position
	while (!objectsDeque.empty() && !objectsDeque.back().hasDependency() && objectsDeque.back().pos.y >= screenHeight)
		objectsDeque.pop_back(); // just remove Obstacle

	while (!objectsDeque.empty() && objectsDeque.back().hasDependency() && (*(objectsDeque.end() - 1)).pos.y >= screenHeight)
	{
		objectsDeque.pop_back(); // remove Obstacle
		objectsDeque.pop_back(); // remove dependency 
	}

	// Check continues sound of Enemy and flyingsaucer
	auto EnemyExist = find_if(objectsDeque.begin(), objectsDeque.end(), [](const Object &x) { return (x.type == ObjectType::Enemy && x.subType != (int)EnemySubType::BlackHoleEnemy) ? true : false; });
	if (EnemyExist == objectsDeque.end())
	{
		monsterMusic.stop();
		ufoMusic.stop();
	}

	// Add object which is not depend On other platform
	bool conditionToAdd = false;
	if (!objectsDeque.empty())
	{
		if (objectsDeque.front().type == ObjectType::Obstacle && objectsDeque.front().subType == (int)ObstacleSubType::GrayObstacle) {
			if (objectsDeque.front().initialpos.y - maxVerticalMovement > distanceBetweenObstacle)
				conditionToAdd = true;
		}
		else if (objectsDeque.front().pos.y > distanceBetweenObstacle)
			conditionToAdd = true;
	}

	ObjectType randomType;
	int randomObject = 0, randomX = 0;
	if (conditionToAdd)
	{
		// First gray Obstacle from top of screen if there is (we use lambda expression here)
		auto FirstGrayObstacle = find_if(objectsDeque.begin(), objectsDeque.end(),
			[](const Object &x) { return (x.type == ObjectType::Obstacle && x.subType == (int)ObstacleSubType::GrayObstacle); });
		if (!objectsDeque.empty() && FirstGrayObstacle != objectsDeque.end()) // if there is any gray Obstacle
		{
			// Check randomObstacle is allowed or not?
			if (!(*FirstGrayObstacle).hasDependency() &&
				(*FirstGrayObstacle).initialpos.y - maxVerticalMovement <= distanceBetweenObstacle)
				return; // There is no way to add new object because of gray object
			else if ((*FirstGrayObstacle).hasDependency() &&
				(*FirstGrayObstacle).initialpos.y - maxVerticalMovement - (*(FirstGrayObstacle - 1)).getGlobalBounds().height <= distanceBetweenObstacle)
				return; // There is no way to add new object because of gray object						
		}
		
		bool safeToAdd = true;
		if (randomObjectDeque.empty()) {
			// choose between evemy and obstacle for adding if it is necessary
			randomType = rand() % 101 <= 100 - (score / 2000) ? ObjectType::Obstacle : ObjectType::Enemy;
			if (randomType == ObjectType::Obstacle)
			{
				randomObject = rand() % 101;
				if (randomObject <= 60)
					randomObject = (int)ObstacleSubType::GreenObstacle;
				else if (randomObject <= 75)
					randomObject = (int)ObstacleSubType::BlueObstacle;
				else if (randomObject <= 80)
					randomObject = (int)ObstacleSubType::GrayObstacle;
				else if (randomObject <= 82.5)
					randomObject = (int)ObstacleSubType::BrownObstacle;
				else if (randomObject <= 85)
					randomObject = (int)ObstacleSubType::OrangeObstacle;
				else if (randomObject <= 100)
					randomObject = (int)ObstacleSubType::WhiteObstacle;
			}
			else if (randomType == ObjectType::Enemy)
				randomObject = rand() % 5; // Only object which can fly include blackhole and falysaucer

			randomObjectDeque.push_back(make_pair(randomType, randomObject));
		}
		else {
			randomType = randomObjectDeque.front().first;
			randomObject = randomObjectDeque.front().second;
		}

		if (randomObject == (int)ObstacleSubType::GrayObstacle) {
			if (!objectsDeque.empty() && FirstGrayObstacle != objectsDeque.end()) // if there is any gray Obstacle
			{
				if (!(*FirstGrayObstacle).hasDependency() &&
					(*FirstGrayObstacle).initialpos.y - maxVerticalMovement <=
					distanceBetweenObstacle + maxVerticalMovement + Object::getSize(ObjectType::Obstacle, randomObject).y)
					return; // It is not sutibale to add
				else if ((*FirstGrayObstacle).hasDependency() &&
					(*FirstGrayObstacle).initialpos.y - maxVerticalMovement - (*(FirstGrayObstacle - 1)).getGlobalBounds().height <=
					distanceBetweenObstacle + maxVerticalMovement + Object::getSize(ObjectType::Obstacle, randomObject).y)
					return; // It is not sutibale to add
			}
			if (!objectsDeque.empty() &&
				objectsDeque.front().initialpos.y > distanceBetweenObstacle +
				maxVerticalMovement + Object::getSize(ObjectType::Obstacle, randomObject).y)
				safeToAdd = true; // It is sutibale to add
			else
				safeToAdd = false;
		}

		if (!safeToAdd)
			return;
		else
			randomObjectDeque.pop_front();

		bool flag = true;
		while (flag) // find random position depend On other Obstacle
		{			
			randomX = randomPosition(Object::getTexture(randomType, randomObject));
			if (!objectsDeque.empty() && FirstGrayObstacle != objectsDeque.end()) // if there is any gray Obstacle
			{				
				sf::Rect<float> firstGrayObstacleRect( (*FirstGrayObstacle).initialpos.x, 0, (*FirstGrayObstacle).getGlobalBounds().width, 0 );
				sf::Rect<float> randomPositionRect( randomX, 0, Object::getSize(randomType, randomObject).x, 0 );

				//if (!IntersectionTwoInterval(FirstGrayObstacleInterval, randomPositionInterval))
				//	flag = false; // found suitable Obstacle
				if (!firstGrayObstacleRect.intersects(randomPositionRect))
					flag = false; // found suitable Obstacle
			}
			else
				flag = false; // There is no limitatiOn			
		}

		// add object to deque
		objectsDeque.push_front(InitializeNewObject(randomType, randomObject, randomX, 0));
	}

	//Add object which depend on other objects (must sit on them)
	if (rand() % 400 <= 1 + (score / 2000))
	{
		while (!objectsDeque.empty() && objectsDeque.front().type == ObjectType::Obstacle &&
			(objectsDeque.front().subType == (int)ObstacleSubType::GreenObstacle
				|| objectsDeque.front().subType == (int)ObstacleSubType::BlueObstacle))
		{
			//randomType = rand() % 101 <= 50 - (score / 1000) ? ObjectType::Equipment : ObjectType::Enemy; // choose between evemy and Equipment for adding if it is necessary
			//if (randomType == ObjectType::Equipment)
			//{
			//	randomObject = rand() % 101;
			//	if (randomObject <= 40)
			//		randomObject = (int)EquipmentSybType::SpringEquipment;
			//	else if (randomObject <= 80)
			//		randomObject = (int)EquipmentSybType::TrampolineEquipment;
			//	else if (randomObject <= 85)
			//		randomObject = (int)EquipmentSybType::PropellerEquipment;
			//	else if (randomObject <= 90)
			//		randomObject = (int)EquipmentSybType::RacketEquipment;
			//	else if (randomObject <= 95)
			//		randomObject = (int)EquipmentSybType::SpringShoesEquipment;
			//	else if (randomObject <= 100)
			//		randomObject = (int)EquipmentSybType::ShieldEquipment;
			//}
			//else if (randomType == ObjectType::Enemy)
			//	randomObject = (rand() % 3) + 6; // Remvoe object which can't fly
			randomType = ObjectType::Equipment;
			randomObject = (int)EquipmentSybType::TrampolineEquipment;
			
			randomX = randomPosition(Object::getTexture(randomType, randomObject), objectsDeque.front().pos.x + 15,
				objectsDeque.front().pos.x + objectsDeque.front().getGlobalBounds().width - 15); // 15 is just for a short distance from sides

			objectsDeque.front().setDependency(true);
			// add object to deque
			objectsDeque.push_front(InitializeNewObject(randomType, randomObject, randomX, objectsDeque.front().pos.y - (Object::getSize(randomType, randomObject).y)));
		}
	}
}

// Initialize new object for adding to deque
// objX and objY gotten and used if neccessary
Object InitializeNewObject(ObjectType objType, int objSubType, float objX = 0, float objY = 0)
{
	Object newobj{ objType, objSubType };
	switch (newobj.type)
	{
	case ObjectType::Obstacle:
		switch ((ObstacleSubType)newobj.subType)
		{
		case ObstacleSubType::GreenObstacle:
		case ObstacleSubType::BrownObstacle:
		case ObstacleSubType::WhiteObstacle:
			newobj.pos.x = objX;
			newobj.pos.y = -1 * newobj.getGlobalBounds().height;
			break;
		case ObstacleSubType::BlueObstacle:
			newobj.pos.x = objX;
			newobj.pos.y = -1 * newobj.getGlobalBounds().height;
			newobj.direction = rand() % 2 ? +1 : -1;
			break;
		case ObstacleSubType::GrayObstacle:
			newobj.pos.x = objX;
			newobj.pos.y = -1 * newobj.getGlobalBounds().height + maxVerticalMovement;
			newobj.direction = rand() % 2 ? +1 : -1;
			break;
		case ObstacleSubType::OrangeObstacle:
			newobj.pos.x = objX;
			newobj.pos.y = -1 * newobj.getGlobalBounds().height;
			newobj.timetolive = 1000 + rand() % 3500;
			break;
		}
		break;
	case ObjectType::Equipment:
		newobj.pos = { objX , objY };
		break;
	case ObjectType::Enemy:
		if ((EnemySubType)newobj.subType == EnemySubType::FlyingSaucerEnemy)
			playMusic(ufoMusic);
		else if ((EnemySubType)newobj.subType != EnemySubType::BlackHoleEnemy)
			playMusic(monsterMusic);
		else
			playMusic(monsterMusic);

		switch ((EnemySubType)newobj.subType)
		{
		case EnemySubType::FlyEnemy1:
		case EnemySubType::FlyEnemy3:
		case EnemySubType::FlyEnemy4:
			newobj.pos.x = objX;
			newobj.pos.y = -1 * newobj.getGlobalBounds().height;
			newobj.direction = rand() % 2 ? +1 : -1;
			break;
		case EnemySubType::FlyEnemy2:
			newobj.pos.x = objX;
			newobj.pos.y = -1 * newobj.getGlobalBounds().height;
			newobj.direction = rand() % 2 ? +1 : -1;
			if (newobj.direction == 1) // go to Right
				newobj.typeState = 0; // Right state of Enemy
			else if (newobj.direction == -1) // go to Left
				newobj.typeState = delayTimeBetweenChange; // Left state of Enemy
			break;
		case EnemySubType::BlackHoleEnemy:
			newobj.pos.x = objX;
			newobj.pos.y = -1 * newobj.getGlobalBounds().height;
			break;
		case EnemySubType::FlyingSaucerEnemy:
			newobj.pos.x = objX;
			newobj.pos.y = -47; // 47 is height of flyingsaucer without light
			break;
		case EnemySubType::NormalEnemy1:
			newobj.pos.x = objX;
			newobj.pos.y = objY;
			break;
		case EnemySubType::NormalEnemy2:
			newobj.pos.x = objX;
			newobj.pos.y = objY;
			newobj.heart = 2;
			break;
		case EnemySubType::NormalEnemy3:
			newobj.pos.x = objX;
			newobj.pos.y = objY;
			newobj.heart = 3;
			break;
		}
		break;
	}

	newobj.initialpos = newobj.pos;
	return newobj;
}

void DrawObjectsInScreen()
{
	// Create objects in screen
	for (const auto& object : objectsDeque)
	{
		if (object.isVisible())
		{
			sf::Sprite temp;
			temp.setTexture(*object.getTexture());
			temp.setPosition(object.pos.x, object.pos.y);
			window.draw(temp);
		}
	}

	// Create doodle in screen
	window.draw(doodlePlayer->getSprite());

	// Create doodle Equipment in screen
	if (doodlePlayer->hasEquipment())
	{
		for (const auto& equipment : doodlePlayer->getEquipment())
		{
			sf::Sprite temp;
			auto correspondTextureSize = equipment.getSize();
			temp.setTexture(equipment.getTexture());
			switch (equipment.type)
			{
			case EquipmentSybType::PropellerEquipment:
				temp.setPosition(doodlePlayer->getPosition().x + doodlePlayer->getWidth() / 2 - correspondTextureSize.x / 2,
					doodlePlayer->getPosition().y + 35 /* distance from above doodle */ - correspondTextureSize.y);
				break;
			case EquipmentSybType::RacketEquipment:
				if (doodlePlayer->getState() == PlayerState::Left || doodlePlayer->getState() == PlayerState::LeftSit)
					temp.setPosition(doodlePlayer->getPosition().x + 65  /* distance from Left until Left foot */ - 12,
						doodlePlayer->getPosition().y + 22 /* distance from above doodle */);
				else if (doodlePlayer->getState() == PlayerState::Right || doodlePlayer->getState() == PlayerState::RightSit)
					temp.setPosition(doodlePlayer->getPosition().x + 25  /* distance from Left until Right foot */ - (correspondTextureSize.x - 12),
						doodlePlayer->getPosition().y + 22 /* distance from above doodle */);
				break;
			case EquipmentSybType::ShieldEquipment:
				temp.setPosition(doodlePlayer->getPosition().x + doodlePlayer->getWidth() / 2 - correspondTextureSize.x / 2,
					doodlePlayer->getPosition().y + doodlePlayer->getHeight() / 2 - correspondTextureSize.y / 2 + 9 /* half of distance from above doodle */);
				break;
			case EquipmentSybType::SpringShoesEquipment:
				if (equipment.heart != 0 && equipment.heart != 1) // we don't draw it before removing so we remove it in One heart
				{
					temp.setPosition(doodlePlayer->getPosition().x + doodlePlayer->getWidth() / 2 - correspondTextureSize.x / 2,
						doodlePlayer->getPosition().y + doodlePlayer->getHeight());
				}
				break;
			}
			window.draw(temp);
		}
	}

	// Create shots in screen
	for (const auto& shot : shotsDeque)
	{
		sf::Sprite temp;
		temp.setTexture(shotTexture);
		temp.setPosition(shot.pos.x, shot.pos.y);
		window.draw(temp);
	}
}

////////////////////////////////////////////////////////////////////////////// Try Again Frame

void TryAgainMain()
{
	window.draw(backgroundSprite);
	gameOverSprite.setPosition(screenWidth / 2 - gameOverSprite.getGlobalBounds().width / 2, 300);
	window.draw(gameOverSprite);
	playAgainButtonSprite.setPosition(screenWidth / 2 - playAgainButtonSprite.getGlobalBounds().width / 2, 650);
	menuButtonSprite.setPosition(screenWidth / 2 - menuButtonSprite.getGlobalBounds().width / 2, 750);

	// your Score text	
	sf::Text text{ "your score: " + to_string(score), defaultfont };
	text.setFillColor(sf::Color::Black);
	text.setPosition(screenWidth / 2 - text.getGlobalBounds().width / 2, 500);
	window.draw(text);

	// your HighScore text	
	text.setString("your high score: " + to_string(records[0]));
	text.setPosition(screenWidth / 2 - text.getGlobalBounds().width / 2, 550);
	window.draw(text);

	// Update records
	if (firstPlay == false)
	{
		int previewRecord;
		for (previewRecord = 0; previewRecord < 10; previewRecord++)
			if (score > records[previewRecord])
				break;

		if (previewRecord != 10) // if we need to replace score
		{
			for (int j = 9; j > previewRecord; j--)
				records[j] = records[j - 1];
			records[previewRecord] = score;

			UpdateSettings(); // call Update settings
		}
		firstPlay = true;
	}

	if (MouseOverSomething(window, playAgainButtonSprite))
	{
		playAgainButtonSprite.setTexture(*playAgainButtonTexture[On]);
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			firstPlay = true;
			currentFrame = Page::PlayPage;
		}
	}
	else
		playAgainButtonSprite.setTexture(*playAgainButtonTexture[Off]);
	window.draw(playAgainButtonSprite);

	if (MouseOverSomething(window, menuButtonSprite))
	{
		menuButtonSprite.setTexture(*menuButtonTexture[On]);
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			firstSeenMenu = true;
			firstPlay = true;
			currentFrame = Page::MenuPage;
		}
	}
	else
		menuButtonSprite.setTexture(*menuButtonTexture[Off]);
	window.draw(menuButtonSprite);
}

////////////////////////////////////////////////////////////////////////////// Score Frame

void ScoresMain()
{
	window.draw(backgroundSprite);
	window.draw(scorePageSprite);
	menuButtonSprite.setPosition(screenWidth / 2 - menuButtonSprite.getGlobalBounds().width / 2, 850);

	// HighScore text
	sf::Text text{ "HIGH SCORES", defaultfont };
	text.setFillColor(sf::Color::Black);
	text.setPosition(screenWidth / 2 - text.getGlobalBounds().width / 2, 300);
	window.draw(text);

	for (int i = 0; i < 10; i++)
	{
		char str[500];
		snprintf(str, 500, "record  %2d:%20lld", i + 1 , records[i]);
		string temp{ str };
		text.setString(str);
		text.setPosition(screenWidth / 2 - text.getGlobalBounds().width / 2, 400 + i * (text.getGlobalBounds().height + 15));
		window.draw(text);
	}

	if (MouseOverSomething(window, menuButtonSprite))
	{
		menuButtonSprite.setTexture(*menuButtonTexture[On]);
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			firstSeenMenu = true;
			firstPlay = true;
			currentFrame = Page::MenuPage;
		}
	}
	else
		menuButtonSprite.setTexture(*menuButtonTexture[Off]);
	window.draw(menuButtonSprite);
}