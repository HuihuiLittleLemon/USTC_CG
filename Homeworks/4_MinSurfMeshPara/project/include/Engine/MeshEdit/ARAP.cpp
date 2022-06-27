#include <Engine/MeshEdit/ARAP.h>
#include <Engine/MeshEdit/Paramaterize.h>
#include <Engine/MeshEdit/MinSurf.h>
#include <Engine/Primitive/TriMesh.h>

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>

using namespace Eigen;
using namespace Ubpa;
using namespace std;

ARAP::ARAP(Ptr<TriMesh> triMesh) : heMesh(make_shared<HEMesh<V>>()) {
	Init(triMesh);
}

void ARAP::Clear() {
	heMesh->Clear();
	triMesh = nullptr;
}

bool ARAP::Init(Ptr<TriMesh> triMesh) {
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

bool ARAP::Run(bool is_uniform_weight) {
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
	local_global_algorithm();
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


void ARAP::generator_ref_triangle() {
	size_t index;
	vecf3 pos1, pos2, pos3;
	pointf2 ref_pos1, ref_pos2, ref_pos3;
	vecf3 a, b;
	float a_norm, b_norm;
	float cos_C, sin_C;
	ref_triangles_.reserve(heMesh->Polygons().size());

	for (auto triangle : heMesh->Polygons()) {
		index = heMesh->Index(triangle);
		auto boundary_vertice = triangle->BoundaryVertice();
		pos1 = boundary_vertice[0]->pos;
		pos2 = boundary_vertice[1]->pos;
		pos3 = boundary_vertice[2]->pos;
		a = pos2 - pos1;
		b = pos3 - pos1;
		a_norm = a.norm();
		b_norm = b.norm();
		ref_pos1 = { 0,0 };
		ref_pos2 = { a_norm,0 };
		cos_C = a.cos_theta(b);
		sin_C = a.sin_theta(b);
		ref_pos3 = { b_norm * cos_C,b_norm * sin_C };
		ref_triangles_[index] = { ref_pos1 ,ref_pos2 ,ref_pos3 };
	}
}

void ARAP::calc_all_cot_theta() {
	size_t index;
	vecf3 pos1, pos2, pos3;
	vecf3 a, b, c;
	float cot_A, cot_B, cot_C;
	cot_theta_.reserve(heMesh->Polygons().size());

	for (auto triangle : heMesh->Polygons()) {
		index = heMesh->Index(triangle);
		auto boundary_vertice = triangle->BoundaryVertice();
		pos1 = boundary_vertice[0]->pos;
		pos2 = boundary_vertice[1]->pos;
		pos3 = boundary_vertice[2]->pos;
		a = pos2 - pos1;
		b = pos3 - pos1;
		c = pos3 - pos2;

		cot_C = a.cos_theta(b) / a.sin_theta(b);
		cot_B = -a.cos_theta(c) / a.sin_theta(c);
		cot_A = b.cos_theta(c) / b.sin_theta(c);
		cot_theta_[index] = { cot_A,cot_C ,cot_B };
	}
}

ComputationInfo ARAP::sparse_matrix_preprocess() {
	size_t col_num = heMesh->NumVertices() - 1;
	size_t row_index = 0;
	int order_in_triangle_boundary_vertice;
	V* pos1, * pos2, * pos3;
	std::vector<Triplet<float>> tripletlist;
	float coff_0, coff_3, coff_v;
	size_t index;
	std::unordered_map<size_t, float> adj_vertices_map_coff;
	A_saprse_.resize(col_num, col_num);
	
	size_t cnt = 0;
	for (auto v : heMesh->Vertices()) {
		if (heMesh->Index(v) == fixed_index_[0]) {
			continue;
		}
		else {
			vertices_map_row_num_[heMesh->Index(v)] = cnt;
			cnt++;
		}
	}

	for (auto v : heMesh->Vertices()) {
		if (heMesh->Index(v) == fixed_index_[0]) {
			continue;
		}

		row_index = vertices_map_row_num_[heMesh->Index(v)];
		coff_v = 0.f;
		adj_vertices_map_coff.clear();

		for (auto triangle : v->AdjPolygons()) {
			if (triangle == nullptr) {
				continue;
			}

			index = heMesh->Index(triangle);
			auto boundary_vertice = triangle->BoundaryVertice();

			if (boundary_vertice[0] == v) {
				order_in_triangle_boundary_vertice = 0;
			}
			else if (boundary_vertice[1] == v) {
				order_in_triangle_boundary_vertice = 1;
			}
			else {
				order_in_triangle_boundary_vertice = 2;
			}

			pos1 = boundary_vertice[order_in_triangle_boundary_vertice];
			pos2 = boundary_vertice[(order_in_triangle_boundary_vertice + 1) % 3];
			pos3 = boundary_vertice[(order_in_triangle_boundary_vertice + 2) % 3];

			if (adj_vertices_map_coff.find(heMesh->Index(pos2)) == adj_vertices_map_coff.end()) {
				adj_vertices_map_coff[heMesh->Index(pos2)] = -cot_theta_[index][order_in_triangle_boundary_vertice];
			}
			else {
				adj_vertices_map_coff[heMesh->Index(pos2)] += -cot_theta_[index][order_in_triangle_boundary_vertice];
			}

			if (adj_vertices_map_coff.find(heMesh->Index(pos3)) == adj_vertices_map_coff.end()) {
				adj_vertices_map_coff[heMesh->Index(pos3)] = -cot_theta_[index][(order_in_triangle_boundary_vertice + 2) % 3];
			}
			else {
				adj_vertices_map_coff[heMesh->Index(pos3)] += -cot_theta_[index][(order_in_triangle_boundary_vertice + 2) % 3];
			}

			auto ref_triangle = ref_triangles_[index];
			coff_0 = cot_theta_[index][order_in_triangle_boundary_vertice];
			coff_3 = cot_theta_[index][(order_in_triangle_boundary_vertice + 2) % 3];

			coff_v = coff_v + coff_0 + coff_3;
		}
		tripletlist.push_back(Triplet<float>(row_index, row_index, coff_v));

		for (auto iter : adj_vertices_map_coff) {
			if (iter.first != fixed_index_[0]) {
				tripletlist.push_back(Triplet<float>(row_index, vertices_map_row_num_[iter.first], iter.second));
			}
		}
	}

	A_saprse_.setFromTriplets(tripletlist.begin(), tripletlist.end());
	solver_sparse_.compute(A_saprse_);
	std::cout << solver_sparse_.info() << "\n";
	//std::cout << A_saprse_ << endl;

	return solver_sparse_.info();
}

void ARAP::local_phase() {
	MatrixXf A(2, 2);
	size_t index;
	vecf3 u0, u1, u2;
	pointf2 x0, x1, x2;
	float cot0, cot1, cot2;
	vector<V*> boundary_vertice;
	vector<pointf2> ref_triangle;
	std::vector<float> cot_theta;
	St_.clear();

	for (auto triangle : heMesh->Polygons()) {
		index = heMesh->Index(triangle);
		boundary_vertice = triangle->BoundaryVertice(); 
		u0 = boundary_vertice[0]->pos;
		u1 = boundary_vertice[1]->pos;
		u2 = boundary_vertice[2]->pos;

		ref_triangle = ref_triangles_[index];
		x0 = ref_triangle[0];
		x1 = ref_triangle[1];
		x2 = ref_triangle[2];

		cot_theta = cot_theta_[index];
		cot0 = cot_theta[0];
		cot1 = cot_theta[1];
		cot2 = cot_theta[2];

		A(0, 0) = cot0 * (u0[0] - u1[0]) * (x0[0] - x1[0]) + cot1 * (u1[0] - u2[0]) * (x1[0] - x2[0]) + cot2 * (u2[0] - u0[0]) * (x2[0] - x0[0]);
		A(0, 1) = cot0 * (u0[0] - u1[0]) * (x0[1] - x1[1]) + cot1 * (u1[0] - u2[0]) * (x1[1] - x2[1]) + cot2 * (u2[0] - u0[0]) * (x2[1] - x0[1]);
		A(1, 0) = cot0 * (u0[1] - u1[1]) * (x0[0] - x1[0]) + cot1 * (u1[1] - u2[1]) * (x1[0] - x2[0]) + cot2 * (u2[1] - u0[1]) * (x2[0] - x0[0]);
		A(1, 1) = cot0 * (u0[1] - u1[1]) * (x0[1] - x1[1]) + cot1 * (u1[1] - u2[1]) * (x1[1] - x2[1]) + cot2 * (u2[1] - u0[1]) * (x2[1] - x0[1]);
		//std::cout << "A :\n" << A << std::endl;

		JacobiSVD<Eigen::MatrixXf> svd(A, ComputeThinU | ComputeThinV);
		MatrixXf V = svd.matrixV(), U = svd.matrixU();
		MatrixXf J = U * V.transpose();
		
		//std::cout << "J :\n" << J << std::endl;
		St_.push_back({ J(0, 0),J(0, 1),J(1, 0),J(1, 1) });
	}
}

void ARAP::global_phase() {
	size_t col_num = heMesh->NumVertices() - 1;
	size_t row_index = 0;
	int order_in_triangle_boundary_vertice;
	V* pos1, * pos2, * pos3;
	pointf2 ref_pos1, ref_pos2, ref_pos3;

	size_t index;
	VectorXf bx(col_num), by(col_num);
	
	std::unordered_map<size_t, V*> boundary_vertices_;

	for (auto v : heMesh->Vertices()) {
		if (heMesh->Index(v) == fixed_index_[0]) {
			continue;
		}

		row_index = vertices_map_row_num_[heMesh->Index(v)];
		bx[row_index] = 0.f;
		by[row_index] = 0.f;

		for (auto triangle : v->AdjPolygons()) {
			if (triangle == nullptr) {
				continue;
			}

			index = heMesh->Index(triangle);
			auto boundary_vertice = triangle->BoundaryVertice();

			if (boundary_vertice[0] == v) {
				order_in_triangle_boundary_vertice = 0;
			}
			else if (boundary_vertice[1] == v) {
				order_in_triangle_boundary_vertice = 1;
			}
			else {
				order_in_triangle_boundary_vertice = 2;
			}

			pos1 = boundary_vertice[order_in_triangle_boundary_vertice];
			pos2 = boundary_vertice[(order_in_triangle_boundary_vertice + 1) % 3];
			pos3 = boundary_vertice[(order_in_triangle_boundary_vertice + 2) % 3];
			auto ref_triangle = ref_triangles_[index];
			ref_pos1 = ref_triangle[order_in_triangle_boundary_vertice];
			ref_pos2 = ref_triangle[(order_in_triangle_boundary_vertice + 1) % 3];
			ref_pos3 = ref_triangle[(order_in_triangle_boundary_vertice + 2) % 3];

			bx[row_index] += cot_theta_[index][order_in_triangle_boundary_vertice] * (St_[index][0] * (ref_pos1[0] - ref_pos2[0]) + St_[index][1] * (ref_pos1[1] - ref_pos2[1]))
				+ cot_theta_[index][(order_in_triangle_boundary_vertice + 2) % 3] * (St_[index][0] * (ref_pos1[0] - ref_pos3[0]) + St_[index][1] * (ref_pos1[1] - ref_pos3[1]));

			by[row_index] += cot_theta_[index][order_in_triangle_boundary_vertice] * (St_[index][2] * (ref_pos1[0] - ref_pos2[0]) + St_[index][3] * (ref_pos1[1] - ref_pos2[1]))
				+ cot_theta_[index][(order_in_triangle_boundary_vertice + 2) % 3] * (St_[index][2] * (ref_pos1[0] - ref_pos3[0]) + St_[index][3] * (ref_pos1[1] - ref_pos3[1]));

			if (heMesh->Index(pos2) == fixed_index_[0]) {
				bx[row_index] += cot_theta_[index][order_in_triangle_boundary_vertice] * pos2->pos[0];
				by[row_index] += cot_theta_[index][order_in_triangle_boundary_vertice] * pos2->pos[1];
			}

			if (heMesh->Index(pos3) == fixed_index_[0]) {
				bx[row_index] += cot_theta_[index][(order_in_triangle_boundary_vertice + 2) % 3] * pos3->pos[0];
				by[row_index] += cot_theta_[index][(order_in_triangle_boundary_vertice + 2) % 3] * pos3->pos[1];
			}
		}
	}
	//std::cout << bx << endl;
	VectorXf x_0 = solver_sparse_.solve(bx);
	VectorXf x_1 = solver_sparse_.solve(by);
	//std::cout << x_0 << endl;

	for (auto v : heMesh->Vertices()) {
		v->pos[2] = 0;
		if (heMesh->Index(v) == fixed_index_[0]) {
			continue;
		}

		v->pos[0] = x_0[vertices_map_row_num_[heMesh->Index(v)]];
		v->pos[1] = x_1[vertices_map_row_num_[heMesh->Index(v)]];
	}
	//std::cout << b << endl;
}

void ARAP::local_global_algorithm() {
	size_t index;
	size_t cnt = 0;
	auto para = Paramaterize::New(triMesh);
	para->Run(true);
	;
	for (auto v : heMesh->Vertices()) {
		index = heMesh->Index(v);
		v->pos[0] = para->texcoords_[heMesh->Index(v)][0];
		v->pos[1] = para->texcoords_[heMesh->Index(v)][1];
	}

	do {
		local_phase();
		global_phase();
		cnt++;
	} while (cnt <6);
}