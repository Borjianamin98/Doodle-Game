#pragma once

#ifndef CommonHeader
#define CommonHeader

#include <random>
#include <string>

const std::string extension = ".png";
extern int screenWidth, screenHeight;
extern bool soundEnabled;

struct Position {
	float x = 0, y = 0;
	bool operator==(const Position& obj) const {
		return obj.x == x && obj.y == y;
	}
};

struct Interval {
	float Start = 0, End = 0;
};

// functions
bool MouseOverSomething(const sf::RenderWindow& window, const sf::Sprite& texture)
{
	return texture.getGlobalBounds().contains(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);
}

int randomPosition(const sf::Texture& texture, int start, int end)
{
	return rand() % (end - start - static_cast<int>(texture.copyToImage().getSize().x) - 2) +start + 1;
}

int randomPosition(const sf::Texture& texture)
{
	return randomPosition(texture, 0, screenWidth);
}

// Get angle in degree and rotate position
// return rotated position
Position rotationMatrix(Position dstPos, Position srcPos, double angle)
{
	double angleRadian = angle * 3.14 / 180;
	double cosAngle = cos(angleRadian);
	double sinAngle = sin(angleRadian);

	// in coordinate system in left of screen
	Position srcPosNew = { srcPos.x, screenHeight - srcPos.y };
	Position dstPosNew = { dstPos.x, screenHeight - dstPos.y };

	// move coordinate system to destination position
	Position srcPosNewMoved = { srcPosNew.x - dstPosNew.x, srcPosNew.y - dstPosNew.y };

	int X = (int)((cosAngle * srcPosNewMoved.x) - (sinAngle * srcPosNewMoved.y));
	int Y = (int)((sinAngle * srcPosNewMoved.x) + (cosAngle * srcPosNewMoved.y));

	// move back coordinate system to the left of screen
	Position result = { X + dstPosNew.x, Y + dstPosNew.y };

	// move back  coordinate system to the top of screen
	result = { result.x, screenHeight - result.y };

	return result;
}

sf::SoundBuffer* loadSoundFromFile(const std::string& path) {
	sf::SoundBuffer* soundBuffer = new sf::SoundBuffer;
	if (!soundBuffer->loadFromFile(path))
		throw std::runtime_error{ "file couldn't load: " + path };
	return soundBuffer;
}

sf::Texture* loadTextureFromFile(const std::string& path) {
	auto texture = new sf::Texture{};
	if (!texture->loadFromFile(path))
		throw std::runtime_error{ "file couldn't load: " + path };
	return texture;
}

// load one image from file to a vector of texture
std::vector<sf::Texture*> LoadOneTextureToVector(std::string path)
{
	std::vector<sf::Texture*> subVector;
	subVector.push_back(loadTextureFromFile(path));
	return subVector;
}

// load images from files with index start to end to a vector of textures
// AddNoIndexToFirst will add first image without index
std::vector<sf::Texture*> LoadTexturesToVector(std::string path, int start, int end, bool AddNoIndexToFirst = false)
{
	std::vector<sf::Texture*> subVector;
	if (AddNoIndexToFirst)
		subVector.push_back(loadTextureFromFile(path + extension));
	for (int i = start; i <= end; i++)
		subVector.push_back(loadTextureFromFile(path + std::to_string(i) + extension));
	return subVector;
}


void playSound(sf::Sound& sound)
{
	if (soundEnabled)
		sound.play();
}

void playMusic(sf::Music& music)
{
	if (soundEnabled)
		music.play();
}

#endif // !CommonHeader


