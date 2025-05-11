#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/pointInstancer.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/vt/value.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/variantSets.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <cstdlib>

using namespace pxr;

// Structure to hold counts of various prim types, mirroring the Python dictionary
struct PrimCounts {
    int prims = 0;           // Total number of prims
    int meshes = 0;          // Number of mesh prims
    int verts = 0;           // Total vertices in meshes
    int prototypes = 0;      // Number of point instancer prototypes
    int instances = 0;       // Number of instances
    std::map<std::string, int> entities; // Counts of entities by classname
};

// Sets stage metadata such as up axis, scale, and time settings
void SetStageInfo(UsdStageRefPtr stage, const std::string& comment) {
    // Set the up axis to Z, aligning with 3D coordinate conventions
    UsdGeomSetStageUpAxis(stage, UsdGeomTokens->z);
    // Set the scale to inches for unit consistency
    UsdGeomSetStageMetersPerUnit(stage, UsdGeomLinearUnits::inches);
    // Set frames per second to 30, common for animations
    stage->SetFramesPerSecond(30);
    // Set time codes per second to 30 for timeline consistency
    stage->SetTimeCodesPerSecond(30);
    // Define the timeline range from 0 to 900 frames
    stage->SetStartTimeCode(0);
    stage->SetEndTimeCode(900);
    // Add a comment to the stage metadata if provided
    if (!comment.empty()) {
        stage->SetMetadata(SdfFieldKeys->Comment, comment);
    }
}

// Creates a new USD stage, adds a source file as a sublayer, and sets stage info
UsdStageRefPtr EditStage(const std::string& edit_usd_file, const std::string& source_usd_file) {
    // Create a new layer for the output USD file
    SdfLayerRefPtr layer = SdfLayer::CreateNew(edit_usd_file);
    if (!layer) {
        std::cerr << "Failed to create layer: " << edit_usd_file << std::endl;
        return nullptr;
    }
    // Open a stage using the new layer
    UsdStageRefPtr stage = UsdStage::Open(layer);
    if (!stage) {
        std::cerr << "Failed to open stage with layer: " << edit_usd_file << std::endl;
        return nullptr;
    }
    // Add the source USD file as a sublayer to include its data
    layer->GetSubLayerPaths().push_back(source_usd_file);
    // Configure stage metadata
    SetStageInfo(stage, "usd file created with the example script from the caldera data set.");
    return stage;
}

// Counts prims, meshes, vertices, instances, and entities in the stage
PrimCounts CountPrims(UsdStageRefPtr stage) {
    PrimCounts counts;
    // Traverse all prims, including instance proxies, to count various types
    for (const auto& prim : UsdPrimRange::Stage(stage, UsdTraverseInstanceProxies(UsdPrimAllPrimsPredicate))) {
        counts.prims++;
        // Check for point instancers and count their prototypes and instances
        if (prim.IsA<UsdGeomPointInstancer>()) {
            UsdGeomPointInstancer instancer(prim);
            auto protoIndicesAttr = instancer.GetProtoIndicesAttr();
            if (protoIndicesAttr) {
                VtIntArray protoIndices;
                protoIndicesAttr.Get(&protoIndices);
                counts.prototypes += protoIndices.size();
            }
            auto positionsAttr = instancer.GetPositionsAttr();
            if (positionsAttr) {
                VtVec3fArray positions;
                positionsAttr.Get(&positions);
                counts.instances += positions.size();
            }
        }
        // Count prims that are instances
        if (prim.IsInstance()) {
            counts.instances++;
        }
        // Count meshes and their vertices
        if (prim.IsA<UsdGeomMesh>()) {
            counts.meshes++;
            UsdGeomMesh mesh(prim);
            auto pointsAttr = mesh.GetPointsAttr();
            if (pointsAttr) {
                VtVec3fArray points;
                pointsAttr.Get(&points);
                counts.verts += points.size();
            }
        } 
        // Count cubes with specific custom data as entities
        else if (prim.IsA<UsdGeomCube>()) {
            auto atvi_classname = prim.GetCustomDataByKey("atvi:classname");
            if (atvi_classname.IsValid()) {
                std::string classname = atvi_classname.Get<std::string>();
                if (classname.find("scriptable_") == 0) {
                    classname = "scriptable";
                }
                counts.entities[classname]++;
            }
        }
    }
    return counts;
}

// Sets the active state of specified prims
void SetPrimsEnabled(UsdStageRefPtr stage, const std::vector<SdfPath>& prim_paths, bool active = true) {
    for (const auto& path : prim_paths) {
        UsdPrim prim = stage->GetPrimAtPath(path);
        if (prim) {
            prim.SetActive(active);
        }
    }
}

// Sets variant sets to a specified state if available
void SetVariantSets(UsdStageRefPtr stage, const std::string& new_state) {
    for (const auto& prim : stage->Traverse()) {
        if (prim.HasVariantSets()) {
            UsdVariantSets variantSets = prim.GetVariantSets();
            std::vector<std::string> names;
            variantSets.GetNames(&names);
            for (const auto& variant_name : names) {
                UsdVariantSet variantSet = variantSets.GetVariantSet(variant_name);
                std::vector<std::string> options = variantSet.GetVariantNames();
                if (std::find(options.begin(), options.end(), new_state) != options.end()) {
                    std::cout << "prim: " << prim.GetPath() << " has variant set " << variant_name << std::endl;
                    std::string before = variantSet.GetVariantSelection();
                    variantSet.SetVariantSelection(new_state);
                    std::string after = variantSet.GetVariantSelection();
                    std::cout << "choices: ";
                    for (const auto& opt : options) {
                        std::cout << opt << " ";
                    }
                    std::cout << "was: " << before << " now is: " << after << std::endl;
                }
            }
        }
    }
}

