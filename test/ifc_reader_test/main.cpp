#include <filesystem>
#include <thread>
#include <vector>
#include <format>

#include <boost/outcome.hpp>

#include <TopoDS_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Shape.hxx>

#include "../ifcgeom/Iterator.h"
#include "../ifcgeom/kernels/opencascade/OpenCascadeConversionResult.h"
#include "../ifcparse/IfcFile.h"

enum class IfcParseError {
    kIfcInitializationFailed,
    kIfcIteratorInitializationFailed,
};

BOOST_OUTCOME_V2_NAMESPACE::result<std::vector<TopoDS_Shape>, IfcParseError, BOOST_OUTCOME_V2_NAMESPACE::policy::terminate> ReadIFCShapes(const std::filesystem::path& a_path, int a_num_threads, const std::set<std::string>& a_exclude_entities) {
    using namespace ifcopenshell;
    using namespace ifcopenshell::geometry;
    using namespace boost;

    std::unique_ptr<IfcParse::IfcFile> ifc_file = std::make_unique<IfcParse::IfcFile>(a_path.string());
    if (!(bool)ifc_file || !ifc_file->good()) {
        return outcome_v2::failure(IfcParseError::kIfcInitializationFailed);
    }

    std::string geometry_kernel = "opencascade";
    geometry::Settings geometry_settings;

    // 来自 ifcConvert 向 stp 转化过程。
    geometry_settings.get<settings::WeldVertices>().value = false;
    geometry_settings.get<settings::UseWorldCoords>().value = true;
    geometry_settings.get<settings::IteratorOutput>().value = settings::NATIVE;

    // 采用文件中单位进行转化。
    geometry_settings.get<settings::ConvertBackUnits>().value = true;

    IfcGeom::entity_filter entity_filter;
    entity_filter.include = false;
    entity_filter.traverse = true;
    if (a_exclude_entities.empty()) {
        // 来自 ifcConvert 向 stp 转化过程。
        entity_filter.entity_names.insert("IfcSpace");
        entity_filter.entity_names.insert("IfcOpeningElement");
    } else {
        entity_filter.entity_names = a_exclude_entities;
    }

    std::vector<IfcGeom::filter_t> filter_funcs;
    filter_funcs.push_back(boost::ref(entity_filter));

    int num_threads = a_num_threads;
    std::unique_ptr<IfcGeom::Iterator> context_iterator = std::make_unique<IfcGeom::Iterator>(geometry_kernel, geometry_settings, ifc_file.get(), filter_funcs, num_threads);

    if (!(bool)context_iterator || !context_iterator->initialize()) {
        return outcome_v2::failure(IfcParseError::kIfcIteratorInitializationFailed);
    }

    std::vector<TopoDS_Shape> result;
    while (true) {
        IfcGeom::Element* geom_object = context_iterator->get();
        if (geom_object != nullptr) {
            auto* o = static_cast<const IfcGeom::BRepElement*>(geom_object);
            std::unique_ptr<IfcGeom::ConversionResultShape> itm(o->geometry().as_compound());
            TopoDS_Shape compound = ((geometry::OpenCascadeShape*)itm.get())->shape();
            result.push_back(compound);
        }

        if (!context_iterator->next()) {
            break;
        }
    }
    return outcome_v2::success(std::move(result));
}

int main() {
    using namespace std;
    
    std::locale::global(std::locale("zh_CN.UTF-8"));

    filesystem::path ifc_dir = LR"(D:\works\tasks\BUGFIX#78415-ifc导入无翼板腹板)";
    ifc_dir = LR"(D:\works\tasks\BUGFIX#96486-ifc导入模型错误)";
    std::string file_name = "3-21";
    file_name = "G5025(3)";
    filesystem::path ifc_path = ifc_dir / (file_name + ".ifc");
    uint32_t n = std::thread::hardware_concurrency();
    if (n == 0) {
        n = 1;
    }
    std::set<std::string> exclude_entities = {"IFCMECHANICALFASTENER", "IFCFASTENER"}; // Example of entities to exclude
    auto res = ReadIFCShapes(ifc_path, n, exclude_entities);
    if (res) {
        TopoDS_Builder builder;
        TopoDS_Compound compound;
        builder.MakeCompound(compound);
        for (const auto& shape : res.value()) {
            builder.Add(compound, shape);
        }

        filesystem::path out_path = ifc_dir / std::format("{}_debug.brep", file_name);
        std::ofstream out_file(out_path);
        BRepTools::Write(compound, out_file);
    }

    return 0;
}
