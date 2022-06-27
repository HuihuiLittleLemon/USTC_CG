#include <Engine/MeshEdit/ASAP.h>

#include <Engine/Primitive/TriMesh.h>

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>

using namespace Eigen;
using namespace Ubpa;
using namespace std;

ASAP::ASAP(Ptr<TriMesh> triMesh) : heMesh(make_shared<HEMesh<V>>()) {
	Init(triMesh);
}

void ASAP::Clear() {
	heMesh->Clear();
	triMesh = nullptr;
}

bool ASAP::Init(Ptr<TriMesh> triMesh) {
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

bool ASAP::Run(bool is_uniform_weight) {
	if (heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::MiParamaterizenSurf::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}


	// half-edge structure -> triangle mesh
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();
	vector<pointf2> texcoords;
	vector<unsigned> indice;


	generator_ref_triangle();
	calc_all_cot_theta();
	sparse_matrix_preprocess();

	//for (auto v : heMesh->Vertices()) {
	//	texcoords.push_back({ v->pos[0], v->pos[1] });
	//}
	//triMesh->Update(texcoords);

	vector<pointf3> positions;
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


void ASAP::generator_ref_triangle() {
	size_t index;
	vecf3 pos0, pos1, pos2;
	pointf2 ref_pos0, ref_pos1, ref_pos2;
	vecf3 a, b;
	float a_norm, b_norm;
	float cos_C, sin_C;
	auto& positions = triMesh->GetPositions();
	ref_triangles_.reserve(heMesh->Polygons().size());

	for (auto triangle : heMesh->Polygons()) {
		index = heMesh->Index(triangle);
		auto boundary_vertice = triangle->BoundaryVertice();
		pos0 = boundary_vertice[0]->pos;
		pos1 = boundary_vertice[1]->pos;
		pos2 = boundary_vertice[2]->pos;
		a = pos1 - pos0;
		b = pos2 - pos0;
		a_norm = a.norm();
		b_norm = b.norm();
		ref_pos0 = { 0,0 };
		ref_pos1 = { a_norm,0 };
		cos_C = a.cos_theta(b);
		sin_C = a.sin_theta(b);
		ref_pos2 = { b_norm * cos_C,b_norm * sin_C };
		ref_triangles_[index] = { ref_pos0 ,ref_pos1 ,ref_pos2 };
	}
}

void ASAP::calc_all_cot_theta() {
	size_t index;
	vecf3 pos0, pos1, pos2;
	vecf3 a, b, c;
	float cot_A, cot_B, cot_C;
	cot_theta_.reserve(heMesh->Polygons().size());

	for (auto triangle : heMesh->Polygons()) {
		index = heMesh->Index(triangle);
		auto boundary_vertice = triangle->BoundaryVertice();
		pos0 = boundary_vertice[0]->pos;
		pos1 = boundary_vertice[1]->pos;
		pos2 = boundary_vertice[2]->pos;
		a = pos1 - pos0;
		b = pos2 - pos0;
		c = pos2 - pos1;

		cot_C = a.cos_theta(b)/ a.sin_theta(b);
		cot_B = -a.cos_theta(c) / a.sin_theta(c);
		cot_A = b.cos_theta(c) / b.sin_theta(c);
		cot_theta_[index] = { cot_A,cot_C ,cot_B };
	}
}

ComputationInfo ASAP::sparse_matrix_preprocess() {
	size_t nV = heMesh->NumVertices();
	size_t col_num = 2 * (heMesh->NumVertices() + heMesh->NumPolygons() - 2);
	size_t row_index = 0;
	int offset;
	V* pos0, * pos1, * pos2;
	pointf2 ref_pos0, ref_pos1, ref_pos2;
	std::vector<Triplet<float>> tripletlist;
	float coff_0, coff_2, coff_v;
	size_t index;
	std::unordered_map<size_t, float> adj_vertices_map_coff;
	A_saprse_.resize(col_num, col_num);
	VectorXf b(col_num);
	int fixed_index[2] = { 0,1 };// { 3161, 2898 };
	float fixed_vertices_distance = 1;//10*(heMesh->Vertices()[fixed_index[0]]->pos - heMesh->Vertices()[fixed_index[1]]->pos).norm();
	std::unordered_map<size_t, size_t> vertices_map_row_num;

	auto HE_boundaries = heMesh->Boundaries()[0];
	auto he1 = HE_boundaries[0];
	auto he2 = HE_boundaries[HE_boundaries.size() / 2];
	fixed_index[0] = heMesh->Index(he1->Origin());
	fixed_index[1] = heMesh->Index(he2->Origin());

	size_t cnt = 0;
	for (auto v : heMesh->Vertices()) {
		if (heMesh->Index(v) == fixed_index[0] || heMesh->Index(v) == fixed_index[1]) {
			continue;
		}
		else {
			vertices_map_row_num[heMesh->Index(v)] = cnt;
			cnt = cnt + 2;
		}
	}

	for (auto v : heMesh->Vertices()) {
		if (heMesh->Index(v) == fixed_index[0] || heMesh->Index(v) == fixed_index[1]) {
			continue;
		}

		row_index = vertices_map_row_num[heMesh->Index(v)];
		coff_v = 0.f;
		adj_vertices_map_coff.clear();

		for (auto triangle : v->AdjPolygons()) {
			if (triangle == nullptr) {
				continue;
			}

			index = heMesh->Index(triangle);
			auto boundary_vertice = triangle->BoundaryVertice();

			if (boundary_vertice[0] == v) {
				offset = 0;
			}
			else if (boundary_vertice[1] == v) {
				offset = 1;
			}
			else {
				offset = 2;
			}

			pos0 = boundary_vertice[offset];
			pos1 = boundary_vertice[(offset + 1) % 3];
			pos2 = boundary_vertice[(offset + 2) % 3];

			if (adj_vertices_map_coff.find(heMesh->Index(pos1)) == adj_vertices_map_coff.end()) {
				adj_vertices_map_coff[heMesh->Index(pos1)] = -cot_theta_[index][offset];
			}
			else {
				adj_vertices_map_coff[heMesh->Index(pos1)] += -cot_theta_[index][offset];
			}

			if (adj_vertices_map_coff.find(heMesh->Index(pos2)) == adj_vertices_map_coff.end()) {
				adj_vertices_map_coff[heMesh->Index(pos2)] = -cot_theta_[index][(offset + 2) % 3];
			}
			else {
				adj_vertices_map_coff[heMesh->Index(pos2)] += -cot_theta_[index][(offset + 2) % 3];
			}

			auto ref_triangle = ref_triangles_[index];
			ref_pos0 = ref_triangle[offset];
			ref_pos1 = ref_triangle[(offset + 1) % 3];
			ref_pos2 = ref_triangle[(offset + 2) % 3];
			coff_0 = cot_theta_[index][offset];
			coff_2 = cot_theta_[index][(offset + 2) % 3];

			tripletlist.push_back(Triplet<float>(row_index, 2 * (nV + heMesh->Index(triangle) - 2), -coff_0 * (ref_pos0 - ref_pos1)[0] - coff_2 * (ref_pos0 - ref_pos2)[0]));
			tripletlist.push_back(Triplet<float>(row_index, 2 * (nV + heMesh->Index(triangle) - 2) + 1, -coff_0 * (ref_pos0 - ref_pos1)[1] - coff_2 * (ref_pos0 - ref_pos2)[1]));
			tripletlist.push_back(Triplet<float>(row_index + 1, 2 * (nV + heMesh->Index(triangle) - 2), -coff_0 * (ref_pos0 - ref_pos1)[1] - coff_2 * (ref_pos0 - ref_pos2)[1]));
			tripletlist.push_back(Triplet<float>(row_index + 1, 2 * (nV + heMesh->Index(triangle) - 2) + 1, coff_0 * (ref_pos0 - ref_pos1)[0] + coff_2 * (ref_pos0 - ref_pos2)[0]));

			coff_v = coff_v + coff_0 + coff_2;
		}
		tripletlist.push_back(Triplet<float>(row_index, row_index, coff_v));
		tripletlist.push_back(Triplet<float>(row_index + 1, row_index + 1, coff_v));

		b[row_index] = 0.f;
		b[row_index + 1] = 0.f;
		for (auto iter : adj_vertices_map_coff) {
			if (iter.first != fixed_index[0] && iter.first != fixed_index[1]) {
				tripletlist.push_back(Triplet<float>(row_index, vertices_map_row_num[iter.first], iter.second));
				tripletlist.push_back(Triplet<float>(row_index + 1, vertices_map_row_num[iter.first] + 1, iter.second));
			}
			else {
				if (iter.first == fixed_index[1]) {
					b[row_index] += fixed_vertices_distance*(-iter.second);
				}
			}
		}
	}

	for (auto triangle : heMesh->Polygons()) {
		auto boundary_vertice = triangle->BoundaryVertice();
		index = heMesh->Index(triangle);
		auto ref_triangle = ref_triangles_[index];

		ref_pos0 = ref_triangle[0];
		ref_pos1 = ref_triangle[1];
		ref_pos2 = ref_triangle[2];

		row_index = 2 * (nV + heMesh->Index(triangle) - 2);
		pos0 = boundary_vertice[0];
		pos1 = boundary_vertice[1];
		pos2 = boundary_vertice[2];

		b[row_index] = 0.f;
		b[row_index + 1] = 0.f;
		if (heMesh->Index(pos0) != fixed_index[0] && heMesh->Index(pos0) != fixed_index[1]) {
			tripletlist.push_back(Triplet<float>(row_index, vertices_map_row_num[heMesh->Index(pos0)], -cot_theta_[index][0] * (ref_pos0 - ref_pos1)[0] - cot_theta_[index][2] * (ref_pos0 - ref_pos2)[0]));
			tripletlist.push_back(Triplet<float>(row_index, vertices_map_row_num[heMesh->Index(pos0)] + 1, -cot_theta_[index][0] * (ref_pos0 - ref_pos1)[1] - cot_theta_[index][2] * (ref_pos0 - ref_pos2)[1]));
			tripletlist.push_back(Triplet<float>(row_index + 1, vertices_map_row_num[heMesh->Index(pos0)], -cot_theta_[index][0] * (ref_pos0 - ref_pos1)[1] - cot_theta_[index][2] * (ref_pos0 - ref_pos2)[1]));
			tripletlist.push_back(Triplet<float>(row_index + 1, vertices_map_row_num[heMesh->Index(pos0)] + 1, cot_theta_[index][0] * (ref_pos0 - ref_pos1)[0] + cot_theta_[index][2] * (ref_pos0 - ref_pos2)[0]));
		}
		else {
			if (heMesh->Index(pos0) == fixed_index[1]) {
				b[row_index] += fixed_vertices_distance * (cot_theta_[index][0] * (ref_pos0 - ref_pos1)[0] + cot_theta_[index][2] * (ref_pos0 - ref_pos2)[0]);
				b[row_index + 1] += fixed_vertices_distance * (cot_theta_[index][0] * (ref_pos0 - ref_pos1)[1] + cot_theta_[index][2] * (ref_pos0 - ref_pos2)[1]);
			}
		}

		if (heMesh->Index(pos1) != fixed_index[0] && heMesh->Index(pos1) != fixed_index[1]) {
			tripletlist.push_back(Triplet<float>(row_index, vertices_map_row_num[heMesh->Index(pos1)], cot_theta_[index][0] * (ref_pos0 - ref_pos1)[0] - cot_theta_[index][1] * (ref_pos1 - ref_pos2)[0]));
			tripletlist.push_back(Triplet<float>(row_index, vertices_map_row_num[heMesh->Index(pos1)] + 1, cot_theta_[index][0] * (ref_pos0 - ref_pos1)[1] - cot_theta_[index][1] * (ref_pos1 - ref_pos2)[1]));
			tripletlist.push_back(Triplet<float>(row_index + 1, vertices_map_row_num[heMesh->Index(pos1)], cot_theta_[index][0] * (ref_pos0 - ref_pos1)[1] - cot_theta_[index][1] * (ref_pos1 - ref_pos2)[1]));
			tripletlist.push_back(Triplet<float>(row_index + 1, vertices_map_row_num[heMesh->Index(pos1)] + 1, -cot_theta_[index][0] * (ref_pos0 - ref_pos1)[0] + cot_theta_[index][1] * (ref_pos1 - ref_pos2)[0]));
		}
		else {
			if (heMesh->Index(pos1) == fixed_index[1]) {
				b[row_index] += fixed_vertices_distance * (-cot_theta_[index][0] * (ref_pos0 - ref_pos1)[0] + cot_theta_[index][1] * (ref_pos1 - ref_pos2)[0]);
				b[row_index + 1] += fixed_vertices_distance * (- cot_theta_[index][0] * (ref_pos0 - ref_pos1)[1] + cot_theta_[index][1] * (ref_pos1 - ref_pos2)[1]);
			}
		}

		if (heMesh->Index(pos2) != fixed_index[0] && heMesh->Index(pos2) != fixed_index[1]) {
			tripletlist.push_back(Triplet<float>(row_index, vertices_map_row_num[heMesh->Index(pos2)], cot_theta_[index][1] * (ref_pos1 - ref_pos2)[0] + cot_theta_[index][2] * (ref_pos0 - ref_pos2)[0]));
			tripletlist.push_back(Triplet<float>(row_index, vertices_map_row_num[heMesh->Index(pos2)] + 1, cot_theta_[index][1] * (ref_pos1 - ref_pos2)[1] + cot_theta_[index][2] * (ref_pos0 - ref_pos2)[1]));
			tripletlist.push_back(Triplet<float>(row_index + 1, vertices_map_row_num[heMesh->Index(pos2)], cot_theta_[index][1] * (ref_pos1 - ref_pos2)[1] + cot_theta_[index][2] * (ref_pos0 - ref_pos2)[1]));
			tripletlist.push_back(Triplet<float>(row_index + 1, vertices_map_row_num[heMesh->Index(pos2)] + 1, -cot_theta_[index][1] * (ref_pos1 - ref_pos2)[0] - cot_theta_[index][2] * (ref_pos0 - ref_pos2)[0]));
		}
		else {
			if (heMesh->Index(pos2) == fixed_index[1]) {
				b[row_index] += fixed_vertices_distance * (-cot_theta_[index][1] * (ref_pos1 - ref_pos2)[0] - cot_theta_[index][2] * (ref_pos0 - ref_pos2)[0]);
				b[row_index + 1] += fixed_vertices_distance * (-cot_theta_[index][1] * (ref_pos1 - ref_pos2)[1] - cot_theta_[index][2] * (ref_pos0 - ref_pos2)[1]);
			}
		}

		tripletlist.push_back(Triplet<float>(row_index, row_index, cot_theta_[index][0] * (ref_pos0 - ref_pos1).norm2() + cot_theta_[index][1] * (ref_pos1 - ref_pos2).norm2() + cot_theta_[index][2] * (ref_pos0 - ref_pos2).norm2()));
		tripletlist.push_back(Triplet<float>(row_index + 1, row_index + 1, cot_theta_[index][0] * (ref_pos0 - ref_pos1).norm2() + cot_theta_[index][1] * (ref_pos1 - ref_pos2).norm2() + cot_theta_[index][2] * (ref_pos0 - ref_pos2).norm2()));
	}

	A_saprse_.setFromTriplets(tripletlist.begin(), tripletlist.end());
	solver_sparse_.compute(A_saprse_);
	std::cout << solver_sparse_.info() << "\n";
	//std::cout << A_saprse_ << endl;
	VectorXf x_0 = solver_sparse_.solve(b);
	//std::cout << x_0 << endl;

	for (auto v : heMesh->Vertices()) {
		v->pos[2] = 0;
		if (heMesh->Index(v) == fixed_index[0]) {
			v->pos[0] = 0;
			v->pos[1] = 0;
			continue;
		}

		if (heMesh->Index(v) == fixed_index[1]) {
			v->pos[0] = fixed_vertices_distance;
			v->pos[1] = 0;
			continue;
		}

		v->pos[0] = x_0[vertices_map_row_num[heMesh->Index(v)]];
		v->pos[1] = x_0[vertices_map_row_num[heMesh->Index(v)] + 1];
	}
	//std::cout << b << endl;
	return solver_sparse_.info();
}