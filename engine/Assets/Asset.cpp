#include "Asset.hpp"

namespace wfe {
	// Static variables
	vector<Asset::AssetType> Asset::assetTypes{};

	// Job functions
	void* AssetManager::LoadAssetJob(void* args) {
		// Get the job's full args
		LoadAssetJobArgs* jobArgs = (LoadAssetJobArgs*)args;

		// Check if the asset wasn't constructed
		if(!jobArgs->asset) {
			// Get the asset file's extension
			string fileExtension = jobArgs->filePath.substr(jobArgs->filePath.rfind('.') + 1);

			// Find the asset type with the current file extension
			const Asset::AssetType* currentType = nullptr;
			for(const auto& assetType : Asset::GetAssetTypes()) {
				if(assetType.fileExtension == fileExtension) {
					currentType = &assetType;
					break;
				}
			}

			// Exit the function if the current file is not an asset
			if(!currentType) {
				jobArgs->finished = true;
				return nullptr;
			}
			
			// Call the asset's constructor to create the asset
			jobArgs->asset = currentType->constructor(jobArgs->manager, true);

			// Open the file input stream
			jobArgs->fileInput.Open(jobArgs->filePath, FileInput::STREAM_TYPE_BINARY);

			// Read and set the asset's ID
			uint64_t assetID;
			jobArgs->fileInput.ReadBuffer(sizeof(uint64_t), &assetID);
			jobArgs->asset->id = assetID;
		} else {
			// Reset the file input stream's position
			jobArgs->fileInput.SetPos(sizeof(uint64_t), FileInput::SET_POS_RELATIVE_BEGIN);
		}

		// Try to load the asset
		bool8_t result = jobArgs->asset->LoadAsset(jobArgs->fileInput);

		// Check if all of the aset's dependendies are loaded
		if(result) {
			// Close the file input stream
			jobArgs->fileInput.Close();

			// Add the asset to the manager's map
			jobArgs->manager->managerMutex.Lock();
			jobArgs->manager->assets[jobArgs->asset->id] = jobArgs->asset;
			jobArgs->manager->managerMutex.Unlock();

			// Set the job as finished
			jobArgs->finished = true;
		}

		return nullptr;
	}
	void* AssetManager::SaveAssetJob(void* args) {
		// Get the pointer to the asset to save
		Asset* asset = (Asset*)args;

		// Save the asset
		asset->Save();
		
		return nullptr;
	}

	// Protected constructor
	Asset::Asset(AssetManager* manager, bool8_t fromFile) : manager(manager) {
		// Set the asset's ID, if it won't be loaded from a file later
		if(!fromFile) {
			// Lock the manager's mutex
			manager->managerMutex.Lock();

			// Set the current asset's ID and add it to the manager's unordered map
			id = manager->nextID;
			manager->assets.insert({ id, this });

			// Find the manager's next available ID
			++manager->nextID;
			for(; manager->assets.count(manager->nextID); ++manager->nextID);

			// Unlock the manager's mutex
			manager->managerMutex.Unlock();
		}
	}

	// Public functions
	bool8_t Asset::Load(const string& filePath) {
		// Open the file stream and load the asset's ID
		FileInput fileInput(filePath, FileInput::STREAM_TYPE_BINARY);

		fileInput.ReadBuffer(sizeof(uint64_t), &id);

		// Try to load the asset
		bool8_t result = LoadAsset(fileInput);

		fileInput.Close();

		// Set the asset's new file path, if successful
		if(result)
			this->filePath = filePath;

		return result;
	}
	void Asset::Import(const string& filePath) {		
		// Open the file stream and import the asset
		FileInput fileInput(filePath, FileInput::STREAM_TYPE_BINARY);

		ImportAsset(fileInput);

		fileInput.Close();
	}
	void Asset::Save() {
		// Open the file stream and save the asset's ID
		FileOutput fileOutput(filePath, FileOutput::STREAM_TYPE_BINARY);

		fileOutput.WriteBuffer(sizeof(uint64_t), &id);

		// Save the asset
		SaveAsset(fileOutput);

		fileOutput.Close();
	}

	Asset::~Asset() {
		// Remove the asset from the manager's umap
		manager->managerMutex.Lock();
		manager->assets.erase(id);
		manager->managerMutex.Unlock();
	}

	AssetManager::AssetManager(const string& assetDir, Program* program) : assetDir(assetDir), program(program) {
		// Format the asset dir's path properly
		for(size_t pos = this->assetDir.find('\\', 0); pos != SIZE_T_MAX && pos != this->assetDir.size() - 1; pos = this->assetDir.find('\\', pos + 1))
			this->assetDir[pos] = '/';
		if(this->assetDir.back() != '/')
			this->assetDir.push_back('/');

		// Try to load all registered asset IDs
		FileInput fileInput(this->assetDir + ".wfeassets", FileInput::STREAM_TYPE_BINARY);

		if(fileInput.IsOpen()) {
			// Get the number of IDs saved in the file and allocate an array for them
			size_t fileSize = (size_t)fileInput.GetSize();
			size_t idCount = fileSize / sizeof(uint64_t);

			uint64_t* ids = (uint64_t*)AllocMemory(fileSize);
			if(!ids)
				throw BadAllocException("Failed to allocate ID array!");
			
			// Load every ID from the file
			fileInput.ReadBuffer(fileSize, ids);
			
			// Close the file input stream
			fileInput.Close();

			// Insert every ID into the asset map
			for(size_t i = 0; i != idCount; ++i)
				assets.insert({ ids[i], nullptr });
			
			// Find the new next ID
			for(; assets.count(nextID); ++nextID);
		}
	}

