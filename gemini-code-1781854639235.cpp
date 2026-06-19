#include <emscripten/bind.h>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace emscripten;

// Structure matching the data structure sent back to JavaScript
struct VoxelNode {
    float screenX;
    float screenY;
    float depth;
    int r;
    int g;
    int b;
    float size;
};

// Represents a single point in our game states
struct VoxelState {
    int r, g, b;
    float h;
};

// Global states initialized by the game
std::vector<VoxelState> state1;
std::vector<VoxelState> state2;
int gridSize = 45;

// Initializes the procedural 3D shapes inside C++ memory space
void initEngine(int size) {
    gridSize = size;
    state1.clear();
    state2.clear();

    for (int z = 0; z < gridSize; z++) {
        for (int x = 0; x < gridSize; x++) {
            // State 1: Concentric Pyramid (Neon Emerald)
            float cx1 = gridSize / 2.0f;
            float cz1 = gridSize / 2.0f;
            float dist1 = std::max(std::abs(x - cx1), std::abs(z - cz1));
            float h1 = std::max(0.0f, (gridSize / 2.0f - dist1) * 4.0f);
            state1.push_back({0, static_cast<int>(255 - (dist1 * 10)), static_cast<int>(100 + (dist1 * 5)), h1});

            // State 2: Sine Wave Valley (Cosmic Ruby)
            float h2 = (std::sin(x * 0.3f) * std::cos(z * 0.3f)) * 15.0f + 15.0f;
            state2.push_back({static_cast<int>(200 + (std::sin(x) * 55)), 0, static_cast<int>(100 + (std::cos(z) * 55)), h2});
        }
    }
}

// Heavy lifting mathematical 3D remapping loop
std::vector<VoxelNode> computeFrame(float progress, float angleDegrees, float canvasWidth, float canvasHeight, float voxelSpacing) {
    float t = progress / 100.0f;
    float angle = angleDegrees * (M_PI / 180.0f);
    
    std::vector<VoxelNode> nodes;
    nodes.reserve(gridSize * gridSize);

    // Calculate explosion offsets mid-morph
    float explosionFactor = std::sin(t * M_PI) * 45.0f;
    float dynamicSpacing = voxelSpacing + (explosionFactor * 0.1f);

    // 1. Calculate transformed coordinates
    for (int z = 0; z < gridSize; z++) {
        for (int x = 0; x < gridSize; x++) {
            int index = z * gridSize + x;
            VoxelState v1 = state1[index];
            VoxelState v2 = state2[index];

            // Linear interpolation of height and colors
            float currentH = v1.h * (1.0f - t) + v2.h * t;
            int r = static_cast<int>(v1.r * (1.0f - t) + v2.r * t);
            int g = static_cast<int>(v1.g * (1.0f - t) + v2.g * t);
            int b = static_cast<int>(v1.b * (1.0f - t) + v2.b * t);

            // Orbit rotation math around center
            float rx = (x - gridSize / 2.0f) * std::cos(angle) - (z - gridSize / 2.0f) * std::sin(angle);
            float rz = (x - gridSize / 2.0f) * std::sin(angle) + (z - gridSize / 2.0f) * std::cos(angle);

            // Project 3D space to 2D Screen space
            float screenX = (canvasWidth / 2.0f) + (rx * dynamicSpacing);
            float screenY = (canvasHeight / 1.7f) + (rz * dynamicSpacing * 0.5f) - (currentH + explosionFactor);
            float size = std::max(1.0f, 4.0f - (rz * 0.03f));

            nodes.push_back({screenX, screenY, rz, r, g, b, size});
        }
    }

    // 2. Depth sort (Painter's Algorithm) so objects in the back are drawn first
    std::sort(nodes.begin(), nodes.end(), [](const VoxelNode& a, const VoxelNode& b) {
        return a.depth > b.depth;
    });

    return nodes;
}

// Emscripten definitions to bridge types seamlessly over to JavaScript arrays
EMSCRIPTEN_BINDINGS(vox_verse_core) {
    value_object<VoxelNode>("VoxelNode")
        .field("screenX", &VoxelNode::screenX)
        .field("screenY", &VoxelNode::screenY)
        .field("depth", &VoxelNode::depth)
        .field("r", &VoxelNode::r)
        .field("g", &VoxelNode::g)
        .field("b", &VoxelNode::b)
        .field("size", &VoxelNode::size);

    register_vector<VoxelNode>("VectorVoxelNode");

    function("initEngine", &initEngine);
    function("computeFrame", &computeFrame);
}