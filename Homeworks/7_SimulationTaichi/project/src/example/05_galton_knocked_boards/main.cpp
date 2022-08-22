//88-Line 2D Moving Least Squares Material Point Method (MLS-MPM)[with comments]

//#define TC_IMAGE_IO       // Uncomment this line for image exporting functionality
#include <taichi.h>         // Note: You DO NOT have to install taichi or taichi_mpm.
using namespace taichi;     // You only need [taichi.h] - see below for instructions.

using Vec = Vector2;
using Mat = Matrix2;

const int n = 80;           //grid resolution (cells)
const int window_size = 800;
const real dt = 1e-4_f;
const real frame_dt = 1e-3_f;
const real dx = 1.0_f / n;
const real inv_dx = 1.0_f / dx;

// Snow material properties
auto particle_mass = 1.0_f;
auto vol = 1.0_f;           // Particle Volume
auto hardening = 100.0_f;    // Snow hardening factor
auto E = 1e4_f;             // Young's Modulus
auto nu = 0.2_f;            // Poisson ratio
bool plastic = true;

// Initial Lam¨¦ parameters
real mu_0 = E / (2 * (1 + nu));
real lambda_0 = E * nu / ((1 + nu) * (1 - 2 * nu));

/***********************************(1)*****************************************/
struct Particle {
    Vec x, v;       // Position and velocity
    Mat F;          // Deformation gradient
    Mat C;          // Affine momentum from APIC
    real Jp;        // Determinant of the deformation gradient (i.e. volume)
    int c;          // Color
    int ptype;      // 0: fluid 1: jelly 2: snow
    Particle(Vec x, int c, Vec v = Vec(0), int ptype = 2) :
        x(x),
        v(v),
        F(1),
        C(0),
        Jp(1),
        c(c),
        ptype(ptype) {}
};
////////////////////////////////////////////////////////////////////////////////
std::vector<Particle> particles;

Vector3 grid[n + 1][n + 1];          // Vector3: [velocity_x, velocity_y, mass], node_res = cell_res + 1
int fluid_height[18] = { 0,0,0,1,3,48,148,263,296,331,252,133,56,5,0,0,0,0 };

