#include "Model.h"

#include "BuildInfo.h"
#include "Debugger.h"

#include <fstream>
#include <sstream>
#include <vector>

namespace mge {
	Model::Model(const Model& other) : vertexC(other.vertexC), indexC(other.indexC), vertices((Vertex*)malloc(sizeof(Vertex) * other.vertexC)), indices((size_t*)malloc(sizeof(size_t) * other.indexC)) {
		if (!vertices || !indices) {
			OutFatalError("Failed to allocate memory!");
			return;
		}

		memcpy(vertices, other.vertices, sizeof(Vertex) * vertexC);
		memcpy(indices, other.indices, sizeof(size_t) * indexC);
	}
	Model::Model(Model&& other) noexcept : vertexC(other.vertexC), indexC(other.indexC), vertices(other.vertices), indices(other.indices) {
		other.vertices = nullptr;
		other.indices = nullptr;
	}
	Model::Model(const char_t* fileLocation) {
		std::ifstream fileInput(fileLocation);

		if (!fileInput)
			OutFatalError("File not found!");

		std::vector<Vector3> positionsV;
		std::vector<Vector2> uvCoordinatesV;
		std::vector<Vector3> normalsV;
		std::vector<Vertex> verticesV;
		std::vector<size_t> indicesV;

		std::string line;
		while (!fileInput.eof()) {
			std::getline(fileInput, line);
			if (line[0] == '#')
				continue;

			std::stringstream sStream(line);

			std::string init;
			sStream >> init;

			if (init == "v") {
				//The curent line is a vector position
				Vector3 vec;
				sStream >> vec.x >> vec.y >> vec.z;
				positionsV.push_back(vec);
			} else if (init == "vt") {
				//The curent line is a vector UV position
				Vector2 vec;
				sStream >> vec.x >> vec.y;
				uvCoordinatesV.push_back(vec);
			} else if (init == "vn") {
				//The curent line is a vector normal
				Vector3 vec;
				sStream >> vec.x >> vec.y >> vec.z;
				normalsV.push_back(vec);
			} else if (init == "f") {
				//The current line is a face
				//Create multiple vertices
				while (!sStream.eof()) {
					//Read every vertex and set its indices
					std::string vertString;
					sStream >> vertString;
					std::stringstream vertStream(vertString);
					std::string pIndS, uvcIndS, nIndS;
					std::getline(vertStream, pIndS, '/');
					std::getline(vertStream, uvcIndS, '/');
					std::getline(vertStream, nIndS, ' ');

#ifdef _WIN64
					Vector3 position = positionsV[std::stoull(pIndS) - 1];
					Vector2 uvCoordinate = uvCoordinatesV[std::stoull(uvcIndS) - 1];
					Vector3 normal = normalsV[std::stoull(nIndS) - 1];
#else
					Vector3 position = positionsV[std::stoul(pIndS) - 1];
					Vector2 uvCoordinate = uvCoordinatesV[std::stoul(uvcIndS) - 1];
					Vector3 normal = normalsV[std::stoul(nIndS) - 1];
#endif
					Vertex vertex{position, uvCoordinate, normal};

					bool8_t vertexFound = false;
					size_t i;
					for (i = 0; i < verticesV.size(); i++)
						if (verticesV[i] == vertex) {
							vertexFound = true;
							break;
						}

					if (vertexFound)
						indicesV.push_back(i);
					else {
						indicesV.push_back(verticesV.size());
						verticesV.push_back(vertex);
					}
				}
			}
		}

		if (fileInput.bad())
			OutFatalError("Error occured while reading file!");

		fileInput.close();

		//Copy everything from the vertices
		vertexC = verticesV.size();
		indexC = indicesV.size();

		vertices = (Vertex*)malloc(sizeof(Vertex) * vertexC);
		indices = (size_t*)malloc(sizeof(size_t) * indexC);

		if (!vertices || !indices) {
			OutFatalError("Failed to allocate memory!");
			return;
		}

		memcpy(vertices, verticesV.data(), sizeof(Vertex) * vertexC);
		memcpy(indices, indicesV.data(), sizeof(size_t) * indexC);
	}

	Model& Model::operator=(const Model& other) {
		if (this == &other)
			return *this;

		//Delete the old vectors
		delete[] vertices;
		delete[] indices;

		//Move over the new variables
		vertexC = other.vertexC;
		indexC = other.indexC;

		vertices = (Vertex*)malloc(sizeof(Vertex) * vertexC);
		indices = (size_t*)malloc(sizeof(size_t) * indexC);

		if (!vertices || !indices) {
			OutFatalError("Failed to allocate memory!");
			return *this;
		}

		//Copy all of the heap allocated vectors
		memcpy(vertices, other.vertices, sizeof(Vertex) * vertexC);
		memcpy(indices, other.indices, sizeof(size_t) * indexC);

		return *this;
	}
	Model& Model::operator=(Model&& other) noexcept {
		//Delete the old vectors
		delete[] vertices;
		delete[] indices;

		//Move over the new variables
		vertexC = other.vertexC;
		indexC = other.indexC;
		vertices = other.vertices;
		indices = other.indices;

		//Reset the rvalue's pointers
		other.vertices = nullptr;
		other.indices = nullptr;

		return *this;
	}

	void Model::SaveToFile(const char_t* fileLocation) const {
		std::ofstream fileOutput(fileLocation, std::ios::binary);

		if (!fileOutput)
			OutFatalError("File not found!");

		fileOutput << "#Saved with MG Engine's epic swag weed edition obj file saver which is very epic swag weed\n";
		fileOutput << "sheesh bussin moment";

		if (fileOutput.bad())
			OutFatalError("Error occured while reading file!");

		fileOutput.close();
	}
}