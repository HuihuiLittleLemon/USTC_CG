#include <Engine/MeshEdit/MinSurf.h>

#include <Engine/Primitive/TriMesh.h>

#include <unordered_map>

using namespace Ubpa;

using namespace std;

MinSurf::MinSurf(Ptr<TriMesh> triMesh)
	: heMesh(make_shared<HEMesh<V>>())
{
	Init(triMesh);
	is_uniform_weight_ = false;
}

void MinSurf::Clear() {
	heMesh->Clear();
	triMesh = nullptr;
}

bool MinSurf::Init(Ptr<TriMesh> triMesh) {
	Clear();

	if (triMesh == nullptr)
		return true;

	if (triMesh->GetType() == TriMesh::INVALID) {
		printf("ERROR::MinSurf::Init:\n"
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
		printf("ERROR::MinSurf::Init:\n"
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

bool MinSurf::Run(bool is_uniform_weight) {
	if (heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::MinSurf::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	Minimize(is_uniform_weight);

	// half-edge structure -> triangle mesh
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();
	vector<pointf3> positions;
	vector<unsigned> indice;
	positions.reserve(nV);
	indice.reserve(3 * nF);
	for (auto v : heMesh->Vertices())
		positions.push_back(v->pos.cast_to<pointf3>());
	for (auto f : heMesh->Polygons()) { // f is triangle
		for (auto v : f->BoundaryVertice()) // vertices of the triangle
			indice.push_back(static_cast<unsigned>(heMesh->Index(v)));
	}

	triMesh->Init(indice, positions);

	return true;
}

float MinSurf::vecf_dot(vecf3 first, vecf3 second) {
	return (first[0] * second[0] + first[1] * second[1] + first[2] * second[2]);
}

vecf3 MinSurf::vecf_cross(vecf3 first, vecf3 second) {
	vecf3 result;
	result[0] = first[1] * second[2] - first[2] * second[1];
	result[1] = first[2] * second[0] - first[0] * second[2];
	result[2] = first[0] * second[1] - first[1] * second[0];
	return result;
}

float  MinSurf::calc_wij(V* centre, V* adj) {
	float cotC;
	vecf3 vect1, vect2, vect1_cross_vect2;
	float wij = 0.;
	if (!centre->IsConnectedWith(adj)) {
		std::cout << "calc_wij ERROR!" << "\n";
		return 0;
	}

	for (auto adj_centre : centre->AdjVertices()) {
		if (adj_centre != adj && adj_centre->IsConnectedWith(adj)) {
			vect1[0] = centre->pos[0] - adj_centre->pos[0];
			vect1[1] = centre->pos[1] - adj_centre->pos[1];
			vect1[2] = centre->pos[2] - adj_centre->pos[2];

			vect2[0] = adj->pos[0] - adj_centre->pos[0];
			vect2[1] = adj->pos[1] - adj_centre->pos[1];
			vect2[2] = adj->pos[2] - adj_centre->pos[2];
			vect1_cross_vect2 = vecf_cross(vect1, vect2);
			cotC = vecf_dot(vect1, vect2) / sqrt(vecf_dot(vect1_cross_vect2, vect1_cross_vect2));
			wij += cotC;
		}
	}

	return wij;
}

ComputationInfo MinSurf::sparse_matrix_preprocess() {
	int col;
	size_t index;
	std::vector<Triplet<float>> tripletlist;

	float wij = 0.0f;
	float sum_wij = 0.0f;
	inner_vertices_num_ = 0;

	for (auto v : heMesh->Vertices()) {
		if (v->IsBoundary()) {
			boundary_vertices_[heMesh->Index(v)] = v;
			continue;
		}

		inner_vertices_index_map_matrix_colum_[heMesh->Index(v)] = inner_vertices_num_;
		inner_vertices_num_++;
	}

	A_saprse_.resize(inner_vertices_num_, inner_vertices_num_);
	for (auto v : heMesh->Vertices()) {
		index = heMesh->Index(v);
		if (boundary_vertices_.find(index) != boundary_vertices_.end()) {
			continue;
		}

		col = inner_vertices_index_map_matrix_colum_[index];
		if (is_uniform_weight_ == true) {
			for (auto adj_v : v->AdjVertices()) {
				if (boundary_vertices_.find(heMesh->Index(adj_v)) == boundary_vertices_.end()) {
					tripletlist.push_back(Triplet<float>(col, inner_vertices_index_map_matrix_colum_[heMesh->Index(adj_v)], -1));
				}
			}
			tripletlist.push_back(Triplet<float>(col, col, static_cast<int>(v->AdjVertices().size())));
		}
		else {
			sum_wij = 0;
			for (auto adj_v : v->AdjVertices()) {
				wij = calc_wij(v, adj_v);
				sum_wij += wij;
				if (boundary_vertices_.find(heMesh->Index(adj_v)) == boundary_vertices_.end()) {

					tripletlist.push_back(Triplet<float>(col, inner_vertices_index_map_matrix_colum_[heMesh->Index(adj_v)], -wij));
				}
			}
			tripletlist.push_back(Triplet<float>(col, col, sum_wij));
		}

	}

	A_saprse_.setFromTriplets(tripletlist.begin(), tripletlist.end());
	A_saprse_.uncompress();
	solver_sparse_.compute(A_saprse_);
	std::cout << solver_sparse_.info() << "\n";

	return solver_sparse_.info();
}

void MinSurf::solve_sparse_matrix() {
	size_t index;
	int col;
	VectorXf b_x(inner_vertices_num_);
	VectorXf b_y(inner_vertices_num_);
	VectorXf b_z(inner_vertices_num_);
	float wij = 0.0f;

	for (auto v : heMesh->Vertices()) {
		index = heMesh->Index(v);
		if (boundary_vertices_.find(index) != boundary_vertices_.end()) {
			continue;
		}

		col = inner_vertices_index_map_matrix_colum_[index];
		b_x(col) = 0;
		b_y(col) = 0;
		b_z(col) = 0;
		for (auto adj_v : v->AdjVertices()) {
			if (boundary_vertices_.find(heMesh->Index(adj_v)) != boundary_vertices_.end()) {
				if (is_uniform_weight_ == true) {
					b_x(col) += adj_v->pos[0];
					b_y(col) += adj_v->pos[1];
					b_z(col) += adj_v->pos[2];
				}
				else {
					wij = calc_wij(v, adj_v);
					b_x(col) += wij * adj_v->pos[0];
					b_y(col) += wij * adj_v->pos[1];
					b_z(col) += wij * adj_v->pos[2];
				}
			}
		}
		//std::cout << b_x(col) << "\t" << b_y(col) << "\t" << b_z(col) << "\n";
	}

	VectorXf x_0 = solver_sparse_.solve(b_x);
	VectorXf x_1 = solver_sparse_.solve(b_y);
	VectorXf x_2 = solver_sparse_.solve(b_z);

	for (auto v : heMesh->Vertices()) {
		index = heMesh->Index(v);
		if (boundary_vertices_.find(index) != boundary_vertices_.end()) {
			continue;
		}

		col = inner_vertices_index_map_matrix_colum_[index];

		v->pos[0] = x_0(col);
		v->pos[1] = x_1(col);
		v->pos[2] = x_2(col);
	}

	std::cout << "Minimize Done!" << "\n";
}

void MinSurf::Minimize(bool is_uniform_weight) {
	is_uniform_weight_ = is_uniform_weight;
	if (0 == sparse_matrix_preprocess()) {
		std::cout << "sparse matrix preprocess success" << "\n";
		solve_sparse_matrix();
	}
	else {
		std::cout << "sparse matrix preprocess error" << "\n";
	}
}

void MinSurf::Minimize(bool is_uniform_weight, std::unordered_map<size_t, vecf3> boundary_vertices_map_on_circle) {
	is_uniform_weight_ = is_uniform_weight;
	if (0 == sparse_matrix_preprocess()) {
		std::cout << "para sparse matrix preprocess success" << "\n";
		solve_sparse_matrix(boundary_vertices_map_on_circle);
	}
	else {
		std::cout << "para sparse matrix preprocess error" << "\n";
	}
}

void MinSurf::solve_sparse_matrix(std::unordered_map<size_t, vecf3> boundary_vertices_map_on_circle) {
	size_t index;
	vecf3 on_circle_vertice;
	int col;
	VectorXf b_x(inner_vertices_num_);
	VectorXf b_y(inner_vertices_num_);
	VectorXf b_z(inner_vertices_num_);
	float wij = 0.0f;

	for (auto v : heMesh->Vertices()) {
		index = heMesh->Index(v);
		if (boundary_vertices_.find(index) != boundary_vertices_.end()) {
			continue;
		}

		col = inner_vertices_index_map_matrix_colum_[index];
		b_x(col) = 0;
		b_y(col) = 0;
		b_z(col) = 0;
		for (auto adj_v : v->AdjVertices()) {
			if (boundary_vertices_.find(heMesh->Index(adj_v)) != boundary_vertices_.end()) {
				on_circle_vertice = boundary_vertices_map_on_circle[heMesh->Index(adj_v)];
				if (is_uniform_weight_ == true) {
					b_x(col) += on_circle_vertice[0];
					b_y(col) += on_circle_vertice[1];
					b_z(col) += on_circle_vertice[2];
				}
				else {
					wij = calc_wij(v, adj_v);
					b_x(col) += wij * on_circle_vertice[0];
					b_y(col) += wij * on_circle_vertice[1];
					b_z(col) += wij * on_circle_vertice[2];
				}
			}
		}
		//std::cout << b_x(col) << "\t" << b_y(col) << "\t" << b_z(col) << "\n";
	}

	VectorXf x_0 = solver_sparse_.solve(b_x);
	VectorXf x_1 = solver_sparse_.solve(b_y);
	VectorXf x_2 = solver_sparse_.solve(b_z);

	for (auto v : heMesh->Vertices()) {
		index = heMesh->Index(v);
		if (boundary_vertices_.find(index) != boundary_vertices_.end()) {
			on_circle_vertice = boundary_vertices_map_on_circle[index];
			v->pos[0] = on_circle_vertice[0];
			v->pos[1] = on_circle_vertice[1];
			v->pos[2] = on_circle_vertice[2];
			continue;
		}

		col = inner_vertices_index_map_matrix_colum_[index];

		v->pos[0] = x_0(col);
		v->pos[1] = x_1(col);
		v->pos[2] = x_2(col);
	}

	std::cout << "Minimize Done!" << "\n";
}


