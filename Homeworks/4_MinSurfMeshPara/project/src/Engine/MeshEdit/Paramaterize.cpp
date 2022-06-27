#include <Engine/MeshEdit/Paramaterize.h>

#include <Engine/MeshEdit/MinSurf.h>

#include <Engine/Primitive/TriMesh.h>

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>

using namespace Eigen;
using namespace Ubpa;
using namespace std;

Paramaterize::Paramaterize(Ptr<TriMesh> triMesh) : heMesh(make_shared<HEMesh<V>>()) {
	Init(triMesh);
}

void Paramaterize::Clear() {
	heMesh->Clear();
	triMesh = nullptr;
}

bool Paramaterize::Init(Ptr<TriMesh> triMesh) {
	Clear();

	if (triMesh == nullptr) {
		return true;
	}
		
	if (triMesh->GetType() == TriMesh::INVALID) {
		printf("ERROR::Paramaterize::Init:\n"
			"\t""trimesh is invalid\n");
		return false;
	}

	// init half-edge structure
	size_t nV = triMesh->GetPositions().size();
	vector<vector<size_t>> triangles;
	triangles.reserve(triMesh->GetTriangles().size());
	for (auto triangle : triMesh->GetTriangles())
		triangles.push_back({ triangle->idx[0], triangle->idx[1], triangle->idx[2] });
	heMesh->Reserve(nV);
	heMesh->Init(triangles);

	if (!heMesh->IsTriMesh() || !heMesh->HaveBoundary()) {
		printf("ERROR::Paramaterize::Init:\n"
			"\t""trimesh is not a triangle mesh or hasn't a boundaries\n");
		heMesh->Clear();
		return false;
	}

	// triangle mesh's positions ->  half-edge structure's positions
	for (int i = 0; i < nV; i++) {
		auto v = heMesh->Vertices().at(i);
		v->pos = triMesh->GetPositions()[i].cast_to<vecf3>();
	}

	this->triMesh = triMesh;
	return true;
}

bool Paramaterize::Run(bool is_uniform_weight) {
	if (heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::MiParamaterizenSurf::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	if (map_trimesh_boundary_vertices_to_unit_circle() == false) {
		printf("ERROR::MiParamaterizenSurf::Run\n"
			"\t maptri mesh boundary vertices to unit square error\n");
		return false;
	}
	auto minSurf = MinSurf::New(triMesh);
	minSurf->Minimize(is_uniform_weight, boundary_vertices_map_on_circle_pos_);

	//// half-edge structure -> triangle mesh
	size_t nV = minSurf->heMesh->NumVertices();
	size_t nF = minSurf->heMesh->NumPolygons();

	for (auto v : minSurf->heMesh->Vertices()) {
		texcoords_.push_back({ v->pos[0], v->pos[1] });
	}

	triMesh->Update(texcoords_);

	return true;
}

bool Paramaterize::map_trimesh_boundary_vertices_to_unit_circle() {
	V *boundary_vertice_begin = nullptr;
	V *boundary_vertice = nullptr;
	V *end_vertices;
	std::vector<V*> boundary_vertices_array;
	std::vector<float> chord_length;
	float total_length = 0.0f;
	float edge_distance = 0.0f;
	float arc_theta = 0.0f;

	//find an boundary vertice
	for (auto v : heMesh->Vertices()) {
		if (v->IsBoundary()) {
			boundary_vertice_begin = v;
			break;
		}
	}
	if (boundary_vertice_begin == nullptr) {
		printf("ERROR::Paramaterize::map_trimesh_boundary_vertices_to_unit_square:\n"
			"\t""trimesh is not a triangle mesh or hasn't a boundaries\n");
		return false;
	}

	boundary_vertice = boundary_vertice_begin; 
	do {
		for (auto v : boundary_vertice->AdjEdges()) {
			end_vertices = (v->HalfEdge()->End() != boundary_vertice) ? v->HalfEdge()->End() : v->HalfEdge()->Origin();
			if (v->IsBoundary() && find(boundary_vertices_array.begin(), boundary_vertices_array.end(), end_vertices) == boundary_vertices_array.end()) {
				//array not contain the begin vertice,so prevent second vertice's connected boundary vertice search back to the begin vertice
				if (boundary_vertices_array.size() == 1 && end_vertices == boundary_vertice_begin) {
					continue;
				}
				boundary_vertices_array.push_back(end_vertices);
				boundary_vertice = end_vertices;
				break;
			}
		}
	} while (boundary_vertice != boundary_vertice_begin);
	printf("Paramaterize::boundary vertices array size:%zd\n", boundary_vertices_array.size());

	for (size_t i = 0; i < boundary_vertices_array.size();i++) {	
		if (i == boundary_vertices_array.size() - 1) {
			edge_distance = (boundary_vertices_array[0]->pos-boundary_vertices_array[i]->pos).norm();
		}
		else {
			edge_distance = (boundary_vertices_array[i]->pos - boundary_vertices_array[i + 1]->pos).norm();
		}
		total_length += edge_distance;
		chord_length.push_back(total_length);
	}
	boundary_vertices_array[0]->pos = { 1,0.5,0 };
	boundary_vertices_map_on_circle_pos_[heMesh->Index(boundary_vertices_array[0])] = boundary_vertices_array[0]->pos;
	for (size_t i = 0; i < chord_length.size() - 1; i++) {
		arc_theta = 2.0f * 3.14159f * chord_length[i] / total_length;
		boundary_vertices_array[i + 1]->pos = { 0.5 + 0.5*cos(arc_theta),0.5 + 0.5*sin(arc_theta),0 };
		boundary_vertices_map_on_circle_pos_[heMesh->Index(boundary_vertices_array[i + 1])] = boundary_vertices_array[i + 1]->pos;
	}

	//map trimesh boundary vertices to unit square by chord length
	//printf("Paramaterize::boundary vertices array size:%f  %f\n", total_length,chord_length.back());
}
