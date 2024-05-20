#pragma once

#include "General/Program.hpp"

#include <Core.hpp>

namespace wfe {
	class AssetManager;
	class Program;

	/// @brief The virtual class used for assets bound to files.
	class Asset {
	public:
		/// @brief A function which constructs an asset with the current type.
		typedef Asset*(*AssetConstructor)(AssetManager* manager, bool8_t fromFile);
	
		/// @brief A struct containing the required information for an asset type.
		struct AssetType {
			/// @brief The name of the asset type.
			string name;
			/// @brief The file extension nof the asset's files.
			string fileExtension;
			/// @brief A vector containing all supported extensions for import files.
			vector<string> importExtensions;
			/// @brief The function used to construct assets with the current type.
			AssetConstructor constructor;
		};

		/// @brief Returns all asset types.
		/// @return A vector containing structs with descriptions of every asset type.
		static const vector<AssetType>& GetAssetTypes() {
			return assetTypes;
		}
		/// @brief Adds the given asset type to the type vector.
		/// @param newType The new asset type to add.
		static void AddAssetType(const AssetType& newType) {
			assetTypes.push_back(newType);
		}

		Asset() = delete;
		Asset(const Asset&) = delete;
		Asset(Asset&&) noexcept = delete;

		Asset& operator=(const Asset&) = delete;
		Asset& operator=(Asset&&) = delete;

		/// @brief Loads the asset from the given file.
		/// @param filePath The path of the file to load the asset from.
		/// @return True if all of the asset's dependencies are supported, otherwise false.
		bool8_t Load(const string& filePath);
		/// @brief Imports the asset from the given file and saves it to a separate file.
		/// @param filePath The path of the file to import the asset from.
		void Import(const string& filePath);
		/// @brief Saves the asset's info to its file.
		void Save();

		/// @brief Gets the asset's unique ID, which is used to identify the asset for whichever dependencies have it.
		/// @return 
		uint64_t GetID() const {
			return id;
		}
		/// @brief Gets the asset's file path.
		/// @return The asset's file path.
		const string& GetFilePath() const {
			return filePath;
		}
		/// @brief Gets the manager which owns this asset.
		/// @return A pointer to the asset's manager.
		AssetManager* GetManager() {
			return manager;
		}
		/// @brief Gets the manager which owns this asset.
		/// @return A const pointer to the asset's manager.
		const AssetManager* GetManager() const {
			return manager;
		}

		/// @brief Destroys the asset.
		virtual ~Asset();
	protected:
		/// @brief Creates an asset.
		/// @param manager The asset's manager.
		/// @param fromFile True if the asset will be loaded from a file, otherwise false.
		Asset(AssetManager* manager, bool8_t fromFile);

		/// @brief Loads the current asset from the given file input stream.
		/// @param fileInput The file input stream to load from.
		/// @return True if all of the asset's dependencies are supported, otherwise false.
		virtual bool8_t LoadAsset(FileInput& fileInput) = 0;
		/// @brief Imports one ore more assets from the given file input stream.
		/// @param fileInput The file input stream to import from.
		virtual void ImportAsset(FileInput& fileInput) = 0;
		/// @brief Saves the current asset to the given file output stream.
		/// @param fileOutput The file output stream to save to.
		virtual void SaveAsset(FileOutput& fileOutput) = 0;

		string filePath;
	private:
		friend AssetManager;

		static vector<AssetType> assetTypes;

		AssetManager* manager;
		uint64_t id;
	};

#define WFE_ASSET_TYPE(typeName, fileExtension, importExtensions) \
static wfe::Asset* typeName ## Constructor(wfe::AssetManager* manager, wfe::bool8_t fromFile) { \
	return (wfe::Asset*)wfe::NewObject<typeName>(manager, fromFile); \
} \
static void typeName ## TypeInfoConstructor() { \
	/* Set the asset type info */ \
	wfe::Asset::AssetType assetType; \
\
	assetType.name = #typeName; \
	assetType.fileExtension = fileExtension; \
	assetType.importExtensions = importExtensions; \
	assetType.constructor = typeName ## Constructor; \
\
	/* Add the asset type to the type vector */ \
	wfe::Asset::AddAssetType(assetType); \
} \
WFE_RUN(TypeInfoConstructor)

	/// @brief A manager class that keeps track for multiple assets.
	class AssetManager {
	public:
		/// @brief Creates an asset manager for the given directory.
		/// @param assetDir The directory to scan for assets.
		/// @param program The program which owns the job manager.
		AssetManager(const string& assetDir, Program* program);

		AssetManager() = delete;
		AssetManager(const AssetManager&) = delete;
		AssetManager(AssetManager&&) noexcept = delete;

		AssetManager& operator=(const AssetManager&) = delete;
		AssetManager& operator=(AssetManager&&) = delete;

		/// @brief Creates an asset owned by this manager.
		/// @tparam T The type of the asset to be created.
		/// @param fromFile True if the asset will be loaded from a file, otherwise false.
		/// @return A pointer to the newly created asset.
		template<class T>
		T* CreateAsset(bool8_t fromFile) {
			return NewObject<T>(this, fromFile);
		}
		/// @brief Destroys an asset owned by this manager.
		/// @param asset The asset to destroy.
		void DestroyAsset(Asset* asset) {
			DestroyObject(asset);
		}

		/// @brief Loads every asset from the given directory.
		/// @param dirPath The directory to load from, relative to the asset manager's root directory.
		void LoadAssets(const string& dirPath);
		/// @brief Saves all of the manager's assets which have a valid file path.
		void SaveAssets();

		/// @brief Gets the manager's assets.
		/// @return A vector containing pointers to every single asset managed 
		vector<Asset*> GetAssets() const;
		/// @brief Gets the manager's asset with the given ID.
		/// @param id The ID to check.
		/// @return The pointer to the asset with the requested ID, otherwise a nullptr.
		Asset* GetAsset(uint64_t id) const;

		/// @brief Destroys the asset manager.
		~AssetManager();
	private:
		friend Asset;

		struct LoadAssetJobArgs {
			AssetManager* manager;
			Asset* asset;
			string filePath;
			FileInput fileInput;
			bool8_t finished;
			bool8_t finishedCached;
		};

		static void* LoadAssetJob(void* args);
		static void* SaveAssetJob(void* args);

		string assetDir;
		Program* program;

		mutable AtomicMutex managerMutex;
	 	unordered_map<uint64_t, Asset*> assets;
		uint64_t nextID;
	};
}