// Adds a sublayer to the stage’s root layer
void AddSublayer(UsdStageRefPtr stage, const std::string& layer_path) {
    SdfLayerRefPtr rootLayer = stage->GetRootLayer();
    rootLayer->GetSubLayerPaths().push_back(layer_path);
}

// Inspects player path data, such as breadcrumbs
void InspectPlayerPaths(UsdStageRefPtr stage) {
    std::cout << stage->GetRootLayer()->GetRealPath() << std::endl;
    // Access the breadcrumbs prim
    UsdPrim root = stage->GetPrimAtPath(SdfPath("/players/breadcrumbs"));
    if (!root) {
        std::cerr << "Prim not found: /players/breadcrumbs" << std::endl;
        return;
    }
    // Get all match prims under breadcrumbs
    std::vector<UsdPrim> matches = root.GetChildren();
    if (matches.empty()) {
        std::cerr << "No matches found under /players/breadcrumbs" << std::endl;
        return;
    }
    // Select a random match
    size_t idx = rand() % matches.size();
    UsdPrim match = matches[idx];
    std::cout << "looking at match " << match.GetName() << std::endl;
    // Get all player prims under the match
    std::vector<UsdPrim> players = match.GetChildren();
    if (players.empty()) {
        std::cerr << "No players found under " << match.GetPath() << std::endl;
        return;
    }
    // Select a random player
    idx = rand() % players.size();
    UsdPrim player = players[idx];
    std::cout << "looking at player " << player.GetName() << std::endl;
    // Use Xformable to access transform properties
    UsdGeomXformable locator(player);
    std::vector<UsdGeomXformOp> ops = locator.GetOrderedXformOps();
    UsdGeomXformOp translate_op;
    for (const auto& op : ops) {
        if (op.GetOpType() == UsdGeomXformOp::TypeTranslate) {
            translate_op = op;
            break;
        }
    }
    if (!translate_op) {
        std::cerr << "No translate op found for player" << std::endl;
        return;
    }
    // Get time samples for the translate operation
    std::vector<double> time_samples;
    translate_op.GetTimeSamples(&time_samples);
    if (time_samples.empty()) {
        std::cerr << "No time samples for translate op" << std::endl;
        return;
    }
    // Process time samples with a time scale of 60
    double time_scale = 60.0;
    double fps = stage->GetFramesPerSecond();
    for (double time_sample : time_samples) {
        double sample_time = time_scale * (time_sample / fps);
        GfVec3d value;
        translate_op.Get(&value, time_sample);
        std::cout << "at " << sample_time << " seconds, the position is " << value << std::endl;
    }
}

// Inspects player endpoint data
void InspectEndpoints(UsdStageRefPtr stage) {
    // Access the endpoints prim
    UsdPrim points_prim = stage->GetPrimAtPath(SdfPath("/players/endpoints/points_0"));
    if (!points_prim) {
        std::cerr << "Prim not found: /players/endpoints/points_0" << std::endl;
        return;
    }
    UsdGeomPoints points(points_prim);
    VtVec3fArray positions;
    points.GetPointsAttr().Get(&positions);
    // Get the time primvar
    UsdGeomPrimvar time_primvar = points.GetPrimvar(UsdTokens->time);
    if (!time_primvar) {
        std::cerr << "No time primvar found" << std::endl;
        return;
    }
    VtFloatArray times;
    time_primvar.Get(&times);
    if (times.size() != positions.size()) {
        std::cerr << "Mismatch in sizes of positions and times" << std::endl;
        return;
    }
    // Find the maximum time and corresponding position
    double max_time = 0.0;
    GfVec3f position_at_max_time;
    for (size_t i = 0; i < times.size(); ++i) {
        double time = times[i];
        if (time > max_time) {
            max_time = time;
            position_at_max_time = positions[i];
        }
    }
    std::cout << "the longest a player lasted was " << (max_time / 1000.0) << " seconds, the position at that time was " << position_at_max_time << std::endl;
}

// Main function demonstrating usage
int main() {
    // Define file paths
    std::string edit_usd_file = "./caldera.usda";
    std::string source_usd_file = "./map_source/mp_wz_island.usd";
    // Create and edit the stage
    UsdStageRefPtr stage = EditStage(edit_usd_file, source_usd_file);
    if (!stage) {
        return 1;
    }
    // Count and display prim statistics
    PrimCounts counts = CountPrims(stage);
    std::cout << "Prims: " << counts.prims << std::endl;
    std::cout << "Meshes: " << counts.meshes << std::endl;
    std::cout << "Verts: " << counts.verts << std::endl;
    std::cout << "Prototypes: " << counts.prototypes << std::endl;
    std::cout << "Instances: " << counts.instances << std::endl;
    std::cout << "Entities:" << std::endl;
    for (const auto& pair : counts.entities) {
        std::cout << "  " << pair.first << ": " << pair.second << std::endl;
    }
    // Save the modified stage
    stage->GetRootLayer()->Save();
    return 0;
}
