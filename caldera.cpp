#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <random>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/pointInstancer.h>
#include <pxr/usd/usdGeom/xformable.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/sdf/layer.h>

using namespace pxr;

// Function to edit a USD stage
UsdStageRefPtr EditStage(const std::string& editUsdFile, const std::string& sourceUsdFile) {
    // Create a new layer and open it
    SdfLayerRefPtr layer = SdfLayer::CreateNew(editUsdFile);
    if (!layer) {
        std::cerr << "Failed to create the new layer: " << editUsdFile << std::endl;
        return nullptr;
    }

    // Open the stage
    UsdStageRefPtr stage = UsdStage::Open(layer);
    if (!stage) {
        std::cerr << "Failed to open stage for layer: " << editUsdFile << std::endl;
        return nullptr;
    }

    // Add the existing USD file as a sublayer
    layer->GetSubLayerPaths().push_back(sourceUsdFile);

    // Set stage metadata
    stage->SetMetadata(SdfFieldKeys->Comment, 
        "USD file created with the example script from the Caldera data set.");

    return stage;
}

// Function to count prims and related attributes
std::map<std::string, int> CountPrims(const UsdStageRefPtr& stage) {
    std::map<std::string, int> info {
        {"prims", 0},
        {"meshes", 0},
        {"verts", 0},
        {"prototypes", 0},
        {"instances", 0}
    };

    // Traverse the stage
    for (auto prim : stage->Traverse()) {
        info["prims"]++;

        // Count meshes and vertices
        if (prim.IsA<UsdGeomMesh>()) {
            info["meshes"]++;
            UsdGeomMesh mesh(prim);
            VtArray<GfVec3f> points;
            if (mesh.GetPointsAttr().Get(&points)) {
                info["verts"] += points.size();
            }
        }

        // Count instances and prototypes
        if (prim.IsA<UsdGeomPointInstancer>()) {
            UsdGeomPointInstancer instancer(prim);
            VtArray<int> protoIndices;
            if (instancer.GetProtoIndicesAttr().Get(&protoIndices)) {
                info["prototypes"] += protoIndices.size();
            }
            VtArray<GfVec3f> positions;
            if (instancer.GetPositionsAttr().Get(&positions)) {
                info["instances"] += positions.size();
            }
        }
    }

    return info;
}

// Function to set stage metadata
void SetStageInfo(const UsdStageRefPtr& stage, const std::string& comment) {
    // Set the up axis to Z
    UsdGeomSetStageUpAxis(stage, UsdGeomTokens->z);

    // Set the scale to inches
    UsdGeomSetStageMetersPerUnit(stage, UsdGeomLinearUnits->inches);

    // Set default frame rate and time codes
    stage->SetFramesPerSecond(30);
    stage->SetTimeCodesPerSecond(30);
    stage->SetStartTimeCode(0);
    stage->SetEndTimeCode(900);

    // Set comment metadata
    if (!comment.empty()) {
        stage->SetMetadata(SdfFieldKeys->Comment, comment);
    }
}

// Entry point of the program
int main() {
    std::cout << "Opening mp_wz_island..." << std::endl;

    std::string editUsdFile = "./caldera.usda";
    std::string sourceUsdFile = "./map_source/mp_wz_island.usd";

    UsdStageRefPtr stage = EditStage(editUsdFile, sourceUsdFile);
    if (!stage) {
        return 1; // Exit if stage creation fails
    }

    std::cout << "Collecting statistics..." << std::endl;
    auto stats = CountPrims(stage);

    for (const auto& entry : stats) {
        std::cout << entry.first << ": " << entry.second << std::endl;
    }

    std::cout << "Done!" << std::endl;
    return 0;
}
