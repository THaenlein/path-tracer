#include "TestBoundingBox.hpp"

#include <filesystem>

#include "../src/Types/BoundingBox.hpp"

// Available gtest framework macros
// 		EXPECT_TRUE
// 		EXPECT_FALSE
// 		EXPECT_EQ
// 		EXPECT_STREQ
//		EXPECT_NO_THROW
//		EXPECT_ANY_THROW
//		EXPECT_THROW
//		EXPECT_DOUBLE_EQ
//		EXPECT_FLOAT_EQ


const aiScene* importAndProcess(Assimp::Importer& importer, const std::string& filePath)
{
	const std::string sceneFilePath(filePath);
	const aiScene* scene = importer.ReadFile(sceneFilePath, 0);

	// Remove single points and lines not forming a face
#ifdef AI_CONFIG_PP_SBP_REMOVE
#undef AI_CONFIG_PP_SBP_REMOVE
#endif
#define AI_CONFIG_PP_SBP_REMOVE aiPrimitiveType_POINTS | aiPrimitiveType_LINES;

	// Apply post processing
	importer.ApplyPostProcessing(
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_FindDegenerates |
		aiProcess_SortByPType);

	return scene;
}


// Regular intersection
TEST(BoundingBox, TestIntersectsRegular)
{
	aiVector3D min(1.f, 1.f, 0.f);
	aiVector3D max(2.f, 2.f, 1.f);
	raytracing::BoundingBox box(min, max);

	aiVector3D position(0.f, 0.f, .5f);
	aiVector3D direction(1.f, 1.f, 0.f);
	aiRay ray(position, direction);

	bool intersects = box.intersects(ray);
	EXPECT_TRUE(intersects);
}

// Negative direction vector
TEST(BoundingBox, TestIntersectsNegative)
{
	aiVector3D min(1.f, 1.f, 0.f);
	aiVector3D max(2.f, 2.f, 1.f);
	raytracing::BoundingBox box(min, max);

	aiVector3D position(3.f, 3.f, .5f);
	aiVector3D direction(-1.f, -1.f, 0.f);
	aiRay ray(position, direction);

	bool intersects = box.intersects(ray);
	EXPECT_TRUE(intersects);
}

// Intersects in both directions
TEST(BoundingBox, TestIntersectsBoth)
{
	aiVector3D min(-1.f, -1.f, 0.f);
	aiVector3D max(1.f, 1.f, 1.f);
	raytracing::BoundingBox box(min, max);

	aiVector3D position(-2.f, 0.f, .5f);
	aiVector3D direction(-1.f, 0.f, 0.f);
	aiRay ray(position, direction);

	bool intersects = box.intersects(ray);
	EXPECT_TRUE(intersects);
}

// Line intersection
TEST(BoundingBox, TestIntersectsLine)
{
	aiVector3D min(0.f, 0.f, 0.f);
	aiVector3D max(1.f, 1.f, 1.f);
	raytracing::BoundingBox box(min, max);

	aiVector3D position(-1.f, 0.f, .5f);
	aiVector3D direction(1.f, 0.f, 0.f);
	aiRay ray(position, direction);

	bool intersects = box.intersects(ray);
	EXPECT_TRUE(intersects);
}


TEST(BoundingBox, TestNotIntersects)
{
	aiVector3D min(1.f, 1.f, 0.f);
	aiVector3D max(2.f, 2.f, 1.f);
	raytracing::BoundingBox box(min, max);

	aiVector3D position(0.f, 0.f, .5f);
	aiVector3D direction(0.f, 0.f, 1.f);
	aiRay ray(position, direction);

	bool doesNotIntersect = !box.intersects(ray);
	EXPECT_TRUE(doesNotIntersect);
}

TEST(BoundingBox, TestNotIntersectsNegative)
{
	aiVector3D min(1.f, 1.f, 0.f);
	aiVector3D max(2.f, 2.f, 1.f);
	raytracing::BoundingBox box(min, max);

	aiVector3D position(3.f, 3.f, .5f);
	aiVector3D direction(-1.f, 0.f, 0.f);
	aiRay ray(position, direction);

	bool doesNotIntersect = !box.intersects(ray);
	EXPECT_TRUE(doesNotIntersect);
}


