#ifndef ENGINE_MAPHANDLER_H
#define ENGINE_MAPHANDLER_H

#include <string>
#include <map>

#include "UrchinCommon.h"
#include "Urchin3dEngine.h"
#include "UrchinSoundEngine.h"
#include "resources/Map.h"

namespace urchin
{

	/**
	* A map handler for 3d objects, physics, sounds...
	*/
	class MapHandler
	{
		//XML tags
		#define SCENE_TAG "scene"

		//XML attributes
		#define WORKING_DIR_ATTR "relativeWorkingDirectory"

		public:
			MapHandler(Renderer3d *, PhysicsWorld *, SoundManager *);
			MapHandler(Renderer3d *, PhysicsWorld *, SoundManager *, const std::string &);
			~MapHandler();

			static std::string getRelativeWorkingDirectory(const std::string &);
			std::string getRelativeWorkingDirectory() const;
			void setRelativeWorkingDirectory(const std::string &);

			void writeMapOnFile(const std::string &) const;

			void refreshMap();

			Map *getMap() const;

		private:
			void loadMapFromFile(const std::string &);

			Renderer3d *renderer3d;
			PhysicsWorld *physicsWorld;
			SoundManager *soundManager;

			std::string relativeWorkingDirectory;
			Map *map;
	};

}

#endif
