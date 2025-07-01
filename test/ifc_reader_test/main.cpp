#include "../ifcgeom/Iterator.h"
#include "../ifcgeom/kernels/opencascade/OpenCascadeConversionResult.h"
#include "../ifcparse/Ifc2x3.h"
#include "../ifcparse/Ifc4.h"
#include "../ifcparse/IfcFile.h"

#include <filesystem>
#include <format>
#include <thread>
#include <TopoDS_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Shape.hxx>
#include <vector>

enum class IfcParseError {
    kIfcInitializationFailed,
    kIfcIteratorInitializationFailed,
    kIfcParsingException,
};

static std::string get_native_string(const std::filesystem::path& a_path) {
    auto u8str = a_path.u8string();
    return std::string(reinterpret_cast<const char*>(u8str.data()), u8str.size());
}

static std::optional<int> GetComponentCount(IfcParse::IfcFile& a_file) {
    if (auto product_2x3 = a_file.instances_by_type<Ifc2x3::IfcProduct>()) {
        return product_2x3->size();
    } else if (auto product_4 = a_file.instances_by_type<Ifc4::IfcProduct>()) {
        return product_4->size();
    } else {
        return std::nullopt;
    }
}

std::variant<std::vector<TopoDS_Shape>, IfcParseError> ReadIFCShapes(const std::filesystem::path& a_path, int a_num_threads, const std::set<std::string>& a_exclude_entities) {
    try {
        using namespace ifcopenshell;
        using namespace ifcopenshell::geometry;
        using namespace boost;

        auto u8str = get_native_string(a_path);
        std::unique_ptr<IfcParse::IfcFile> ifc_file = std::make_unique<IfcParse::IfcFile>(u8str);
        if (!(bool)ifc_file || !ifc_file->good()) {
            return IfcParseError::kIfcInitializationFailed;
        }

        auto count_model = GetComponentCount(*ifc_file);

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
            return IfcParseError::kIfcIteratorInitializationFailed;
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
        return result;
    } catch (...) {
        return IfcParseError::kIfcParsingException;
    }
}

int main() {
    using namespace std;

    std::locale::global(std::locale("zh_CN.UTF-8"));

    filesystem::path ifc_dir = LR"(D:\works\tasks\BUGFIX#78415-BUGFIX#78907-ifc导入错误)";
    //ifc_dir = LR"(D:\works\tasks\BUGFIX#96486-ifc导入模型错误)";
    std::string file_name = "2A5-ZB2 33m(1-7.23.25)-A";
    file_name = "3-21";
    filesystem::path ifc_path = ifc_dir / (file_name + ".ifc");
    uint32_t n = std::thread::hardware_concurrency();
    if (n == 0) {
        n = 1;
    }
    std::set<std::string> exclude_entities = {"IFCMECHANICALFASTENER", "IFCFASTENER"}; // Example of entities to exclude
    auto res = ReadIFCShapes(ifc_path, n, exclude_entities);
    if (auto* shapes = std::get_if<std::vector<TopoDS_Shape>>(&res)) {
        TopoDS_Builder builder;
        TopoDS_Compound compound;
        builder.MakeCompound(compound);
        for (const auto& shape : *shapes) {
            builder.Add(compound, shape);
        }

        filesystem::path out_path = ifc_dir / std::format("{}_debug.brep", file_name);
        std::ofstream out_file(out_path);
        BRepTools::Write(compound, out_file);
    }

    return 0;
}