void advance(real dt) {
    std::memset(grid, 0, sizeof(grid));                                 // Reset grid
    // P2G
    for (auto& p : particles) {
        Vector2i base_coord = (p.x * inv_dx - Vec(0.5_f)).cast<int>();  //element-wise floor
        Vec fx = p.x * inv_dx - base_coord.cast<real>();

        // Quadratic kernels  [http://mpm.graphics   Eqn. 123, with x=fx, fx-1,fx-2]
        Vec w[3] = {
            Vec(0.5) * sqr(Vec(1.5) - fx),
            Vec(0.75) - sqr(fx - Vec(1.0)),
            Vec(0.5) * sqr(fx - Vec(0.5))
        };
        /***********************************(2)*****************************************/
                // Compute current Lam¨¦ parameters [http://mpm.graphics Eqn. 86]
        auto e = std::exp(hardening * (1.0_f - p.Jp));
        if (p.ptype == 1) 
            e = 1.5;

        auto mu = mu_0 * e ;
        auto lambda = lambda_0 * e;
        if (p.ptype == 0)
            mu = 0; 

        ////////////////////////////////////////////////////////////////////////////////
        real J = determinant(p.F);          //Current volume

        //Polar decomposition for fixed corotated model
        Mat r, s;
        polar_decomp(p.F, r, s);

        // [http://mpm.graphics Paragraph after Eqn. 176]
        real Dinv = 4 * inv_dx * inv_dx;
        // [http://mpm.graphics Eqn. 52]
        auto PF = (2 * mu * (p.F - r) * transposed(p.F) + lambda * (J - 1) * J);

        // Cauchy stress times dt and inv_dx
        auto stress = -(dt * vol) * (Dinv * PF);

        // Fused APIC momentum + MLS-MPM stress contribution
        // See http://taichi.graphics/wp-content/uploads/2019/03/mls-mpm-cpic.pdf
        // Eqn 29
        auto affine = stress + particle_mass * p.C;

        // P2G
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++) {                       // Scatter to grid
                auto dpos = (Vec(i, j) - fx) * dx;
                Vector3 mv(p.v * particle_mass, particle_mass); // Translational momentum
                grid[base_coord.x + i][base_coord.y + j] += w[i].x * w[j].y * (mv + Vector3(affine * dpos, 0));
            }
    }

    //For all grid nodes
    for (int i = 0; i <= n; i++)
        for (int j = 0; j <= n; j++) {
            auto& g = grid[i][j];
            if (g[2] > 0) {                                         // No need for epsilon here
                g /= g[2];                                          // Normalize by mass
                g += dt * Vector3(0, -5000, 0);                      // Gravity
                //g += dt * (Vector3(g.y * 100, -g.x * 100, 0) + Vector3(0, -50, 0));                      // Gravity
                real boundary = 0.05;                               // Boundary thick
                real x = (real)i / n, y = real(j) / n;              // Node coord

                //Sticky boundary
                if (x < boundary || x > 1 - boundary || y > 1 - boundary )
                    g = Vector3(0);
                //Separate boundary
                if (y < boundary) g[1] = std::max(0.0_f, g[1]);

                if (y > 0.43330127 && y < 0.9296 && x > 0.065 && x < 0.91) {
                    int cur_row = (y - 0.43330127) / 0.04330127;

                    if (cur_row % 2 == 0) {
                        int cur_col = (x - 0.09) / 0.05;
                        real dis = Vec(0.1 + cur_col * 0.05 - x, 0.44330127 + cur_row * 0.04330127 - y).length();
                        if (dis < 0.005)
                            g = Vector3(0);
                    }
                    else {
                        int cur_col = (x - 0.065) / 0.05;
                        real dis = Vec(0.075 + cur_col * 0.05 - x, 0.4433017 + cur_row * 0.04330127 - y).length();
                        if (dis < 0.005)
                            g = Vector3(0);
                    }
                }

                if (y > 0.05 && y < 0.4483) {
                    int cur_col = (x - 0.049) / 0.05;
                    if (x > 0.049 + cur_col * 0.05 && x < 0.051 + cur_col * 0.05) {
                        g = Vector3(0);    
                    }
                    
                    if (y < 0.3) {
                        particles.clear();
                        fluid_height[cur_col]++;
                        return;
                    }
                }
            }
        }

    // G2P
    for (auto& p : particles) {                                                 // Grid to particle
        Vector2i base_coord = (p.x * inv_dx - Vec(0.5_f)).cast<int>();          // Element-wise floor
        Vec fx = p.x * inv_dx - base_coord.cast<real>();
        Vec w[3] = {
            Vec(0.5) * sqr(Vec(1.5) - fx),
            Vec(0.75) - sqr(fx - Vec(1.0)),
            Vec(0.5) * sqr(fx - Vec(0.5))
        };

        p.C = Mat(0);
        p.v = Vec(0);
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++) {
                auto dpos = (Vec(i, j) - fx);
                auto grid_v = Vec(grid[base_coord.x + i][base_coord.y + j]);
                auto weight = w[i].x * w[j].y;
                p.v += weight * grid_v;                                         // Velocity
                p.C += 4 * inv_dx * Mat::outer_product(weight * grid_v, dpos);  // APIC C
            }
        p.x += dt * p.v;                       // Advection
        auto F = (Mat(1) + dt * p.C) * p.F;    // MLS-MPM F-update
/***********************************(3)*****************************************/
        if (p.ptype == 0) {
            p.F = Mat(1) * sqrt(determinant(F));
        }
        else if (p.ptype == 1) {
            p.F = F;
        }
        else if (p.ptype == 2) {
            Mat svd_u, sig, svd_v;
            svd(F, svd_u, sig, svd_v);
            for (int i = 0; i < 2 * int(plastic); i++) {     // Snow Plasticity
                sig[i][i] = clamp(sig[i][i], 1.0_f - 2.5e-2_f, 1.0_f + 7.5e-3_f);
            }

            real oldJ = determinant(F); F = svd_u * sig * transposed(svd_v);
            real Jp_new = clamp(p.Jp * oldJ / determinant(F), 0.6_f, 20.0_f);
            p.Jp = Jp_new; p.F = F;
        }
        ////////////////////////////////////////////////////////////////////////////////
    }
}
/***********************************(4)*****************************************/
void add_object_rectangle(Vec v1, Vec v2, int c, int num = 500, Vec velocity = Vec(0.0_f), int ptype = 2)
{
    Vec box_min(min(v1.x, v2.x), min(v1.y, v2.y)), box_max(max(v1.x, v2.x), max(v1.y, v2.y));
    int i = 0;
    while (i < num) {
        auto pos = Vec::rand();
        if (pos.x > box_min.x && pos.y > box_min.y && pos.x < box_max.x && pos.y < box_max.y) {
            particles.push_back(Particle(pos, c, velocity, ptype));
            i++;
        }
    }
}