	void AssetManager::LoadAssets(const string& dirPath) {
		// Scan the directory for files
		vector<string> files = GetDirectoryFiles(this->assetDir + dirPath);

		// Exit the function if there are no files in the given directory
		if(files.empty())
			return;

		// Allocate a job args and results array
		LoadAssetJobArgs* jobArgs = NewArray<LoadAssetJobArgs>(files.size());
		JobManager::Result* results = NewArray<JobManager::Result>(files.size());

		// Set the job args' info
		for(size_t i = 0; i != files.size(); ++i) {
			jobArgs[i].manager = this;
			jobArgs[i].asset = nullptr;
			jobArgs[i].filePath = files[i];
			jobArgs[i].finished = false;
		}

		// Submit asset load jobs until all assets have finished loading
		size_t unfinishedCount = files.size();
		for(size_t loopCount = 0; unfinishedCount && loopCount != files.size(); ++loopCount) {
			// Loop through all files and submit jobs for the unloaded ones
			for(size_t i = 0; i != files.size(); ++i) {
				// Set the new finished cached value
				jobArgs[i].finishedCached = jobArgs[i].finished;

				// Submit the job if the current file isn't loaded
				if(!jobArgs[i].finishedCached)
					program->GetJobManager()->SubmitJob(LoadAssetJob, jobArgs + i, results[i]);
			}

			// Wait for all unfinished jobs to finish
			for(size_t i = 0; i != files.size(); ++i) {
				if(jobArgs[i].finishedCached)
					continue;
				
				// Wait for the current job
				results[i].WaitForResult();

				// Check if the current job is now finished and update the unfinished count
				unfinishedCount -= jobArgs[i].finished;
			}
		}

		// Check if any circular or unresolved dependencies exist in the directory
		if(unfinishedCount)
			throw Exception("Found circular or unresolved dependencies in asset directory %s!", dirPath.c_str());

		// Free the two allocated arrays
		DestroyArray(jobArgs, files.size());
		DestroyArray(results, files.size());
	}
	void AssetManager::SaveAssets() {
		// Lock the manager's mutex
		managerMutex.Lock();

		// Exit the function if the asset manager doesn't own any assets
		if(!assets.size()) {
			managerMutex.Unlock();
			return;
		}

		// Allocate a results array and id array
		uint64_t* ids = (uint64_t*)AllocMemory(sizeof(uint64_t) * assets.size());
		if(!ids)
			throw BadAllocException("Failed to allocate asset IDs array!");
		JobManager::Result* results = NewArray<JobManager::Result>(assets.size());

		// Submit asset save jobs for every asset and add their ids to the array
		size_t resultsTop = 0, idsTop = 0;
		for(auto assetPair : assets) {
			ids[idsTop++] = assetPair.first;

			if(assetPair.second)
				program->GetJobManager()->SubmitJob(SaveAssetJob, assetPair.second, results[resultsTop++]);
		}

		// Unlock the manager's mutex
		managerMutex.Unlock();

		// Open the asset IDs file and save the asset IDs in it
		FileOutput fileOutput(assetDir + ".wfeassets", FileOutput::STREAM_TYPE_BINARY);
		fileOutput.WriteBuffer(sizeof(uint64_t) * idsTop, ids);
		fileOutput.Close();

		// Wait for every asset to finish saving
		for(size_t i = 0; i != resultsTop; ++i)
			results[i].WaitForResult();
		
		// Free the two allodated arrays
		FreeMemory(ids);
		DestroyArray(results, idsTop);
	}

	vector<Asset*> AssetManager::GetAssets() const {
		// Lock the manager's mutex
		managerMutex.Lock();

		// Loop through the manager's assets and add them to a vector
		vector<Asset*> assetsVec;
		for(auto assetPair : assets)
			if(assetPair.second)
				assetsVec.push_back(assetPair.second);
		
		// Unlock the manager's mutex and return the vector
		managerMutex.Unlock();

		return assetsVec;
	}
	Asset* AssetManager::GetAsset(uint64_t id) const {
		// Lock the manager's mutex
		managerMutex.Lock();

		// Try to find an asset with the given ID
		auto assetIter = assets.find(id);
		Asset* asset;
		if(assetIter == assets.end()) {
			asset = nullptr;
		} else {
			asset = assetIter->second;
		}

		// Unlock the manager's mutex and return the result
		managerMutex.Unlock();

		return asset;
	}

	AssetManager::~AssetManager() {
		// Destroy every owned asset
		for(auto assetPair : assets)
			if(assetPair.second)
				DestroyObject(assetPair.second);
	}
}