#include <atomic>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "sdl2/SDL.h"

#include "Application.hpp"
#include "PathTracer.hpp"
#include "Timer.hpp"
#include "Types/KdNode.hpp"
#include "Utility/ApplicationArgParser.hpp"
#include "settings.hpp"

namespace filesystem = std::filesystem;

namespace
{
	const aiScene* importAndPostProcessScene(Assimp::Importer& importer, const filesystem::path& scenePath)
	{
		const aiScene* scene = importer.ReadFile(scenePath.string(), 0);
		if (!scene)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Import of 3D scene failed: %s", importer.GetErrorString());
			return nullptr;
		}

#ifdef AI_CONFIG_PP_SBP_REMOVE
#undef AI_CONFIG_PP_SBP_REMOVE
#endif
#define AI_CONFIG_PP_SBP_REMOVE aiPrimitiveType_POINTS | aiPrimitiveType_LINES

		scene = importer.ApplyPostProcessing(
			aiProcess_ImproveCacheLocality |
			aiProcess_RemoveRedundantMaterials |
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_FindDegenerates |
			aiProcess_SortByPType);
		if (!scene)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Post processing of 3D scene failed: %s", importer.GetErrorString());
			return nullptr;
		}

		return scene;
	}

	void transformCamerasAndLightsToWorldSpace(const aiScene* scene)
	{
		for (unsigned int currentCamera = 0; currentCamera < scene->mNumCameras; currentCamera++)
		{
			aiCamera* camera = scene->mCameras[currentCamera];
			const aiNode* cameraNode = scene->mRootNode->FindNode(camera->mName);
			if (cameraNode)
			{
				aiMatrix4x4 transform = cameraNode->mTransformation;
				camera->Transform(transform);
			}
		}

		for (unsigned int currentLight = 0; currentLight < scene->mNumLights; currentLight++)
		{
			aiLight* light = scene->mLights[currentLight];
			const aiNode* lightNode = scene->mRootNode->FindNode(light->mName);
			if (lightNode)
			{
				aiMatrix4x4 transform = lightNode->mTransformation;
				light->Transform(transform);
			}
		}
	}

	void printSceneDebugInformation(const aiScene* scene)
	{
		for (unsigned int currentCamera = 0; currentCamera < scene->mNumCameras; currentCamera++)
		{
			scene->mCameras[currentCamera]->print(std::cout);
		}

		for (unsigned int currentLight = 0; currentLight < scene->mNumLights; currentLight++)
		{
			scene->mLights[currentLight]->print(std::cout);
		}

		for (unsigned int currentMesh = 0; currentMesh < scene->mNumMeshes; currentMesh++)
		{
			scene->mMeshes[currentMesh]->print(std::cout);
		}

		if (scene->mMetaData)
		{
			scene->mMetaData->print(std::cout);
		}
	}

	std::vector<raytracing::KdTriangle> collectTriangles(const aiScene* scene)
	{
		std::vector<raytracing::KdTriangle> triangles;
		for (unsigned int currentMesh = 0; currentMesh < scene->mNumMeshes; currentMesh++)
		{
			aiMesh* mesh = scene->mMeshes[currentMesh];
			if (mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
			{
				continue;
			}

			for (unsigned int currentFace = 0; currentFace < mesh->mNumFaces; currentFace++)
			{
				aiFace* face = mesh->mFaces + currentFace;
				triangles.push_back({ std::make_pair(face, mesh), raytracing::ChildSide::UNDEFINED });
			}
		}
		return triangles;
	}
}

int main(int argc, char* argv[])
{
	utility::ApplicationOptions options;
	try
	{
		utility::ApplicationArgParser parser(argc, argv);
		options = parser.parse();
		if (options.showHelp)
		{
			std::cout << utility::ApplicationArgParser::usage();
			return 0;
		}
	}
	catch (const std::exception& exception)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", exception.what());
		std::cout << utility::ApplicationArgParser::usage();
		return 1;
	}

	if (!filesystem::create_directories(options.outputDir) && !filesystem::exists(options.outputDir))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create output directory %s", options.outputDir.string().c_str());
		return 1;
	}

	raytracing::Settings renderSettings(
		options.width,
		options.height,
		options.maxSamples,
		options.maxDepth,
		options.bias,
		options.aperture,
		options.focalDistance,
		options.useDOF,
		options.useAA);
	raytracing::Application app(renderSettings);

	try
	{
		app.initialize();
		app.setUpSdl();
		app.createScreenTexture();
	}
	catch (raytracing::SdlException& exception)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", exception.what(), exception.getSdlError());
		app.cleanUp();
		return 1;
	}

	Assimp::Importer assetImporter;
	const aiScene* scene = importAndPostProcessScene(assetImporter, options.inputScene);
	if (!scene)
	{
		app.cleanUp();
		return 1;
	}

	transformCamerasAndLightsToWorldSpace(scene);
	if (options.verbose)
	{
		printSceneDebugInformation(scene);
	}

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Building KD-Tree..");
	raytracing::Timer::getInstance().start();
	std::vector<raytracing::KdTriangle> triangleMeshCollection = collectTriangles(scene);
	std::unique_ptr<raytracing::KdNode> kdTree;
	try
	{
		kdTree.reset(raytracing::KdNode::buildTreeSAH(triangleMeshCollection));
	}
	catch (raytracing::AccStructure& exception)
	{
		SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Building kd-tree failed: %s", exception.what());
		app.cleanUp();
		return 1;
	}
	const double kdBuildingTime = raytracing::Timer::getInstance().stop();
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Done. Took %.2f seconds", kdBuildingTime);

	raytracing::PathTracer rayTracer(app, scene, renderSettings, std::move(kdTree));
	try
	{
		filesystem::path sceneDir = options.inputScene;
		sceneDir = sceneDir.remove_filename();
		rayTracer.initialize(sceneDir.string());
	}
	catch (const std::exception& exception)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", exception.what());
		app.cleanUp();
		return 1;
	}

	std::vector<std::thread> threadPool;
	std::atomic<uint8_t> threadsTerminated(0);

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Start rendering..");
	raytracing::Timer::getInstance().start();
	for (uint8_t i = 0; i < options.threadCount; i++)
	{
		threadPool.push_back(rayTracer.createRenderThread(threadsTerminated));
	}

	app.handleEvents(rayTracer.getViewport(), threadPool, threadsTerminated, options.outputDir);
	assetImporter.FreeScene();
	return 0;
}