TEST(BoundingBox, TestSplitX)
{
	aiVector3D min(0.f, 0.f, 0.f);
	aiVector3D max(2.f, 1.f, 1.f);
	raytracing::BoundingBox box(min, max);

	raytracing::BoundingBox left;
	raytracing::BoundingBox right;

	box.split(left, right);

	raytracing::BoundingBox expectedLeft(min, aiVector3D(1.f, 1.f, 1.f));
	EXPECT_EQ(left, expectedLeft);

	raytracing::BoundingBox expectedRight(aiVector3D(1.f, 0.f, 0.f), max);
	EXPECT_EQ(right, expectedRight);
}

TEST(BoundingBox, TestSplitY)
{
	aiVector3D min(0.f, 0.f, 0.f);
	aiVector3D max(1.f, 3.f, 1.f);
	raytracing::BoundingBox box(min, max);

	raytracing::BoundingBox left;
	raytracing::BoundingBox right;

	box.split(left, right);

	raytracing::BoundingBox expectedLeft(min, aiVector3D(1.f, 1.5f, 1.f));
	EXPECT_EQ(left, expectedLeft);

	raytracing::BoundingBox expectedRight(aiVector3D(0.f, 1.5f, 0.f), max);
	EXPECT_EQ(right, expectedRight);
}

TEST(BoundingBox, TestSplitZ)
{
	aiVector3D min(0.f, 0.f, 0.f);
	aiVector3D max(1.f, 1.f, 4.f);
	raytracing::BoundingBox box(min, max);

	raytracing::BoundingBox left;
	raytracing::BoundingBox right;

	box.split(left, right);

	raytracing::BoundingBox expectedLeft(min, aiVector3D(1.f, 1.f, 2.f));
	EXPECT_EQ(left, expectedLeft);

	raytracing::BoundingBox expectedRight(aiVector3D(0.f, 0.f, 2.f), max);
	EXPECT_EQ(right, expectedRight);
}

TEST(BoundingBox, CreateBBoxFromSphere)
{
	Assimp::Importer assetImporter;
	const std::string sceneFilePath(
		(std::filesystem::path(PATH_TRACER_PROJECT_DIR) / "res" / "test_sphere.dae").string());
	const aiScene* scene = importAndProcess(assetImporter, sceneFilePath);
	EXPECT_TRUE(scene);
	EXPECT_TRUE(scene->HasMeshes());

	// Create BBox of first mesh in scene
	raytracing::BoundingBox bBoxOfMesh(scene->mMeshes[0]);
	aiVector3D expectedMin(3.5f, 0.25f, 0.f);
	aiVector3D expectedMax(4.5f, 1.25f, 1.f);
	EXPECT_TRUE(expectedMin.Equal(bBoxOfMesh.getMin(), 1e-2));
	EXPECT_TRUE(expectedMax.Equal(bBoxOfMesh.getMax(), 1e-2));
}

TEST(BoundingBox, CreateBBoxFromMesh)
{
	Assimp::Importer assetImporter;
	const std::string sceneFilePath(
		(std::filesystem::path(PATH_TRACER_PROJECT_DIR) / "res" / "test_bunny.dae").string());
	const aiScene* scene = importAndProcess(assetImporter, sceneFilePath);
	EXPECT_TRUE(scene);
	EXPECT_TRUE(scene->HasMeshes());

	// Create BBox of first mesh in scene
	raytracing::BoundingBox bBoxOfMesh(scene->mMeshes[0]);
	aiVector3D expectedMin(-5.222f, -3.332f, -4.820f);
	aiVector3D expectedMax(0.258f, 3.744f, 2.186f);
	EXPECT_TRUE(expectedMin.Equal(bBoxOfMesh.getMin(), 1e-2));
	EXPECT_TRUE(expectedMax.Equal(bBoxOfMesh.getMax(), 1e-2));
}
