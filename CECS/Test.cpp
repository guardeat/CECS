#include <iostream>
#include <type_traits>
#include <chrono>

#include "CECS.h"

using namespace CECS;

#define print(arg) std::cout<<arg<<std::endl;

class Timer
{
public:
	using Millisecond = double;

private:
	std::chrono::time_point<std::chrono::steady_clock> startTime;

public:
	void start()
	{
		startTime = std::chrono::high_resolution_clock::now();
	}

	Millisecond getPassedTime()
	{
		std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - startTime;
		return duration.count() * 1000;
	}

	void reset()
	{
		start();
	}
};

struct Mass
{
	size_t kg;
};

struct Speed
{	
	size_t x;
	size_t y;
};

struct Mesh
{
	float x;
	float y;
	float z;
	float w;
	float data[10]{};
};

struct Position
{
	size_t x;
	size_t y;
};

struct GravitySystem : public ISystem
{
	void update() override
	{
		PoolView<Mass,Speed> view{};

		for (auto [id, mass, speed] : view)
		{
			speed.y += mass.kg * 10;
			if (speed.y > 100)
			{
				speed.y = 100;
			}
		}

	}
};

struct RenderSystem : public ISystem
{
	void update() override
	{
		PoolView<Mesh> view{};

		for (auto [id, mesh] : view)
		{
			mesh.w = mesh.x * mesh.y * mesh.z;
		}

	}
};

struct MoveSystem : public ISystem
{
	void update() override
	{
		PoolView<Position,Speed> view{};

		for (auto [id, position,speed] : view)
		{
			position.x += speed.x;
			position.y += speed.y;
		}

	}
};

int main()
{
	Timer timer;
	timer.start();
	for (size_t it{}; it < 200000; ++it)
	{
		EntityID id{ Accessor::createEntity() };
		Accessor::addComponents<Speed, Mass>(id, 
			Speed{ 5,5 },
			Mass{ 150 });
	}

	for (size_t it{}; it < 100000; ++it)
	{
		EntityID id{ Accessor::createEntity() };
		Accessor::addComponents<Speed, Mass,Position>(id, 
			Speed{ 5,5 },
			Mass{ 150 },
			Position{10});
	}
	
	for (size_t it{}; it < 100000; ++it)
	{
		EntityID id{ Accessor::createEntity() };
		Accessor::addComponents<Speed, Mass, Position,Mesh>(id,
			Speed{ 5,5 },
			Mass{ 150 }, 
			Position{ 10 },
			Mesh{1,2,3,1});
	}
	print(timer.getPassedTime());

	GravitySystem gs;
	MoveSystem ms;
	RenderSystem rs;

	while (true)
	{
		timer.start();
		gs.update();
		ms.update();
		rs.update();
		print("FPS: " << 1000 / timer.getPassedTime() << " with a million components");
	}
	
	return 0;
}