void add_object_circle(Vec center, real radius, int c, int num = 500, Vec velocity = Vec(0.0_f), int ptype = 2)
//input: circle center & radius, color, number of particles, initial velocity
{
    int i = 0;
    auto radius_square = radius * radius;
    while (i < num) {
        auto pos = (Vec::rand() * 2.0_f - Vec(1)) * radius;
        auto dis = pos.x * pos.x + pos.y * pos.y;
        if (dis < radius_square) {
            particles.push_back(Particle(pos + center, c, velocity, ptype));
            i++;
        }
    }
}

void add_jet() {
    add_object_circle(Vec(0.5, 0.91), 0.01, 0xFFFAFA, 1000, Vec(0, 0.0), 1);
    //particles.push_back(Particle(Vec(0.5, 0.92), 0xFFFAFA, Vec(0, 0.0), 1));
    //add_object_rectangle(Vec(0.5, 0.5), Vec(0.51, 0.51), 0x87CEFA, 2, Vec(0, 10), 1);
}
////////////////////////////////////////////////////////////////////////////////
// Main Loop
int main() {
    GUI gui("Galton knocked boards", window_size, window_size);
    /***********************************(5)*****************************************/
    //add_object_rectangle(Vec(0.04, 0.04), Vec(0.96_f, 0.2_f), 0xFFFAFA, 5000, Vec(0.0), 2);
    //add_object_circle(Vec(0.5, 0.35), 0.01, 0xFFFAFA, 200, Vec(0, 0.0), 1);
    //add_object_circle(Vec(0.6, 0.35), 0.01, 0xFFFAFA, 200, Vec(-10, 0.0), 1);
    //add_object_circle(Vec(0.2, 0.5), 0.02, 0xFFFAFA, 500, Vec(100, 0.0), 2);
    //add_object(Vec(0.45, 0.65), 0xFFFAFA, 2);
    //add_object(Vec(0.55, 0.85), 0xED553B, 1);
    ////////////////////////////////////////////////////////////////////////////////


    auto& canvas = gui.get_canvas(); 
    int f = 0;

    for (int cnt = 0;; cnt++) {
        advance(dt);                                                                // Advance simulation
        if (cnt % int(frame_dt / dt) == 0) {                                          // Visualize frame
            canvas.clear(0x112F41);                                                 // Clear background
            canvas.rect(Vec(0.05), Vec(0.95)).radius(2).color(0x4FB99F).close();    // Box

            for (int i = 0; i < 17; i++) {
                canvas.rect(Vec(0.099 + i * 0.05, 0.05), Vec(0.101 + i * 0.05, 0.44330127)).radius(1).color(0x4FB99F).close();
            }

            for (int i = 0; i < 18; i++) {
                canvas.rect(Vec(0.053 + i * 0.05, 0.053), Vec(0.097 + i * 0.05, 0.05 + fluid_height[i] * 0.001)).radius(1).color(0xFFFAFA).close();
            }

            //canvas.rect(Vec(0.49, 0.05), Vec(0.51, 0.2)).radius(2).color(0x4FB99F).close();

            for (int k = 2; k < 12; k++) {
                if (k % 2 == 1) {
                    for (int j = 0; j < 17; j++) {
                        canvas.circle(Vec(0.1 + j * 0.05, 0.4 + k * 0.04330127)).radius(4).color(0xFFF30B);
                    }
                }
                else {
                    for (int j = 0; j < 18; j++) {
                        canvas.circle(Vec(0.075 + j * 0.05, 0.4 + k * 0.04330127)).radius(4).color(0xFFF30B);
                    }
                }
            }

            //Particles
            for (auto p : particles) {
                canvas.circle(p.x).radius(1).color(p.c);
            }

            gui.update();                                                           // Update image

            if (particles.size() == 0){              
                add_jet();
            }

            // canvas.img.write_as_image(fmt::format("tmp/{:05d}.png", f++));       // Write to disk(optional)
        }
    }
}

//----------------------------------------------------------------------------
//This sample shows how to simulate Galton's pegboard experiment with taichi